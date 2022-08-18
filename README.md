# 并行程序设计导论 读书笔记

## 第四章 用Pthreads进行共享内存编程

实验环境

> windows subsystem for linux WSL2:Ubuntu-22.04

仓库

> https://github.com/TrymenT-AlphA/AIPP-Study

Pthreads手册

```bash
# 手册安装
$ sudo apt-get install glibc-doc
$ sudo apt-get install manpages-posix manpages-posix-dev
# 手册查询
$ man pthreads
```

使用time指令测试

```shell
# cpu time 和 wall time
# cpu time：程序占用的cpu时间，多线程下为所有线程占用cpu时间之和
# wall time：墙上时间，实际流逝的时间
$ time [-v] ./a.out
```

Makefile

```makefile
# 自动编译当前目录下所有C文件为对应的二进制文件
# Author: ChongKai
# Usage: make [options]
# options:
# 	D=DEFINE	#define DEFINE in C

SRC = $(wildcard *.c)
BIN = $(SRC:%.c=%)

CC = gcc
CFLAGS = -Wall -O0 -g -lpthread

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

### Pth_hello.c

#### pthread_create

```C
#include <pthread.h>

int pthread_create(
    pthread_t *thread,              /* out */ /* thread ID */
    const pthread_attr_t *attr,     /* in */ /* attr of new thread, if NULL using default */
    void *(*start_routine) (void *),/* in */ /* thread routine */
    void *arg                       /* in */ /* start_routine sole arg, using addr, using struct */
);
```

> The pthread_create() function starts a new thread in the calling process. The new thread starts execution by invoking start_routine(); arg is passed as the sole argument of start_routine().

#### pthread_join

```C
#include <pthread.h>

int pthread_join(
    pthread_t thread,/* in */ /* thread ID */
    void **retval    /* out */ /* return value from start_routine, if NULL, ignore */
);
```

> The pthread_join() function waits for the thread specified by thread to terminate.  If that thread has already terminated, then pthread_join() returns immediately.  The thread specified by thread must be joinable.

#### 同时等待多个线程

```C
for (thread = 0; thread < thread_count; thread++)
    pthread_join(thread_handles[thread], NULL);
```

当主线程运行到pthread_join(thread_handle[0], NULL)时就被阻塞，待线程0运行结束后，继续被线程1阻塞，以此类推。因为线程在被创建后（调用pthread_create）就可以开始运行，实际的结果就是阻塞至所有线程中最晚结束的线程返回。

#### start_routine函数

```C
void* thread_function(void*);
```

thread_function形式固定，只接受一个参数，且必须为void\*指针，返回值也必须是void\*指针，都代表一个地址。参数和返回值在使用前按需要的类型进行强制类型转换。若希望使用多个参数，可以将多个参数定义为一个结构体，返回值同理。start_routine = &thread_function，start_routine为一个函数指针，指向thread_function的起始地址。

#### 线程执行的顺序

线程执行没有固定的顺序，在线程被创建完成后，其会被设置为就绪态，等待系统的调度，线程执行的顺序都由系统调度决定。

#### 线程绑定

线程与cpu的绑定关系，由操作系统决定

#### printf是线程安全的吗

是线程安全的。

```C
man 3 printf
```

```C
make D=DEBUG && ./pth_hello 2
```

#### 第一个多线程程序

```C
/* 
    pth_hello.c
    Author: CongKai
*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

/* Shared variables */
int thread_count;

/* Thread function */
void* Thread_work(void* rank); 

/* Main routine */
int main(int argc, char* argv[]){
    if (argc <= 1){ /* Error input */
        printf("Usage: %s <thread_count>\n", argv[0]);
        return 0;
    }
    /* Get number of thread_count */
    thread_count = strtol(argv[1], NULL, 10);

    /* Initialize */

    /* Create threads */
    long thread;
    pthread_t* thread_handles;

    thread_handles = malloc(thread_count*sizeof(pthread_t));

    for (thread = 0; thread < thread_count; thread++)
        pthread_create(&thread_handles[thread], NULL, Thread_work, (void*)thread);

    printf("Thread [main]: Hello!\n");

    for (thread = 0; thread < thread_count; thread++)
        pthread_join(thread_handles[thread], NULL);

    free(thread_handles);

    return 0;
} /* main */

void* Thread_work(void* rank){
    long my_rank = (long)rank;

    printf("Thread [%ld]: Hello!\n", my_rank);

    #ifdef DEBUG
    int my_count = 10000;
    while(my_count--)
        if (my_rank == 0)
            printf("Thread [%ld] %s\n", my_rank, "0000000000000000000000000000000000000000");
        else
            printf("Thread [%ld] %s\n", my_rank, "1111111111111111111111111111111111111111");
    #endif

    return NULL;
} /* Thread_work */

