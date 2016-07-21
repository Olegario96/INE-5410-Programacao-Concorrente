#include <stdio.h>
#include <pthread.h>

#define MAX_THREADS 128

int count_global = 0;
pthread_mutex_t mutex;

void *incGlobal(void *arg) {
	for (int i = 0; i < 100; ++i) {
		pthread_mutex_lock(&mutex);
		++count_global;
		pthread_mutex_unlock(&mutex);
	}
}

int main(int argc, char const *argv[]) {
	pthread_t threads[MAX_THREADS];

	pthread_mutex_init(&mutex, NULL);
	printf("%d threads will be created!\n", MAX_THREADS);

	for (int i = 0; i < MAX_THREADS; ++i) {
		pthread_create(&threads[i], NULL, incGlobal, (void *) NULL);
	}

	for (int i = 0; i < MAX_THREADS; ++i) {
		pthread_join(threads[i], NULL);
	}

	printf("Count Global: %d\n", count_global);

	pthread_exit(NULL);
	return 0;
}