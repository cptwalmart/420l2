#include <stdio.h>
#include <mpi.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include "mfunctions.h"
#include <time.h>

int main(int argc, char **argv)
{
	log_init();
	//Create our matrix A1 = rows A2 = cols
	int A1 = 20, A2 = 20, B1 = 20, B2 = 20;
	struct matrix A;
	struct matrix B;
	struct matrix C;
	struct matrix D;
	struct matrix E;

	//initialize data fields
	srand(time(0));
	initMatrix(&A, A1, A2);
	printf("printing matrix A\n");
	printMatrix(&A);
	//printf("\n");
	initMatrix(&B, B1, B2);
	printf("\n printing matrix b\n");
	printMatrix(&B);


	initMatrix(&C, A1, A2);
	initMatrix(&D, A1, A2);
	initMatrix(&E, A1, B2);
	
	printf("entering matrix add\n");
	
	matrixadd(&A, &B, &C, A1, A2);

	printf("exiting matrix add\n");

//	matrixsub(&A, &B, &D, &status, &world, cores, &rank, A1, A2);
//	matrixdotproduct(&A, &B, &E, &status, &world, cores, &rank, A1, A2, B1, B2);

	free(A.arr);
	free(B.arr);
	free(C.arr);
	free(D.arr);
	free(E.arr);
	

	return 0;
}