```

> 这是一个最基本的多线程程序，可以作为框架使用

#### 输出

```c
$ ./pth_hello 8      
Thread [0]: Hello!
Thread [1]: Hello!
Thread [2]: Hello!
Thread [3]: Hello!
Thread [4]: Hello!
Thread [5]: Hello!
Thread [6]: Hello!
Thread [7]: Hello!
Thread [main]: Hello!
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
/* Global variables */
#define M 10000
#define N 10000
int thread_count;
double A[M][N];
double y[M];
double x[N];

int main(...){...}

void* Thread_work(void* rank){
    long my_rank = (long)rank;
    int i, j;
    int local_m = (M+thread_count-1)/thread_count;
    int my_first_row = my_rank*local_m;
    int my_last_row = my_first_row+local_m;
    if (my_last_row > M)
        my_last_row = M;

    #ifdef DEBUG
    printf("Thread [%ld]: my_first_row: %d, my_last_row: %d\n", my_rank, my_first_row, my_last_row);
    #endif

    for (i = my_first_row; i < my_last_row; i++){
        y[i] = 0.0;
        for (j = 0; j < N; j++)
            y[i] += A[i][j]*x[j];
    }

    return NULL;
} /* Thread_work */

```

#### 输出

```c
$ time ./pth_mat_vect 1
Thread [0]: my_first_row: 0, my_last_row: 10000
Thread [main]: Using [1] threads, total time: [0.267072] s
0.26user 0.00system 0:00.26elapsed 98%CPU (0avgtext+0avgdata 2052maxresident)k
0inputs+0outputs (0major+738minor)pagefaults 0swaps
$ time ./pth_mat_vect 4
Thread [0]: my_first_row: 0, my_last_row: 2500
Thread [1]: my_first_row: 2500, my_last_row: 5000
Thread [2]: my_first_row: 5000, my_last_row: 7500
Thread [3]: my_first_row: 7500, my_last_row: 10000
Thread [main]: Using [4] threads, total time: [0.088884] s
0.30user 0.00system 0:00.08elapsed 339%CPU (0avgtext+0avgdata 1920maxresident)k
0inputs+0outputs (0major+743minor)pagefaults 0swaps
```

### Pth_sum.c

#### 临界区

更新共享资源的代码段，一次只允许一个线程执行该代码段。

#### 忙等待

```C
y = Compute(my_rank);
while !Condiction; /* 忙等待spin */
/* Critical zone */
```

忙等待可以实现临界区，并且能够保证线程按顺序进入临界区，但是会占用较多的cpu资源。

> 编译优化可以更改某些指令的执行顺序，这可能无法保证忙等待的正确性！

#### 多线程求和

$$
\frac{\pi}{4}=1-\frac{1}{3}+\frac{1}{5}-\frac{1}{7}+···+(-1)^n\frac{1}{2n+1}
$$

#### Pth_sum_busy_0.c

##### 伪代码

```c
void* Thread_work(void* rank){
    long my_rank = (long)rank;
    double factor;
    long long i;
    long long my_n = N/thread_count;
    long long my_first_i = my_n*my_rank;
    long long my_last_i = my_first_i+my_n;

    #ifdef DEBUG
    printf("Thread [%ld]: my_first_i: %lld, my_last_i: %lld\n", my_rank, my_first_i, my_last_i);
    #endif

    if (my_first_i % 2 == 0)
        factor = 1.0;
    else
        factor = -1.0;
    for (i = my_first_i; i < my_last_i; i++, factor = -factor){
        while (flag != my_rank); /* 频繁 Spin */
        sum += factor/(2*i+1);
        flag = (flag+1) % thread_count;
    }

    return NULL;
} /* Thread_work */

```

#### Pth_sum_busy_1.c

##### 伪代码

```c
void* Thread_work(void* rank){
    long my_rank = (long)rank;
    double factor, my_sum = 0.0;
    long long i;
    long long my_n = N/thread_count;
    long long my_first_i = my_n*my_rank;
    long long my_last_i = my_first_i+my_n;

    #ifdef DEBUG
    printf("Thread [%ld]: my_first_i: %lld, my_last_i: %lld\n", my_rank, my_first_i, my_last_i);
    #endif

    if (my_first_i % 2 == 0)
        factor = 1.0;
    else
        factor = -1.0;
    for (i = my_first_i; i < my_last_i; i++, factor = -factor)
        my_sum += factor/(2*i+1);

    #ifdef DEBUG
    printf("Thread [%ld]: my_sum: %.10lf\n", my_rank, my_sum);
    #endif

    while (flag != my_rank); /* 利用局部变量减少 Spin */
    sum += my_sum;
    flag = (flag+1) % thread_count;

    return NULL;
} /* Thread_work */

