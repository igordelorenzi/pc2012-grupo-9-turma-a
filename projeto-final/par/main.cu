/**
* compilar: nvcc main.cu -o main -arch sm_11
* executar: sh exec.sh <num_blocos> <num_threads>
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>
#include <curand_kernel.h>

// Tamanho da palavra
#define PTAM 5
// Define Palavra como array de char de 6 posições
typedef char Palavra[PTAM + 1];

/*
* Estrutura do dicionário
* Guarda palavra e se foi encontrada	
*/
struct palavraStruct
{
	Palavra pal;
	unsigned int encontrado;
};

__global__
void semearCuda(curandState *estado, int semente);
__host__ __device__
int comparaCuda(Palavra p1, Palavra p2);
__device__
void geraPalavraCuda(Palavra palGerada, unsigned int tam, curandState* estado);
__device__
void verificaPalavraCuda(Palavra palGerada, palavraStruct *dic, unsigned int dicTam, unsigned int* cont);
__global__
void processoCuda(palavraStruct *dic, unsigned int dicTam, curandState *estado, unsigned int *cont);
int nrLinhas(char *arquivo);
int comparaQuick(const void *a, const void *b);

int main(int argc,char **argv)
{
	// Variaveis host
	timeval tempo;
	int blocks, threads, porcentagem;
	unsigned int i;
	FILE* in;
	char palavra[PTAM + 1];
	palavraStruct *hDic;
	curandState *hEst;
	unsigned int hTam;	
	unsigned int hCont = 0;

	// Variaveis device
	palavraStruct *dDic;
	curandState *dEst;
	unsigned int *dCont;
	// Verifica argumentos
	if(argc != 4)
    	{
        	printf("Uso:\n\t%s <arquivo_filtrado> <num_blocos> <num_threads>\n",argv[0]);
		return -1;
    	}
	// Converte parametros de blocos e threads
	blocks = atoi(argv[2]);
	threads = atoi(argv[3]);
	// Le o numero de palavras no dicionario
	hTam = nrLinhas(argv[1]);
	// Aloca vetor de estados no host
	hEst = (curandState *)malloc(threads * blocks * sizeof(curandState));

	printf("Lendo palavras filtradas...");
	// Aloca dicionario no host
	hDic = (palavraStruct *) malloc(hTam * sizeof(palavraStruct));

	// Povoa o dicionario com as palavras do arquivo filtrado	
	in = fopen(argv[1],"r");
	i = 0;
	while (fscanf(in, "%s", palavra) != EOF) {
		strcpy(hDic[i].pal, palavra);
		hDic[i].encontrado = 0;
		i++;
	}
	fclose(in);
	printf("OK!\n");

	// Ordena o dicionario em memória
	printf("Ordenando as palavras...");
	qsort(hDic, hTam , sizeof(struct palavraStruct), comparaQuick); 
	printf("OK!\n");
	
	// Aloca as variaveis em device
	cudaMalloc((void **)&dEst, threads * blocks * sizeof(curandState));
	cudaMalloc((void **)&dDic, sizeof(palavraStruct)*hTam);
	cudaMalloc((void **)&dCont, sizeof(unsigned int));

	// Inicializa variavel contador em device	
	cudaMemset(dCont, 0, sizeof(unsigned int));

	// Copia os dados do host para device
	cudaMemcpy(dDic, hDic, sizeof(palavraStruct)*hTam, cudaMemcpyHostToDevice);
	cudaMemcpy(dEst, hEst, sizeof(curandState)*threads * blocks, cudaMemcpyHostToDevice);	
	semearCuda<<<blocks, threads>>>(dEst, time(NULL));
	
	porcentagem = 10;
	gettimeofday(&tempo, NULL);
	double t1 = tempo.tv_sec + (tempo.tv_usec/1000000.0);

	// Laço principal, para quando encontra todas palavras do dicionario
	while(hCont < hTam)
	{
		// Pula pelo numero total de threads trabalhando
		for(i = 0 ; i < hTam ; i += blocks * threads)
		{
			// Chama processo para cada threads de todos os blocos
			processoCuda<<<blocks, threads>>>(dDic, hTam, dEst, dCont);
	    	}
		// Copia contador de device para host, para verificacao
		cudaMemcpy(&hCont, dCont, sizeof(unsigned int), cudaMemcpyDeviceToHost);
		// Verifica a porcentagem ja encontrada
		if((hCont*100) / hTam >= porcentagem)
		{
			gettimeofday(&tempo, NULL);
			double t2 = tempo.tv_sec + (tempo.tv_usec/1000000.0);
			printf("%d %% (%.3lf segundos)\n", porcentagem, t2-t1);
			porcentagem += 10;
		}	
	}
	// Copia o dicionario para o host novamente	
	cudaMemcpy(hDic, dDic, sizeof(palavraStruct)*hTam, cudaMemcpyDeviceToHost);
	
	// Libera memoria em device e no host
	free(hDic);
	free(hEst);
	cudaFree(dEst);
	cudaFree(dDic);
	cudaFree(dCont);

	return 0;
}

