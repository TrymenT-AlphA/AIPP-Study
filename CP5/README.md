# 并行程序设计导论 读书笔记

> https://github.com/TrymenT-AlphA/AIPP-Study

## 第五章 用OpenMP进行共享内存编程

OpenMP提供“基于指令”的共享内存API。

### 第一个OpenMP程序

```C
#include <stdio.h>
#include <stdlib.h>
#ifdef _OPENMP
#include <omp.h>
#endif

void Hello(void); /* Thread function */

int main(int argc, char* argv[]){
    /* Get number of threads from command line */
    int thread_count = strtol(argv[1], NULL, 10);

    #pragma omp parallel num_threads(thread_count)
    Hello();

    return 0;
}

void Hello(void){
    #ifdef _OPENMP
    int my_rank = omp_get_thread_num ( );
    int thread_count = omp_get_num_threads ( );
    #else
    int my_rank = 0;
    int thread_count = 1;
    #endif

    printf("Thread [%d]: Hello from thread %d of %d\n", my_rank, my_rank, thread_count);
} /* Hello */
```

引入头文件

> #include <omp.h>

为了用gcc编译这个文件，需要包含`-fopenmp`选项

> gcc -Wall -Werror -O0 -g -fopenmp omp_hello.c -o omp_hello

`#pragma`指令，指示使每个编译程序在保留C和C++语言的整体兼容性时提供不同机器和操作系统特定的功能。OpenMP的指令总是以omp开头

> #pragma omp

parallel指令之后的下一个结构化代码块，将使用`num_threads`个线程来执行（如果可以的话）。在OpenMP中，当程序运行到parallel指令后，原线程继续执行，另外`num_threads-1`个线程会被创建。这`num_threads`个线程集合被称为线程组，原线程被称为主线程（master），额外的线程被称为从线程（slave）。在该例子中，存在一个隐式路障，完成代码块的线程将等待未完成的线程。

> #pragma omp parallel num_threads(thread_count)
> Hello();

OpenMP提供的函数

> int omp_get_thread_num(void);
> int omp_get_num_threads(void);

编译器不一定支持OpenMP，错误处理（为了代码简洁，之后不做错误处理）

> #ifdef _OPENMP
> #include <omp.h>
> #endif

可能的输出

```bash
Thread [2]: Hello from thread 2 of 4
Thread [3]: Hello from thread 3 of 4
Thread [0]: Hello from thread 0 of 4
Thread [1]: Hello from thread 1 of 4
```

### 梯形积分法

如果定义$h = (b-a)/n$，$x_i=a+ih$，$i=0,1,...,n$

$$
S \approx h[f(x_0)/2 + f(x_1) + f(x_2) + ... + f(x_{n-1}) + f(x_n)/2]
$$

对应的串行代码

```C
/* Input: a, b, n */
h = (b-a)/n;
apporx = (f(a)+f(b))/2.0;
for (i = 1; i <= n-1; i++){
    x_i = a + i*h;
    approx += f(x_i);
}
approx = h*approx;
```

第一个OpenMP版本

```C
int main(int argc, char* argv[]){
    ...
    #pragma omp parallel num_threads(thread_count)
    Trap(a, b, n, &global_result);
    ...
}

void Trap(double a, double b, int n, double* global_result_p){
    ...
    for (i = 1; i <= local_n-1; i++){
        x = local_a + i*h;
        my_result += f(x);
    }
    my_result = my_result*h;

    #pragma omp critical
    *global_result_p += my_result;
} /* Trap */
```

使用critical指令声明临界区，实现线程间的互斥访问

> #pragma omp critical
> *global_result_p += my_result;

> ! 默认情况下，所有变量默认在所有线程间共享

使用规约子句

在上一个例子中，我们倾向于将函数定义如下

```C
double Local_trap(double a, double b, int n);
...
global_result += Local_trap(a, b, n);
```

我们可以使用parallel和critical指令来实现

```C
global_result = 0.0;
#pragma omp parallel num_threads(thread_count)
{
    #pragma omp critical
    global_result += Local_trap(double a, double b, int n);
}
```

但是如此一来程序将变成串行执行，改进如下

```C
#pragma omp parallel num_threads(thread_count)
{
    double my_result = Local_trap(double a, double b, int n);
    #pragma omp critical
    global_result += my_result;
}
```

OpenMP提供了另一种机制更方便的实现

```C
int main(int argc, char* argv[]){
    ...
    #pragma omp parallel num_threads(thread_count) \
        reduction(+: global_result)
    global_result += Local_trap(a, b, n);

    return 0;
}
```