```

##### 输出

```C
$ time ./pth_sum_busy_0 4
Thread [0]: my_first_i: 0, my_last_i: 25000000
Thread [1]: my_first_i: 25000000, my_last_i: 50000000
Thread [2]: my_first_i: 50000000, my_last_i: 75000000
Thread [3]: my_first_i: 75000000, my_last_i: 100000000
Thread [main]: Using [4] threads, total time: [12.822754] s
Thread [main]: Pi: 3.1415926436
51.27user 0.00system 0:12.82elapsed 399%CPU (0avgtext+0avgdata 1764maxresident)k
0inputs+0outputs (1major+84minor)pagefaults 0swaps
$ time ./pth_sum_busy_1 1
Thread [0]: my_first_i: 0, my_last_i: 100000000
Thread [0]: my_sum: 0.7853981609
Thread [main]: Using [1] threads, total time: [0.257886] s
Thread [main]: Pi: 3.1415926436
0.24user 0.00system 0:00.25elapsed 96%CPU (0avgtext+0avgdata 1976maxresident)k
0inputs+0outputs (0major+79minor)pagefaults 0swaps
$ time ./pth_sum_busy_1 4
Thread [0]: my_first_i: 0, my_last_i: 25000000
Thread [1]: my_first_i: 25000000, my_last_i: 50000000
Thread [2]: my_first_i: 50000000, my_last_i: 75000000
Thread [3]: my_first_i: 75000000, my_last_i: 100000000
Thread [1]: my_sum: 0.0000000050
Thread [2]: my_sum: 0.0000000017
Thread [3]: my_sum: 0.0000000008
Thread [0]: my_sum: 0.7853981534
Thread [main]: Using [4] threads, total time: [0.066411] s
Thread [main]: Pi: 3.1415926436
0.24user 0.00system 0:00.06elapsed 366%CPU (0avgtext+0avgdata 1728maxresident)k
0inputs+0outputs (0major+88minor)pagefaults 0swaps
```
> 多线程程序的正确性没有问题，也得到了性能提升。

> 频繁的spin会带来极大的性能损失！

> 使用03优化编译
> 
> > gcc -DDEBUG -g -Wall -O3 -lpthread -o pth_sum_busy_1 ./pth_sum_busy_1.c
> 
> 可以看到每个线程都能成功打印my_sum但主线程无法结束！

#### 互斥量

忙等待虽然能够简单的实现临界区，但是有许多缺点。另一种实现临界区的方法是使用互斥量。互斥量有且只有两种状态，lock状态和unlock状态。一个互斥量同一时刻只能被一个线程拥有，当它被其他线程拥有时，为lock状态，否则为unlock状态。pthread_mutex_init将一个互斥量初始化，并设置为unlock状态。

#### pthread_mutex_t

> A mutex is a MUTual EXclusion device, and is useful for protecting shared data  structures from concurrent modifications, and implementing critical sections and monitors. A mutex has two possible states: unlocked (not owned by any thread), and locked (owned by one thread). A mutex can never be owned by two different threads simultaneously. A thread attempting to lock a mutex that is already locked by another thread is suspended until the owning thread unlocks the mutex first. 

#### pthread_mutex_init

```C
#include <pthread.h>

int pthread_mutex_init(
    pthread_mutex_t *mutex, /* in */ /* pointer to mutex(pthread_mutex_t) */
    const pthread_mutexattr_t *mutexattr /* in */ /* if NULL, default */
);
```

> pthread_mutex_init initializes the mutex object pointed to by mutex according to the mutex attributes specified in mutexattr. If mutexattr is NULL, default attributes are used instead.

#### pthread_mutex_lock

```c
#include <pthread.h>

int pthread_mutex_lock(
    pthread_mutex_t *mutex /* in */ /* pointer to mutex(pthread_mutex_t) */
);
```

> pthread_mutex_lock locks the given mutex. If the mutex is currently unlocked, it becomes locked and owned by the calling thread, and pthread_mutex_lock returns immediately. If the mutex is already locked by another thread, pthread_mutex_lock suspends the calling thread until the mutex is unlocked.

#### pthread_mutex_unlock

```c
#include <pthread.h>

