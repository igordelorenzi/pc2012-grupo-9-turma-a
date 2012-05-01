/*
 * Para compilar: gcc teste.c -o teste -lm -fopenmp
 */

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <omp.h>

#define BLOCK_SIZE 100
#define WORD_SIZE 50

char *strRev(char *);
int chkPal(char *);
int chkPrimo(char *);
void strUpper(char *);
void parser(char *, char **, int *);

int main(int argc, char *argv[])
{
	int cont = 0, cont2 = 0, i, n = 0;
	char *message, pch[BLOCK_SIZE], *words[WORD_SIZE];
	FILE *fpin;

	if((fpin=fopen(argv[1], "r")) == NULL)
	{	
		printf("Problema no arquivo.\n");
		return -1;
	}

	while(!feof(fpin))
	{	
		fgets(pch, BLOCK_SIZE, fpin);
		parser(pch,words,&n);
		#pragma omp parallel for num_threads(2) schedule(dynamic,30) reduction(+:cont) reduction(+:cont2)		  	
		for(i=0;i<n;i++){		 			
			if(chkPal(words[i])){			
				cont++;
				if(chkPrimo(words[i]))
					cont2++;
			}
		}  	
	}
	printf("# pal: %d\n# primo: %d\n", cont, cont2);

	return 0;
}

void parser(char *bloco, char **words, int *n)
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

int chkPrimo(char palavra[])
{
	int i, divisor, divisores, num=0;
	float max;
	
	#pragma omp parallel for reduction(+:num)
	for (i=0;i<strlen(palavra);i++) 
		num += (int)palavra[i];

	if(num == 1 || num == 2 || num == 3)
		return 1;
	else if(num % 2 == 0)
		return 0;
	else{
		divisor = 2;
		divisores = 0;
		max = sqrt(num);
		while(divisor <= max && divisores == 0){
			if(num % divisor == 0)
				divisores++;
			divisor++;
		}
		if(divisores != 0)
			return 0; /* Nao eh primo */
		else
			return 1; /* Eh primo */
	}
}

