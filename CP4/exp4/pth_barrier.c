/*
    pth_barrier.c
    Author: ChongKai
*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

/* Global variables */
int thread_count;
int counter;
pthread_mutex_t mutex;
pthread_cond_t cond;

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
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    /* Create threads */
    long thread;
    pthread_t* thread_handles = malloc(thread_count*sizeof(pthread_t));

    for (thread = 0; thread < thread_count; thread++)
        pthread_create(&thread_handles[thread], NULL, Thread_work, (void*)thread);

    for (thread = 0; thread < thread_count; thread++)
        pthread_join(thread_handles[thread], NULL);

    free(thread_handles);

    /* Destroy */
    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&mutex);

    return 0;
}

void* Thread_work(void* rank){
    long my_rank = (long)rank;

    pthread_mutex_lock(&mutex);
    counter++;
    if (counter == thread_count){
        counter = 0;
        printf("Thread [%ld]: broadcast\n", my_rank);
        pthread_cond_broadcast(&cond);
    }
    else{
        printf("Thread [%ld]: waitting\n", my_rank);
        while(pthread_cond_wait(&cond, &mutex) != 0);
    }
    printf("Thread [%ld]: pass\n", my_rank);
    pthread_mutex_unlock(&mutex);

    return NULL;
} /* Thread_work */
