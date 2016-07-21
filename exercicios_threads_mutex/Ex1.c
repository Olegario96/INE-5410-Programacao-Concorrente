#include <stdio.h>
#include <pthread.h>

void *printTID(void *arg) {
	pthread_t tid = pthread_self();
	printf("Nova thread criada: %u\n!", (unsigned int) tid);
}

int main(int argc, char const *argv[]) {
	pthread_t thread1;
	pthread_create(&thread1, NULL, printTID, (void *) NULL);
	pthread_join(thread1, NULL);
	pthread_exit(NULL);
	return 0;
}