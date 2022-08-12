# 并行程序设计导论 读书笔记

* talk is easy, show me the code
* read the friendly manual

## 第四章 用Pthreads进行共享内存编程

实验环境

> windows subsystem for linux 2 WSL: Ubuntu-22.04

安装Pthreads手册

```bash
#!/bin/sh
sudo apt-get install glibc-doc
sudo apt-get install manpages-posix manpages-posix-dev
```

查看Pthreads

```c
$ man pthreads
```

Makefile

```makefile
# 自动编译当前目录下所有C文件为对应的二进制文件
# Usage: make [options]
# options:
#   D=DEFINE    #define DEFINE in C

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
$ time ./a.out
```

源码

> https://github.com/TrymenT-AlphA/AIPP-Study

### Pth_hello.c

#### pthread_create

```C
man pthread_create
```

##### SYNOPSIS

```C
#include <pthread.h>

int pthread_create(
    pthread_t *thread,              /* out */ /* thread ID */
    const pthread_attr_t *attr,     /* in */ /* attr of new thread, if NULL using default */
    void *(*start_routine) (void *),/* in */ /* thread routine */
    void *arg                       /* in */ /* start_routine sole arg, using addr, using struct */
);

Compile and link with -pthread.
```

##### DESCRIPTION

> The pthread_create() function starts a new thread in the calling process. The new thread starts execution by invoking start_routine(); arg is passed as the sole argument of start_routine().

#### pthread_join

```C
man pthread_join
```

##### SYNOPSIS

```C
#include <pthread.h>

int pthread_join(
    pthread_t thread,/* in */ /* thread ID */
    void **retval    /* out */ /* return value from start_routine, if NULL, ignore */
);

Compile and link with -pthread.
```

##### DESCRIPTION

> The  pthread_join()  function  waits  for  the  thread  specified by thread to terminate.  If that thread has already terminated, then pthread_join() returns immediately.  The thread specified by thread must be joinable.

#### 同时等待多个线程

```C
for (thread = 0; thread < thread_count; thread++)
    pthread_join(thread_handles[thread], NULL);
```

当程序运行到pthread_join(thread_handle[0], NULL)时就被阻塞，线程0运行结束后，继续被线程1阻塞，以此类推。注意到`If that thread has already terminated, then pthread_join() returns immediately`线程在被创建后就可以开始运行，实际的结果就是阻塞至所有线程中最晚结束的线程返回。

#### start_routine函数

```C
void* thread_function(void*);
```

thread_function形式固定，只接受一个参数，且必须为void\*指针，返回值也必须是void\*指针，都代表一个地址。参数和返回值在使用前按需要的类型进行强制类型转换。若希望使用多个参数，可以将多个参数定义为一个结构体，返回值同理。

实际上start_routine = &thread_function，start_routine为一个函数指针，指向thread_function的起始地址。

#### 线程执行的顺序

线程执行没有固定的顺序，在线程被创建完成后，其会被设置为就绪态，等待系统的调度，线程执行的顺序都由系统调度决定。

#### printf是线程安全的吗

是线程安全的

```C
man 3 printf
```

```C
make D=DEBUG && ./pth_hello 2
```

#### 伪代码

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

#### 输出

```c
$ ./pth_hello 4   
Hello from thread 0 of 4
Hello from the main thread
Hello from thread 1 of 4
Hello from thread 3 of 4
Hello from thread 2 of 4
```

### Pth_mat_vect.c

#### 计算矩阵向量乘法

$$
y_i = \sum_{j=0}^{n-1}a_{ij}x_j
$$

```C
/* 这是串行程序的伪代码 */
/* for each row of A */
for (i = 0; i < m; i++){
    y[i] = 0.0;
    /* for each element of the row and each element of x */
    for (j = 0; j < n; j++)
        y[i] += A[i][j]*x[j]
}
```

通过将外层循环分块，每个线程计算y的一部分，可以很容易实现并行化。将A，x，y都作为全局变量，实际上并没有产生竞争条件，并且可以方便输入输出。

#### 伪代码

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

#### 输出

