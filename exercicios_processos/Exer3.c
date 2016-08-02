#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>

int main() {

	pid_t pid;

	for (int i = 0; i < 2; ++i) {
		pid = fork();
		if (pid == 0) {
			printf("Process %d, son of %d\n", getpid(), getppid());
			break;
		} else {
			wait(NULL);
		}
	}

	if (pid == 0){
		for (int i = 0; i < 2; ++i) {
			pid = fork();
			if (pid == 0) {
				printf("Process %d, son of %d\n", getpid(), getppid());
				break;
			} else {
				wait(NULL);
			}
		}
	}
	return 0;
}