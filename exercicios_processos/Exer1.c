#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>

int main() {

	pid_t pid = fork();

	printf("New process created!\n");

	return 0;
}

/*
	pid_t pid;
	pid < 0 -> o processo não pode ser criado.
	pid > 0 -> é o processo pai, ele sabe disso e ele sabe o pid do filho.
	pid = 0 -> é o processo filho, ele sabe disso.

	Os n processos criados, NÃO compartilha espaço de endereçamento.
	Processos pai e filho terão cópias idênticas dos dados após um fork().
*/