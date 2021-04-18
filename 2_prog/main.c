#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define check_error(er, msg) \
		do { if (er != 0) { errno = er; perror(msg); exit(EXIT_FAILURE); } } while (0)

void pthread_function(void * arg);

int main(int argc, char *argv[])
{
	// Check argument's amount
	if (argc < 2) {
		printf("Too less arguments\n");
		return -1;
	} else if (argc > 2) {
		printf("Too many arguments\n");
		return -2;
	}

	int input = takenumber (argv[1]);
	//----------------------------------------------------------------------------input

	int err = 0;
	int number_of_proc = get_nprocs_conf();
	int * processors = (int *) calloc (number_of_proc, sizeof(processors[0]));

	for (int i = 0; i < number_of_proc; i++){

		struct stat statbuf = {};
		char file_pass[] = sprintf(file_pass, "/sys/devices/system/cpu/cpu%d/topology/core_id", i);
		int fd = open (file_pass, O_RDONLY);
		fstat(fd, &statbuf);

		char * buf = (char *) calloc (statbuf.st_size, sizeof(buf[0]));
		read (fd, buf, statbuf.st_size);

		processors[i] = atoi(buf);
		free(buf);
		close(fd);
		printf ("%d %d\n", i, processors[i]);
	}
	/*
	//----------------------------------------------------------------------------
	cpu_set_t cpuset;
	pthread_t * threads = (pthread_t *) calloc (number_of_proc, sizeof(threads[0]));

	for (int i = 0; i < input; i++){
		err = pthread_create(&(threads[i]), NULL, pthread_function,(void*) &pthread_info[i]);
		check_error(err, "pthread_create");
	}

	for (int i = 0; i < input; i++){
		CPU_ZERO(&cpuset);
		CPU_SET(2, &cpuset);

		err = pthread_setaffinity_np(threads[i], sizeof(cpuset), &cpuset);
		check_error(err, "pthread_setaffinity_np");
	}

	for (int i = 0; i < input; i++){
		pthread_join(threads[i], NULL);
		check_error(err, "pthread_join");
	}
	

	CPU_ZERO(&cpuset);
	//for (int j = 0; j < 8; j++)
		CPU_SET(2, &cpuset);

	free (threads);*/
	free (processors);
	exit (EXIT_SUCCESS);
}

int takenumber (char * str)
{
	char *strptr = NULL, *endptr = NULL;

	long input = 0;
	input = strtol(str, &endptr, 10);
 
 
	if (endptr == strptr || *endptr != '\0') {
		printf("Wront input string\n");
		exit (-3);
	}
 
	if (input <= 0) {
		printf("The number must be greater then 0\n");
		exit(-4);
	}
 
	if (errno == ERANGE && (input == LONG_MAX || input == LONG_MIN)) {
		printf("Out of range\n");
		exit(-5);
	}

	return input;
}
/*
#include "errno.h"

#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>

void *incriment(void * arg);
int takenumber (char * input);
 
int main(int argc, char **argv) {

	// Check argument's amount
	if (argc < 2) {
		printf("Too less arguments\n");
		return -1;
	} else if (argc > 2) {
		printf("Too many arguments\n");
		return -2;
	}

	int input = takenumber (argv[1]);
	//-------------------------------------------------------------------------------------input

	int var = 0;
	pthread_t thread[input];

	for (int i = 0; i < input; i++)
		pthread_create(&(thread[i]), NULL, incriment,(void*) &var);
	for (int i = 0; i < input; i++)
		pthread_join(thread[i], NULL);
	
	printf ("%d\n", var);

	return 0;
}



void *incriment(void * arg)
{
	int * variable = (int *) arg;
	
	for (int i =0; i < 10000; i++)
		*variable += 1;

	return 0;
}

*/
