#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <ctype.h>
#include "proj4.h"

/*
 * initializedGrid:
 * Reads a grid of digits from a file and populates a grid structure.
 * 
 * Input:
 * - *g: Pointer to the grid structure being initialized.
 * - *filename: The name of the file containing the n-by-n digits.

 * Output:
 * - Modifies g->n to the grid dimension and populates g->p with the grid data.
*/
void initializeGrid(grid *g, char *filename) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) return;

    unsigned int count = 0;
    int ch;

    //Determines "n" by counting digits in the first line.
    while ((ch = fgetc(fp)) != EOF && ch != '\n' && ch != '\r') {
        if (isdigit(ch)) {
            count++;
        }
    }

    g->n = count;
    rewind(fp);

    g->p = (unsigned char **)malloc(g->n * sizeof(unsigned char *));
    for (unsigned int i = 0; i < g->n; i++) {
        g->p[i] = (unsigned char*)malloc(g->n * sizeof(unsigned char));
    }

    for (unsigned int i = 0; i < g->n; i++) {
        for (unsigned int j = 0; j < g->n; j++) {
            int temp;
            if (fscanf(fp, "%1d", &temp) == 1) {
                g->p[i][j] = (unsigned char)temp;
            }
        }
    }
    fclose(fp);
}

/*
 * freeGrid:
 * Deallocates dynamic memory used by the grid structure.
 * 
 * Input:
 * - *g: Pointer to the grid structure being freed.

 * Output:
 * - Memory is released. g->p is set to NULL and g->n to 0.
*/
void freeGrid(grid *g) {
    if (g != NULL && g->p != NULL) {
        for (unsigned int i = 0; i < g->n; i++) {
            free(g->p[i]);
        }
        free(g->p);
        g->p = NULL;
        g->n = 0;
    }
}

/*
 * threadWorker:
 * Computes diagonal sums for a specific range of rows.
 * 
 * Input:
 * - *arguments: Apointer to threadArgs struct containing input grid, 
 *   output grid, target sum, and the specific row range.

 * Output:
 * - Returns NULL.
 * - Modifies args->output grid by copying digits from args->input when a diagonal sum equals the target.
*/
void *threadWorker(void *arguments) {
    threadArgs *args = (threadArgs*)arguments;
    int n = (int)args->input->n;
    unsigned long target = args->target;

    for (int i = args->startRow; i <= args->endRow; i++) {
        for (int j = 0; j < n; j++) {
            
            //Main diagonal.
            unsigned long sum1 = 0;
            int k1 = 0;
            while ((i + k1 < n) && (j + k1 < n) && (sum1 < target)) {
                sum1 += args->input->p[i + k1][j + k1];
                if (sum1 == target) {
                    for (int x = 0; x <= k1; x++) {
                        args->output->p[i + x][j + x] = args->input->p[i + x][j + x];
                    }
                }
                k1++;
            }
            
            //Anti-diagonal.
            unsigned long sum2 = 0;
            int k2 = 0;
            while ((i + k2 < n) && (j - k2 >= 0) && (sum2 < target)) {
                sum2 += args->input->p[i + k2][j - k2];
                if (sum2 == target) {
                    for (int x = 0; x <= k2; x++) {
                        args->output->p[i + x][j - k2 + (k2 - x)] = args->input->p[i + x][j - x];
                    }
                }
                k2++;
            }
        }
    }
    return NULL;
}

/*
 * writeGrid:
 * Saves the grid content to a text file and sets file permissions.
 * 
 * Input:
 * - *g: Pointer to the grid to write.
 * - *fileName: The name of the output file.

 * Output:
 * - Creates and overwrites a file in the directory. Sets owner read/write permissions.
*/
void writeGrid(grid *g, char *fileName) {
    FILE *fp = fopen(fileName, "w");
    if (fp == NULL) return;

    for (unsigned int i = 0; i < g->n; i++) {
        for (unsigned int j = 0; j < g->n; j++) {
            fprintf(fp, "%u", g->p[i][j]);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
    chmod(fileName, S_IRUSR | S_IWUSR);
}

/*
 * diagonalSums:
 * Manages threads and splitting the work.
 * 
 * Input:
 * - *input: Pointer to the source digit grid.
 * - s: The target sum.
 * - *output: Pointer to the grid where results are stored.
 * - t: Number of threads being used (1 - 3).

 * Output:
 * - Populates the output grid with digits that contribute to diagonal sums of s.
*/
void diagonalSums(grid * input, unsigned long s, grid * output, int t) {
    unsigned int n = input->n;
    output->n = n;
    output->p = (unsigned char **)malloc(n * sizeof(unsigned char *));

    for (unsigned int i = 0; i < n; i++) {
        output->p[i] = (unsigned char *)calloc(n, sizeof(unsigned char));
    }

    pthread_t threads[3];
    threadArgs args[3];
    unsigned int rowsPerThread = n / t;

    for (int k = 0; k < t; k++) {
        args[k].input = input;
        args[k].output = output;
        args[k].target = s;
        args[k].startRow = k * rowsPerThread;
        if (k == (t - 1)) {
            args[k].endRow = n - 1;
        } else {
            args[k].endRow = (k + 1) * rowsPerThread - 1;
        }
        
        if (t == 1) {
            threadWorker(&args[k]);
        } else {
            pthread_create(&threads[k], NULL, threadWorker, &args[k]);
        }
    }

    if (t > 1) {
        for (int k = 0; k < t; k++) {
            pthread_join(threads[k], NULL);
        }
    }
}
