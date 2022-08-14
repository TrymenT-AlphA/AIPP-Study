#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define m 8
#define n 800000

/* Global variables */
int thread_count;
double A[m][n];
double y[m];
double x[n];

void* Pth_mat_vect(void* rank);

int main(int argc, char* argv[]){
    if (argc <= 1){
        printf("Usage: ./a.out thread_count\n");
        return 0;
    }
    
    clock_t st_time, ed_time;
    st_time = time(NULL); /* start the timer */

    long thread;
    pthread_t* thread_handles;

    thread_count = strtol(argv[1], NULL, 10);

    thread_handles = malloc(thread_count*sizeof(pthread_t));

    for (thread = 0; thread < thread_count; thread++)
        pthread_create(&thread_handles[thread], NULL, Pth_mat_vect, (void*)thread);

    for (thread = 0; thread < thread_count; thread++)
        pthread_join(thread_handles[thread], NULL);

    free(thread_handles);

    ed_time = time(NULL); /* end the timer */

    printf(
        "Thread [main] Using [\033[31m%d\033[0m] threads, total time: [\033[31m%ld\033[0m] s\n", thread_count, ed_time - st_time
    );

    return 0;
} /* main */

void* Pth_mat_vect(void* rank){
    long my_rank = (long)rank;
    int i, j;
    int local_m = (m+thread_count-1)/thread_count;
    int my_first_row = my_rank*local_m;
    int my_last_row = my_first_row+local_m;

    if (my_last_row > m)
        my_last_row = m;

    #ifdef LOG
    printf("Thread [%ld]: calc from row:%d to row:%d\n", my_rank, my_first_row, my_last_row);
    #endif

    for (i = my_first_row; i < my_last_row; i++){
        y[i] = 0.0;
        for (j = 0; j < n; j++)
            y[i] += A[i][j]*x[j];
    }

    return NULL;
} /* Pth_mat_vect */
