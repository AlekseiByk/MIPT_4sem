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

const int port_id = 8321;
const int magic   = 0xBEAF;

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
//**********************************************************************************************************
	sk = socket(PF_INET, SOCK_STREAM, 0);
	CHECK_ERROR(ret, "tcp socket");

	struct sockaddr_in addr = {
		.sin_family = AF_INET,
		.sin_port	= htons(port_id),
		.sin_addr	= htonl(INADDR_ANY)
	};

	ret = bind(sk, (struct sockaddr*) &addr, sizeof(addr));
	CHECK_ERROR(ret, "bind error");

	ret = listen(sk, 256);
	CHECK_ERROR(ret, "listen error");		

	int sk2 = accept(sk, (struct sockaddr*) &addr, NULL);
	CHECK_ERROR(sk2, "accept error");

	write(sk2, "hello world!...", sizeof("hello world!..."));
	

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
