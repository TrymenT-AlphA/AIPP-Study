#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

/* Global variables: accessible to all threads */
int thread_count;

void* Pth_hello(void* rank); /* Thread function */

int main(int argc, char* argv[]) {
    long thread; /* use long in case of 64-bit system */
    pthread_t* thread_handles;

    /* Get number of all threads from command line */
    if (argc <= 1) {
        printf("Usage: ./a.out thread_count\n");
        return 0;
    }
    thread_count = strtol(argv[1], NULL, 10);

    thread_handles = malloc(thread_count*sizeof(pthread_t));

    for (thread = 0; thread < thread_count; thread++)
        pthread_create(&thread_handles[thread], NULL, Pth_hello, (void*)thread);

    printf("Hello from the main thread\n");

    for (thread = 0; thread < thread_count; thread++)
        pthread_join(thread_handles[thread], NULL);

    free(thread_handles);
    return 0;
} /* main */

void* Pth_hello(void* rank) {
    long my_rank = (long) rank; /* use long in case of 64-bit system */

    printf("Hello from thread %ld of %d\n", my_rank, thread_count);

    return NULL;
} /* Hello */
