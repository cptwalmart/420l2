//set up a C macro to calculate the location of an element (i,j)
#include <stdio.h>
#include <mpi.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "mfunctions.h"

#define EXTERN
#include "mfunctions.h"
#undef EXTERN

#define INDEX(n, m, i, j) m *i + j
#define ACCESS(A, i, j) A->arr[INDEX(A->rows, A->cols, i, j)]

FILE * _log;
int _rank;

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

void log_init()
{
    _rank = -1;
    char buf[256] = {0};
    sprintf(buf, "mpi_%d.log", getpid());
    _log = fopen(buf, "w");
    if (!_log)
    {
        fprintf(stderr, "Couldn't open '%s': %s\n", buf, strerror(errno));
        exit(1);
    }
}

void log_set_rank(int rank)
{
    _rank = rank;
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

void matrixdotproduct(matrix *A, matrix *B, matrix *ans, MPI_Status *status, MPI_Comm *world, int cores, int *ran, int A1, int A2, int B1, int B2)
{
    int rank = (*ran);
    int indexcol = 0;
    int indexrow = 0;
    int sendArows = A1 / cores;
    int colLength = A2;
    unsigned int sendNum = colLength * sendArows;
    int *C = malloc(sizeof(int) * (sendNum));
    int *D = malloc(sizeof(int) * (B1 * B2));
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
            for (i = 0; i < B1; i++)
            {
                for (j = 0; j < B2; j++)
                {
                    D[(B1 * i) + j] = B->arr[index_calc(B, i, j)];
                }
            }
            MPI_Send(D, B1 * B2, MPI_INT, n, 0, (*world));
        }
    }
    MPI_Recv(C, (sendNum), MPI_INT, 0, 0, (*world), status);
    MPI_Recv(D, B1 * B2, MPI_INT, 0, 0, (*world), status);
    int *E;
    int pos = 0;
    //do for each row
    for (n = 0; n < sendArows; n++)
    {
        //do for each number in a row
        for (i = 0; i < A1; i++)
        {
            //for each column
            for (j = 0; j < B2; j++)
            {
                ans->arr[index_calc(ans, n, pos)] += C[(colLength * n) + i] * D[(B2 * j)];
                pos++;
            }
            pos = 0;
        }
    }
    E = ans->arr;
    MPI_Send(E, A1 * B2, MPI_INT, 0, 0, (*world));
    free(E);
    free(C);
    //handle cases in which the problem is not perfectly divisable by the number of cores
    int offSet = 0;
    int stopPoint = 0;
    if (rank == 0 && (A1 % cores) > 0)
    {
        offSet = sendArows * cores;
        stopPoint = (A1 % cores);

        //do for each row
        for (n = 0; n < stopPoint; n++)
        {
            //do for each number in a row
            for (i = 0; i < A1; i++)
            {
                //for each column
                for (j = 0; j < B2; j++)
                {
                    ans->arr[index_calc(ans, n + offSet, pos)] += A->arr[(colLength * (n + offSet)) + i] * B->arr[(B2 * j)];
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
            MPI_Recv(D, A1 * B2, MPI_INT, n, 0, (*world), status);
            for (i = 0; i < A1; i++)
            {
                for (j = 0; j < B2; j++)
                {
                    ans->arr[index_calc(ans, i, j)] += D[(A1 * i) + j];
                }
            }
        }
    }
    free(D);
}

void matrixadd(matrix *A, matrix *B, matrix *ans, int A1, int A2)
{
	MPI_Init(NULL, NULL);
	
	//init cores
	int cores;
	MPI_Comm world = MPI_COMM_WORLD;
	MPI_Comm_size(world, &cores);
	int rank;
	
	//sets rank = to what core
	MPI_Comm_rank(world, &rank);
	log_set_rank(rank);
	MPI_Status stat;
	MPI_Status * status = &stat;
	
	int indexcol = 0;
	int indexrow = 0;
	int sendArows = A1 / cores;
	int colLength = A2;
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
			MPI_Send(C, (sendNum), MPI_INT, n, 0, world);
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
			MPI_Send(C, (sendNum), MPI_INT, n, 0, world);
		}
	}
	MPI_Recv(C, (sendNum), MPI_INT, 0, 0, world, status);
	MPI_Recv(D, (sendNum), MPI_INT, 0, 0, world, status);
	int *E = malloc(sizeof(int) * (sendNum));
	for (i = 0; i < sendArows; i++)
	{
		for (j = 0; j < colLength; j++)
		{
			E[(colLength * i) + j] = addMatrix((C[(colLength * i) + j]), (D[(colLength * i) + j]));
		}
	}
	MPI_Send(E, (sendNum), MPI_INT, 0, 0, world);
	free(C);
	
	//handle cases in which the problem is not perfectly divisable by the number of cores
	int remainderFlag = 0;
	int offSet = 0;
	int stopPoint = 0;
	if(rank == 0 && (A1 % cores) > 0)
	{
		remainderFlag = 1;
		offSet = sendArows * cores;
		stopPoint = (A1 % cores);
		int x = 0;
		for (i = 0; i < stopPoint; i++)
		{
			for (j = 0; j < colLength; j++)
			{
				E[(colLength * i) + j] = addMatrix((A->arr[index_calc(A, i + offSet, j)]), (B->arr[index_calc(B, i + offSet, j)]));
			}
		}
	}
	
	if (rank == 0)
	{
		indexcol = 0;
		indexrow = 0;
		for (n = 0; n < cores; n++)
		{
			MPI_Recv(D, (sendNum), MPI_INT, n, 0, world, status);
			for (i = indexrow; i < (indexrow + sendArows); i++)
			{
				for (j = indexcol; j < colLength; j++)
				{
					ans->arr[index_calc(ans, i, j)] = D[(sendArows * (i % sendArows)) + j];
				}
			}
			indexrow += sendArows;
		}
		if(remainderFlag)
		{
			for(n = 0; n < stopPoint; n++)
			{
				for(j = 0; j< colLength; j++)
				{
					ans->arr[index_calc(ans, n + offSet, j)] = E[(colLength * n) + j];
				}
			}
		}
	}
	free(D);
	free(E);
	MPI_Finalize();
}

void matrixsub(matrix *A, matrix *B, matrix *ans, MPI_Status *status, MPI_Comm *world, int cores, int *ran, int A1, int A2)
{
    int rank = (*ran);
    int indexcol = 0;
    int indexrow = 0;
    int sendArows = A1 / cores;
    int colLength = A2;
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
    if (rank == 0 && (A1 % cores) > 0)
    {
        remainderFlag = 1;
        offSet = sendArows * cores;
        stopPoint = (A2 % cores);
        int x = 0;
        for (i = 0; i < stopPoint; i++)
        {
            for (j = 0; j < colLength; j++)
            {
                E[(colLength * i) + j] = subMatrix((A->arr[index_calc(A, i + offSet, j)]), (B->arr[index_calc(B, i + offSet, j)]));
            }
        }
    }

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
                    ans->arr[index_calc(ans, i, j)] = D[(sendArows * (i % sendArows)) + j];
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
                    ans->arr[index_calc(ans, n + offSet, j)] = E[(colLength * n) + j];
                }
            }
        }
    }
    free(D);
    free(E);
}