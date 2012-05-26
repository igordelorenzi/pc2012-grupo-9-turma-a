#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void gaussJacobi(double **,double *,int,double,int,int);
double subtratorio(double*,int,int,double**);
double max(double*,int);
void diferenca(double*,double*,double*,int);
void copia(double*,double*,int);

int main(int argc, char *argv[])
{
	double **A, *B, jError;
	int jOrder, jRowTest, jIteMax, i, j;
	FILE *entrada;

	if (argc != 2) {
		printf("USO:\n\t<nome_do_programa> <arquivo_sistema_linear>\n");
		exit(1);
	}
	entrada = fopen(argv[1], "r");
	if (entrada == NULL) {
		printf("Erro na abertura do arquivo!\n");
		exit(1);
	}
	fscanf(entrada, "%d", &jOrder);	// LENDO TAMANHO
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
	/* Rodando o algoritmo de Gauss-Jacobi	*/
	gaussJacobi(A,B,jOrder,jError,jIteMax,jRowTest);
	
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
	for(iteracao = 1 ; iteracao < jIteMax ; iteracao++)
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
	
