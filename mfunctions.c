//set up a C macro to calculate the location of an element (i,j)
#include <stdio.h>
#include <mpi.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include "mfunctions.h"
#define INDEX(n, m, i, j) m *i + j
#define ACCESS(A, i, j) A->arr[INDEX(A->rows, A->cols, i, j)]

void initMatrix(matrix *A, int r, int c)
{
    A->rows = r;
    A->cols = c;
    A->arr = calloc(r * c, sizeof(int));

    int i, j;
    for (i = 0; i < r; i++)
        for (j = 0; j < c; j++)
            ACCESS(A, i, j) = rand() % 2 + 1;
}

void printMatrix(matrix *A)
{
    int i, j;
    for (i = 0; i < A->rows; i++)
    {
        for (j = 0; j < A->cols; j++)
        {
            printf("%d ", ACCESS(A, i, j));
        }
        //printf("\n");
        puts("");
    }
}

int index_calc(matrix *A, int i, int j)
{
    return INDEX(A->rows, A->cols, i, j);
}

int addMatrix(int c, int d){
    int ans = 0;
    ans += c + d;
    return ans;
}

int subMatrix(int c, int d){
    int ans = 0;
    ans += c - d;
    return ans;
}