reduction使用方法如下

> reduction(\<operator\>, \<reduction variable list\>)



### $\pi$值估计

$$
\frac{\pi}{4}=1-\frac{1}{3}+\frac{1}{5}-\frac{1}{7}+···+(-1)^n\frac{1}{2n+1}
$$

```C
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
```

此处使用了一条新的指令，parallel for指令，可以方便的将一个循环并行化。parallel for指令默认循环变量作用域为私有，其余变量作用域均为公有。default(none)指示所有变量作用域未确定，接下来分别设定每个变量的作用域

数据依赖性，迭代中的计算依赖一个或多个先前迭代的计算结果，典型的如斐波那契数列的计算，此时不能使用parallel for指令并行化。有些情况下能够消除数据依赖性，如此例。

有数据依赖的情况

```C
sum += factor/(2*k+1);
factor = -factor;
```

消除数据依赖性

```C
if (k % 2) factor = -1.0;
else factor = 1.0;
sum += factor/(2*k+1);
```

> ! parallel for指令只能用于一开始就能确定循环次数的典型循环，并且不能有数据依赖

### 更复杂的循环：排序

冒泡排序

```C
for (list_length = n; list_length >= 2; list_length--)
    for (i = 0; i < list_length; i++)
        if (a[i] > a[i+1])
            Swap(&a[i], &a[i+1]);
```

在大多数情况下，对于相对复杂的循环，即使我们能够找到所有的数据依赖，也无法解决它们，另一个和冒泡排序类似但是更容易并行化的算法如下

```C
for (phase = 0; phase < n; phase++)
    if (phase % 2 == 0){
        if (a[i-1] > a[i]) Swap(&a[i-1], &a[i]);
    }
    else{
        if (a[i] > a[i+1]) Swap(&a[i], &a[i+1]);
    }
```

对此，我们能够并行化如下

```C
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
```

我们虽然实现了并行化，但是不断的创建销毁线程，我们希望复用它们

```C
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
```

### 循环调度

默认情况下，parallel for指令总是将循环大致平均分配给每个线程，但是这样并不总是最好的。如果一个函数的执行时间与参数成正比，我们希望以不同的方式进行调度，比如

```C
int f(int x){
    int res = 0;
    for (int i = 0; i < x; i++)
        res += i*i;
    return res;
}
```

此时我们可以这样

```C
#pragma omp parallel for num_threads(thread_count) \
    reduction(+: sum) schedule(static, 1)
for (i = 0; i < n; i++)
    sum += f(i);
```

指定每次调度一次，最后的调度可能是这样的

> Thread [0]: 0, 4, 
> Thread [1]: 1, 5,
> Thread [2]: 2, 6,
> Thread [3]: 3, 7,

### 生产者消费者模型

生产者消费者问题是一个不适合用parallel for指令来并行化的更一般的问题。在之前我们已经了解到，消费者只有在生产者生产出消息之后才能消费，这并不能用一个简单的for解决

消息传递

```C
for (sent_msgs = 0; sent_msgs < send_max; sent_msgs++){
    Send_msg();
    Try_receive();
}
while(!Done())
    Try_receive();
```

发送消息

```C
mesg = random();
dest = random % thread_count;
#pragma omp critical
Enqueue(queue, dest, my_rank, mesg);
```

接受消息

```C
queue_size = enqueue - dequeue;
if (queue_size == 0)
    return;
else if (queue_size == 1){
    #pragma omp critical
    Dequeue(queue, &src, &mesg);
}
else{
    Dequeue(queue, &src, &mesg);
}
```

终止检测

```C
int Done(void){
    queue_size = enqueue - dequeue;
    if (queue_size == 0 && done_sending == thread_count)
        return 1;
    else
        return 0;
}
```

启动

```C
/* 各线程创建队列 */
#pragma omp barrier
/* 尝试发送消息 */
```

更新done_sending

```C
#pragma omp atomic
done_sending++;
```

在之前的实现中，所有临界区都不能同时执行，但是实际上我们并不需要这样，当一个线程读取的时候另一个线程也能读取。我们希望更细致地操控临界区

临界区命名

> #pragma omp critical(name)

锁

> void omp_init_lock(omp_lock_t* lock_p);
> void omp_set_lock(omp_lock_t* lock_p);
> void omp_unset_lock(omp_lock_t* lock_p);
> void omp_destroy_lock(omp_lock_t* lock_p);

> 具体实现参见代码
