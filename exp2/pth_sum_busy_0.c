/*
    pth_sum_busy_0.c
    Author: ChongKai
*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "timer.h"

/* Global variables */
#define N 1E8
int thread_count;
int flag;
double sum;

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
        "Thread [main]: Using [\033[31m%d\033[0m] threads, total time: [\033[31m%lf\033[0m] s\n",
        thread_count, ed_time-st_time
    );
    printf("Thread [main]: Pi: %.10lf\n", 4.0*sum);

    return 0;
}

void* Thread_work(void* rank){
    long my_rank = (long)rank;
    double factor;
    long long i;
    long long my_n = N/thread_count;
    long long my_first_i = my_n*my_rank;
    long long my_last_i = my_first_i+my_n;

    #ifdef DEBUG
    printf("Thread [%ld]: my_first_i: %lld, my_last_i: %lld\n", my_rank, my_first_i, my_last_i);
    #endif

    if (my_first_i % 2 == 0)
        factor = 1.0;
    else
        factor = -1.0;
    for (i = my_first_i; i < my_last_i; i++, factor = -factor){
        while (flag != my_rank); /* Spin */
        sum += factor/(2*i+1);
        flag = (flag+1) % thread_count;
    }

    return NULL;
} /* Thread_work */
