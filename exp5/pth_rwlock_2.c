/*
    第三种实现：读写锁
*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "linklist.h"

int thread_count;
pthread_rwlock_t rwlock;
struct linklist_node* head = NULL;

void* Thread_work(void* rank);

int main(int argc, char* argv[]){
    long thread;
    pthread_t* thread_handles;

    thread_count = 8;
    pthread_rwlock_init(&rwlock, NULL);
    int init_num, val; /* init list */
    FILE * in = fopen("input.txt", "r");
    if (fscanf(in, "%d", &init_num) < 0) exit(-1);
    while(init_num--){
        if (fscanf(in, "%d", &val) < 0) exit(-1);
        Insert(&head, val);
    }

    thread_handles = malloc(thread_count*sizeof(pthread_t));

    for (thread = 0; thread < thread_count; thread++)
        pthread_create(&thread_handles[thread], NULL, Thread_work, (void*)thread);

    for (thread = 0; thread < thread_count; thread++)
        pthread_join(thread_handles[thread], NULL);

    free(thread_handles);

    pthread_rwlock_destroy(&rwlock);

    #ifdef DEBUG
    Layout(&head);
    #endif

    return 0;
}

void* Thread_work(void* rank){
    long my_rank = (long)rank;
    char filename[32];
    char op;
    int op_num, val;

    sprintf(filename, "input%ld.txt", my_rank);
    FILE * in = fopen(filename, "r");

    if (fscanf(in, "%d", &op_num) < 0)
        exit(-1);
    while(op_num--){
        if (fscanf(in, " %c %d\n", &op, &val) < 0) exit(-1);
        switch(op){
            case 'M':
            pthread_rwlock_rdlock(&rwlock);
            Member(&head, val);
            pthread_rwlock_unlock(&rwlock);
            break;
            case 'I':
            pthread_rwlock_wrlock(&rwlock);
            Insert(&head, val);
            pthread_rwlock_unlock(&rwlock);
            break;
            case 'D':
            pthread_rwlock_wrlock(&rwlock);
            Delete(&head, val);
            pthread_rwlock_unlock(&rwlock);
            break;
        }
    }

    return NULL;
}
