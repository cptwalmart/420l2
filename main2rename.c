// mpiexec -n #ofrows+1 ./test2

#include <stdio.h>
#include <mpi.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include "mfunctions.h"
#include <time.h>

int IP(int *array, int *array2, int num_elem, int rank)
{
    printf("%d is array1 element, %d is array2 element, this is rank:%d\n", array[0], array2[0], rank);
    int sum = 0;
    for (int i = 0; i < num_elem; i++)
    {
        sum += array[i] + array2[i];
    }
    printf("%d thinks the sum is %d\n",rank, sum);
    return sum;
}

int main(int argc, char **argv)
{
	//MUST BE FORM ACols==BRows
	int ARows = 3, ACols = 3, BRows = 3, BCols = 3;
    struct matrix A;
    struct matrix B;
	struct matrix C;
	int SendAmount = ACols;
	int EndMatrixSize = ARows*BCols;
	
	int LoopCurrent;
	int Calc=0;
	
    MPI_Init(NULL, NULL);

    //init cores
    int cores;
    MPI_Comm world = MPI_COMM_WORLD;
    MPI_Comm_size(world, &cores);
    int rank;

	int LoopTotal = (ACols/(cores-1))+(ACols%(cores-1));

    //sets rank = to what core
    MPI_Comm_rank(world, &rank);
    MPI_Status status;
	
	initMatrix(&A, ARows, ACols);
	initMatrix(&B, BRows, BCols);
	initMatrix(&C, ARows, BCols);

	if(rank==0)
	{
		//initialize data fields
		srand(time(0));
		initMatrix(&A, ARows, ACols);
		printf("\n---------------------------\n");
		printMatrix(&A);
		printf("\n---------------------------\n");
		initMatrix(&B, BRows, BCols);
		printf("\n---------------------------\n");
		printMatrix(&B);
		printf("\n---------------------------\n");
		printf("\n---------------------------\n");
		printMatrix(&C);
		printf("\n---------------------------\n");
	}

	int n, i, j;
	if(rank == 0)
	{	
		for(n = 0; n <= LoopTotal; n++){
			LoopCurrent=(n%(cores-1)+1);
			
			
			MPI_Send(&A.arr[(n*ACols)],SendAmount, MPI_INT, LoopCurrent, 0, world);

		}

		for(n = 1; n < cores; n++){
			
			MPI_Send(&B.arr[0],BRows*BCols, MPI_INT, n, 1, world);
			
		}
	}

	if(rank !=0){
		
		MPI_Recv(&B.arr[0],BRows*BCols, MPI_INT, 0, 1, world, MPI_STATUS_IGNORE);

		for(n = 0; n <= LoopTotal; n++){
			
			LoopCurrent=(n%(cores-1)+1);
			MPI_Recv(&A.arr[(n*ACols)],SendAmount, MPI_INT, 0, 0, world, MPI_STATUS_IGNORE);
			for(j = 0; j < ACols; j++){
				Calc=Calc+(A.arr[n*j]*B.arr[j*BCols]);
			}
			
			C.arr[n]=Calc;
			MPI_Send(&C.arr[n],1, MPI_INT, 0, 2, world);
			Calc=0;
						
		}
	}

	if(rank == 0){
		for(n = 0; n < EndMatrixSize; n++){
			LoopCurrent=(n%(cores-1)+1);
			printf("%d\n",LoopCurrent);
//Problem is here
//all printf's were for testing
			printf("\n---------------------------\n");
			printMatrix(&C);
			printf("\n---------------------------\n");

			MPI_Recv(&C.arr[n],1, MPI_INT, LoopCurrent, 2, world, MPI_STATUS_IGNORE);	
			printf("%d\n",LoopCurrent);	
		}		

		printf("\n---------------------------\n");
		printMatrix(&C);
		printf("\n---------------------------\n");
	}

	

	free(A.arr);
	free(B.arr);
	free(C.arr);
    
	
    MPI_Finalize();
    return 0;
}