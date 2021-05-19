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
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <malloc.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <ifaddrs.h>

#define CHECK_ERROR(ret, msg)       \
            do {if (ret == -1){     \
                perror(msg);        \
                exit(EXIT_FAILURE); \
            }} while(0);

enum{
	NO_INET = -2,
	TIMEOUT = -1,
	SUCCESS = 0
};

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
const int 	 timewait  = 40;

int takenumber (char * str);			//take number from comand line
void enable_keepalive(int sock);

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
//**********************************************************************************************************	create udp connection to send magic message
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
//**********************************************************************************************************	connect to workers through tcp
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

	fd_set fdset;
	FD_ZERO(&fdset);
	FD_SET(sk, &fdset);
	struct timeval timeout = {
		.tv_usec = 0,
   		.tv_sec = 5
	};

	unsigned int size = sizeof(addr);
	worker_info_t *workers = (worker_info_t*) calloc(input, sizeof(worker_info_t));

	for (int i = 0; i < input; i++){

		ret = select(sk + 1, &fdset, NULL, NULL, &timeout);
		CHECK_ERROR(ret, "select");
		if (ret == 0){
			printf("timeout\n");
			exit(EXIT_FAILURE);
		}

		workers[i].socket = accept(sk, (struct sockaddr*) &addr, &size); //, SOCK_NONBLOCK
		CHECK_ERROR(workers[i].socket, "accept error");
		enable_keepalive(workers[i].socket);
	}

	//--------------------------------------------------------------------------------------all sockets connected//---> calculate limits for each worker
	
	int threads_sum = 0;
	for (int i = 0; i < input; i++){
		ret = read(workers[i].socket, &workers[i].thread_num, sizeof(int));		//read number of threads from workers
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
		ret = read(workers[i].socket, &worker_result, sizeof(double));
		CHECK_ERROR(ret, "read error");
		if (ret == 0){
			free(workers);
			close(sk);
			for (int i = 0; i < input; i++)
				close(workers[i].socket);

			printf("wrong read result\n");
			exit(EXIT_FAILURE);
		}

		result += worker_result;
	}

	printf("calculation result: %lf\n", result * result *4);


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

void enable_keepalive(int sock) {
    int yes = 1;
    int ret = setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(int));
    CHECK_ERROR(ret, "setsockopt SO_KEEPALIVE");

    int idle = 1;
    ret = setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &idle, sizeof(int));
    CHECK_ERROR(ret, "setsockopt TCP_KEEPIDLE");

    int interval = 1;
   	ret = setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &interval, sizeof(int));
   	CHECK_ERROR(ret, "setsockopt TCP_KEEPINTVL");

    int maxpkt = 10;
    ret = setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &maxpkt, sizeof(int));
    CHECK_ERROR(ret, "setsockopt TCP_KEEPCNT");

    return;
}