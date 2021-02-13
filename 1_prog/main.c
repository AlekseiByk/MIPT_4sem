#include "errno.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include "sys/types.h"
#include <pthread.h>

void *incriment(void * arg);
 
int main(int argc, char **argv) {
	char *strptr = NULL, *endptr = NULL;
	// Check argument's amount
	if (argc < 2) {
		printf("Too less arguments\n");
		return -1;
	} else if (argc > 2) {
		printf("Too many arguments\n");
		return -2;
	}
 
	// Check the string
	long input = 0;
	strptr = argv[1];
	input = strtol(strptr, &endptr, 10);
 
 
	if (endptr == strptr || *endptr != '\0') {
		printf("Wront input string\n");
		return -3;
	}
 
	if (input <= 0) {
		printf("The number must be greater then 0\n");
		return -4;
	}
 
	if (errno == ERANGE && (input == LONG_MAX || input == LONG_MIN)) {
		printf("Out of range\n");
		return -5;
	}

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
