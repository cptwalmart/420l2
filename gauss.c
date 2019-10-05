#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <time.h>
#include "mfunctions.h"

int main(void) {
  log_init();

  MPI_Init(NULL, NULL);

  int cores;
  MPI_Comm world = MPI_COMM_WORLD;
  MPI_Comm_size(world, &cores);

  int rank;
  MPI_Comm_rank(world, &rank);
  log_set_rank(rank);

  // Case 1 - matrix x vector
  
  int A_cols = 3 * 4;
  int A_rows = 3 * 4;

  struct matrix A, A_orig;
  double *ROOT_b;

  A.rows = A_rows;		/* these need to be available on all
				 * threads for gauss_mv */
  A.cols = A_cols;
  
  if(rank == 0) {
    srand(time(NULL));
    
    initMatrix(&A, A_rows, A_cols);
    A_orig = A;
    A_orig.arr = calloc(A_rows * A_cols, sizeof(double));
    memcpy(A_orig.arr, A.arr, A_rows * A_cols * sizeof(double));
    
    printf("Inputs:\n");
    printf("A:\n");
    printMatrix(&A);
    ROOT_b = calloc(A_rows, sizeof(double));
    int i;
    printf("b:\n");
    for(i = 0; i < A_rows; i++) {
      ROOT_b[i] = rand() % 10 + 1;
      printf("% 10lf ", ROOT_b[i]);
    }
    printf("\n");
  }

  gauss_mv(world, rank, cores, A, ROOT_b);
  
  if(rank == 0) {
    printf("Outputs:\n");
    printf("A:\n");
    printMatrix(&A);
    int i;
    printf("b:\n");
    for(i = 0; i < A_rows; i++) {
      printf("% 10lf ", ROOT_b[i]);
    }
    printf("\n");
    printf("Check:\n");
    int j;
    for(i = 0; i < A_rows; i++) {
      double r = 0;
      for(j = 0; j < A_cols; j++) {
	r += A_orig.arr[i * A_cols + j] * ROOT_b[j];
      }
      printf("% 10lf ", r);
    }
    printf("\n");
    free(A.arr);
    free(A_orig.arr);
    free(ROOT_b);
  }
  MPI_Finalize();
}