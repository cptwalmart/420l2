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

matrix matrixsub(matrix * A, matrix * B, MPI_Status * status, MPI_Comm * world, int cores, int * ran);

matrix matrixadd(matrix * A, matrix * B, MPI_Status * status, MPI_Comm * world, int cores, int * ran);

matrix matrixdotproduct(matrix * A, matrix * B, MPI_Status * status, MPI_Comm * world, int cores, int * ran);

#endif
