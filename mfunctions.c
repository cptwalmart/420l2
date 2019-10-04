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

int addMatrix(int c, int d)
{
    int ans = 0;
    ans += c + d;
    return ans;
}

int subMatrix(int c, int d)
{
    int ans = 0;
    ans += c - d;
    return ans;
}

matrix matrixdotproduct(matrix *A, matrix *B, MPI_Status *status, MPI_Comm *world, int cores, int *ran)
{
    int rank = (*ran);
    //initialize the return matrix
    matrix retval;
    initMatrix(&retval, A->rows, B->cols);

    int indexcol = 0;
    int indexrow = 0;
    int sendArows = A->rows / cores;
    int colLength = A->cols;
    unsigned int sendNum = colLength * sendArows;
    int *C = malloc(sizeof(int) * (sendNum));
    int *D = malloc(sizeof(int) * (B->rows * B->cols));
    int n, i, j;
    if (rank == 0)
    {
        for (n = 0; n < cores; n++)
        {
            for (i = indexrow; i < (indexrow + sendArows); i++)
            {
                for (j = indexcol; j < colLength; j++)
                {
                    C[(sendArows * (i % sendArows)) + j] = A->arr[index_calc(A, i, j)];
                }
            }
            indexrow += sendArows;
            MPI_Send(C, (sendNum), MPI_INT, n, 0, (*world));
        }
        indexcol = 0;
        indexrow = 0;
        for (n = 0; n < cores; n++)
        {
            for (i = 0; i < B->rows; i++)
            {
                for (j = 0; j < B->cols; j++)
                {
                    D[(B->cols * i) + j] = B->arr[index_calc(B, i, j)];
                }
            }
            MPI_Send(D, B->rows * B->cols, MPI_INT, n, 0, (*world));
        }
    }
    MPI_Recv(C, (sendNum), MPI_INT, 0, 0, (*world), status);
    MPI_Recv(D, B->rows * B->cols, MPI_INT, 0, 0, (*world), status);
    int *E;
    int pos = 0;
    //do for each row
    for (n = 0; n < sendArows; n++)
    {
        //do for each number in a row
        for (i = 0; i < A->rows; i++)
        {
            //for each column
            for (j = 0; j < B->cols; j++)
            {
                retval.arr[index_calc(&retval, n, pos)] += C[(colLength * n) + i] * D[(B->cols * j)];
                pos++;
            }
            pos = 0;
        }
    }
    E = retval.arr;
    MPI_Send(E, A->rows * B->cols, MPI_INT, 0, 0, (*world));
    free(E);
    free(C);
    if (rank != 0)
    {
        free(retval.arr);
    }
    //handle cases in which the problem is not perfectly divisable by the number of cores
    int offSet = 0;
    int stopPoint = 0;
    if (rank == 0 && (A->rows % cores) > 0)
    {
        offSet = sendArows * cores;
        stopPoint = (A->rows % cores);

        //do for each row
        for (n = 0; n < stopPoint; n++)
        {
            //do for each number in a row
            for (i = 0; i < A->rows; i++)
            {
                //for each column
                for (j = 0; j < B->cols; j++)
                {
                    retval.arr[index_calc(&retval, n + offSet, pos)] += A->arr[(colLength * (n + offSet)) + i] * B->arr[(B->cols * j)];
                    pos++;
                }
                pos = 0;
            }
        }
    }
    if (rank == 0)
    {
        for (n = 0; n < cores; n++)
        {
            MPI_Recv(D, A->rows * B->cols, MPI_INT, n, 0, (*world), status);
            for (i = 0; i < retval.rows; i++)
            {
                for (j = 0; j < retval.cols; j++)
                {
                    retval.arr[index_calc(&retval, i, j)] += D[(retval.rows * i) + j];
                }
            }
        }
    }
    free(D);
    return retval;
}

