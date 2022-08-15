/* 
    pth_mat_vect.c
    Author: CongKai
*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "timer.h"

/* Global variables */
#define M 8
#define N 800000
int thread_count;
double A[M][N];
double y[M];
double x[N];

/* Thread function */
void* Thread_work(void* rank);

/* Main routine */
int main(int argc, char* argv[]){
    if (argc <= 1){ /* Error input */
        printf("Usage: %s <thread_count>\n", argv[0]);
        return 0;
    }
    /* Get number of thread_count */
    thread_count = strtol(argv[1], NULL, 10);
    
    /* Initialize */

    double st_time, ed_time;
    GET_TIME(st_time);

    /* Create threads */
    long thread;
    pthread_t* thread_handles = malloc(thread_count*sizeof(pthread_t));

    for (thread = 0; thread < thread_count; thread++)
        pthread_create(&thread_handles[thread], NULL, Thread_work, (void*)thread);

    for (thread = 0; thread < thread_count; thread++)
        pthread_join(thread_handles[thread], NULL);

    free(thread_handles);

    GET_TIME(ed_time);

    /* Log */
    printf(
        "Thread [main]: Using [\033[31m%d\033[0m] threads, total time: [\033[31m%lf\033[0m] s\n", thread_count, ed_time-st_time
    );

    return 0;
} /* main */

void* Thread_work(void* rank){
    long my_rank = (long)rank;
    int i, j;
    int local_m = (M+thread_count-1)/thread_count;
    int my_first_row = my_rank*local_m;
    int my_last_row = my_first_row+local_m;
    if (my_last_row > M)
        my_last_row = M;

    #ifdef DEBUG
    printf("Thread [%ld]: my_first_row: %d, my_last_row: %d\n", my_rank, my_first_row, my_last_row);
    #endif

    for (i = my_first_row; i < my_last_row; i++){
        y[i] = 0.0;
        for (j = 0; j < N; j++)
            y[i] += A[i][j]*x[j];
    }

    return NULL;
} /* Thread_work */