int pthread_mutex_unlock(
    pthread_mutex_t *mutex /* in */ /* pointer to mutex(pthread_mutex_t) */
);
```

> pthread_mutex_unlock unlocks the given mutex. The mutex is assumed to be locked and owned by the calling thread on entrance to pthread_mutex_unlock. If the  mutex is of the `fast` kind, pthread_mutex_unlock  always  returns  it  to  the unlocked state. If it is of the `recursive` kind, it decrements the locking count of the mutex (number of pthread_mutex_lock operations performed on it by the calling thread), and only when this count reaches zero is the mutex actually unlocked.

#### pthread_mutex_destroy

```c
#include <pthread.h>

int pthread_mutex_destroy(
    pthread_mutex_t *mutex /* in */ /* pointer to mutex(pthread_mutex_t) */
);
```

> pthread_mutex_destroy destroys a mutex object, freeing the resources it might hold. The mutex must be unlocked  on  entrance.  In  the LinuxThreads  implementation,  no resources are associated with mutex objects, thus pthread_mutex_destroy actually does nothing except checking that the mutex is unlocked.

#### demo

```C
/* A shared global variable x can be protected by a mutex as follows: */
int x;
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
/* or pthread_mutex_init(&mut, NULL); */

/* 
All accesses and modifications to x should be bracketed by calls to  
pthread_mutex_lock and pthread_mutex_unlock as follows:
*/
pthread_mutex_lock(&mut);
/* operate on x */
pthread_mutex_unlock(&mut);
```

> 简而言之，临界区就像厕所，互斥量就是厕所的锁。你不想和别人一起上厕所，就要先拿到锁，然后进厕所上锁，上完厕所把锁放回原处，不然别人上不了厕所（笑。

#### Pth_sum_mutex_0.c

##### 伪代码

```C
pthread_muext_t lock;

int main(int argc, char* argv[]){
    ...;
    pthread_mutex_init(&lock);
	..;
}

void* Thread_work(void* rank){
    ...;
    for (i = my_first_i; i < my_last_i; i++, factor = -factor){
        pthread_mutex_lock(&lock); /* Mutex */
        sum += factor/(2*i+1);
        pthread_mutex_unlock(&lock);
    }
    ...;
} /* Thread_work */

```

#### Pth_sum_mutex_1.c

##### 伪代码

```C
pthread_muext_t lock;

int main(int argc, char* argv[]){
    ...;
    pthread_mutex_init(&lock);
	..;
}

void* Thread_work(void* rank){
    ...;
    for (i = my_first_i; i < my_last_i; i++, factor = -factor)
        my_sum += factor/(2*i+1);

    pthread_mutex_lock(&lock); /* Mutex */
    sum += my_sum;
    pthread_mutex_unlock(&lock);
    ...;
} /* Thread_work */
```

##### 输出

```C
$ time ./pth_sum_mutex_0 4 
Thread [0]: my_first_i: 0, my_last_i: 25000000
Thread [2]: my_first_i: 50000000, my_last_i: 75000000
Thread [1]: my_first_i: 25000000, my_last_i: 50000000
Thread [3]: my_first_i: 75000000, my_last_i: 100000000
Thread [main]: Using [4] threads, total time: [3.886149] s
Thread [main]: Pi: 3.1415926436
5.07user 8.70system 0:03.88elapsed 354%CPU (0avgtext+0avgdata 1812maxresident)k
0inputs+0outputs (0major+84minor)pagefaults 0swaps
$ time ./pth_sum_mutex_1 1
Thread [0]: my_first_i: 0, my_last_i: 100000000
Thread [0]: my_sum: 0.7853981609
Thread [main]: Using [1] threads, total time: [0.256746] s
Thread [main]: Pi: 3.1415926436
0.24user 0.00system 0:00.25elapsed 96%CPU (0avgtext+0avgdata 1744maxresident)k
0inputs+0outputs (0major+77minor)pagefaults 0swaps
$ time ./pth_sum_mutex_1 4
Thread [0]: my_first_i: 0, my_last_i: 25000000
Thread [1]: my_first_i: 25000000, my_last_i: 50000000
Thread [2]: my_first_i: 50000000, my_last_i: 75000000
Thread [3]: my_first_i: 75000000, my_last_i: 100000000
Thread [0]: my_sum: 0.7853981534
Thread [1]: my_sum: 0.0000000050
Thread [3]: my_sum: 0.0000000008
Thread [2]: my_sum: 0.0000000017
Thread [main]: Using [4] threads, total time: [0.068719] s
Thread [main]: Pi: 3.1415926436
0.24user 0.00system 0:00.06elapsed 352%CPU (0avgtext+0avgdata 1996maxresident)k
0inputs+0outputs (0major+85minor)pagefaults 0swaps
```

> 多线程下互斥量更快，占用cpu也较少。

### Pth_msg.c

#### 信号量

信号量是一种特殊的无符号整数，实现了原子性的加一（`sem_post`）和原子性的减一（`sem_wait`）。

#### 生产者-消费者模型

生产者不断向仓库中放入商品，消费者不断从仓库中取出商品。在这个模型中，所有生产者和所有消费者之间是并行的，仓库是共享资源区。
1. 生产者和生产者之间是互斥的
2. 消费者和消费者之间是互斥的
3. 生产者和消费者之间是同步的

#### 发送消息

利用信号量实现任意一个进程与其他进程之间的通信。不妨实现一个真正的生产者-消费者模型。仓库使用队列来描述

```C
    生产者  -\  +------------------------+  -\   消费者
    生产者  -->            仓库             -->  消费者
    生产者  -/  +------------------------+  -/   消费者
