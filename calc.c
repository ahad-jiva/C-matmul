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

void synch(int par_id, int par_count, int* ready){
    int synchid = ready[par_count] + 1;
    ready[par_id] = synchid;
    int breakout = 0;
    while (1){
        breakout = 1;
        for (int i = 0; i < par_count; i++){
            if (ready[i] < synchid){
                breakout = 0;
                break;
            }
        }
    if (breakout == 1){
        ready[par_count] = synchid;
        break;
    }
    }
}

int main(int argc, char *argv[]){   // calc.c, process id, total process count

    // getting cli args
    int process_id = atoi(argv[1]);
    int process_count = atoi(argv[2]);

    // declare matrix pointers
    int *matrixA;
    int *matrixB;
    int *matrixM;

    // declare file descriptors
    int A;
    int B;
    int M;

    int matrix_dimension = 100;
    int matrix_size = matrix_dimension * matrix_dimension;

    // gather function intialization
    int ready_fd;
    int *ready;

    // setting up shared memory if we are the first process
    if (process_id == 0){

        // initial "ready" array for synchronization
        ready_fd = shm_open("ready", O_CREAT | O_RDWR, 0777);
        ftruncate(ready_fd, (process_count + 1) * 4);
        ready = (int *)mmap(NULL, (process_count + 1) * 4, PROT_READ | PROT_WRITE, MAP_SHARED, ready_fd, 0);
        for (int i = 0; i < process_count + 1; i++){
            ready[i] = 0;
        }

        // matrix A
        A = shm_open("matrixA", O_CREAT | O_RDWR, 0777);
        ftruncate(A, matrix_size*4);
        matrixA = (int *)mmap(NULL, matrix_size*4, PROT_READ | PROT_WRITE, MAP_SHARED, A, 0);

        // matrix B
        B = shm_open("matrixB", O_CREAT | O_RDWR, 0777);
        ftruncate(B, matrix_size*4);
        matrixB = (int *)mmap(NULL, matrix_size*4, PROT_READ | PROT_WRITE, MAP_SHARED, B, 0);

        // matrix M
        M = shm_open("matrixM", O_CREAT | O_RDWR, 0777);
        ftruncate(M, matrix_size*4);
        matrixM = (int *)mmap(NULL, matrix_size*4, PROT_READ | PROT_WRITE, MAP_SHARED, M, 0);
        
        // initialize matrix with random values
        //srand(time(NULL));  // random seed
        for (int i = 0; i < matrix_size; i++){
            matrixA[i] = rand() % 10;
            matrixB[i] = rand() % 10;
        }

        // zero out matrix M
        for (int i = 0; i < matrix_size; i++){
            matrixM[i] = 0;
        }

    } else {
        while(1){
            if (shm_open("matrixM", O_RDWR, 0777) != -1){
                break;
            }
        }

        // ready array for synchronization
        ready_fd = shm_open("ready", O_RDWR, 0777);
        ready = (int *)mmap(NULL, (process_count + 1) * 4, PROT_READ | PROT_WRITE, MAP_SHARED, ready_fd, 0);

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

    int offset = matrix_dimension % process_count;
    if (process_id + 1 == process_count){
        synch(process_id, process_count, ready);
    // matrix multiplication M = A * B
    for (int i = process_id * (matrix_dimension / process_count); i < (process_id + 1) * (matrix_dimension / process_count) + offset; i++){
        for (int j = 0; j < matrix_dimension; j++){
            for (int k = 0; k < matrix_dimension; k++){
                matrixM[i * matrix_dimension + j] += matrixA[i * matrix_dimension + k] * matrixB[k * matrix_dimension + j];
            }
        }
    }

    synch(process_id, process_count, ready);
    // matrix muliplication B = M * A
    for (int i = process_id * (matrix_dimension / process_count); i < (process_id + 1) * (matrix_dimension / process_count) + offset; i++){
        for (int j = 0; j < matrix_dimension; j++){
            for (int k = 0; k < matrix_dimension; k++){
                matrixB[i * matrix_dimension + j] += matrixM[i * matrix_dimension + k] * matrixA[k * matrix_dimension + j];
            }
        }
    }

    synch(process_id, process_count, ready);
    // matrix multiplication M = B * A
    for (int i = process_id * (matrix_dimension / process_count); i < (process_id + 1) * (matrix_dimension / process_count) + offset; i++){
        for (int j = 0; j < matrix_dimension; j++){
            for (int k = 0; k < matrix_dimension; k++){
                matrixM[i * matrix_dimension + j] += matrixB[i * matrix_dimension + k] * matrixA[k * matrix_dimension + j];
            }
        }
    }
    } else {

    synch(process_id, process_count, ready);
    // matrix multiplication M = A * B
    for (int i = process_id * (matrix_dimension / process_count); i < (process_id + 1) * (matrix_dimension / process_count); i++){
        for (int j = 0; j < matrix_dimension; j++){
            for (int k = 0; k < matrix_dimension; k++){
                matrixM[i * matrix_dimension + j] += matrixA[i * matrix_dimension + k] * matrixB[k * matrix_dimension + j];
            }
        }
    }

    synch(process_id, process_count, ready);
    // matrix muliplication B = M * A
    for (int i = process_id * (matrix_dimension / process_count); i < (process_id + 1) * (matrix_dimension / process_count); i++){
        for (int j = 0; j < matrix_dimension; j++){
            for (int k = 0; k < matrix_dimension; k++){
                matrixB[i * matrix_dimension + j] += matrixM[i * matrix_dimension + k] * matrixA[k * matrix_dimension + j];
            }
        }
    }

    synch(process_id, process_count, ready);
    // matrix multiplication M = B * A
    for (int i = process_id * (matrix_dimension / process_count); i < (process_id + 1) * (matrix_dimension / process_count); i++){
        for (int j = 0; j < matrix_dimension; j++){
            for (int k = 0; k < matrix_dimension; k++){
                matrixM[i * matrix_dimension + j] += matrixB[i * matrix_dimension + k] * matrixA[k * matrix_dimension + j];
            }
        }
    }
}

    synch(process_id, process_count, ready);
    // print all matrices (only for debugging)
    if (process_id == 0){
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

        // sum of diagonal elements of matrix M
        long double sum = 0;
        for (int i = 0; i < matrix_dimension; i++){
            sum += matrixM[i * matrix_dimension + i];
        }
        printf("∑diag(M) = %.Lf\n", sum);  
    } else {
        return 0;
    }

    

    // // det(M) calculation with LU decomposition, only for process 0 to do
    // if (process_id == 0){
    //     long double determinant = 1;
    //     int i;
    //     int j;
    //     int k;
    //     int pivot_point;
    //     int sign = 1;

    //     // allocate memory for LU matrix
    //     double *LU = (double *)malloc(matrix_size * sizeof(double));

    //     // copy matrix M to LU
    //     for (i = 0; i < matrix_size; i++){
    //         LU[i] = matrixM[i];
    //     }

    //     // LU decomposition
    //     for (k = 0; k < matrix_dimension; k++){

    //         // find pivot point
    //         pivot_point = k;
    //         double max = fabs(LU[k * matrix_dimension + k]);
    //         for (i = k + 1; i < matrix_dimension; i++){
    //             if (fabs(LU[i * matrix_dimension + k]) > max){
    //                 max = fabs(LU[i * matrix_dimension + k]);
    //                 pivot_point = i;
    //             }
    //         }

    //         // swap rows, if necessary
    //         if (pivot_point != k){
    //             for (j = 0; j < matrix_dimension; j++){
    //                 double temporary = LU[k * matrix_dimension + j];
    //                 LU[k * matrix_dimension + j] = LU[pivot_point * matrix_dimension + j];
    //                 LU[pivot_point * matrix_dimension + j] = temporary;
    //             }
    //             sign *= -1;
    //         }

    //         // gaussian elimination
    //         for (i = k + 1; i < matrix_dimension; i++){
    //             LU[i * matrix_dimension + k] /= LU[k * matrix_dimension + k];
    //             for (j = k + 1; j < matrix_dimension; j++){
    //                 LU[i * matrix_dimension + j] -= LU[i * matrix_dimension + k] * LU[k * matrix_dimension + j];
    //             }
    //         }
    //     }

    //     // calculate determinant from diagonal entries of U
    //     for (i = 0; i < matrix_dimension; i++){
    //         determinant *= LU[i * matrix_dimension + i];
    //     }
    //     determinant *= sign;

    //     printf("det(M): %.Lf\n", determinant);

    //     // make sure to free memory
    //     free(LU);       
    // }

    // clean up
    close(A);
    close(B);
    close(M);
    close(ready_fd);
    shm_unlink("ready");
    shm_unlink("matrixA");
    shm_unlink("matrixB");
    shm_unlink("matrixM");
    munmap(ready, (process_count + 1) * 4);
    munmap(matrixA, matrix_size);
    munmap(matrixB, matrix_size);
    munmap(matrixM, matrix_size);

    return 0;
}