/*
* Cria uma semente para cada thread
*/
__global__
void semearCuda(curandState *estado, int semente)
{
	int idx = threadIdx.x + blockIdx.x * blockDim.x;

	curand_init(semente + idx, 0, 0, &estado[idx]);
}

/*
* Verifica na GPU se palavra gerada é igual do dicionário
*/
__host__ __device__
int comparaCuda(Palavra p1, Palavra p2)
{
	unsigned int i;
	for(i = 0 ; i < PTAM ; i++)
	{
		if(p1[i] != p2[i] || p1[i] == '\0' || p2[i] == '\0')
	    		return p1[i] - p2[i];
	}
	return 0;
}

/*
* Gera uma palavra randomica de tamanho 'tam'
*/
__device__
void geraPalavraCuda(Palavra palGerada, unsigned int tam, curandState* estado)
{
	unsigned int i;
	// ID global da thread
	int idx = threadIdx.x + blockIdx.x * blockDim.x;
	// Estado local
	curandState estadoLocal = estado[idx];

	for(i = 0; i < tam + 1; i++)
		palGerada[i] = curand(&estadoLocal) % 26 + 97;

	// Insere '\0'
	palGerada[tam] = '\0';
	estado[idx] = estadoLocal;
}

/*
* Verifica se palavra está no dicionário por busca binária
* se encontra verifica (atomicamente) se já foi encontrada anteriormente
*/
__device__
void verificaPalavraCuda(Palavra palGerada, palavraStruct *dic, unsigned int dicTam, unsigned int* cont)
{
	// Inicializa a busca binária
	unsigned int ini = 0, fim = dicTam - 1, med;
	int aux;
	// Enquanto for um intervalo válido
	while(ini <= fim)
	{
		// Pega o meio
		med = (ini + fim)/2;

		// Compara com a palavra do meio
		aux = comparaCuda(palGerada, dic[med].pal);

		// Se for menor
		if(aux < 0)
	    		fim = med - 1;
		// Se for maior
		else if(aux > 0)
    			ini = med + 1;
		// Se acertou
		else
		{
			if(!atomicCAS(&(dic[med].encontrado), 0, 1))
				atomicInc(cont, dicTam + 1);
			break;
		}
	}
}

/*
* Processo que cada thread CUDA faz, gera uma palavra de 1,2,3,4 e 5 letras
* e verifica se já foi gerada no dicionário
*/
__global__
void processoCuda(palavraStruct *dic, unsigned int dicTam, curandState *estado, unsigned int *cont)
{
	unsigned int i;
	for(i = 1 ; i < PTAM + 1 ; i++)
	{
		// Gera uma palavra aleatória
		Palavra palGerada;
		geraPalavraCuda(palGerada, i, estado);
		// Verifica quantas acertaram
		verificaPalavraCuda(palGerada, dic, dicTam, cont);
	}
}

/*
* Conta o número de linhas (palavras) do arquivo filtrado
*/
int nrLinhas(char *arquivo)
{
	char c;
	int cont = 0;
	FILE* ent;

	ent = fopen(arquivo,"r");
	do
	{
		c = fgetc(ent);
		if (c == (int) '\n' || c == (int) '\r')
			cont++;
	}while (c != EOF);
	fclose(ent);
	return cont;
}

/*
* Compara as palavras para ordenar o dicionário
*/
int comparaQuick(const void *a, const void *b) 
{ 
	struct palavraStruct *ia = (struct palavraStruct *)a;
	struct palavraStruct *ib = (struct palavraStruct *)b;
	return strcmp(ia->pal, ib->pal);
}