```

#### 互斥量和信号量

互斥量可以视为弱化版的信号量，其取值只能是0和1。互斥量主要用来解决临界区的互斥问题，而信号量主要用来实现线程间的同步问题。

#### sem_init

```C
#include <semaphore.h>

int sem_init(
    sem_t *sem,        /* in */ /* unnamed  semaphore */
    int pshared,       /* in */ /* 0 shared between threads, else processes */
    unsigned int value /* in */ /* init value */
);
```

> sem_init()  initializes  the  unnamed  semaphore at the address pointed to by sem. The value argument specifies the initial value for the semaphore.

#### sem_destroy

```C
#include <semaphore.h>

int sem_destroy(
    sem_t *sem /* in */ /* unnamed  semaphore */
);
```

> sem_destroy() destroys the unnamed semaphore at the address pointed to by sem.

#### sem_post

```C
#include <semaphore.h>

int sem_post(
    sem_t *sem /* in */ /* initialized sem */
);
```

> sem_post() increments (unlocks) the semaphore pointed to by sem. If the semaphore's value consequently becomes greater than zero, then another process or thread blocked in a sem_wait(3) call will be woken up and proceed to lock the semaphore.

#### sem_wait

```C
#include <semaphore.h>

int sem_wait(
    sem_t *sem /* in */ /* initialized sem */
);
```

> sem_wait() decrements (locks) the semaphore pointed to by sem. If the semaphore's value is greater than zero, then the decrement proceeds, and the function returns, immediately. If the semaphore currently has the value zero, then the call blocks until either it becomes possible to perform the decrement (i.e., the semaphore value rises above zero), or a signal handler interrupts the call.

#### sem_destroy

```C
#include <semaphore.h>

int sem_destroy(
    sem_t *sem /* in */ /* initialized sem */
);
```

> sem_destroy() destroys the unnamed semaphore at the address pointed to by sem.

#### 伪代码

```C
...; /* head, data struct and func define */

struct Queue que;
int thread_count;
sem_t mutex, msg_num;

int main(int argc, char* argv[]){
    ...;
    init_que(&que);
    thread_count = 8;
    sem_init(&mutex, 0, 1);
    sem_init(&msg_num, 0, 0);
    ...;
}

void* Thread_work(void* rank){
    long my_rank = (long)rank;
    struct Message message;
    unsigned int seed = my_rank;

    while(1){ /* This program keeps running */
        if (my_rank < 4){ /* Thread [0,1,2,3]: consumer */
            sem_wait(&msg_num); /* wait for a message */
            peek_que(&que, &message); /* 此处错误，但在当前情况下没有问题，见读写锁 */
            if (message.dst_thread != my_rank)
                sem_post(&msg_num);
            else{
                printf("Thread [%ld]: received a message: \033[31m%s\033[0m\n", my_rank, message.msg);
                sem_wait(&mutex); /* enter critical zone */
                pop_que(&que, NULL);
                sem_post(&mutex); /* leave critical zone */
            }
        }
        else{ /* Thread [4,5,6,7]: producer */
            if (rand_r(&seed)%10 != 9)
                sleep(1);
            else{ /* 1/10 send a message */
                message.dst_thread = rand_r(&seed)%(thread_count/2);
                sprintf(message.msg, "Hello! thread [%ld] , i'm thread [%ld]", message.dst_thread, my_rank);
                sem_wait(&mutex); /* enter critical zone */
                push_que(&que, message);
                sem_post(&mutex); /* leave critical zone */
                printf("Thread [%ld]: sended message to thread [%ld]\n", my_rank, message.dst_thread);
                sem_post(&msg_num); /* produce a message */
            }
        }
    }

    return NULL;
} /* Thread_work */

