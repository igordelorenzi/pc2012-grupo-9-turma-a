/*
 * Para compilar: gcc teste.c -o teste -lm
 */

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

char *strRev(char *);
int chkPal(char *);
int chkPrimo(char *);
void strUpper(char *);

int main(int argc, char *argv[])
{
	int cont = 0, cont2 = 0;
	char *message, pch[100];
	FILE *fpin;

	if((fpin=fopen(argv[1], "r")) == NULL)
	{	
		printf("Problema no arquivo.\n");
		return -1;
	}

	while(!feof(fpin))
	{	
		fgets(pch, 100, fpin);
	  	message = strtok (pch, " ,./?'\";:|^-!$#@`~*&%)(+=_}{][\n\t\\");
	  	while (message != NULL)
	  	{	
			if(chkPal(message)){
				cont++;
				if(chkPrimo(message))
					cont2++;
			}
	    		message = strtok(NULL, " ,./?'\";:|^-!$#@`~*&%)(+=_}{][\n\t\\");
	  	}
	}
	printf("# pal: %d\n# primo: %d\n", cont, cont2);

	return 0;
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
	for (i=0;i<strlen(str);i++)
		str[i] = toupper(str[i]);
}

int chkPrimo(char palavra[])
{
	int i, divisor, divisores, num=0;
	float max;

	for (i=0;i<strlen(palavra);i++) num += (int)palavra[i];

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

