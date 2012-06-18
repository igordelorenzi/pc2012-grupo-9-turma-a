#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define BLOCK_SIZE 2400000 //tamanho do bloco em bytes
#define WORD_MAX_SIZE 5 //tamanho maximo de uma palavra
#define PRIME 997 //numero primo usado na tabela de dispersao (hash)

//nó da tabela hash
typedef struct {
	char word[WORD_MAX_SIZE + 1];
	int found;
} WORD;

//estrtura da tabela hash
typedef struct _NODE {
    char word[WORD_MAX_SIZE];
    struct _NODE *next;
} NODE;

//deixa td minusculo
void tolower_word(char *);

int main(int argc, char *argv[])
{
	FILE *in;
	//long size;
	char c, word[100];
	int counter=0,wordlen,random,i;

	if (argc != 2)
	{
		printf("USO:\n\t%s <arquivo_palavras>\n", argv[0]);
		exit(1);
	}

	if ((in = fopen(argv[1], "r")) == NULL)
	{
		printf("Impossivel abrir arquivo de palavras.\n");
		exit(1);
	}

/*--------------- begin-TESTES ---------------*/

	c = getc(in);
	while (c != ',')
	{
		if (c == 32 || c == ' ')
			word[counter++] = '-';
		else
			word[counter++] = c;
		c = getc(in);
	}
	word[counter]='\0';
	tolower_word(word);
	printf("palavra: '%s'\n",word);
	//printf("%d\n",word[1]); 32

	srand(time(NULL));

	wordlen = strlen(word), counter=0;
	while (counter < wordlen-1)
	{
		random = rand() % 26 + 'a';
		printf("rand(%d/%d) = %d (%c) | %s\n",counter,wordlen,random,(char)random,word);

		for (i=0; i<wordlen; i++)
			if (word[i] == random) {
				word[i]='-';
				counter++;
			}
	}
	printf("palavra: '%s' | %d\n",word,counter);

/*--------------- end-TESTES ---------------*/

	/*// Obtém o tamanho aproximado do arquivo em bytes 
	fseek(in, 0, SEEK_END);
	size = ftell(in);
	rewind(in);
	printf("%ld\n",size);

	while (!feof(in))
	{
		fread(buffer, BLOCK_SIZE, 1, in);
		
		process(buffer);
	}*/

	fclose(in);

	return 0;
}

void tolower_word(char *word)
{
	int i,len = strlen(word);
	for (i=0;i<len;i++)
		word[i]=tolower(word[i]);
}

/*
void process(char *buffer)
{
	NODE *dictionary[PRIME];

	while ()
	{
		read_words(current_word->word);
		//printf("%s\n",current_word->word);

		current_word->found = 0;

		add(dictionary, current_word);
	}
}

void read_words(NODE **dictionary)
{
	char c;
	unsigned counter = 0;

	c = getc(fp);

	while (c != '\n' && c != ',' && c != ' ' && counter < size - 1)
	{
		word[counter++] = c;
		c = getc(fp);
	}

	word[counter] = '\0';
}
*/

