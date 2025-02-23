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

int main(int argc, char *argv[]){   // mpi.c, called program, total process count
    
    // getting cli args
    int process_count = atoi(argv[2]);       // staying as a char for now

    // setting up string to call child program
    char exe[100] = {'\0'};
    strcpy(exe, "./");
    strcat(exe, argv[1]);

    // executing multiple processes
    for (int i = 0; i < process_count; i++){
        if (fork() == 0){   // child process
            // child process
            char process_id[10];
            char process_count_char[10];
            sprintf(process_id, "%d", i);
            sprintf(process_count_char, "%d", process_count);
            char *args[] = {"./matmul", process_id, process_count_char, NULL};
            execv(exe, args);
        }
    }
    while(wait(NULL) > 0);  // wait for all children to terminate
    return 0;
}