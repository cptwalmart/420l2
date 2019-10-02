#ifndef MFUNCTIONS_H //Header protection
#define MFUNCTIONS_H

typedef struct matrix
{
    int rows, cols;
    int *arr;
} matrix;

void initMatrix(matrix *A, int r, int c);

void printMatrix(matrix *A);

int index_calc(matrix *A, int i, int j);

int addMatrix(int c, int d);

int subMatrix(int c, int d);

#endif