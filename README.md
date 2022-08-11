# 并行程序设计导论 读书笔记

## 第四章 用Pthreads进行共享内存编程

安装Pthreads手册

```bash
#!/bin/sh
sudo apt-get install glibc-doc
sudo apt-get install manpages-posix manpages-posix-dev
```

查看Pthreads

```c
PS> man pthreads
```

Makefile

```makefile
# 自动编译当前目录下所有C文件为对应的二进制文件
# Usage: make [options]
# options:
# 	D=DEFINE	#define DEFINE in C

SRC = $(wildcard *.c)
BIN = $(SRC:%.c=%)

CC = gcc
CFLAGS = -Wall -g -lpthread

all: $(BIN)

ifndef D
$(BIN): %:%.c
	$(CC) $(CFLAGS) $^ -o $@
else
$(BIN): %:%.c
	$(CC) -D$(D) $(CFLAGS) $^ -o $@
endif

.PHONY: clean
clean:
	-rm $(BIN)

```

使用time指令测试

```c
PS> time ./a.out
```

源码

```C
https://github.com/TrymenT-AlphA/AIPP-Study
```

### Pth_hello.c

#### src

```C
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

/* Global variables: accessible to all threads */
int thread_count;

void* Pth_hello(void* rank); /* Thread function */

int main(int argc, char* argv[]){
    if (argc <= 1){ /* error input */
        printf("Usage: ./a.out thread_count\n");
        return 0;
    }

    long thread;
    pthread_t* thread_handles;

    /* Get number of all threads from command line */
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

void* Pth_hello(void* rank){
    long my_rank = (long)rank;

    printf("Hello from thread %ld of %d\n", my_rank, thread_count);

    return NULL;
} /* Hello */

```

#### output

```c
PS> ./pth_hello 4   
Hello from thread 0 of 4
Hello from the main thread
Hello from thread 1 of 4
Hello from thread 3 of 4
Hello from thread 2 of 4
```

### Pth_mat_vect.c

#### src

```C
#define m 10000
#define n 10000

/* Global variables */
int thread_count;
double A[m][n];
double y[m];
double x[n];

...; /* main */

void* Pth_mat_vect(void* rank){
    long my_rank = (long)rank;
    int i, j;
    int local_m = m/thread_count;
    int my_first_row = my_rank*local_m; /* easily divide task */
    int my_last_row = (my_rank+1)*local_m-1;

	...; /* log */

    for (i = my_first_row; i <= my_last_row; i++){ /* do job */
        y[i] = 0.0;
        for (j = 0; j < n; j++)
            y[i] += A[i][j]*x[j];
    }

    return NULL;
} /* Pth_mat_vect */

```

#### output

```C
PS> time ./pth_mat_vect 1
Thread[0]: calc from row:0 to row:9999
Thread [main] Using [1] threads, total time: [0] s
./pth_mat_vect 1  0.26s user 0.00s system 96% cpu 0.272 total
PS> time ./pth_mat_vect 4
Thread[0]: calc from row:0 to row:2499
Thread[1]: calc from row:2500 to row:4999
Thread[3]: calc from row:7500 to row:9999
Thread[2]: calc from row:5000 to row:7499
Thread [main] Using [4] threads, total time: [0] s
./pth_mat_vect 4  0.27s user 0.00s system 386% cpu 0.070 total
```

### Pth_sum

#### src

```C
int thread_count, flag;
double sum;

#define n 0xFFFFFFFF

...; /* main */

void* Pth_sum(void* rank){
    long my_rank = (long)rank;
    double factor, my_sum = 0.0;
    long long i;
    long long my_n = n/thread_count;
    long long my_first_i = my_n*my_rank;
    long long my_last_i = my_first_i+my_n;

	...; /* log */
	...; /* calc */
	...; /* log */

    while (flag != my_rank); /* spin */
    sum += my_sum;
    flag = (flag+1) % thread_count;

    return NULL;
} /* Pth_sum */

```

#### output

```c
PS> time ./pth_sum 1
Thread [0] my_first_i: 0, my_last_i: 4294967295
Thread [0] mysum: 0.7853981635
Thread [main] Using [1] threads, total time: [12] s
Thread [main] pi: 3.1415926538
./pth_sum 1  11.15s user 0.00s system 99% cpu 11.156 total
PS> time ./pth_sum 4
Thread [0] my_first_i: 0, my_last_i: 1073741823
Thread [1] my_first_i: 1073741823, my_last_i: 2147483646
Thread [2] my_first_i: 2147483646, my_last_i: 3221225469
Thread [3] my_first_i: 3221225469, my_last_i: 4294967292
Thread [2] mysum: 0.0000000002
Thread [1] mysum: -0.0000000003
Thread [3] mysum: -0.0000000001
Thread [0] mysum: 0.7853981636
Thread [main] Using [4] threads, total time: [3] s
Thread [main] pi: 3.1415926534
./pth_sum 4  11.53s user 0.00s system 398% cpu 2.893 total
```

### Pth_sum(2)

#### src

```C
int thread_count;
double sum;
pthread_mutex_t lock;

#define n 0xFFFFFFFF

...; /* main */

void* Pth_sum(void* rank){
    long my_rank = (long)rank;
    double factor, my_sum = 0.0;
    long long i;
    long long my_n = n/thread_count;
    long long my_first_i = my_n*my_rank;
    long long my_last_i = my_first_i+my_n;

	...; /* log */
	...; /* calc */
	...; /* log */

    pthread_mutex_lock(&lock); /* using mutex */
    sum += my_sum;
    pthread_mutex_unlock(&lock);

    return NULL;
} /* Pth_sum */

```

#### output

```C
PS> time ./pth_sum 1
Thread [0] my_first_i: 0, my_last_i: 4294967295
Thread [0] mysum: 0.7853981635
Thread [main] Using [1] threads, total time: [12] s
Thread [main] pi: 3.1415926538
./pth_sum 1  11.04s user 0.00s system 99% cpu 11.045 total
PS> time ./pth_sum 4
Thread [0] my_first_i: 0, my_last_i: 1073741823
Thread [2] my_first_i: 2147483646, my_last_i: 3221225469
Thread [1] my_first_i: 1073741823, my_last_i: 2147483646
Thread [3] my_first_i: 3221225469, my_last_i: 4294967292
Thread [1] mysum: -0.0000000003
Thread [0] mysum: 0.7853981636
Thread [2] mysum: 0.0000000002
Thread [3] mysum: -0.0000000001
Thread [main] Using [4] threads, total time: [3] s
Thread [main] pi: 3.1415926534
./pth_sum 4  11.54s user 0.00s system 396% cpu 2.910 total
```

