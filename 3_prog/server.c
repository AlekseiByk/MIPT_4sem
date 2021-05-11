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

typedef struct{
	int socket;
	int thread_num;
} worker_info_t;

typedef struct{
	double a;
	double b;
	double result;
	int proc_number;
} mission_t;

const double left_lim  = 0;
const double right_lim = 8;
const int 	 port_id   = 8321;
const int 	 magic     = 0xBEAF;

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
	CHECK_ERROR(sk, "udp socket fail");

	struct sockaddr_in addr1 = {
		.sin_family = AF_INET,
		.sin_port	= htons(port_id),
		.sin_addr	= htonl(INADDR_BROADCAST)
	};

	int a = 1;
	ret = setsockopt(sk, SOL_SOCKET, SO_BROADCAST, &a, sizeof(a));
	CHECK_ERROR(ret, "setsockopt fail");

	sendto(sk, &magic, sizeof(magic), 0,(struct sockaddr*) &addr1, sizeof(addr1));
	CHECK_ERROR(ret, "sendto error");

	shutdown(sk, SHUT_RDWR);
	close(sk);
//**********************************************************************************************************
	sk = socket(PF_INET, SOCK_STREAM, 0);
	CHECK_ERROR(ret, "tcp socket");

	struct sockaddr_in addr = {
		.sin_family = AF_INET,
		.sin_port	= htons(port_id),
		.sin_addr	= htonl(INADDR_ANY)
	};

	ret = setsockopt(sk, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
	CHECK_ERROR(ret, "setsockopt fail");

	ret = bind(sk, (struct sockaddr*) &addr, sizeof(addr));
	CHECK_ERROR(ret, "bind error");

	ret = listen(sk, 256);
	CHECK_ERROR(ret, "listen error");

	unsigned int size = sizeof(addr);
	worker_info_t *workers = (worker_info_t*) calloc(input, sizeof(worker_info_t));
	for (int i = 0; i < input; i++){
		workers[i].socket = accept(sk, (struct sockaddr*) &addr, &size);
		CHECK_ERROR(workers[i].socket, "accept error");
	}
	
	int threads_sum = 0;
	for (int i = 0; i < input; i++){
		ret = read(workers[i].socket, &workers[i].thread_num, sizeof(int));
		CHECK_ERROR(ret, "read error");

		threads_sum += workers[i].thread_num;
	}
	printf("threads num = %d\n", threads_sum);

	double left_lim_temp = left_lim;

	for (int i = 0; i < input; i++){
		write(workers[i].socket, &left_lim_temp, sizeof(double));
		left_lim_temp += ((double) (right_lim - left_lim)) / threads_sum * workers[i].thread_num;
		write(workers[i].socket, &left_lim_temp, sizeof(double));
	}

	double result = 0;

	for(int i = 0; i < input; i++){
		double worker_result = 0;
		read(workers[i].socket, &worker_result, sizeof(double));
		result += worker_result;
	}

	printf("calculation result: %lf", result);


	free(workers);
	close(sk);
	for (int i = 0; i < input; i++)
		close(workers[i].socket);
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
