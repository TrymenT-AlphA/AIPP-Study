#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int f(int x){
    int res = 0;
    while(x--) res += x;
    return res;
}

int main(int argc, char* argv[]){
    int n = 10000;
    int sum = 0;
    int i;
    int thread_count;

    thread_count = strtol(argv[1], NULL, 10);

    #ifndef SCHE
    #pragma omp parallel for num_threads(thread_count) \
        reduction(+: sum)
    #else
    #pragma omp parallel for num_threads(thread_count) \
        reduction(+: sum) schedule(static, 1)
    #endif
    for (i = 0; i < n; i++)
        sum += f(i);

    return 0;
}