```C
$ time ./pth_mat_vect 1
Thread[0]: calc from row:0 to row:9999
Thread [main] Using [1] threads, total time: [0] s
./pth_mat_vect 1  0.26s user 0.00s system 96% cpu 0.272 total
$ time ./pth_mat_vect 2
Thread [0]: calc from row:0 to row:5000
Thread [1]: calc from row:5000 to row:10000
Thread [main] Using [2] threads, total time: [1] s
./pth_mat_vect 2  0.25s user 0.01s system 194% cpu 0.134 total
$ time ./pth_mat_vect 3
Thread [0]: calc from row:0 to row:3333
Thread [1]: calc from row:3333 to row:6666
Thread [2]: calc from row:6666 to row:9999
Thread [main] Using [3] threads, total time: [0] s
./pth_mat_vect 3  0.34s user 0.00s system 254% cpu 0.134 total
$ time ./pth_mat_vect 4
Thread[0]: calc from row:0 to row:2499
Thread[1]: calc from row:2500 to row:4999
Thread[3]: calc from row:7500 to row:9999
Thread[2]: calc from row:5000 to row:7499
Thread [main] Using [4] threads, total time: [0] s
./pth_mat_vect 4  0.27s user 0.00s system 386% cpu 0.070 total
```

> 2核至3核的性能几乎没有提升，并且3核占用了更多的cpu资源，应该是伪共享产生了大量不必要的访存操作。我的cpu每个核两个线程，当使用到第三个线程时就必须考虑两个核之间的Cache一致性问题

### Pth_sum.c

#### 临界区

更新共享资源的代码段，一次只允许一个线程执行该代码端。

#### 忙等待

```C
y = Compute(my_rank);
while !Condiction; /* 忙等待spin */
/* Critical zone */
```

忙等待可以实现临界区，并且能够保证线程按顺序进入临界区，但是会占用较多的cpu资源

> 编译优化可以更改某些指令的执行顺序，这可能无法保证忙等待的正确性！

#### 多线程求和

$$
\frac{\pi}{4}=1-\frac{1}{3}+\frac{1}{5}-\frac{1}{7}+···+(-1)^n\frac{1}{2n+1}
$$

#### Pth_sum_0.c

##### 伪代码

```c
void* Pth_sum(void* rank){
    long my_rank = (long)rank;
    double factor;
    long long i;
    long long my_n = n/thread_count;
    long long my_first_i = my_n*my_rank;
    long long my_last_i = my_first_i+my_n;
    
    ...; /* log */

    if (my_first_i % 2 == 0)
        factor = 1.0;
    else
        factor = -1.0;

    for (i = my_first_i; i < my_last_i; i++, factor = -factor){
        /* ！！频繁的spin会带来极大的性能损失！！ */
        while (flag != my_rank); /* spin */
        sum += factor/(2*i+1);
        flag = (flag+1) % thread_count;
    }

    return NULL;
} /* Pth_sum */
```

##### 输出

```C
$ time ./pth_sum_0 1
Thread [0] my_first_i: 0, my_last_i: 4294967295
Thread [main] Using [1] threads, total time: [20] s
Thread [main] pi: 3.1415926538
./pth_sum_0 1  19.58s user 0.00s system 99% cpu 19.585 total
$ time ./pth_sum_0 4
Thread [0] my_first_i: 0, my_last_i: 1073741823
Thread [1] my_first_i: 1073741823, my_last_i: 2147483646
Thread [2] my_first_i: 2147483646, my_last_i: 3221225469
Thread [3] my_first_i: 3221225469, my_last_i: 4294967292
Thread [main] Using [4] threads, total time: [640] s
Thread [main] pi: 3.1415926534
./pth_sum_0 4  2559.37s user 0.00s system 399% cpu 10:39.85 total
```

> ！！频繁的spin会带来极大的性能损失！！

#### Pth_sum_1.c

#####  伪代码

```C
void* Pth_sum(void* rank){
    ...;

    for (i = my_first_i; i < my_last_i; i++, factor = -factor)
        my_sum += factor/(2*i+1);
    
    while (flag != my_rank); /* spin */
    sum += my_sum;
    flag = (flag+1) % thread_count;

    return NULL;
} /* Pth_sum */
```

#### 输出

```c
$ time ./pth_sum_1 1
Thread [0] my_first_i: 0, my_last_i: 4294967295
Thread [0] mysum: 0.7853981635
Thread [main] Using [1] threads, total time: [12] s
Thread [main] pi: 3.1415926538
./pth_sum 1  11.15s user 0.00s system 99% cpu 11.156 total
$ time ./pth_sum_1 4
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

> 多线程程序的正确性没有问题，也得到了性能提升

#### 互斥量

忙等待虽然能够简单的实现临界区，但是有许多缺点。另一种实现互斥量的方法是使用互斥量。

#### pthread_mutex_t

##### DESCRIPTION

> A mutex is a MUTual EXclusion device, and is useful for protecting shared data  structures from concurrent modifications, and implementing critical sections and monitors. A mutex has two possible states: unlocked (not owned by any thread), and locked (owned by one thread). A mutex can never be owned by two different threads simultaneously. A thread attempting to lock a mutex that is already locked by another thread is suspended until the owning thread unlocks the mutex first. 

#### pthread_mutex_init

```C
man pthread_mutex_init
```

##### SYNOPSIS

```C
#include <pthread.h>

