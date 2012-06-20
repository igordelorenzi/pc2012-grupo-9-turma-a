/**
 * gcc trie.c filter.c -o filter
 * ./filter ../input/palavras.txt ../input/palavras2.txt
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "trie.h"

int main(int argc, char *argv[])
{
    int c, i = 0;
    char word[100];
    void *val;
    int count = 0;
    Trie *trie;
    FILE *file, *file2;

    if (argc != 3)
        exit(1);

    if ((file = fopen(argv[1], "r")) == NULL)
        exit(1);
    
    if ((file2 = fopen(argv[2], "w")) == NULL)
        exit(1);

    trie = trie_new();

    do
    {
        c = fgetc(file);

        if (c != (int) ',' && c != (int) ' ' && c != (int) '\n' && i < 5)
        {
            word[i] = (char) c;
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
    }
    while (c != EOF);

    printf("trie_num_entries: %d\ncount: %d\n", trie_num_entries(trie), count);

    fclose(file);
    fclose(file2);
    trie_free(trie);

    return 0;
}
