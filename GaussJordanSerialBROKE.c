#include <stdio.h>
#include <mpi.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include "mfunctions.h"
#include <time.h>

int main(int argc, char **argv)
{
	//Create our matrix A1 = rows A2 = cols
	int A1 = 2, A2 = 2, B1 = 2, B2 = 1;
	struct matrix A;
	struct matrix B;
	struct matrix C;
	double* pivot =  calloc(A2+1, sizeof(double));

	MPI_Init(NULL, NULL);

	//init cores
	int cores;
	MPI_Comm world = MPI_COMM_WORLD;
	MPI_Comm_size(world, &cores);
	int rank;

	//sets rank = to what core
	MPI_Comm_rank(world, &rank);
	MPI_Status status;

	int Block = A1/cores;
	int leftover = A1%cores;

	if (rank == 0)
	{
		//initialize data fields
		srand(time(0));
		initMatrix(&A, A1, A2);
		printf("printing matrix A\n");
		printMatrix(&A);
		printf("\n");
		initMatrix(&B, B1, B2);
		printf("\n printing matrix B\n");
		printMatrix(&B);
		initMatrix(&C, A1, (A2+1));


		int i, j, l;
		int k = 0;
		int Aindex=0;
		int Bindex=0;
		for(i = 0; i <(A1*(A2+1)); i++){
			if(((i+1)%(A2+1))==0){
				C.arr[i]=B.arr[Bindex];
				Bindex++;
				Aindex++;
			}
			else{
				C.arr[i]=A.arr[i-Aindex];
			}
		}

		printf("\n printing matrix C\n");
		printMatrix(&C);

		printf("\n");
	

	int Divider;
	for(i = 0; i < A1; i++){
		Divider = ((A2+1)*i)+i;
		for(j = 0; j < A2+1; j++){
			pivot[j] = C.arr[(A1*i)+j];
		}
		for(j = (((A2+1)*i)+i); j < (((A2+1)*i)+(A2+1)); j++){
			C.arr[j]=C.arr[j]/C.arr[Divider];
			k++;
		}
		k = 0;
		for(j = 0; j < A1; j++){
			if(j!=i){
				for(l = ((A2+1)*j); l < (((A2+1)*j)+(A2+1)); l++){
					C.arr[l]=C.arr[l]+(pivot[k]*(-1*(C.arr[l])));
					k++;
				}
				k = 0;
			}
		}
	}

		printf("\n printing matrix C\n");
		printMatrix(&C);

		printf("\n");
	}
	    
	MPI_Finalize();
	return 0;
}