#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define BLOCK_SIZE 2400000 //tamanho do bloco em bytes
#define WORD_MAX_SIZE 5 //tamanho maximo de uma palavra
#define PRIME 997 //numero primo usado na tabela de dispersao (hash)

/*nó da tabela hash*/
typedef struct
{
    char word[WORD_MAX_SIZE + 1];
    int found;
} WORD;

/*estrtura da tabela hash*/
typedef struct _NODE
{
    char word[WORD_MAX_SIZE];
    struct _NODE *next;
} NODE;

/*deixa td minusculo*/
void tolower_word(char *);

void process1(char *);
void process2(char *);
void process3(char *);

int main(int argc, char *argv[])
{
    FILE *in;
    //long size;
    char c, word[100], word2[100], *subword;
    int counter = 0;

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
    word[counter] = '\0';
    tolower_word(word);
    printf("palavra: '%s'\n", word);
    //printf("%d\n",word[1]); 32

    srand(time(NULL));

    /**
     * Geram-se letras aleatórias e as comparam com todas as posições da palavra
     * Caso o caracter testado combine, em qualquer posição, 
     * substitui-o por '-'
     * O processo termina quando todas as posições combinaram
     */
    strcpy(word2, word);
    process1(word);

    /**
     * Geram-se letras aleatórias e as comparam com cada posição da palavra, 
     * ou seja, a cada nova letra gerada, o teste é feito apenas na última
     * posição que não combinou
     * Caso o caracter testado combine, substitui-o por '-'
     * O processo termina quando todas as posições combinaram
     */
    process2(word2);

    /**
     * Geram-se palavras inteiramente aleatórias e as comparam com a 
     * palavra passada por parâmetro
     * O processo termina quando a palavra aleatória combinar com a testada
     */
    /*
        subword = strtok(word, "-, ");
        while (subword != NULL)
        {
            process3(subword); //break;
            subword = strtok(NULL, "-, ");
        }
     */

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

void process1(char *word)
{
    int wordlen, counter, i, random, letters = 0;

    wordlen = strlen(word), counter = 0;

    for (i = 0; i < wordlen; i++)
        if (word[i] >= 'a' && word[i] <= 'a' + 26)
            letters++;
    
    while (letters != counter)
    {
        random = rand() % 26 + 'a';
        printf("process1: rand(%d) = %d (%c) | %s\n", counter, random, (char) random, word);

        for (i = 0; i < wordlen; i++)
            if (word[i] == random)
            {
                word[i] = '-';
                counter++;
            }
    }
    printf("palavra(%d caracteres): '%s' | %d\n\n", wordlen, word, counter);
}

void process2(char *word)
{
    int wordlen, counter, random;

    wordlen = strlen(word), counter = 0;
    while (counter < wordlen)
    {
        random = rand() % 26 + 'a';
        printf("process2: rand(%d/%d) = %d (%c) | %s\n", counter, wordlen, random, (char) random, word);

        if (word[counter] == '-')
        {
            counter++;
            continue;
        }

        if (word[counter] == random)
        {
            word[counter] = '-';
            counter++;
        }
    }
    printf("palavra(%d caracteres): '%s' | %d\n\n", wordlen, word, counter);
}

void process3(char *word)
{
    int wordlen, counter, i;
    char *aux;

    wordlen = strlen(word);
    counter = 0;
    aux = (char *) malloc(sizeof (char) * wordlen);

    for (i = 0; i < wordlen; i++)
        if (word[i] == '-')
            return;

    while (1)
    {
        for (i = 0; i < wordlen; i++)
            aux[i] = rand() % 26 + 'a';

        printf("process3: aux(%d) = %s | %s\n", counter, aux, word);

        if (strcmp(word, aux))
            counter++;
        else
            break;
    }
    printf("palavra: '%s' | %d\n\n", word, counter);
}

void tolower_word(char *word)
{
    int i = 0;

    while (word[i] != '\0')
    {
        word[i] = tolower(word[i]);
        i++;
    }
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

