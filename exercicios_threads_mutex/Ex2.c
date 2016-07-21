#include <stdio.h>
#include <pthread.h>

#define MAX_THREADS 20

void *printTID(void *arg) {
	pthread_t tid = pthread_self();
	printf("New thread created: %u!\n", (unsigned int) tid);
}

int main(int argc, char const *argv[]) {
	pthread_t threads[MAX_THREADS];

	printf("20 threads will be created!\n");

	for (int i = 0; i < MAX_THREADS; ++i) {
		pthread_create(&threads[i], NULL, printTID, (void *) NULL);
	}

	for (int i = 0; i < MAX_THREADS; ++i) {
		pthread_join(threads[i], NULL);
	}

	pthread_exit(NULL);
	return 0;
}