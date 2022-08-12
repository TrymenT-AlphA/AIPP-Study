#include <stdio.h>
#include <stdlib.h>
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
} que;

int init_que(struct Queue* _que);/* 0 success */
int full_que(struct Queue* _que);/* 1 full, else 0 */
int empty_que(struct Queue* _que);/* 1 empty, else 0 */
int push_que(struct Queue* _que, struct Message msg);/* 0 success */
int pop_que(struct Queue* _que, struct Message* msg);/* 0 success */
int peek_que(struct Queue* _que, struct Message* msg);/* 0 success */

void* Pth_msg(void* rank);

int thread_count;
sem_t mutex, msg_num;

int main(int argc, char* argv[]){
    long thread;
    pthread_t* thread_handles;

    sem_init(&mutex, 0, 1);
    sem_init(&msg_num, 0, 0);
    init_que(&que);

    thread_count = 8;

    thread_handles = malloc(thread_count*sizeof(pthread_t));

    for (thread = 0; thread < thread_count; thread++)
        pthread_create(&thread_handles[thread], NULL, Pth_msg, (void*)thread);

    for (thread = 0; thread < thread_count; thread++)
        pthread_join(thread_handles[thread], NULL);

    free(thread_handles);

    return 0;
}

void* Pth_msg(void* rank){
    long my_rank = (long)rank;
    struct Message message;

    message.dst_thread = (my_rank+1)%thread_count;
    sprintf(message.msg, "Hello! thread [%ld] , i'm thread [%ld]", message.dst_thread, my_rank);

    sem_wait(&mutex); /* enter critical zone */
    push_que(&que, message);
    sem_post(&mutex); /* leave critical zone */
    sem_post(&msg_num); /* produce a msg */

    printf("Thread [%ld]: sended message to thread [%ld]\n", my_rank, message.dst_thread);

    while(1){
        sem_wait(&msg_num); /* consume a msg */
        peek_que(&que, &message);
        if (message.dst_thread != my_rank)
            sem_post(&msg_num);
        else{
            sem_wait(&mutex); /* enter critical zone */
            pop_que(&que, NULL);
            sem_post(&mutex); /* leave critical zone */

            printf("Thread [%ld]: received a message: %s\n", my_rank, message.msg);
            break;
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
