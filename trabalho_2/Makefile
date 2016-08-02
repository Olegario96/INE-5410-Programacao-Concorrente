default: all

all:
	mpicc -O3 -Wall -o km_mpi km_mpi.c -g -lm

run:
	mpirun -np 2 xterm -e gdb -tui ./km_mpi 

clean:
	rm km_mpi