```

#### 输出

```C
$ timeout 20 ./pth_msg
Using [8] threads default
Thread [4]: sended message to thread [0]
Thread [0]: received a message: Hello! thread [0] , i'm thread [4]
Thread [6]: sended message to thread [3]
Thread [3]: received a message: Hello! thread [3] , i'm thread [6]
Thread [4]: sended message to thread [2]
Thread [2]: received a message: Hello! thread [2] , i'm thread [4]
Thread [6]: sended message to thread [3]
Thread [6]: sended message to thread [1]
Thread [3]: received a message: Hello! thread [3] , i'm thread [6]
Thread [1]: received a message: Hello! thread [1] , i'm thread [6]
Thread [6]: sended message to thread [3]
Thread [3]: received a message: Hello! thread [3] , i'm thread [6]
Thread [4]: sended message to thread [1]
Thread [6]: sended message to thread [0]
Thread [1]: received a message: Hello! thread [1] , i'm thread [4]
Thread [0]: received a message: Hello! thread [0] , i'm thread [6]
Thread [4]: sended message to thread [1]
Thread [1]: received a message: Hello! thread [1] , i'm thread [4]
```

### Pth_barrier.c

#### 路障

顾名思义，阻塞当前线程，直到条件满足。

#### 忙等待和互斥量实现路障

维护一个全局计数器，统计进入忙等待的线程个数，互斥量保证全局计数器的互斥访问，每个线程进入忙等待前让计数器加一。当所有线程都进入忙等待后，退出忙等待。

```C
/* Shared and initialized by the main thread */
int counter;
int thread_count;
pthread_mutex_t barrier_mutex;
...;
void* Thread_work(...){
    ...;
    /* Barrier */
    pthread_mutex_lock(&barrier_mutex);
    counter++;
    pthread_mutex_unlock(&barrier_mutex);
    while(counter < thread_cout>);
    ...;
}
```

>  这种实现无法复用！

#### 信号量实现路障

如果不是最后一个线程就直接进入路障，如果是最后一个线程，将count清零，并为其他所有线程post路障信号量。

```C
/* Shared and initialized by the main thread */
int count; /* init 1 */
int thread_count;
sem_t count_sem; /* init 1 */
sem_t barrier_sem; /* init 0 */
...;
void* Thread_work(...){
    ...;
    /* Barrier */
    sem_wait(&count_sem);
    if (count == thread_count - 1){
        count = 0;
        sem_post(&count_sem);
        for (j = 0; j < thread_count-1; j++)
            sem_post(&barrier_sem);
    }
    else{
        count++;
        sem_post(&count_sem);
        sem_wait(&barrier_sem);
    }
    ...;
}
```

>  这种实现无法复用！

#### 条件变量实现路障

条件变量提供了一组能够方便的实现阻塞和唤醒的接口

#### pthread_cond_t

> A  condition (short for `condition variable`) is a synchronization device that allows threads to suspend execution and relinquish the processors until some predicate on shared data is satisfied. The basic operations on conditions are: signal the condition (when the predicate becomes true), and wait for the condition, suspending the thread execution until another thread signals the condition.
> A condition variable must always be associated with a mutex, to avoid the race condition where a thread prepares to wait on a condition variable and another thread signals the condition just before the first thread actually waits on it.

#### pthread_cond_init

```C
#include <pthread.h>

int pthread_cond_init(
    pthread_cond_t *cond, /* in */ /* condition variable */
    pthread_condattr_t *cond_attr /* in */ /* if NULL, default */
);
```

> pthread_cond_init initializes the condition variable cond, using the condition attributes specified in cond_attr, or default attributes if cond_attr is NULL. The LinuxThreads implementation supports no attributes for conditions, hence the cond_attr parameter is actually ignored.

#### pthread_cond_destroy

```C
#include <pthread.h>

int pthread_cond_destroy(
    pthread_cond_t *cond /* in */ /* condition variable */
);
```

> pthread_cond_destroy destroys a condition variable, freeing the resources it might hold. No threads must be waiting on the condition variable on entrance to pthread_cond_destroy. In the LinuxThreads implementation, no resources are associated with condition variables, thus pthread_cond_destroy actually does nothing except checking that the condition has no waiting threads.

#### pthread_cond_wait

```C
#include <pthread.h>

int pthread_cond_wait(
    pthread_cond_t *cond, /* in */ /* condition variable */
    pthread_mutex_t *mutex /* in */ /* mutex to be unlock */
);
```

> pthread_cond_wait atomically unlocks the mutex (as per pthread_unlock_mutex) and waits for the condition variable cond to be signaled. The thread execution is suspended and does not consume any CPU time until the condition variable is signaled. The mutex must be locked by the calling thread on entrance to pthread_cond_wait. Before returning to the calling thread, pthread_cond_wait re-acquires mutex (as per  pthread_lock_mutex).

#### pthread_cond_broadcast

```C
#include <pthread.h>

