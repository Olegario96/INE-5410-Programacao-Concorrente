#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char **argv) {
	int size, rank;

	MPI_Status st;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	int ranks[size];

	if (rank == 0) {
		for (int i = 1; i < size; ++i) {
			MPI_Send(NULL, 0, MPI_INT, i, 0, MPI_COMM_WORLD);
			MPI_Recv(&ranks, size, MPI_INT, i, MPI_ANY_TAG, MPI_COMM_WORLD, &st);
			printf("I received a menssage from process: %d\n", ranks[i]);
		}
	} else {
			MPI_Recv(NULL, 0, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &st);
			ranks[rank] = rank;
			MPI_Send(&ranks, size, MPI_INT, 0, 0, MPI_COMM_WORLD);
	}

	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Finalize();

	return 0;
}