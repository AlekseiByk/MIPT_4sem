#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <sys/types.h>
#include <unistd.h>

#define handle_error(er, msg) \
		do { errno = er; perror(msg); exit(EXIT_FAILURE); } while (0)

int main(int argc, char *argv[])
{
	int s;
	cpu_set_t cpuset;
	pthread_t thread;

	printf ("%d %d\n",  get_nprocs_conf(),  get_nprocs());

	thread = pthread_self();

	/* Set affinity mask to include CPUs 0 to 7. */

	CPU_ZERO(&cpuset);
	//for (int j = 0; j < 8; j++)
		CPU_SET(2, &cpuset);

		

	s = pthread_setaffinity_np(thread, sizeof(cpuset), &cpuset);
	if (s != 0)
		handle_error(s, "pthread_setaffinity_np");

	/* Check the actual affinity mask assigned to the thread. */

	s = pthread_getaffinity_np(thread, sizeof(cpuset), &cpuset);
	if (s != 0)
		handle_error(s, "pthread_getaffinity_np");

	printf("Set returned by pthread_getaffinity_np() contained:\n");
	for (int j = 0; j < CPU_SETSIZE; j++)
		if (CPU_ISSET(j, &cpuset))
			printf("    CPU %d\n", j);

		while (1);

	exit(EXIT_SUCCESS);
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

void *incriment(void * arg)
{
	int * variable = (int *) arg;
	
	for (int i =0; i < 10000; i++)
		*variable += 1;

	return 0;
}

*/
