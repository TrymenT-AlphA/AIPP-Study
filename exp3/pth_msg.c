#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include "queue.h"

void* Pth_msg(void* rank);

struct Queue que;
int thread_count;
sem_t mutex, msg_num;

int main(int argc, char* argv[]){
    long thread;
    pthread_t* thread_handles;

    init_que(&que);
    thread_count = 8;
    sem_init(&mutex, 0, 1);
    sem_init(&msg_num, 0, 0);

    thread_handles = malloc(thread_count*sizeof(pthread_t));

    for (thread = 0; thread < thread_count; thread++)
        pthread_create(&thread_handles[thread], NULL, Pth_msg, (void*)thread);

    for (thread = 0; thread < thread_count; thread++)
        pthread_join(thread_handles[thread], NULL);

    free(thread_handles);

    sem_destroy(&msg_num);
    sem_destroy(&mutex);

    return 0;
}

void* Pth_msg(void* rank){
    long my_rank = (long)rank;
    struct Message message;
    unsigned int seed = my_rank;

    while(1){ /* This program keep running */
        if (my_rank < 4){ /* thread 0,1,2,3 consumer */
            sem_wait(&msg_num); /* wait for a message */
            peek_que(&que, &message);
            if (message.dst_thread != my_rank)
                sem_post(&msg_num);
            else{
                printf("Thread [%ld]: received a message: %s\n", my_rank, message.msg);
                sem_wait(&mutex); /* enter critical zone */
                pop_que(&que, NULL);
                sem_post(&mutex); /* leave critical zone */
            }
        }
        else{ /* thread 4,5,6,7 producer */
            if (rand_r(&seed)%10 != 9)
                sleep(1);
            else{ /* 1/100 send a message */ /* 1+1=9 */
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
}
