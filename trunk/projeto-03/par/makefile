# Makefile

MPI_COMPILE_FLAGS = $(shell mpiCC --showme:compile)
MPI_LINK_FLAGS = $(shell mpiCC --showme:link) -fopenmp

CC = mpicc
MAIN = par3

all: build

clean: 
	rm -rf *.o $(MAIN)

build: $(MAIN).o
	$(CC) $(MPI_COMPILE_FLAGS) $(MPI_LINK_FLAGS) $(MAIN).o -o $(MAIN)

par.o: $(MAIN).c
	$(CC) -c $(MPI_COMPILE_FLAGS) $(MPI_LINK_FLAGS) $(MAIN).c
