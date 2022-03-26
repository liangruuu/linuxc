# 线程-线程的概念和线程标识

1. 线程的概念
2. 线程的创建、线程终止、线程的取消选项、栈清理
3. 线程同步
4. 线程相关的属性线程同步的属性
5. 线程与信号、线程与fork、线程重入



之前讲到一个会话session的概念，当时讲这个概念的时候是为了实现一个守护进程，当前的一次成功的shell登录其实就是一个会话的产生，会话是用来承载进程组的，会话中可以有一个或多个进程组，每个进程组里有一个或多个进程，一个进程中有一个或多个线程

之前写的都是单进程或者单线程的程序，包括信号机制在内，虽然信号已经有了并发的概念，但是在处理信号处理函数st_handle的同时，被打断的现场肯定是不会去运行的，这就相当于被阻塞了，即没有出现多个分支并行去执行任务的情况

线程说白了就是一个正在运行的函数，之前写的函数，不管是main函数还是一个常规函数在运行，进程当中始终至少会有一个函数在运行，但是到目前为止还没有办法让多于一个的函数在进行运行，线程机制就能使得多个函数能同时运行

一个进程中至少会有一个线程，也就是说一个进程空间中最少会有一个函数在运行。假如说就用一个main函数来进行编程的话，那其实程序当中就有一个main线程在运行的，很多人其实在接触到函数这个概念的时候通常会人为main函数是一个特殊的函数，进程以main作为入口，并且以main作为出口，这个认知是正确的，但无非是我们人为的规定，因为我们必须制定一个入口和出口，其实我们可以不以main函数作为入口和出口。有人习惯把main函数作为主函数，但是到了线程阶段，main函数就被视作为main线程而非进程，并且是main线程而非主线程在运行，因为多个线程之间是兄弟关系，不分先后，而非有主次之分的关系，即使当前线程创建出了一个兄弟线程，这个兄弟线程也可以反过来给当前线程收尸或取消掉

多个线程的内存空间是共享的，在同一个进程空间内如果运行着多个线程，那么这些线程进行通信的方法会很简单，因为一个进程的空间是在32位环境下是4G的虚拟地址空间，有两个函数正在同时运行，如果想让他们两个线程通信的话就不需要借助外来机制了，可以直接在程序中创建一个全局变量，这个全局变量是这两个函数同时能获取的，这就是一个典型的通信机制

线程有很多不同的标准，现在用的比较多的是posix线程，posix线程是一套标准而非实现。比如线程为pthread_t类型，p表示的是posix，thread表示的是线程，pthread_t表示的就是posix线程下的线程标识，而pthread_t是什么类型是不确定的，有可能是整型，有可能是结构体...因为posix只是一个标准而非实现，pthread_t就相当于pid_t类型，在不同环境下代表的类型是不同的，但是如果要申明一个线程就必须用pthread_t来表示

在终端输出`ps axm`命令

```shell
liangruuu@liangruuu-virtual-machine:~/study/linuxc$ ps axm

# result
5720 ?        -      0:00 [kworker/6:1-events]
- -        I      0:00 -
5762 ?        -      0:00 [kworker/8:0-events]
- -        I      0:00 -
5763 ?        -      0:00 /usr/lib/firefox/firefox -contentproc -childID 25 -isForBrowser -prefsLen 8832 -prefMapSize 247345 -jsI
- -        Sl     0:00 -
- -        Sl     0:00 -

```

可以看到每一个进程下面都有一条--记录，甚至一些进程下面有多个--，--所在行代表的就是一个线程，因为一个进程空间至少有一个线程，所以每个进程下面至少有一个--

再使用`ps ax -L`命令查看进程和线程的关系，-L表示以linux模式查看，LWP是轻量级进程，这里可以理解为线程，可以看到相同进程下有不同的线程，线程号其实就是用进程号来进行描述的	

```shell
liangruuu@liangruuu-virtual-machine:~$ ps ax -L
    PID     LWP TTY      STAT   TIME COMMAND
   1422    1423 ?        Ssl    0:00 /usr/libexec/gvfs-afc-volume-monitor
   1422    1424 ?        Ssl    0:00 /usr/libexec/gvfs-afc-volume-monitor
   1422    1426 ?        Ssl    0:00 /usr/libexec/gvfs-afc-volume-monitor
   1428    1428 ?        Ssl    0:00 /usr/lib/upower/upowerd
   1428    1435 ?        Ssl    0:00 /usr/lib/upower/upowerd
   1428    1436 ?        Ssl    0:00 /usr/lib/upower/upowerd


```

# 线程-线程创建

>NAME
>
>> pthread_equal - compare thread IDs
>
>SYNOPSIS
>
>> #include <pthread.h>
>>
>> int pthread_equal(pthread_t t1, pthread_t t2);
>>
>> Compile and link with -pthread.
>
>1. int pthread_equal(pthread_t t1, pthread_t t2)：比较两个线程ID，因为pthread这个类型是不确定的，所以不能用等于号来表示两个进程的关系，如果相同返回非零值，否则返回零
>
>2. 在线程阶段编译任何和线程相关的程序都需要使用makefile文件，并且在makefile文件中加上-pthread.设置

2. makefile文件设置

```makefile
CFLAGS+=-pthread
LDFLAGS+=-pthread
```

>NAME
>
>> pthread_self - obtain ID of the calling thread
>
>SYNOPSIS
>
>> #include <pthread.h>
>>
>> pthread_t pthread_self(void);
>>
>> Compile and link with -pthread.
>
>1. pthread_t pthread_self(void)：获取当前线程的ID，类似于于进程的getpid函数

线程的创建

>NAME
>
>> pthread_create - create a new thread
>
>SYNOPSIS
>
>> #include <pthread.h>
>>
>> int pthread_create(pthread_t \*thread, const pthread_attr_t \*attr, void \*(\*start_routine) (void \*), void \*arg);
>>
>> Compile and link with -pthread.
>
>1. pthread_create：第一个参数是pthread_t类型的指针，不是让用户提供指针，而是让用户提供一个存放pthread_t数据的地址，然后把创建成的线程标识回填到这个地址空间上；第二个参数pthread_attr_t表示线程的属性，在多数情况下可以使用默认的NULL即可；第三个参数是一个函数指针，表示并行执行的兄弟线程；第四个参数就是传递给第三个函数指针的参数

1. 成功创建返回值为0，否则返回一个error number

> RETURN VALUE
>
> > On success, pthread_create() returns 0; on error, it returns an error number, and the contents of *thread are undefined.

* 使用pthread_create创建线程

```c
// create1.c

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

static void *func(void *p)
{
    puts("Thread is working...");
    return NULL;
}

int main()
{
    pthread_t tid;
    int err;

    puts("Begin!");

    err = pthread_create(&tid, NULL, func, NULL);
    if (err)
    {
        fprintf(stderr, "pthread_create():%s\n", strerror(err));
        exit(1);
    }

    puts("End!");

    exit(0);
}

```

















