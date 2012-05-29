# Makefile

MPI_COMPILE_FLAGS = $(shell mpiCC --showme:compile)
MPI_LINK_FLAGS = $(shell mpiCC --showme:link) -fopenmp

CC = mpicc
MAIN = par

all: build

clean: 
	rm -rf *.o $(MAIN)

build: $(MAIN).o
	$(CC) $(MPI_COMPILE_FLAGS) $(MPI_LINK_FLAGS) $(MAIN).o -o $(MAIN)

par.o: $(MAIN).c
	$(CC) -c $(MPI_COMPILE_FLAGS) $(MPI_LINK_FLAGS) $(MAIN).c

#MPI_COMPILE_FLAGS = $(shell mpicc --showme:compile)
#MPI_LINK_FLAGS = $(shell mpicc --showme:link) -fopenmp

#MY_APP=par
#CC=gcc-4.2

#my_app: $(MY_APP).c
#	$(CC) $(MPI_COMPILE_FLAGS) $(MY_APP).c $(MPI_LINK_FLAGS) -o $(MY_APP)
