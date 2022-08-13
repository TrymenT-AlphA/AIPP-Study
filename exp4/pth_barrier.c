#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void* Thread_work(void* rank);

int thread_count;
int counter;
pthread_mutex_t mutex;
pthread_cond_t cond;

int main(int argc, char* argv[]){
    if (argc <= 1){
        printf("Usage: ./a.out thread_count\n");
        return 0;
    }

    long thread;
    pthread_t* thread_handle;

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    thread_count = strtol(argv[1], NULL, 10);

    thread_handle = (pthread_t*)malloc(thread_count*sizeof(pthread_t));

    for (thread = 0; thread < thread_count; thread++)
        pthread_create(&thread_handle[thread], NULL, Thread_work, (void*)thread);
    
    for (thread = 0; thread < thread_count; thread++)
        pthread_join(thread_handle[thread], NULL);

    free(thread_handle);

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
        printf("Thread [%ld]: passing\n", my_rank);
    }
    pthread_mutex_unlock(&mutex);

    return NULL;
}
