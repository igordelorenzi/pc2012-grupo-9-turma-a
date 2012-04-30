/*
 * Para compilar: gcc str.c -o str -lm
 */

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

char *strRev(char *);
int chkPal(char *);
void strUpper(char *);

int main(int argc, char *argv[])
{
	int contW = 0, contS = 0;
	char *str, pch[300], *endStr;
	FILE *fpin;

	if((fpin=fopen(argv[1], "r")) == NULL)
	{	
		printf("Problema no arquivo.\n");
		return -1;
	}

	while(!feof(fpin))
	{	
		fgets(pch, 300, fpin);
		str = strtok_r(pch, ".\t\n", &endStr);
		while(str != NULL)
		{
			char *endToken;
			printf("%s\n",str);
			if(chkPal(str))
				contS++;
		  	char *message = strtok_r(str, " ,/?'\";:|^-!$#@`~*&%)(+=_}{][\\", &endToken);
		  	while (message != NULL)
		  	{
				printf("%s\n", message);
				if(chkPal(message))
					contW++;
		    		message = strtok_r(NULL, " ,/?'\";:|^-!$#@`~*&%)(+=_}{][\\", &endToken);
	  		}	
			str = strtok_r(NULL, ".\t\n", &endStr);
		}
	}
	printf("# palW: %d\n# palS: %d\n", contW, contS);
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
