/*
 * Para compilar: mpicc par.c -o par -Wall
 * Para rodar: time mpirun -np 10 par ../input/matriz250.txt
 * No cluster: time mpirun -np 10 -machinefile machinefile.xmen par ../input/matriz250.txt
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

/*typedef struct {*/
/*	int jOrder;*/
/*	int jRowTest;*/
/*	int jIteMax;*/
/*} ASD;*/

void gaussJacobi(double **,double *,int,double,int,int);
double subtratorio(double*,int,int,double**);
double max(double*,int);
void diferenca(double*,double*,double*,int);
void copia(double*,double*,int);

int main(int argc, char *argv[])
{
	double **A, **A_recv, *B, *B_recv, jError;
	int jOrder, jRowTest, jIteMax, i, j;
	FILE *entrada;
	int numprocs, rank, block;
	
	double *X, *X1, *dif;
	double maxi, error;
	double sum = 0;
	int iteracao, totalIteracoes;

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
			printf("Erro na abertura do arquivo!\n");
			MPI_Finalize();
			exit(-1);
		}
		fscanf(entrada, "%d", &jOrder);	// LENDO TAMANHO


		printf("jORDER %d\n",jOrder);
		if (jOrder % numprocs != 0)
		{
			printf("Impossivel dividir a carga de processamento.\n");
			exit(-1);
		}


		/* Alocando matriz e vetor	*/
		B = (double *) malloc(jOrder*sizeof(double));
		A = (double **) malloc(jOrder * sizeof(double *));
		for(i = 0 ; i< jOrder ; i++){
			A[i] = (double *) malloc(jOrder * sizeof(double));
		}
		/* Lendo os outros dados de entrada	*/
		fscanf(entrada, "%d", &jRowTest);
		fscanf(entrada, "%lf", &jError);
		fscanf(entrada, "%d", &jIteMax);		
		/* Preenche a matriz do sistema linear */
		for (i = 0; i < jOrder; i++) {
			for (j = 0; j < jOrder; j++)
				fscanf(entrada, "%lf", &A[i][j]);
		}
		/* Preenche o vetor B	*/
		for (i = 0; i < jOrder; i++) {
			fscanf(entrada, "%lf", &B[i]);
		}

	}

	MPI_Bcast(&jOrder, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&jError, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	MPI_Bcast(&jIteMax, 1, MPI_INT, 0, MPI_COMM_WORLD);

	X = (double *) malloc(jOrder*sizeof(double));

	if (rank == 0) {
		/* Inicia o vetor solução	*/
		for (i = 0 ; i < jOrder ; i++)
			X[i] = 0;
	}

	block = jOrder / numprocs;

	A_recv = (double **) malloc(block * sizeof(double *));
	for(i = 0 ; i < block ; i++){
		A_recv[i] = (double *) malloc(jOrder * sizeof(double));	
	}
	B_recv = (double *) malloc(block*sizeof(double));
	X1 = (double *) malloc(block*sizeof(double));
	dif = (double *) malloc(block*sizeof(double));

	MPI_Scatter(A, jOrder*block, MPI_DOUBLE, A_recv, jOrder*block, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	MPI_Scatter(B, block, MPI_DOUBLE, B_recv, block, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	MPI_Bcast(X, jOrder, MPI_DOUBLE, 0, MPI_COMM_WORLD);

	printf("rank %d:\n"
			"\tblock (%d)\n"
			"\tA_recv (%lf)\n"
			"\tB_recv (%lf)\n"
			"\tjOrder (%d)\n"
			"\tjIteMax (%d)\n"
			"\tjError (%lf)\n\n",
			rank,block,A_recv[0][0],B_recv[0],jOrder,jIteMax,jError);


	/* Laço que vai rodar tudo	*/
	for(iteracao = 0 ; iteracao < jIteMax ; iteracao++)
	{
		for(i = 0 ; i < block ; i++)
			X1[i] = ((B_recv[i] + subtratorio(X,i,jOrder,A_recv))/A_recv[i][i]);
		/* Gera o vetor com a diferença das soluções.		*/
		diferenca(X,X1,dif,jOrder);	
		/* Verifica o máximo valor desse valor.		*/
		error = max(dif,jOrder);
		if (error < 0)
			error *= (-1);
		/* Transforma para o valor absoluto */		
		maxi = max(X1,jOrder);		
		if (maxi < 0)
			maxi *= (-1);
		/* Verifica erro foi alcançado.		*/
		error /= maxi;		
		if(error < jError)
			break;
		/* Se chegou aqui, vai precisar de mais iteração
		  Então copia o vetor solução para começar outra */	
		copia(X,X1,jOrder);
	}
	

	MPI_Reduce(&iteracao, &totalIteracoes, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
	MPI_Gather(X,jOrder,MPI_INT,X,jOrder,MPI_INT, 0,MPI_COMM_WORLD);


	if (rank == 0) {
		/* Saiu do laço antes do número máximo de iterações.	*/
		if(totalIteracoes < jIteMax)
		{
			for(i = 0 ; i < jOrder ; i++)
			{
				sum += A[jRowTest][i]*X1[i];
			}
			printf("Iterations: %d\n", totalIteracoes);
			printf("RowTest: %d => [%f] =? %f\n", jRowTest, sum, B[jRowTest]);
		}
		/* Saiu porque iterações ultrapassaram o número máximo.	*/
		else
			printf("ERROR: Number of iterations overflowed.\n");

		for(i = 0 ; i < block ; i++){
			free(A[i]);
		}
		free(A);
		free(B);
	}

	free(X);
	free(X1);
	free(dif);
	for(i = 0 ; i < block ; i++){
		free(A_recv[i]);
	}
	free(A_recv);

	MPI_Finalize();

	return 0;
}
/**
* Função que faz a subtração das multiplicações do vetor
* solução pela linha da matriz A.
*/
double subtratorio(double* X1,int lin, int jOrder, double** A)
{
	double total = 0;
	int i;
	for(i = 0 ; i < jOrder ; i++)
	{
		if(i != lin) 
			total -= X1[i]*A[lin][i];
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
	for( i = 0 ; i < jOrder ; i++)
	{
		if(X1[i] > max)
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
	for(i = 0 ; i < jOrder ; i++)
	{
		X1[i] = X2[i];
	}		
}
/**
* O algoritmo de Gauss-Jacobi utilizando as funções
* anteriores como auxiliares.
*/
void gaussJacobi(double **A, double *B, int jOrder, double jError, int jIteMax, int jRowTest)
{
	/* Variaveis locais */
	double X[jOrder];
	double X1[jOrder];
	double dif[jOrder];	
	int i;
	double maxi, error;
	double sum = 0;
	int iteracao;
	/* Inicia o vetor solução	*/
	for (i = 0 ; i < jOrder ; i++)
		X[i] = 0;
	/* Laço que vai rodar tudo	*/
	for(iteracao = 0 ; iteracao < jIteMax ; iteracao++)
	{
		for(i = 0 ; i < jOrder ; i++)
			X1[i] = ((B[i] + subtratorio(X,i,jOrder,A))/A[i][i]);
		/* Gera o vetor com a diferença das soluções.		*/
		diferenca(X,X1,dif,jOrder);	
		/* Verifica o máximo valor desse valor.		*/
		error = max(dif,jOrder);
		if (error < 0)
			error *= (-1);
		/* Transforma para o valor absoluto */		
		maxi = max(X1,jOrder);		
		if (maxi < 0)
			maxi *= (-1);
		/* Verifica erro foi alcançado.		*/
		error /= maxi;		
		if(error < jError)
			break;
		/* Se chegou aqui, vai precisar de mais iteração
		  Então copia o vetor solução para começar outra */	
		copia(X,X1,jOrder);
	}
	/* Saiu do laço antes do número máximo de iterações.	*/
	if(iteracao < jIteMax)
	{
		for(i = 0 ; i < jOrder ; i++)
		{
			sum += A[jRowTest][i]*X1[i];
		}
		printf("Iterations: %d\n", iteracao);
		printf("RowTest: %d => [%f] =? %f\n", jRowTest, sum, B[jRowTest]);
	}
	/* Saiu porque iterações ultrapassaram o número máximo.	*/
	else
		printf("ERROR: Number of iterations overflowed.\n");
}

