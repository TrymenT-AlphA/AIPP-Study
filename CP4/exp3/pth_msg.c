/*
    pth_msg.c
    Author: ChongKai
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include "queue.h"

/* Global variables */
struct Queue que;
int thread_count;
sem_t mutex, msg_num;

/* Thread function */
void* Thread_work(void* rank);

/* Main routine */
int main(int argc, char* argv[]){
    printf("Using [8] threads default\n");
    thread_count = 8;

    /* Initialize */
    init_que(&que);
    sem_init(&mutex, 0, 1);
    sem_init(&msg_num, 0, 0);

    /* Create threads */
    long thread;
    pthread_t* thread_handles = malloc(thread_count*sizeof(pthread_t));

    for (thread = 0; thread < thread_count; thread++)
        pthread_create(&thread_handles[thread], NULL, Thread_work, (void*)thread);

    for (thread = 0; thread < thread_count; thread++)
        pthread_join(thread_handles[thread], NULL);

    free(thread_handles);

    /* Destroy */
    sem_destroy(&msg_num);
    sem_destroy(&mutex);
    free_que(&que);

    return 0;
}

void* Thread_work(void* rank){
    long my_rank = (long)rank;
    struct Message message;
    unsigned int seed = my_rank;

    while(1){ /* This program keeps running */
        if (my_rank < 4){ /* Thread [0,1,2,3]: consumer */
            sem_wait(&msg_num); /* wait for a message */
            peek_que(&que, &message);
            if (message.dst_thread != my_rank)
                sem_post(&msg_num);
            else{
                printf("Thread [%ld]: received a message: \033[31m%s\033[0m\n", my_rank, message.msg);
                sem_wait(&mutex); /* enter critical zone */
                pop_que(&que, NULL);
                sem_post(&mutex); /* leave critical zone */
            }
        }
        else{ /* Thread [4,5,6,7]: producer */
            if (rand_r(&seed)%10 != 9)
                sleep(1);
            else{ /* 1/10 send a message */
                message.dst_thread = rand_r(&seed)%(thread_count/2);
                sprintf(message.msg, "Hello! thread [%ld] , i'm thread [%ld]", message.dst_thread, my_rank);
                sem_wait(&mutex); /* enter critical zone */
                push_que(&que, message);
                sem_post(&mutex); /* leave critical zone */
                printf("Thread [%ld]: sended message to thread [%ld]\n", my_rank, message.dst_thread);
                sem_post(&msg_num); /* produce a message */
            }
        }
    }

    return NULL;
} /* Thread_work */
