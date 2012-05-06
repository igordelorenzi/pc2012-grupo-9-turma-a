/*
 * Para compilar: mpicc wordPrimo_OMPI.c -o wordPrimo_OMPI -Wall
 * Para rodar: mpirun -np 10 wordPrimo_OMPI ../input/wikipedia.txt
 * No cluster: time mpirun -np 10 -machinefile machinefile.xmen wordPrimo_OMPI ../input/wikipedia.txt
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <mpi.h>

#define FOLGA 10

/* Estrutura usada para armazenar resultados parciais */
typedef struct {
	int contPal;
	int contPri;
	double tPal;
	double tPri;
} PartialResult;

char *strRev(char *);
int chkPal(char *, double *);
int chkPrimo(char *, double *);
void strUpper(char *);

int main(int argc, char *argv[])
{
	int i, block_size;
	int namelen, curr_rank, num_procs, rc, tag, dest=1;
	char processor_name[MPI_MAX_PROCESSOR_NAME];
	int contPal_total = 0, contPri_total = 0;
	char *message, *pch;
	FILE *fpin;
	long lSize;
	double t_aux, t_totalPal = 0.0, t_totalPri = 0.0;
	MPI_Status status;
	int count = 4;
	int lengths[4] = {1, 1, 1, 1};
	MPI_Datatype prDatatype;
	PartialResult pr;

	/* Inicializa e sincroniza MPI para o processo atual */
	rc = MPI_Init(&argc, &argv);
	if (rc != MPI_SUCCESS)
	{	
		printf("Problema ao iniciar processo MPI.\n");
		return -1;
	}

	/* Definição da struct PartialResult */
	MPI_Aint offsets[4] = {0, sizeof(int), sizeof(int) + sizeof(int), 
		sizeof(int) + sizeof(int) + sizeof(double)};
	MPI_Datatype types[4] = {MPI_INT, MPI_INT, MPI_DOUBLE, MPI_DOUBLE};	
	MPI_Type_struct(count, lengths, offsets, types, &prDatatype);
	MPI_Type_commit(&prDatatype);

	/* Pega id do processo atual */
	MPI_Comm_rank(MPI_COMM_WORLD, &curr_rank);
	/* Pega o número de processos do grupo */
	MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
	/* Pega o nome do nó que está executando o processo */
	MPI_Get_processor_name(processor_name, &namelen);

	printf("Nome do processador: %s | rank: %d\n", processor_name, curr_rank);

	if (curr_rank == 0)
	{
		/* Abre o arquivo que será processado */
		if ((fpin = fopen(argv[1], "r")) == NULL)
		{	
			printf("Problema no arquivo.\n");
			MPI_Finalize();
			return -1;
		}

		/* Obtém o tamanho aproximado do arquivo em bytes */
		fseek(fpin, 0, SEEK_END);
		lSize = ftell(fpin);
		rewind(fpin);

		/* Inicializa os blocos que serão divididos entre os nós */
		if (num_procs == 1) {
			MPI_Finalize();
			return -1;
		}
		else if (num_procs == 2) {
			/* Dada a imprecisão de lSize, coloca-se uma folga de 10 bytes */
			block_size = lSize + FOLGA;
		}
		else
			block_size = lSize / (num_procs-1) + FOLGA;

		printf("num_procs: %d\ntotal_size: %ld\nblock_size: %d\n",num_procs,
				lSize,block_size);
		pch = (char *) malloc(block_size * sizeof(char));

		while (!feof(fpin))
		{	
			fread(pch, 1, block_size, fpin);

			tag = dest;
			MPI_Send(&block_size, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);

			tag = dest+1;
			MPI_Send(pch, block_size, MPI_CHAR, dest, tag, MPI_COMM_WORLD);
			
			dest++;	  	
		
		}

		/* Faz chamada Recieve para todos os subprocessos de forma bloqueante */
		for (i=1; i<num_procs; i++)
		{
			tag = 2;
			MPI_Recv(&pr, 1, prDatatype, i, i+tag, MPI_COMM_WORLD, 
				&status);

			/* Incrementa resultados parciais */
			contPal_total += pr.contPal;
			contPri_total += pr.contPri;
			t_totalPal += pr.tPal;
			t_totalPri += pr.tPri;
		}

		/* Exibe resultado dos cálculos */
		printf("Quantidade de palindromos de palavras: %d\n"
			"Quantidade de palindromos de soma prima: %d\n"
			"Tempo para calculo de: \n\t"
			"-> palindromos: %lf\n\t"
			"-> soma prima (crivo): %lf\n", 
			contPal_total, contPri_total, t_totalPal, t_totalPri);

	}
	else
	{
		/* Recebe do processo root, o tamanho do bloco */
		tag = curr_rank;
		MPI_Recv(&block_size, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);

		/* Aloca memória de acordo com o tamanho definido acima */
		pch = (char *) malloc(block_size * sizeof(char));

		/* Recebe do processo root, o bloco com tamanho já definido */
		tag = curr_rank+1;
		MPI_Recv(pch, block_size, MPI_CHAR, 0, tag, MPI_COMM_WORLD, &status);

		/* Inicializa variáveis que armazenam os resultados parciais */
		pr.contPal = pr.contPri = 0;
		pr.tPal = pr.tPri = 0.0;

		message = strtok(pch, " ,./?'\";:|^-!$#@`~*&%)(+=_}{][\n\t\\");
		while (message != NULL)
		{
			if (chkPal(message, &t_aux))
			{			
				(pr.contPal)++;
				pr.tPal += t_aux;

				if (chkPrimo(message, &t_aux))
				{
					(pr.contPri)++;
					pr.tPri += t_aux;
				}
			}
			message = strtok(NULL, " ,./?'\";:|^-!$#@`~*&%)(+=_}{][\n\t\\");
		}

		/* Envia o resultado parcial para o processo root */
		tag = 2;
		MPI_Send(&pr, 1, prDatatype, 0, tag+curr_rank, MPI_COMM_WORLD);

		free(pch);

	}

	MPI_Finalize();
	return 0;
}

