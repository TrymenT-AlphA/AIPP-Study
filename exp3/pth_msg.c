#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

struct Message{
    long dst_thread;
    char msg[256];
};

struct Queue{
    unsigned long fron, rear;
    unsigned long msize;
    struct Message buffer[256];
};

int init_que(struct Queue* _que);/* 0 success */
int full_que(struct Queue* _que);/* 1 full, else 0 */
int empty_que(struct Queue* _que);/* 1 empty, else 0 */
int push_que(struct Queue* _que, struct Message msg);/* 0 success */
int pop_que(struct Queue* _que, struct Message* msg);/* 0 success */
int peek_que(struct Queue* _que, struct Message* msg);/* 0 success */

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

int init_que(struct Queue* _que){/* 0 success */
    _que->fron = _que->rear = 0;
    _que->msize = 256;
    return 0;
}

int full_que(struct Queue* _que){ /* 1 full, else 0 */
    return (_que->rear+1)%_que->msize == _que->fron;
}

int empty_que(struct Queue* _que){ /* 1 empty, else 0 */
    return _que->fron == _que->rear;
}

int push_que(struct Queue* _que, struct Message msg){/* 0 success */
    if (full_que(_que))
        return -1;
    _que->buffer[_que->rear] = msg;
    _que->rear = (_que->rear+1)%_que->msize;
    return 0;
}

int pop_que(struct Queue* _que, struct Message* msg){/* 0 success */
    if (empty_que(_que))
        return -1;
    if (msg != NULL)
        *msg = _que->buffer[_que->fron];
    _que->fron = (_que->fron+1)%_que->msize;
    return 0;
}

int peek_que(struct Queue* _que, struct Message* msg){/* 0 success */
    if (empty_que(_que))
        return -1;
    *msg = _que->buffer[_que->fron];
    return 0;
}
