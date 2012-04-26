#include <stdio.h>
#include <string.h>

char *strrev(char *str);
int chkPal(char *word);

int main(int argc, char *argv[])
{
	int cont = 0;
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
		printf("%s\n",pch);
	  	message = strtok (pch," ,.-!$#@%*&%)(+=_}{][");
	  	while (message != NULL)
	  	{
			if(chkPal(message))
				cont++;
	    		message = strtok(NULL, " ,.-!$#@%*&%)(+=_}{][");
	  	}
	}
	printf("# pal: %d\n", cont);

	return 0;
}

char *strrev(char *str)
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

	strcpy(aux,str);
	strrev(aux);

	if( strcmp(str,aux) == 0 )
		return 1;
	else 
		return 0;
}