/* Reverte string */
char *strRev(char *str)
{
	char *p1, *p2;

	if (! str || ! *str)
		return str;
	for (p1 = str, p2 = str + strlen(str) - 1; p2 > p1; ++p1, --p2)
	{
		*p1 ^= *p2;
		*p2 ^= *p1;
		*p1 ^= *p2;
	}
	return str;
}

/* Função que verifica palindromos */
int chkPal(char *str, double *t)
{
	char aux[1000];
	double t1, t2;

	/* Registra a marca de tempo t1 antes de iniciar os cálculos */
	t1 = MPI_Wtime();
		
	if (strlen(str) > 1) {
		strUpper(str);
		strcpy(aux,str);
		strRev(aux);
		if (strcmp(str,aux) == 0) {
			/* Registra a marca de tempo t2 depois de terminar os cálculos */
			t2 = MPI_Wtime();
			*t = t2 - t1;
			return 1;
		}
		else {
			/* Registra a marca de tempo t2 depois de terminar os cálculos */
			t2 = MPI_Wtime();
			*t = t2 - t1;
			return 0;
		}
	}
	else {
		/* Registra a marca de tempo t2 depois de terminar os cálculos */
		t2 = MPI_Wtime();
		*t = t2 - t1;
		return 0;
	}
}

/* Transforma string para apenas letras maiusculas */
void strUpper(char *str)
{
	int i;

	for (i=0;i<strlen(str);i++)
		str[i] = toupper(str[i]);
}

/* Verifica se a soma do mapeamento ASCII de cada caracter da palavra
 * resulta em um número primo */
int chkPrimo(char palavra[], double *t)
{
	int i, divisor, divisores, num=0;
	float max;
	double t1, t2;

	/* Registra a marca de tempo t1 antes de iniciar os cálculos */
	t1 = MPI_Wtime();
	
	for (i=0;i<strlen(palavra);i++) 
		num += (int)palavra[i];

	if (num == 1 || num == 2 || num == 3) {
		/* Registra a marca de tempo t2 depois de terminar os cálculos */
		t2 = MPI_Wtime();
		*t = t2 - t1;
		return 1; /* É primo */
	}
	else if (num % 2 == 0) {
		/* Registra a marca de tempo t2 depois de terminar os cálculos */
		t2 = MPI_Wtime();
		*t = t2 - t1;
		return 0; /* Nao é primo */
	}
	else {
		divisor = 2;
		divisores = 0;
		max = sqrt(num);
		while(divisor <= max && divisores == 0){
			if(num % divisor == 0)
				divisores++;
			divisor++;
		}
		if(divisores != 0) {
			/* Registra a marca de tempo t2 depois de terminar os cálculos */
			t2 = MPI_Wtime();
			*t = t2 - t1;
			return 0; /* Nao é primo */
		}
		else {
			/* Registra a marca de tempo t2 depois de terminar os cálculos */
			t2 = MPI_Wtime();
			*t = t2 - t1;
			return 1; /* É primo */
		}
	}
}

