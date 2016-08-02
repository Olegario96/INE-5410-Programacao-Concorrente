#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

#define MAX_THREADS 100

int var_compartilhada = 0;

sem_t semaphore;

void *func_thread(void *argumento) {
	int i;
	pthread_t tid = pthread_self();

	sem_wait(&semaphore);
	for(i = 0; i < 100; i++) {
		var_compartilhada++;
	}
	printf("Worker thread %ld: var_compartilhada = %d.\n", (long)tid, var_compartilhada);
	sem_post(&semaphore);
	return 0;
}

int main(int argc, char **argv) {
	int i;
	pthread_t threads[MAX_THREADS];

	sem_init(&semaphore, 0, 1);

	printf("Main thread iniciada.\n");

	for(i = 0; i < MAX_THREADS; i++)
		pthread_create(&threads[i], NULL, func_thread, NULL);

	for(i = 0; i < MAX_THREADS; i++)
		pthread_join(threads[i], NULL);

	printf("Main thread: var_compartilhada = %d.\n", var_compartilhada);

	sem_destroy(&semaphore);

	return 0;
}