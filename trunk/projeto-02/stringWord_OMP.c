/*
 * Para compilar: gcc str.c -o str -lm -fopenmp
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
void parserPh(char *, char **, int *);
void parserW(char *, char **, int *);

int main(int argc, char *argv[])
{
	int contW = 0, contS = 0, n = 0, m = 0, i, j;
	char *str, pch[BLOCK_SIZE], *phrases[PHRASE_SIZE], *words[WORD_SIZE];
	FILE *fpin;

	if((fpin=fopen(argv[1], "r")) == NULL)
	{	
		printf("Problema no arquivo.\n");
		return -1;
	}

	while(!feof(fpin))
	{	
		fgets(pch, BLOCK_SIZE, fpin);
		parserPh(pch,phrases,&n);
		#pragma omp parallel for num_threads(2) schedule(dynamic,30) reduction(+:contS) reduction(+:contW)
		for(i=0;i<n;i++){
			if(chkPal(phrases[i]))
				contS++;
			parserW(phrases[i],words,&m);
			#pragma omp parallel for num_threads(2) shared(i,n) schedule(dynamic,30) reduction(+:contW)
			for(j=0;j<m;j++){
				if(chkPal(words[j]))
					contW++;
			}
		}		
	}
	printf("# palW: %d\n# palS: %d\n", contW, contS);
	return 0;
}

void parserPh(char *bloco, char **phrases, int *n)
{
	int m = 0;	
	char *message = strtok(bloco, ".\t\n");
	while(message != NULL){
		phrases[m] = message;
		message = strtok(NULL, " .\t\n");
		m++;
	}
	*n = m;
}

void parserW(char *bloco, char **words, int *n)
{
	int m = 0;	
	char *message = strtok(bloco, " ,./?'\";:|^-!$#@`~*&%)(+=_}{][\n\t\\");
	while(message != NULL){
		words[m] = message;
		message = strtok(NULL, " ,./?'\";:|^-!$#@`~*&%)(+=_}{][\n\t\\");
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
	char aux[100];
	
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
	#pragma omp parallel for
	for (i=0;i<strlen(str);i++)
		str[i] = toupper(str[i]);
}