int pthread_mutex_init(
    pthread_mutex_t *mutex, /* in */ /* pointer to mutex(pthread_mutex_t) */
    const pthread_mutexattr_t *mutexattr /* in */ /* if NULL, default */
);
```

##### DESCRIPTION

> pthread_mutex_init initializes the mutex object pointed to by mutex according to the mutex attributes specified in mutexattr. If mutexattr is NULL, default attributes are used instead.

#### pthread_mutex_lock

```C
man pthread_mutex_lock
```

##### SYNOPSIS

```c
#include <pthread.h>

int pthread_mutex_lock(
    pthread_mutex_t *mutex /* in */ /* pointer to mutex(pthread_mutex_t) */
);
```

##### DESCRIPTION

> pthread_mutex_lock locks the given mutex. If the mutex is currently unlocked, it becomes locked and owned by the calling thread, and pthread_mutex_lock returns immediately. If the mutex is already locked by another thread, pthread_mutex_lock suspends the calling thread until the mutex is unlocked.

#### pthread_mutex_unlock

```C
man pthread_mutex_unlock
```

##### SYNOPSIS

```c
#include <pthread.h>

int pthread_mutex_unlock(
    pthread_mutex_t *mutex /* in */ /* pointer to mutex(pthread_mutex_t) */
);
```

##### DESCRIPTION

> pthread_mutex_unlock unlocks the given mutex. The mutex is assumed to be locked and owned by the calling thread on entrance to pthread_mutex_unlock. If the  mutex is of the `fast` kind, pthread_mutex_unlock  always  returns  it  to  the unlocked state. If it is of the `recursive` kind, it decrements the locking count of the mutex (number of pthread_mutex_lock operations performed on it by the calling thread), and only when this count reaches zero is the mutex actually unlocked.

#### demo

```C
/* A shared global variable x can be protected by a mutex as follows: */
int x;
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

/* 
All accesses and modifications to x should be bracketed by calls to  
pthread_mutex_lock and pthread_mutex_unlock as follows:
*/
pthread_mutex_lock(&mut);
/* operate on x */
pthread_mutex_unlock(&mut);
```

> 简而言之，临界区就像厕所，互斥量就是厕所的锁。你不想和别人一起上厕所，就要先拿到锁，然后进厕所上锁，上完厕所把锁放回原处，不然别人上不了厕所（笑

#### Pth_sum_2.c

##### 伪代码

```C
pthread_muext_t lock;

int main(int argc, char* argv[]){
    ...;
    pthread_mutex_init(&lock);
	..;
}

void* Pth_sum(void* rank){
    ...;

    pthread_mutex_lock(&lock); /* using mutex */
    sum += my_sum;
    pthread_mutex_unlock(&lock);

    return NULL;
} /* Pth_sum */
```

##### 输出

```C
$ time ./pth_sum 1
Thread [0] my_first_i: 0, my_last_i: 4294967295
Thread [0] mysum: 0.7853981635
Thread [main] Using [1] threads, total time: [12] s
Thread [main] pi: 3.1415926538
./pth_sum 1  11.04s user 0.00s system 99% cpu 11.045 total
$ time ./pth_sum 4
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

> 貌似和忙等待没有多大提升，不妨将Pth_sum_0.c改成互斥量实现看看效果

#### Pth_sum_3.c

##### 伪代码

```C
void* Pth_sum(void* rank){
    ...;
    for (i = my_first_i; i < my_last_i; i++, factor = -factor)
        pthread_mutex_lock(&lock); /* using mutex */
        sum += factor/(2*i+1);
        pthread_mutex_unlock(&lock);

    return NULL;
} /* Pth_sum */
```

##### 输出

```C
$ time ./pth_sum_3 1
Thread [0] my_first_i: 0, my_last_i: 4294967295
Thread [main] Using [1] threads, total time: [45] s
Thread [main] pi: 3.1415926538
./pth_sum_3 1  44.82s user 0.00s system 99% cpu 44.830 total
$ time ./pth_sum_3 4
Thread [0] my_first_i: 0, my_last_i: 1073741823
Thread [1] my_first_i: 1073741823, my_last_i: 2147483646
Thread [3] my_first_i: 3221225469, my_last_i: 4294967292
Thread [2] my_first_i: 2147483646, my_last_i: 3221225469
Thread [main] Using [4] threads, total time: [189] s
Thread [main] pi: 3.1415926534
./pth_sum_3 4  278.95s user 396.18s system 356% cpu 3:09.38 total
```

> 事实证明，多线程下互斥量确实快，占用cpu也较少
