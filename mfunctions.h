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

void matrixsub(matrix *A, matrix *B, matrix *ans, MPI_Status *status, MPI_Comm world, int cores, int *ran, int A1, int A2);

void matrixadd(matrix *A, matrix *B, matrix *ans, MPI_Status *status, MPI_Comm world, int cores, int *ran, int A1, int A2);

void matrixdotproduct(matrix *A, matrix *B, matrix *ans, MPI_Status *status, MPI_Comm world, int cores, int *ran, int A1, int A2, int B1, int B2);

#include <stdio.h>
void log_init();
#ifndef EXTERN
#define EXTERN extern
#endif
EXTERN FILE *_log;
EXTERN int _rank;
#undef EXTERN
#define LOG(fmt, ...) fprintf(_log, "[%s %d %d] " fmt "\n", __FILE__, __LINE__, _rank __VA_OPT__(, ) __VA_ARGS__)

#endif