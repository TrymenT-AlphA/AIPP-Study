#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int main(int argc, char* argv[]){
    double factor = 1.0;
    double sum = 0.0;
    int    n, k;
    int    thread_count;

    thread_count = strtol(argv[1], NULL, 10);
    printf("Thread [main]: Enter n\n");
    scanf("%d", &n);

    #pragma omp parallel for num_threads(thread_count) \
        default(none) reduction(+: sum) private(k, factor) shared(n)
    for (k = 0; k < n; k++){
        if (k % 2) factor = -1.0;
        else factor = 1.0;
        sum += factor/(2*k+1);
    }

    printf("Thread [main]: pi: %.14e", 4*sum);

    return 0;
}
