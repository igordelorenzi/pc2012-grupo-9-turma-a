/*
 * Para compilar: mpicc par.c -o par -Wall
 * Para rodar: time mpirun -np 10 par ../input/matriz250.txt
 * No cluster: time mpirun -np 10 -machinefile machinefile.xmen par ../input/matriz250.txt
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#include <omp.h>

#define nThreads 2

double subtratorio(double*,int,int,int,int,double*);
double max(double*,int);
void diferenca(double*,double*,double*,int);
void copia(double*,double*,int);

int main(int argc, char *argv[])
{
	double *A, *B, jError, *buf;
	int jOrder, jRowTest, jIteMax, i, j;
	FILE *entrada;
	int numprocs, rank, block;
	
	double *X, *X1, *dif;
	double maxi, error;
	double sum = 0.0, total = 0.0;
	int iteracao, totalIteracoes, para = 0;

	MPI_Init(&argc,&argv);
	MPI_Comm_rank( MPI_COMM_WORLD, &rank);
	MPI_Comm_size( MPI_COMM_WORLD, &numprocs);

	/* Master pega os dados	*/
	if(rank == 0)
	{
		if (argc != 2) {
			printf("USO:\n\t<nome_do_programa> <arquivo_sistema_linear>\n");
			exit(1);
		}
		entrada = fopen(argv[1], "r");
		if (entrada == NULL) {
			printf("Error: \tfile!\n");
			MPI_Finalize();
			exit(-1);
		}
		fscanf(entrada, "%d", &jOrder);	// LENDO TAMANHO

		if (jOrder % numprocs != 0)
		{
			printf("Impossivel dividir a carga de processamento.\n");
			MPI_Finalize();
			exit(-1);
		}

		/* Alocando matriz e vetor	*/
		B = (double *) malloc(jOrder*sizeof(double));
		A = (double *) malloc(jOrder * jOrder * sizeof(double));

		/* Lendo os outros dados de entrada	*/
		fscanf(entrada, "%d", &jRowTest);
		fscanf(entrada, "%lf", &jError);
		fscanf(entrada, "%d", &jIteMax);		
		/* Preenche a matriz do sistema linear */
		for (i = 0; i < jOrder * jOrder; i++) {
				fscanf(entrada, "%lf", &A[i]);
		}
		/* Preenche o vetor B	*/
		for (i = 0; i < jOrder; i++) {
			fscanf(entrada, "%lf", &B[i]);
		}
		X = (double *) malloc(jOrder*sizeof(double));
		X1 = (double *) malloc(jOrder*sizeof(double));
	}
	MPI_Bcast(&jOrder, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&jError, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	MPI_Bcast(&jIteMax, 1, MPI_INT, 0, MPI_COMM_WORLD);

	block = jOrder / numprocs;
	
	if(rank != 0)
	{
		A = (double *) malloc(block * jOrder * sizeof(double));
		B = (double *) malloc(block*sizeof(double));
		X = (double *) malloc(jOrder*sizeof(double));
		X1 = (double *) malloc(block*sizeof(double));
	}
	MPI_Scatter(A, block*jOrder, MPI_DOUBLE, A, block*jOrder, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	MPI_Scatter(B, block, MPI_DOUBLE, B, block, MPI_DOUBLE, 0, MPI_COMM_WORLD);	
	/*	 Inicia o vetor solução	*/
	#pragma omp parallel for num_threads(nThreads)
	for (i = 0 ; i < jOrder ; i++)
		X[i] = 0.0;
	
	if(rank == 0){
		buf = (double *) malloc(jOrder*sizeof(double));
		dif = (double *) malloc(jOrder*sizeof(double));	
	}	
	/* Laço que vai rodar tudo	*/
	for (iteracao = 0 ; para == 0 && iteracao < jIteMax ; iteracao++)
	{
		for (i = 0 ; i < block ; i++){
			X1[i] = ((B[i] + subtratorio(X,i,block,rank,jOrder,A))
				/A[(jOrder + 1)*i+block*rank]);
		}
		if(rank == 0)
		{
			copia(buf,X,jOrder);
		}		
		MPI_Allgather(X1, block, MPI_DOUBLE, X, block, MPI_DOUBLE,MPI_COMM_WORLD);		

		if(rank == 0)
		{
			/* Gera o vetor com a diferença das soluções. */
			diferenca(X,buf,dif,jOrder);
			/* Verifica o máximo valor desse valor. */
			error = max(dif,jOrder);
			if (error < 0.0)
				error *= (-1.0);
			/* Transforma para o valor absoluto */		
			maxi = max(buf,jOrder);		
			if (maxi < 0.0)
				maxi *= (-1.0);
			/* Verifica erro foi alcançado.		*/
			error /= maxi;	
			if(error < jError)
				para = 1;
		}
		MPI_Bcast(&para, 1, MPI_INT, 0, MPI_COMM_WORLD);
	}

	if (rank == 0) 
	{
		/* Saiu do laço antes do número máximo de iterações.	*/
		if(iteracao < jIteMax)
		{
			#pragma omp parallel for num_threads(nThreads) shared(A,X,jRowTest,jOrder) reduction(+:sum)
			for(i = 0 ; i < jOrder ; i++)
			{
				sum += A[(jRowTest*jOrder)+i]*X[i];
			}
			printf("Iterations: %d\n", iteracao);
			printf("RowTest: %d => [%f] =? %f\n", jRowTest, sum, B[jRowTest]);
		}
		/* Saiu porque iterações ultrapassaram o número máximo.	*/
		else
			printf("ERROR: Number of iterations overflowed.\n");
	}
	free(A);
	free(B);
	free(X);
	free(X1);

	MPI_Finalize();

	return 0;
}
/**
* Função que faz a subtração das multiplicações do vetor
* solução pela linha da matriz A.
*/
double subtratorio(double *X, int i, int block, int rank, int jOrder, double *A)
{
	double total = 0.0;
	int j;
	#pragma omp parallel for num_threads(nThreads) private(i,block,jOrder,rank) reduction(-:total)
	for(j = 0 ; j < jOrder ; j++)
	{
		if ((i*jOrder)+j == (jOrder + 1)*i+block*rank)continue;
		#pragma omp atomic		
		total -= X[j]*A[(i*jOrder)+j];
	}	
	return total;
}

/**
* Função que retorna o máximo valor do vetor solução
*
*/ 
double max(double* X1, int jOrder)
{
	double max = X1[0];
	int i;
	#pragma omp parallel for num_threads(nThreads) shared(max)
	for( i = 0 ; i < jOrder ; i++)
	{
		if(X1[i] <= max)continue;
		#pragma omp critical		
		max = X1[i];
	}	
	return max;
}
/**
* Função que retorna um vetor com as diferenca entre
* os vetores de uma iteração.
*/
void diferenca(double* X1,double* X, double* dif, int jOrder)
{
	int i;
	#pragma omp parallel for num_threads(nThreads)
	for(i = 0 ; i < jOrder ; i++)
	{
		dif[i] = X1[i] - X[i];
	}
}
/**
* Função que copia o conteúdo de uma solução
* para outra.
*/
void copia(double* X1, double* X2, int jOrder)
{
	int i;
	#pragma omp parallel for num_threads(nThreads)
	for(i = 0 ; i < jOrder ; i++)
	{
		X1[i] = X2[i];
	}		
}


