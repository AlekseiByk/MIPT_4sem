#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <malloc.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#define CHECK_ERROR(ret, msg)       \
            do {if (ret == -1){     \
                perror(msg);        \
                exit(EXIT_FAILURE); \
            }} while(0);


#define MAX(first, second) \
		((first > second)? first : second)

typedef struct{
	double a;
	double b;
	double result;
	int proc_number;
} mission_t;

const int    port_id = 8321;
const double step 	 = 1./100000000;

void *pthread_function(void * arg);
double func(double x);
int takenumber (char * str);

int main(int argc, char ** argv) {

	if (argc < 2) {
		printf("Too less arguments\n");
		return -1;
	} else if (argc > 2) {
		printf("Too many arguments\n");
		return -2;
	}

	int input = takenumber (argv[1]);
    int ret = 0;
//**********************************************************************************************************
	int sk = socket(AF_INET, SOCK_DGRAM, 0);
    CHECK_ERROR(sk, "socket fail")

    struct sockaddr_in addr1 = {
        .sin_family = AF_INET,
        .sin_port	= htons(port_id),
        .sin_addr	= htonl(INADDR_ANY)
    };
    //*------------------------------------------------------------------------------------------------------------
    ret = setsockopt(sk, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
	CHECK_ERROR(ret, "setsockopt fail");
    //*------------------------------------------------------------------------------------------------------------
    ret = bind(sk, (struct sockaddr*) &addr1, sizeof(addr1));
    CHECK_ERROR(ret, "bind fail");

    int code_word = 0;
    unsigned int size = sizeof(addr1);
    ret = recvfrom(sk, &code_word, sizeof(code_word), 0, (struct sockaddr*) &addr1, &size);
    CHECK_ERROR(ret, "recv fail");

    printf("%x\n", code_word);

    shutdown(sk, SHUT_RDWR);
    close(sk);
//**********************************************************************************************************
    sk = socket (PF_INET, SOCK_STREAM, 0);
    CHECK_ERROR(sk, "tcp socket error");

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port	= htons(port_id),
        .sin_addr	= ((struct sockaddr_in *) &addr1) -> sin_addr
    };
 /*
    ret = setsockopt(sk, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
	CHECK_ERROR(ret, "setsockopt fail");
*/
    ret = connect (sk, (struct sockaddr*) &addr, sizeof(addr));
    CHECK_ERROR(ret, "connect fail");

    printf("connected to server: %s\n", inet_ntoa(((struct sockaddr_in *) &addr1) -> sin_addr));
//**********************************************************************************************************
    ret = write(sk, &input, sizeof(int));
    CHECK_ERROR(ret, "write fail");

	FILE* fin = fopen ("/sys/devices/system/cpu/cpu0/cache/index0/coherency_line_size", "r");
	if (!fin){
		fprintf(stderr, "error while fopen cache file\n");
		exit(EXIT_FAILURE);
	}

	int proc_number = get_nprocs();
	number_of_proc = MAX(proc_number, input);
	int cache_size = 0;
	fscanf(fin, "%d", &cache_size);
	fclose (fin);

	pthread_t * threads = (pthread_t *) calloc(number_of_proc, sizeof(threads[0]));
	mission_t ** pthreads_info = (mission_t **) calloc ( number_of_proc, sizeof(pthreads_info[0])); 

	double left_lim = 0;
	double right_lim = 0;
	read(sk, &left_lim, sizeof(double));
	read(sk, &right_lim, sizeof(double));

	for (int i = 0; i < number_of_proc; i++){

		pthreads_info[i] = (mission_t *) memalign (cache_size, sizeof(mission_t) );

		pthreads_info[i] -> a = left_lim + ((double) (right_lim - left_lim)) / input * (i % input);
		pthreads_info[i] -> b = (pthreads_info[i] -> a) + ((double) (right_lim - left_lim)) / input;
		pthreads_info[i] -> proc_number = i % proc_number;
		pthreads_info[i] -> result = 0;
		//printf ("%f %f %10f %f\n", pthreads_info[i] -> a, pthreads_info[i] -> b, pthreads_info[i] -> step, pthreads_info[i] -> result);
	}

	for (int i = 0; i < number_of_proc ; i++){

		err = pthread_create(&(threads[i]), NULL, pthread_function,(void*) pthreads_info[i]);
		check_error(err, "pthread_create");
	}

	for (int i = 0; i < number_of_proc; i++){

		pthread_join(threads[i], NULL);
		check_error(err, "pthread_join");
	}

	double self_result = 0;

	for (int i = 0; i < input; i++){
		self_result += pthreads_info[i] -> result;
	}

	printf("colculations complete");

	ret = write(sk, &self_result, sizeof(double));
	CHECK_ERROR(ret, "write error)");

	for (int i = 0; i < number_of_proc; i++){
		free(pthreads_info[i]);
	}
	free(pthreads_info);
	free (threads);
    close(sk);
	return 0;
}

int takenumber (char * str)
{
	char *strptr = NULL, *endptr = NULL;

	long input = 0;
	input = strtol(str, &endptr, 10);
 
//-------------------------------------------------------------
	if (endptr == strptr || *endptr != '\0') {
		printf("Wront input string\n");
		exit (-3);
	}
 
	if (errno == ERANGE && (input == LONG_MAX || input == LONG_MIN)) {
		printf("Out of range\n");
		exit(-5);
	}

	return input;
}

void *pthread_function(void * arg)
{
	mission_t * mission = (mission_t * ) arg;

	cpu_set_t cpuset;
	CPU_ZERO(&cpuset);

	CPU_SET(mission -> proc_number, &cpuset);

	int err = pthread_setaffinity_np( pthread_self(), sizeof(cpuset), &cpuset);
	CHECK_ERROR(err, "pthread_setaffinity_np");

	double x = mission -> a;
	double end = mission -> b;

	while ( x < end ){
		mission-> result += func( x ) * step;
		x += step;
	}

	return 0;
}

double func(double x)
{
	return exp (-x * x);
}