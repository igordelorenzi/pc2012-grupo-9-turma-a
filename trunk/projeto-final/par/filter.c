/**
 * gcc trie.c filter.c -o filter
 * ./filter ../input/palavras.txt ../input/palavras2.txt
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "trie.h"


int main(int argc, char *argv[])
{
	int c, i = 0, length, z, j;
	char word[100], aux[6];
	void *val;
	int count = 0;
	Trie *trie, *trie2;
	FILE *file, *file2, *file3;

	if (argc != 3)
		exit(1);

	if ((file = fopen(argv[1], "r")) == NULL)
		exit(1);

	if ((file2 = fopen("f.txt", "w")) == NULL)
		exit(1);

	trie = trie_new();

	do
	{
		c = fgetc(file);
		if ((c >= 97) && (c<= 122))
		{
			word[i] = (char) tolower(c);
			++i;
		}
		else
		{
			word[i] = '\0';
			if (!trie_lookup(trie, word))
			{
				fprintf(file2, "%s\n", word);
				trie_insert(trie, word, val);
			}
	    		i = 0;
	    		++count;
		}
	}while (c != EOF);

	fclose(file);
	fclose(file2);
	trie_free(trie);
	count = 0;
	i = 0;
	if ((file2 = fopen("f.txt", "r")) == NULL)
		exit(1);

	if ((file3 = fopen(argv[2], "w")) == NULL)
		exit(1);

	trie2 = trie_new();

	do
	{
		c = fgetc(file2);

		if ((c >= 97) && (c<= 122))
		{
			word[i] = (char) tolower(c);
			++i;
		}
		else
		{
			word[i] = '\0';
			length = strlen(word);
	
			if(length > 1)
			{
				if(length < 6)
				{
					if (!trie_lookup(trie2, word))
					{
						fprintf(file3, "%s\n", word);
						trie_insert(trie2, word, val);
						i = 0;
						++count;
					}
				}
				else{
					j = 0;
			
					for(z = 0; z < length; z++)
					{
						if(j == 5)
						{
							aux[j] = '\0';
							if (!trie_lookup(trie2, aux))
							{
								fprintf(file3, "%s\n", aux);
								trie_insert(trie2, aux, val);
								count++;
							}
							aux[0] = tolower(word[z]);
							j = 1;
						}
						else
						{
							aux[j] = tolower(word[z]);
							j++;
						}
					}
					aux[j] = '\0';
					if (!trie_lookup(trie2, aux))
					{
						fprintf(file3, "%s\n", aux);
						trie_insert(trie2, aux, val);
						count++;			
					}
					i = 0;
				}
			}
		}
	}while (c != EOF);

	fclose(file2);
	fclose(file3);
	remove("f.txt");
	trie_free(trie2);
    
	return 0;
}