int pthread_cond_broadcast(
    pthread_cond_t *cond /* in */ /* condition variable */
);
```

> pthread_cond_broadcast restarts all the threads that are waiting on the condition variable cond. Nothing happens if no threads are waiting on cond.

#### pthread_cond_signal

```C
#include <pthread.h>

int pthread_cond_signal(
    pthread_cond_t *cond /* in */ /* condition variable */
);
```

> pthread_cond_signal restarts one of the threads that are waiting on the condition variable cond. If no threads are waiting on cond, nothing happens. If several threads are waiting on cond, exactly one  is  restarted, but it is not specified which.

#### demo

```C
/* shared */
int counter = 0;
pthread_mutex_t mutex;
pthread_cond_t cond_var;
...;
void* Thread_work(...){
	...;
    /* barrier */
    pthread_mutex_lock(&mutex);
    counter++;
    if (counter == thread_counter){
        counter = 0;
        pthread_cond_broadcast(&cond_var);
    }
    else while(pthread_cond_wait(&cond_var, &mutex) != 0);
    ...;
}
```

#### 伪代码

```C
...;

int main(int argc, char* argv[]){
    ...;
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);
    ...;
}

void* Thread_work(void* rank){
    long my_rank = (long)rank;

    pthread_mutex_lock(&mutex);
    counter++;
    if (counter == thread_count){
        counter = 0;
        printf("Thread [%ld]: broadcast\n", my_rank);
        pthread_cond_broadcast(&cond);
    }
    else{
        printf("Thread [%ld]: waitting\n", my_rank);
        while(pthread_cond_wait(&cond, &mutex) != 0);
    }
    printf("Thread [%ld]: passing\n", my_rank);
    pthread_mutex_unlock(&mutex);

    return NULL;
}
```

pthread_cond_wait 相当于以下操作
```C
pthread_mutex_unlock(&mutex);
wait_on_signal(&cond);
pthread_mutex_lock(&mutex);
```


#### 输出

```C
$ ./pth_barrier 8
Thread [0]: waitting
Thread [1]: waitting
Thread [2]: waitting
Thread [3]: waitting
Thread [6]: waitting
Thread [5]: waitting
Thread [4]: waitting
Thread [7]: broadcast
Thread [7]: pass
Thread [0]: pass
Thread [1]: pass
Thread [2]: pass
Thread [3]: pass
Thread [6]: pass
Thread [5]: pass
Thread [4]: pass
```

### Pth_linklist.c

#### 维护一个共享的复杂数据结构（单向链表）

```C
/*
    +--------+   +---+---+   +---+---+   +---+------+
    | head_p-|-->| 2 | ·-|-->| 5 | ·-|-->| 8 | NULL |
    +--------+   +---+---+   +---+---+   +---+------+
*/
int Member(int value, struct list_node_s** head_p); /* 查找链表中值为value的节点 */
int Insert(int value, struct list_node_s** head_p); /* 在链表的正确位置插入值为value的节点 */
int Delete(int value, struct list_node_s** head_p); /* 删除值为value的节点 */
int Free(struct list_node_s** head_p); /* 释放整个链表 */
```

多个线程能够同时读取一个共享内存单元，只有一个线程同时能写入一个共享内存单元

> pth_msg.c
> ```C
> sem_wait(&msg_num); /* wait for a message */
> peek_que(&que, &message);
> if (message.dst_thread != my_rank)
>     	sem_post(&msg_num);
> else{
>     	sem_wait(&mutex); /* enter critical zone */
>     	pop_que(&que, NULL);
>     	sem_post(&mutex); /* leave critical zone */
>     	printf("Thread [%ld]: received a message: %s\n", my_rank, message.msg);
> }
> ```
> 这样是错误的，问题出在当前线程读（peek_que）的时候，该队列不能被其他线程写（pop_que），但是在此处，该队列只可能被自己写，因此没有问题。

#### 方案一 粗粒度互斥量

互斥访问整个链表

```C
pthread_mutex_lock(&lock);
Member(value);
pthread_mutex_unlock(&lock);
```

> 这样只能串行访问链表！

#### 方案二 细粒度互斥量

互斥访问每个节点

```C
struct list_node_s {
    int data;
    struct list_node_s* next;
    pthread_mutex_t mutex;
}
```

> 频繁lock(unlock)导致性能下降，内存占用增大！

#### 方案三 读写锁

读写锁的思想也很简单，当一个线程读的时候，与任意线程写是互斥的，当一个线程写的时候，和任意线程的读/写是互斥的。
1. 当读锁lock时，阻塞写
2. 当写锁lock时，阻塞读/写

> 读写锁的操作与互斥量几乎一样，此处不再赘述，详细可参考手册 `man pthread_rwlock_init` etc.

```C
pthread_rwlock_rdlock(&rwlock);
Member(value);
pthread_rwlock_unlock(&rwlock);
...;
pthread_rwlock_wrlock(&rwlock);
Insert(value);
pthread_rwlock_unlock(&rwlock);
...;
pthread_rwlock_wrlock(&rwlock);
Delete(value);
pthread_rwlock_unlock(&rwlock);
```

#### 伪代码

##### 互斥量
```C
...;
switch(op){
    case 'M':
    pthread_mutex_lock(&mutex);
    Member(&head, val);
    pthread_mutex_unlock(&mutex);
    break;
    case 'I':
    pthread_mutex_lock(&mutex);
    Insert(&head, val);
    pthread_mutex_unlock(&mutex);
    break;
    case 'D':
    pthread_mutex_lock(&mutex);
    Delete(&head, val);
    pthread_mutex_unlock(&mutex);
    break;
    ...;
}
```

##### 读写锁

```C
switch(op){
    case 'M':
    pthread_rwlock_rdlock(&rwlock);
    Member(&head, val);
    pthread_rwlock_unlock(&rwlock);
    break;
    case 'I':
    pthread_rwlock_wrlock(&rwlock);
    Insert(&head, val);
    pthread_rwlock_unlock(&rwlock);
    break;
    case 'D':
    pthread_rwlock_wrlock(&rwlock);
    Delete(&head, val);
    pthread_rwlock_unlock(&rwlock);
    break;
}
```

#### 输出

```C
$ ./generate_input 95
$ time ./pth_list_mutex
Using [8] threads default
Thread [main]: Using [8] threads, total time: [0.237056] s
0.32user 1.08system 0:00.23elapsed 591%CPU (0avgtext+0avgdata 3280maxresident)k
0inputs+0outputs (0major+121minor)pagefaults 0swaps
$ time ./pth_list_rwlock 
Using [8] threads default
Thread [main]: Using [8] threads, total time: [0.182092] s
0.15user 0.29system 0:00.18elapsed 244%CPU (0avgtext+0avgdata 3552maxresident)k
0inputs+0outputs (0major+125minor)pagefaults 0swaps
```

> 在95%读操作的情况下，读写锁更快。

### Cache，Cache一致性和伪共享

共享变量在同一个Cache line中被多个线程共享，当任一线程修改，为了维护Cache一致性，该Cache line立即失效，造成不必要的访存操作

#### 方案一 Cache对齐

用假元素填充共享数据结构，使其是Cache对齐的

#### 方案二 使用局部变量

使用局部变量，减少操作共享变量的次数，达到减少访存次数的目的。

#### 如何复现伪共享

查看Cache line大小为64字节，每个double为8字节，每行8个double，更改矩阵向量乘法中的矩阵维数，使待计算的double y[8]正好占一整个Cache line，分别用单线程和八线程运行，观察耗时。

#### 输出

```C
$ time ./pth_mat_vect 1 
Thread [0]: my_first_row: 0, my_last_row: 8
Thread [main]: Using [1] threads, total time: [0.017103] s
0.00user 0.00system 0:00.01elapsed 47%CPU (0avgtext+0avgdata 1892maxresident)k
0inputs+0outputs (0major+854minor)pagefaults 0swaps
$ time ./pth_mat_vect 8
Thread [0]: my_first_row: 0, my_last_row: 1
Thread [1]: my_first_row: 1, my_last_row: 2
Thread [3]: my_first_row: 3, my_last_row: 4
Thread [4]: my_first_row: 4, my_last_row: 5
Thread [2]: my_first_row: 2, my_last_row: 3
Thread [5]: my_first_row: 5, my_last_row: 6
Thread [6]: my_first_row: 6, my_last_row: 7
Thread [7]: my_first_row: 7, my_last_row: 8
Thread [main]: Using [8] threads, total time: [0.016683] s
0.06user 0.00system 0:00.01elapsed 370%CPU (0avgtext+0avgdata 3176maxresident)k
0inputs+0outputs (0major+865minor)pagefaults 0swaps
```

> 单线程和八线程耗时几乎一样

> 关于wsl2：当指令有明显的并行性的时候，wsl2的行为有些怪异，hyper-v可能自动调用gpu进行加速

### 线程安全性

> 大多数库函数不是线程安全的

> 错误的程序能产生正确的输出
