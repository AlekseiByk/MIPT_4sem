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
//**********************************************************************************************************
    ret = write(sk, &input, sizeof(int));
    CHECK_ERROR(ret, "write fail");
    

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