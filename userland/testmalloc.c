#include <stdio.h>
#include <stdlib.h>

int main ()
{
    int rounds;
    int counter = 0;
    for (rounds = 0; rounds < 5; rounds++)
    {
        int i;
        printf("Round %d\n", rounds);

        printf("Allocating 4M chars...\n");
        char **x = (char **) malloc(4 * 1024 * 1024 * sizeof(char *));
        for (i = 0; i < 4 * 1024 * 1024; i++)
        {
            x[i] = (char *) malloc(1);
            counter++;
            if (!x[i])
            {
                printf("Failed after %d iterations\n", counter);
                return -1;
            }
        }
        for (i = 0; i < 4 * 1024 * 1024; i++)
        {
            free(x[i]);
        }
        free(x);

        printf("Allocating 2M ints...\n");
        int **y = (int **) malloc(2 * 1024 * 1024 * sizeof(char *));
        for (i = 0; i < 2 * 1024 * 1024; i++)
        {
            y[i] = (int *) malloc(sizeof(int));
            counter++;
            if (!y[i])
            {
                printf("Failed after %d iterations\n", counter);
                return -1;
            }

        }
        for (i = 0; i < 2 * 1024 * 1024; i++)
        {
            free(y[i]);
        }
        free(y);

        printf("Allocating 1M random sizes...\n");
        char **z = (char **) malloc(1024 * 1024 * sizeof(char *));
        for (i = 0; i < 1024 * 1024; i++)
        {
            z[i] = (char *) malloc(sizeof(char) * (rand() % 5) + 1);
            counter++;
            if (!z[i])
            {
                printf("Failed after %d iterations\n", counter);
                return -1;
            }
        }
        for (i = 0; i < 1024 * 1024; i++)
        {
            free(z[i]);
        }
        free(z);
    }
    printf("Leave 16M residue behind\n");
    char * bla = malloc(16 * 1024 * 1024);
    *bla = 1;
    return 0;
}

