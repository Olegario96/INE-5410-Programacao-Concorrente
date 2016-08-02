#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>

#define N 10

sem_t cheio, vazio;
sem_t lock_prod, lock_cons;

int inicio = -1;
int fim = -1;
int buffer[N];

void *produtor() {
	while(1) {
		sem_wait(&vazio);
		sem_wait(&lock_prod);
		sleep(1);
		fim = (fim + 1) % N;
		buffer[fim] = 1;
		for (int i = 0; i < N; ++i) {
			printf("%d |", buffer[i]);
		}
		printf("\n");
		sem_post(&lock_prod);
		sem_post(&cheio);
	}
}

void *consumidor() {
	while(1) {
		sem_wait(&cheio);
		sem_wait(&lock_cons);
		sleep(5);
		inicio = (inicio + 1) % N;
		buffer[inicio] = 0;
		for (int i = 0; i < N; ++i) {
			printf("%d |", buffer[i]);
		}
		printf("\n");
		sem_post(&lock_cons);
		sem_post(&vazio);
	}
}

int main(int argc, char const *argv[]) {
	pthread_t threads[N];

	sem_init(&cheio, 0, 0);
	sem_init(&vazio, 0, N);
	sem_init(&lock_prod, 0, 1);
	sem_init(&lock_cons, 0, 1);

	for (int i = 0; i < N/2; ++i) {
		pthread_create(&threads[i], NULL, produtor, NULL);
	}

	for (int i = N/2; i < N; ++i) {
		pthread_create(&threads[i], NULL, consumidor, NULL);
	}

	for (int i = 0; i < N; ++i) {
		pthread_join(threads[i], NULL);
	}

	printf("Main thread criada!\n");

	return 0;
}