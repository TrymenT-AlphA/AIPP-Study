#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int thread_count, flag;
double sum;

#define n 1E8

void* Pth_sum(void* rank);

int main(int argc, char* argv[]){
    if (argc <= 1){
        printf("Usage: ./a.out thread_count\n");
        return 0;
    }

    int st_time, ed_time;
    st_time = clock();

    long thread; /* use long in case of 64-bit system */
    pthread_t* thread_handles;

    /* Get number of all threads from command line */
    thread_count = strtol(argv[1], NULL, 10);

    thread_handles = malloc(thread_count*sizeof(pthread_t));

    for (thread = 0; thread < thread_count; thread++)
        pthread_create(&thread_handles[thread], NULL, Pth_sum, (void*)thread);

    for (thread = 0; thread < thread_count; thread++)
        pthread_join(thread_handles[thread], NULL);

    free(thread_handles);

    ed_time = clock();

    printf(
        "Thread [main] Using [\033[31m%d\033[0m] threads, total time: [\033[31m%lf\033[0m] ms\n", 
        thread_count, (double)(ed_time - st_time)*1000 / CLOCKS_PER_SEC
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

    while (flag != my_rank);
    #ifdef LOG
    printf("Thread [%ld] mysum: %.10lf\n", my_rank, my_sum);
    #endif
    sum += my_sum;
    flag = (flag+1) % thread_count;
    return NULL;
} /* Pth_sum */
