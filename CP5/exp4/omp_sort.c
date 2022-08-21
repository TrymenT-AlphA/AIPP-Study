#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

void Swap(int* a_p, int* b_p){
    int tmp = *a_p;
    *a_p = *b_p;
    *b_p = tmp;
}

int main(int argc, char* argv[]){
    int i, phase, n = 10000;
    int thread_count;
    int* a;

    thread_count = strtol(argv[1], NULL, 10);
    a = malloc(n*sizeof(a));

    for (i = 0; i < n; i++)
        a[i] = n - i;

    #ifdef FIRST
    for (phase = 0; phase < n; phase++)
        if (phase % 2 == 0){
            #pragma omp parallel for num_threads(thread_count) \
                default(none) shared(a, n) private(i)
            for (i = 1; i < n; i += 2){
                if (a[i-1] > a[i]) 
                    Swap(&a[i-1], &a[i]);
            }
        }
        else{
            #pragma omp parallel for num_threads(thread_count) \
                default(none) shared(a, n) private(i)
            for (i = 1; i < n-1; i += 2){
                if (a[i] > a[i+1])
                    Swap(&a[i], &a[i+1]);
            }
        }
    #endif

    #ifdef SECOND
    #pragma omp parallel num_threads(thread_count) \
        default(none) shared(a, n) private(i, phase)
    for (phase = 0; phase < n; phase++)
        if (phase % 2 == 0){
            #pragma omp for
            for (i = 1; i < n; i += 2){
                if (a[i-1] > a[i]) 
                    Swap(&a[i-1], &a[i]);
            }
        }
        else{
            #pragma omp for
            for (i = 1; i < n-1; i += 2){
                if (a[i] > a[i+1])
                    Swap(&a[i], &a[i+1]);
            }
        }
    #endif

    for (i = 0; i < n; i++)
        printf("%d\n", a[i]);

    return 0;
}
