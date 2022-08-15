/* 
    pth_hello.c
    Author: CongKai
*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

/* Shared variables */
int thread_count;

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

    /* Create threads */
    long thread;
    pthread_t* thread_handles;

    thread_handles = malloc(thread_count*sizeof(pthread_t));

    for (thread = 0; thread < thread_count; thread++)
        pthread_create(&thread_handles[thread], NULL, Thread_work, (void*)thread);

    printf("Thread [main]: Hello!\n");

    for (thread = 0; thread < thread_count; thread++)
        pthread_join(thread_handles[thread], NULL);

    free(thread_handles);

    return 0;
} /* main */

void* Thread_work(void* rank){
    long my_rank = (long)rank;

    printf("Thread [%ld]: Hello!\n", my_rank);

    #ifdef DEBUG
    int my_count = 10000;
    while(my_count--)
        if (my_rank == 0)
            printf("Thread [%ld] %s\n", my_rank, "0000000000000000000000000000000000000000");
        else
            printf("Thread [%ld] %s\n", my_rank, "1111111111111111111111111111111111111111");
    #endif

    return NULL;
} /* Thread_work */
