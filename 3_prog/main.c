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

#define check_error(er, msg) \
		do { if (er != 0) { errno = er; perror(msg);printf ("%d\n", er); exit(EXIT_FAILURE); } } while (0)
#define MAX(first, second) \
		((first > second)? first : second)

const double left_lim = 0;
const double right_lim = 10;
const double step = 1./100000000;

struct mission_t
{
	double a;
	double b;
	double result;
	int proc_number;
};

int takenumber (char * str);
void *pthread_function(void * arg);
double func(double x);

int main(int argc, char *argv[])
{
	// Check argument's amount
	if (argc < 2) { 
		return -1;
	} else if (argc > 2) {
		printf("Too many arguments\n");
		return -2;
	}

	int input = takenumber (argv[1]);

	if (input <= 0) {
		printf("The number must be greater then 0\n");
		exit(-4);
	}

	
	//----------------------------------------------------------------------------input
	/*
	int err = 0;
	int number_of_proc = get_nprocs();

	FILE* fin = fopen ("/sys/devices/system/cpu/cpu0/cache/index0/coherency_line_size", "r");
	if (!fin)
		fprintf(stderr, "error while fopen cache file\n");

	int cache_size = 0;

	fscanf(fin, "%d", &cache_size);

	pthread_t * threads = (pthread_t *) calloc ( MAX( number_of_proc, input), sizeof(threads[0]));
	struct mission_t ** pthreads_info = (struct mission_t **) calloc ( MAX( number_of_proc, input), sizeof(pthreads_info[0])); 

	for (int i = 0; i < MAX( number_of_proc, input); i++){

		pthreads_info[i] = (struct mission_t *) memalign (cache_size, sizeof(struct mission_t) );
		pthreads_info[i] -> a = left_lim + ((double) (right_lim - left_lim)) / input * (i % input);
		pthreads_info[i] -> b = (pthreads_info[i] -> a) + ((double) (right_lim - left_lim)) / input;
		pthreads_info[i] -> proc_number = i % number_of_proc;
		pthreads_info[i] -> result = 0;
		//printf ("%f %f %10f %f\n", pthreads_info[i] -> a, pthreads_info[i] -> b, pthreads_info[i] -> step, pthreads_info[i] -> result);
	}



	for (int i = 0; i < MAX( number_of_proc, input) ; i++){

		err = pthread_create(&(threads[i]), NULL, pthread_function,(void*) pthreads_info[i]);
		check_error(err, "pthread_create");
	}

	for (int i = 0; i < MAX( number_of_proc, input); i++){

		
	}

	for (int i = 0; i < MAX( number_of_proc, input); i++){

		pthread_join(threads[i], NULL);
		check_error(err, "pthread_join");
	}

	double result = 0;


	for (int i = 0; i < input; i++){
		result += pthreads_info[i] -> result;
	}

	printf("res: %.10f\n", result * result * 4);

	free (threads);
	exit (EXIT_SUCCESS);*/
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
 
	if (errno == ERANGE && (input == LONG_MAX || input == LONG_MIN)) {
		printf("Out of range\n");
		exit(-5);
	}

	return input;
}

void *pthread_function(void * arg)
{
	struct mission_t * mission = (struct mission_t * ) arg;

	cpu_set_t cpuset;
	CPU_ZERO(&cpuset);
	CPU_SET(mission -> proc_number, &cpuset);

	int err = pthread_setaffinity_np( pthread_self(), sizeof(cpuset), &cpuset);
	check_error(err, "pthread_setaffinity_np");

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
