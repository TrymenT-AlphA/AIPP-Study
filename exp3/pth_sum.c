#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int thread_count;
double sum;
pthread_mutex_t lock;

#define n 0xFFFFFFFF

void* Pth_sum(void* rank);

int main(int argc, char* argv[]){
    if (argc <= 1){
        printf("Usage: ./a.out thread_count\n");
        return 0;
    }

    clock_t st_time, ed_time;
    st_time = time(NULL);

    long thread;
    pthread_t* thread_handles;

    thread_count = strtol(argv[1], NULL, 10);

    pthread_mutex_init(&lock, NULL); /* init mutex */

    thread_handles = malloc(thread_count*sizeof(pthread_t));

    for (thread = 0; thread < thread_count; thread++)
        pthread_create(&thread_handles[thread], NULL, Pth_sum, (void*)thread);

    for (thread = 0; thread < thread_count; thread++)
        pthread_join(thread_handles[thread], NULL);

    free(thread_handles);

    pthread_mutex_destroy(&lock);

    ed_time = time(NULL);

    printf(
        "Thread [main] Using [\033[31m%d\033[0m] threads, total time: [\033[31m%ld\033[0m] s\n", 
        thread_count, (ed_time - st_time)
    );
    printf("Thread [main] pi: %.10lf\n", 4.0*sum);

    return 0;
}

void* Pth_sum(void* rank){
    long my_rank = (long)rank;
    double factor, my_sum = 0.0;
    long long i;
    long long my_n = n/thread_count;
    long long my_first_i = my_n*my_rank;
    long long my_last_i = my_first_i+my_n;

    #ifdef LOG
    printf("Thread [%ld] my_first_i: %lld, my_last_i: %lld\n", my_rank, my_first_i, my_last_i);
    #endif

    if (my_first_i % 2 == 0)
        factor = 1.0;
    else
        factor = -1.0;

    for (i = my_first_i; i < my_last_i; i++, factor = -factor)
        my_sum += factor/(2*i+1);

    #ifdef LOG
    printf("Thread [%ld] mysum: %.10lf\n", my_rank, my_sum);
    #endif

    pthread_mutex_lock(&lock); /* using mutex */
    sum += my_sum;
    pthread_mutex_unlock(&lock);

    return NULL;
} /* Pth_sum */
