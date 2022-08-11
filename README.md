# 并行程序设计导论 读书笔记

* talk is easy, show me the code
* read the friendly manual

## 第四章 用Pthreads进行共享内存编程

实验环境

> windows subsystem for linux 2

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

#### pthread_create

```C
SYNOPSIS
        #include <pthread.h>

        int pthread_create(
            pthread_t *thread,              /* out */ /* thread ID */
            const pthread_attr_t *attr,     /* in */ /* attr of new thread, if NULL using default */
            void *(*start_routine) (void *),/* in */ /* thread routine */
            void *arg                       /* in */ /* start_routine sole arg, using addr, using struct */
        );

        Compile and link with -pthread.

DESCRIPTION
       The  pthread_create()  function  starts  a  new thread in the calling process.  The new thread starts execution by invoking start_rou‐
       tine(); arg is passed as the sole argument of start_routine().

详见：man pthread_create
```

#### pthread_join

```C
SYNOPSIS
        #include <pthread.h>

        int pthread_join(
            pthread_t thread,/* in */ /* thread ID */
            void **retval    /* out */ /* return value from start_routine, if NULL, ignore */
        );

        Compile and link with -pthread.

DESCRIPTION
       The  pthread_join()  function  waits  for  the  thread  specified by thread to terminate.  If that thread has already terminated, then
        pthread_join() returns immediately.  The thread specified by thread must be joinable.

详见：man pthread_join
```

#### 为什么能同时等待多个线程

```C
for (thread = 0; thread < thread_count; thread++)
    pthread_join(thread_handles[thread], NULL);
```

当程序运行到pthread_join(thread_handle[0], NULL)时就被阻塞，等线程0运行结束后，继续等待线程1，以此类推。

注意到`If that thread has already terminated, then pthread_join() returns immediately`

线程在被创建后就可以开始运行，实际的结果就是阻塞至所有线程中最晚结束的线程返回

#### 如何定义start_routine函数

```C
void* Pth_hello(void* rank);
```

Pth_hello形式固定，只接受一个参数，且必须为void\*指针，返回值也必须是void\*指针，都代表一个纯地址

参数和返回值在使用前要使用强制类型转换

实际上*start_routine = Pth_hello，start_routine为一个函数指针，指向Pth_hello的起始地址

若希望使用多个参数，可以将多个参数定义为一个结构体

#### 线程执行的顺序

线程执行没有固定的顺序，在线程被创建完成后，其会被设置为就绪态，等待系统的调度，线程执行的顺序都由系统调度决定

#### printf是线程安全的吗

```C
man 3 printf
```

是线程安全的

```C
make D=DEBUG && ./pth_hello 2
```

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

    #ifdef DEBUG /* ? is printf thread safe ? probably yes */
    int my_count = 100;
    while(my_count--)
        if (my_rank == 0)
            printf("Thread [%ld] %s\n", my_rank, "0000000000000000000000000000000000000000");
        else
            printf("Thread [%ld] %s\n", my_rank, "1111111111111111111111111111111111111111");
    #endif

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

