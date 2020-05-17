#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>

#define NCPUS 4

pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cndn = PTHREAD_COND_INITIALIZER;
int idx = 0;
int n;
int solved = 0; /* set to 1 when all N numbers are found */
int num_threads_alive = 0;

typedef struct s_num_res {
	unsigned long long int num;
	unsigned long long int* arr;
} num_res;

/**
 * Utility function to return the smaller of two numbers
 * @param a	The first number
 * @param b	The second number
 * 
 * @return the smaller of two numbers
 */
int min(int a, int b) {
	return ((a < b) ? a : b);
}

/* Comparator for quick sort library call */
int cmpfunc (const void * a, const void * b) {
   return (*(unsigned long long int*)a > *(unsigned long long int*)b);
}

/**
 * Checks if a given number is a perfect number. If yes, stores it in the 
 * provided array and updates array index. Otherwise, makes no updates
 * @param argv	The input number, and array to store result
 * 
 * @return NULL
 */ 
void* check_perfect(void* argv) {
	num_res* val = (num_res*) argv;

	/* check if it is a perfect number */
	unsigned long long int i, sum = 0;
	unsigned long long int max_iter = (unsigned long long int) sqrt((double) val->num);
	for(i = 1; i <= max_iter && solved == 0; i++) {
		if(val->num % i == 0) {
			sum += i;
			if(i != 1) {
				sum += val->num / i;
			}
			if(max_iter*max_iter == val->num && i == max_iter) {
				sum -= max_iter;
			}
		}
	}

	/* acquire lock */
	pthread_mutex_lock(&mtx);
	/* entering critical section */
	if((solved == 0) && (sum == val->num)) {
		/* store the value */
		val->arr[idx++] = val->num;
	} else if((solved != 0) && (sum == val->num)) {
		qsort(val->arr, n, sizeof(unsigned long long int), cmpfunc);
		if(val->arr[n-1] > val->num) {
			/* current thread has smaller value: insert this instead, replacing largest value */
			val->arr[n-1] = val->num;
		}
	}
	if(idx == n) {
		/* all required solutions found */
		solved = 1;
	}
	/* register that current thread is about to die */
	num_threads_alive--;

	/* notify main thread that thread is about to die */
	pthread_cond_signal(&cndn);

	/* critical section over, release lock */
	pthread_mutex_unlock(&mtx);

	free(val);

	return NULL;
}

/**
 * Takes a power of 2 (= 2^p), and returns a number = (2^p - 1)* (2^(p-1))
 * @param num	Integer which is a power of 2
 * 
 * @return a possible perfect number (true if input - 1 was prime)
 */ 
unsigned long long int get_perfect_check(unsigned long long int num) {
	return (num - 1) * (num / 2);
}

int main(int argc, char** argv) {
	if(argc < 2) {
		fprintf(stderr, "Expected one argument, found none\nUsage: %s <value_of_n>\n", argv[0]);
		return EXIT_FAILURE;
	}

	n = atoi(argv[1]);

	if(n < 1) {
		fprintf(stderr, "Expected a positive integer as argument, found %d\n", n);
		return EXIT_FAILURE;
	}

	/* specify number of threads to be maintained till solution is found */
	int max_num_threads = min(NCPUS, n);

	/* create array for storing results */
	unsigned long long int* results = (unsigned long long int*) malloc(sizeof(unsigned long long int) * n);

	unsigned long long int num = 4;

	solved = 0;

	/* Continue checking till required number of answers are found */
	do {

		/* acquire lock */
		pthread_mutex_lock(&mtx);

		/* in case max number of threads are running, wait */
		if(num_threads_alive == max_num_threads) {
			pthread_cond_wait(&cndn, &mtx);
		}

		/* lesser than max threads, launch new thread if all solutions haven't been obtained */
		if(solved == 0) {

			unsigned long long int next_check = get_perfect_check(num);
			num_res* val = (num_res*) malloc(sizeof(num_res));
			val->num = next_check;
			val->arr = results;

			/* launch new thread */
			pthread_t thread_id;
			pthread_create(&thread_id, NULL, check_perfect, val);

			/* detach thread */
			pthread_detach(thread_id);

			/* update number of running threads */
			num_threads_alive++;

			/* update value for next thread */
			num = num * 2;
		}

		/* critical section over, release lock */
		pthread_mutex_unlock(&mtx);
	} while (solved == 0 || num_threads_alive != 0);

	/* sort the results */
	qsort(results, n, sizeof(unsigned long long int), cmpfunc);

	/* display the results */
	int i;
	printf("The first %d perfect numbers are: ", n);
	for(i = 0; i < n; i++) {
		printf("%llu ", results[i]);
	}
	printf("\n");

	return EXIT_SUCCESS;
}