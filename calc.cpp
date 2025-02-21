#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <time.h>

int main(){

    // getting cli args
    int process_count = 0;
    int process_id = 0;

    // declare matrix pointers
    int *matrixA;
    int *matrixB;
    int *matrixM;

    // declare file descriptors
    int A;
    int B;
    int M;

    int matrix_dimension = 3;
    int matrix_size = matrix_dimension * matrix_dimension;

    // setting up shared memory if we are the first process
    if (process_id == 0){

        // matrix A
        A = shm_open("matrixA", O_CREAT | O_RDWR, 0777);
        ftruncate(A, matrix_size);
        matrixA = (int *)mmap(NULL, matrix_size, PROT_READ | PROT_WRITE, MAP_SHARED, A, 0);

        // matrix B
        B = shm_open("matrixB", O_CREAT | O_RDWR, 0777);
        ftruncate(B, matrix_size);
        matrixB = (int *)mmap(NULL, matrix_size, PROT_READ | PROT_WRITE, MAP_SHARED, B, 0);

        // matrix C
        M = shm_open("matrixM", O_CREAT | O_RDWR, 0777);
        ftruncate(M, matrix_size);
        matrixM = (int *)mmap(NULL, matrix_size, PROT_READ | PROT_WRITE, MAP_SHARED, M, 0);
    } else {
        // matrix A
        A = shm_open("matrixA", O_RDWR, 0777);
        matrixA = (int *)mmap(NULL, matrix_size, PROT_READ | PROT_WRITE, MAP_SHARED, A, 0);

        // matrix B
        B = shm_open("matrixB", O_RDWR, 0777);
        matrixB = (int *)mmap(NULL, matrix_size, PROT_READ | PROT_WRITE, MAP_SHARED, B, 0);

        // matrix C
        M = shm_open("matrixM", O_RDWR, 0777);
        matrixM = (int *)mmap(NULL, matrix_size, PROT_READ | PROT_WRITE, MAP_SHARED, M, 0);
    }

    // initialize matrix with random values
    srand(time(NULL));  // random seed
    for (int i = 0; i < matrix_size; i++){
        matrixA[i] = rand() % 10;
        matrixB[i] = rand() % 10;
    }

    // zero out matrix M
    for (int i = 0; i < matrix_size; i++){
        matrixM[i] = 0;
    }

    // matrix multiplication
    for (int i = 0; i < matrix_dimension; i++){
        for (int j = 0; j < matrix_dimension; j++){
            for (int k = 0; k < matrix_dimension; k++){
                matrixM[i * matrix_dimension + j] += matrixA[i * matrix_dimension + k] * matrixB[k * matrix_dimension + j];
            }
        }
    }
    
    // print all matrices
    printf("Matrix A:\n");
    for (int i = 0; i < matrix_dimension; i++){
        for (int j = 0; j < matrix_dimension; j++){
            printf("%d ", matrixA[i * matrix_dimension + j]);
        }
        printf("\n");
    }
    printf("Matrix B:\n");
    for (int i = 0; i < matrix_dimension; i++){
        for (int j = 0; j < matrix_dimension; j++){
            printf("%d ", matrixB[i * matrix_dimension + j]);
        }
        printf("\n");
    }
    printf("Matrix M:\n");
    for (int i = 0; i < matrix_dimension; i++){
        for (int j = 0; j < matrix_dimension; j++){
            printf("%d ", matrixM[i * matrix_dimension + j]);
        }
        printf("\n");
    }

    // clean up
    close(A);
    close(B);
    close(M);
    shm_unlink("matrixA");
    shm_unlink("matrixB");
    shm_unlink("matrixM");
    munmap(matrixA, matrix_size);
    munmap(matrixB, matrix_size);
    munmap(matrixM, matrix_size);

    return 0;
}