/*
 * Para compilar: mpicc stringWord_OMPI.c -o stringWord_OMPI -Wall
 * Para rodar: mpirun -np 10 stringWord_OMPI ../input/shakespe.txt
 * No cluster: time mpirun -np 10 -machinefile machinefile.xmen stringWord_OMPI ../input/shakespe.txt
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <mpi.h>

#define FOLGA 10

/* Estrutura usada para armazenar resultados parciais */
typedef struct {
	int contS;
	int contW;
	double tS;
	double tW;
} PartialResult;

/* Protótipos das funções */
char *strRev(char *);
int chkPal(char *, double *);
void strUpper(char *);

int main(int argc, char *argv[])
{
	int curr_rank, num_procs, rc, dest=1, tag, block_size;
	int contW_total = 0, contS_total = 0;
	int i, namelen;
	char processor_name[MPI_MAX_PROCESSOR_NAME];
	char *message, *message2, *endPToken, *endWToken;
	char *pch;
	FILE *fpin;
	long lSize;
	MPI_Status status;
	double t_aux, t_totalS=0.0, t_totalW=0.0;
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

	//printf("Nome do processador: %s | rank: %d\n", processor_name, curr_rank);

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

		printf("Nro de processos: %d\nTam. total do arquivo: %ld\n"
			"Tam. bloco: %d\n", num_procs, lSize, block_size);

		/* Aloca memória para o bloco que será lido */
		pch = (char *) malloc(block_size * sizeof(char));

		while (!feof(fpin))
		{
			/* Faz leitura sequencial de blocos com tamanho definido acima */
			fread(pch, 1, block_size, fpin);
			
			/* Envia o tamanho do bloco que será processado para o nó 
			 * correspondente */
			tag = dest;
			MPI_Send(&block_size, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);

			/* Envia o bloco lido acima */
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
			contS_total += pr.contS;
			contW_total += pr.contW;
			t_totalS += pr.tS;
			t_totalW += pr.tW;
		}

		/* Exibe resultado dos cálculos */
		printf("Quantidade de palindromos de palavras: %d\n"
			"Quantidade de palindromos de frases: %d\n"
			"Tempo para calculo dos palindromos: \n\t"
			"-> palavras: %lf\n\t"
			"-> frases: %lf\n", 
			contW_total, contS_total, t_totalW, t_totalS);

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
		pr.contS = pr.contW = 0;
		pr.tS = pr.tW = 0.0;

		/* Início do parsing e chamadas da função que irá verificar se é 
		 * palindromo. A função strtok_r foi usada para evitar que se perca
		 * a referencia do ultimo token encontrado. Esse problema aparece
		 * em chamadas aninhadas de strtok() */
		message = strtok_r(pch, ".\t\n\r", &endPToken);
		while (message != NULL)
		{
			/* chkPal é chamado para calcular o palindromo do token message.
			 * Retorna o tempo gasto para o cálculo na variável t_aux */
			if (chkPal(message, &t_aux))
			{
				(pr.contS)++;
				pr.tS += t_aux;
			}

			/* Chamada aninhada de strtok_r para calcular palindromo de 
			 * palavra */
			message2 = strtok_r(message, " ,/?'\";:|^-!$#@`~*&%)(+=_}{][\\", 
				&endWToken);

			while (message2 != NULL)
			{
				/* chkPal é chamado para calcular o palindromo do token message.
				 * Retorna o tempo gasto para o cálculo na variável t_aux */
				if (chkPal(message2, &t_aux))
				{
					(pr.contW)++;
					pr.tW += t_aux;
				}

				message2 = strtok_r(NULL, " ,/?'\";:|^-!$#@`~*&%)(+=_}{][\\", 
					&endWToken);
			}

			message = strtok_r(NULL, ".\t\n\r", &endPToken);
			
		}

		/* Envia o resultado parcial para o processo root */
		tag = 2;
		MPI_Send(&pr, 1, prDatatype, 0, tag+curr_rank, MPI_COMM_WORLD);

		free(pch);
	}

	/* Finaliza e libera memória do processo MPI */
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