matrix matrixadd(matrix *A, matrix *B, MPI_Status *status, MPI_Comm *world, int cores, int *ran)
{
    int rank = (*ran);
    int indexcol = 0;
    int indexrow = 0;
    int sendArows = A->rows / cores;
    int colLength = A->cols;
    unsigned int sendNum = colLength * sendArows;
    int *C = malloc(sizeof(int) * (sendNum));
    int *D = malloc(sizeof(int) * (sendNum));
    int n, i, j;
    if (rank == 0)
    {
        for (n = 0; n < cores; n++)
        {
            for (i = indexrow; i < (indexrow + sendArows); i++)
            {
                for (j = indexcol; j < colLength; j++)
                {
                    C[(sendArows * (i % sendArows)) + j] = A->arr[index_calc(A, i, j)];
                }
            }
            indexrow += sendArows;
            MPI_Send(C, (sendNum), MPI_INT, n, 0, (*world));
        }
        indexcol = 0;
        indexrow = 0;
        for (n = 0; n < cores; n++)
        {
            for (i = indexrow; i < (indexrow + sendArows); i++)
            {
                for (j = indexcol; j < colLength; j++)
                {
                    C[(sendArows * (i % sendArows)) + j] = B->arr[index_calc(B, i, j)];
                }
            }
            indexrow += sendArows;
            MPI_Send(C, (sendNum), MPI_INT, n, 0, (*world));
        }
    }
    MPI_Recv(C, (sendNum), MPI_INT, 0, 0, (*world), status);
    MPI_Recv(D, (sendNum), MPI_INT, 0, 0, (*world), status);
    int *E = malloc(sizeof(int) * (sendNum));
    for (i = 0; i < sendArows; i++)
    {
        for (j = 0; j < colLength; j++)
        {
            E[(colLength * i) + j] = addMatrix((C[(colLength * i) + j]), (D[(colLength * i) + j]));
        }
    }
    MPI_Send(E, (sendNum), MPI_INT, 0, 0, (*world));
    free(C);

    //handle cases in which the problem is not perfectly divisable by the number of cores
    int remainderFlag = 0;
    int offSet = 0;
    int stopPoint = 0;
    if (rank == 0 && (A->rows % cores) > 0)
    {
        remainderFlag = 1;
        offSet = sendArows * cores;
        stopPoint = (A->rows % cores);
        int x = 0;
        for (i = 0; i < stopPoint; i++)
        {
            for (j = 0; j < colLength; j++)
            {
                E[(colLength * i) + j] = addMatrix((A->arr[index_calc(A, i + offSet, j)]), (B->arr[index_calc(B, i + offSet, j)]));
            }
        }
    }

    struct matrix F;
    initMatrix(&F, A->rows, A->cols);

    if (rank == 0)
    {
        indexcol = 0;
        indexrow = 0;
        for (n = 0; n < cores; n++)
        {
            MPI_Recv(D, (sendNum), MPI_INT, n, 0, (*world), status);
            for (i = indexrow; i < (indexrow + sendArows); i++)
            {
                for (j = indexcol; j < colLength; j++)
                {
                    F.arr[index_calc(&F, i, j)] = D[(sendArows * (i % sendArows)) + j];
                }
            }
            indexrow += sendArows;
        }
        if (remainderFlag)
        {
            for (n = 0; n < stopPoint; n++)
            {
                for (j = 0; j < colLength; j++)
                {
                    F.arr[index_calc(&F, n + offSet, j)] = E[(colLength * n) + j];
                }
            }
        }
    }
    free(D);
    free(E);
    return F;
}

matrix matrixsub(matrix *A, matrix *B, MPI_Status *status, MPI_Comm *world, int cores, int *ran)
{
    int rank = (*ran);
    int indexcol = 0;
    int indexrow = 0;
    int sendArows = A->rows / cores;
    int colLength = A->cols;
    unsigned int sendNum = colLength * sendArows;
    int *C = malloc(sizeof(int) * (sendNum));
    int *D = malloc(sizeof(int) * (sendNum));
    int n, i, j;
    if (rank == 0)
    {
        for (n = 0; n < cores; n++)
        {
            for (i = indexrow; i < (indexrow + sendArows); i++)
            {
                for (j = indexcol; j < colLength; j++)
                {
                    C[(sendArows * (i % sendArows)) + j] = A->arr[index_calc(A, i, j)];
                }
            }
            indexrow += sendArows;
            MPI_Send(C, (sendNum), MPI_INT, n, 0, (*world));
        }
        indexcol = 0;
        indexrow = 0;
        for (n = 0; n < cores; n++)
        {
            for (i = indexrow; i < (indexrow + sendArows); i++)
            {
                for (j = indexcol; j < colLength; j++)
                {
                    C[(sendArows * (i % sendArows)) + j] = B->arr[index_calc(B, i, j)];
                }
            }
            indexrow += sendArows;
            MPI_Send(C, (sendNum), MPI_INT, n, 0, (*world));
        }
    }
    MPI_Recv(C, (sendNum), MPI_INT, 0, 0, (*world), status);
    MPI_Recv(D, (sendNum), MPI_INT, 0, 0, (*world), status);
    int *E = malloc(sizeof(int) * (sendNum));
    for (i = 0; i < sendArows; i++)
    {
        for (j = 0; j < colLength; j++)
        {
            E[(colLength * i) + j] = subMatrix((C[(colLength * i) + j]), (D[(colLength * i) + j]));
        }
    }
    MPI_Send(E, (sendNum), MPI_INT, 0, 0, (*world));
    free(C);

    //handle cases in which the problem is not perfectly divisable by the number of cores
    int remainderFlag = 0;
    int offSet = 0;
    int stopPoint = 0;
    if (rank == 0 && (A->rows % cores) > 0)
    {
        remainderFlag = 1;
        offSet = sendArows * cores;
        stopPoint = (A->rows % cores);
        int x = 0;
        for (i = 0; i < stopPoint; i++)
        {
            for (j = 0; j < colLength; j++)
            {
                E[(colLength * i) + j] = subMatrix((A->arr[index_calc(A, i + offSet, j)]), (B->arr[index_calc(B, i + offSet, j)]));
            }
        }
    }

    struct matrix F;
    initMatrix(&F, A->rows, A->cols);

    if (rank == 0)
    {
        indexcol = 0;
        indexrow = 0;
        for (n = 0; n < cores; n++)
        {
            MPI_Recv(D, (sendNum), MPI_INT, n, 0, (*world), status);
            for (i = indexrow; i < (indexrow + sendArows); i++)
            {
                for (j = indexcol; j < colLength; j++)
                {
                    F.arr[index_calc(&F, i, j)] = D[(sendArows * (i % sendArows)) + j];
                }
            }
            indexrow += sendArows;
        }
        if (remainderFlag)
        {
            for (n = 0; n < stopPoint; n++)
            {
                for (j = 0; j < colLength; j++)
                {
                    F.arr[index_calc(&F, n + offSet, j)] = E[(colLength * n) + j];
                }
            }
        }
    }
    free(D);
    free(E);
    return F;
}