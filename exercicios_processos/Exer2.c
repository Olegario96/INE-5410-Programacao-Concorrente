#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>

int main() {

	pid_t pid = fork();

	for (int i = 0; i < 4; ++i) {
		if (pid == 0){
			printf("Process father %d has created: %d\n", 
					getppid(), getpid());
			printf("Process son %d\n", getpid());
			break;
		} else {
			fork();
		}
	}

	return 0;
}

/*
pid_t pid[4];

	for (int i = 0; i < 4; ++i) {
		pid[i] = fork();

		if(pid[i] == 0){
			printf("Process father %d has created: %d\n", 
					getppid(), getpid());
			printf("Process son %d\n", getpid());
			break;
		}
		wait(NULL);
	}

*/
