/*
 * Para compilar: gcc stringWord_OMP.c -o sw_omp -lm -fopenmp
 */

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <omp.h>

#define BLOCK_SIZE 300
#define WORD_SIZE 50
#define PHRASE_SIZE 100

char *strRev(char *);
int chkPal(char *);
void strUpper(char *);
void parser(char *, char **, int *);

int main(int argc, char *argv[])
{
	int contW = 0, contS = 0, n = 0, m = 0, i, j;
	char *str, pch[BLOCK_SIZE], *phrases[PHRASE_SIZE], *message, *endToken;
	double star, end, pT = 0.0, wT = 0.0;	
	FILE *fpin;

	if((fpin=fopen(argv[1], "r")) == NULL)
	{	
		printf("Problema no arquivo.\n");
		return -1;
	}
	
	while(!feof(fpin))
	{
		fread(pch, 1, BLOCK_SIZE, fpin);
		parser(pch,phrases,&n);
	       #pragma omp parallel for num_threads(2) private(message,endToken) shared(phrases) reduction(+:contS,contW,wT,pT)
		for(i=0;i<n;i++){
			star = omp_get_wtime();
			if(chkPal(phrases[i])){
				end = omp_get_wtime();
				pT += end - star;
				contS++;
			}
			message = strtok_r(phrases[i], " ,/?'\";:|^-!$#@`~*&%)(+=_}{][\\", &endToken);
			while(message != NULL){
				star = omp_get_wtime();				
				if(chkPal(message)){
					end = omp_get_wtime();
					wT += end - star;
					contW++;
				}
				message = strtok_r(NULL, " ,/?'\";:|^-!$#@`~*&%)(+=_}{][\\", &endToken);
			}
		}
		*phrases = NULL;		
	}
	printf("# Tempo para palavra: %fs\n# Tempo para frase: %fs\n# palW: %d\n# palS: %d\n", wT, pT, contW, contS);
	return 0;
}

void parser(char *bloco, char **phrases, int *n)
{
	int m = 0;	
	char *message = strtok(bloco, ".\t\n\r");
	while(message != NULL){
		phrases[m] = message;
		message = strtok(NULL, ".\t\n\r");
		m++;
	}
	*n = m;
}

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

int chkPal(char *str)
{
	char aux[1000];
		
	if(strlen(str) > 1){
		strUpper(str);
		strcpy(aux,str);
		strRev(aux);
		if(strcmp(str,aux) == 0 )
			return 1;
		else 
			return 0;
	}else{ return 0;}
}

void strUpper(char *str)
{
	int i;
	
	for (i=0;i<strlen(str);i++)
		str[i] = toupper(str[i]);
}
