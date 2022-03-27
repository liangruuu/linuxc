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

```shell
liangruuu@liangruuu-virtual-machine:~/study/linuxc/code/parallel/thread/posix$ ./create1 

# result
Begin!
End!
```

按道理创建出来的兄弟线程会打印“Thread is working...”这句话，但是结果并没有。因为线程的调度取决于调度器的调度策略，当前main线程打印begin，然后创建一个线程，然后打印end，即被创建的线程还没来得及被调度执行打印函数的时候main线程就执行了exit(0)了，exit就是进程正常终止的方式之一

# 线程-线程终止和栈清理

线程的终止包括三种方式：

1. 线程从启动例程返回，返回值为线程的退出码
2. 线程可以被统一进程中的其他线程取消
3. 线程调用pthread_exit()函数，相当于进程阶段的exit函数

统一进程中的最后一个线程结束运行后，这个进程也会结束运行

>NAME
>
>> pthread_exit - terminate calling thread
>
>SYNOPSIS
>
>> #include <pthread.h>
>>
>> void pthread_exit(void *retval);
>>
>> Compile and link with -pthread.
>
>1. void pthread_exit(void *retval)

在刚才的create1.c函数中使用`return NULL`来表示线程的退出，那么现在使用`pthread_exit(NULL)`来替换，当前看好像没有什么区别，但是之后要进行一些进程栈的清理，如果使用return null就相当于不能主动做线程栈的清理了

```c
static void *func(void *p)
{
    puts("Thread is working...");
    pthread_exit(NULL);
}
```

既然线程线程会被终止，那么就会有一个函数来进行线程终的收尸

>NAME
>
>> pthread_join - join with a terminated thread
>
>SYNOPSIS
>
>> #include <pthread.h>
>>
>> int pthread_join(pthread_t thread, void **retval);
>>
>> Compile and link with -pthread. 
>
>1. int pthread_join(pthread_t thread, void **retval)：第一个参数指的是要被收尸的线程；第二个参数如果为NULL的话就表示只收尸不关心状态，如果要把收尸的状态加以查看的话就传一个void\*类型变量的地址，可以发现这个参数跟pthread_exit函数的参数是一样的变量名，所以这个pthread_join函数的retval变量就是pthread_exit函数的参数

1. 这个函数相当于进程中的wait函数，但是跟wait函数不一样的是，wait函数不需要指定进程，而pthread_join需要指定所要回收的线程

使用这两个函数来重构之前的代码

```c
// create.c


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

static void *func(void *p)
{
    puts("Thread is working...");
    pthread_exit(NULL);
}

int main()
{
    pthread_t tid;
    int err;

    puts("Begin!");

    err =  pthread_create(&tid, NULL, func, NULL);
    if(err)
    {
        fprintf(stderr, "pthread_create():%s\n", strerror(err));
        exit(1);
    }

    pthread_join(tid, NULL);

    puts("End!");

    exit(0);
}

```

* 29：回收被创建的线程tid，第二个参数为空，因为pthread_exit的参数为空，pthread_join相当于在等待pthread_exit函数的返回值

```shell
liangruuu@liangruuu-virtual-machine:~/study/linuxc/code/parallel/thread/posix$ ./create

# result
Begin!
Thread is working...
End!
```

先打印Begin!，然后调用pthread_join函数等待创建的线程执行完成，因为线程的中pthread_exit函数的返回值为NULL，即没有返回值，所以这里的第二个参数设置为NULL

栈清理有两个函数需要掌握：

1. pthread_cleanup_push();
2. pthread_cleanup_pop();

这两个函数相当于钩子函数，pthread_cleanup_push相当于往钩子上挂函数，pthread_cleanup_pop相当于从钩子上取函数

>NAME
>
>> pthread_cleanup_push, pthread_cleanup_pop - push and pop thread cancellation clean-up handlers
>
>SYNOPSIS
>
>> #include <pthread.h>
>>
>> void pthread_cleanup_push(void (\*routine)(void \*), void \*arg);
>> void pthread_cleanup_pop(int execute);
>>
>> Compile and link with -pthread.
>
>1. void pthread_cleanup_push(void (\*routine)(void \*), void \*arg)：第一个参数为函数指针；第二个参数为函数指针对应函数传入的参数
>2. void pthread_cleanup_pop(int execute)：参数execute指的是在拿到钩子函数的时候决定钩子函数是否被调用，如果为真则调用，如果为佳假则不被调用

```c
// cleanup.c 

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

static void cleanup_func(void *p)
{
    puts(p);
}

static void *func(void *p)
{
    puts("Thread is working...");
    pthread_cleanup_push(cleanup_func, "cleanup:1");
    pthread_cleanup_push(cleanup_func, "cleanup:2");
    pthread_cleanup_push(cleanup_func, "cleanup:3");

    pthread_cleanup_pop(1);
    pthread_cleanup_pop(1);
    // 不调用
    pthread_cleanup_pop(0);

    puts("push over!");

    pthread_exit(NULL);
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

    pthread_join(tid, NULL);

    puts("End!");

    exit(0);
}

```

```shell
liangruuu@liangruuu-virtual-machine:~/study/linuxc/code/parallel/thread/posix$ ./clearup

# result
Begin!
Thread is working...
cleanup:3
cleanup:2
push over!
End!
```

* 16-18：挂载钩子函数
* 20-23：把钩子函数从栈中弹出，传参为1表示执行弹出的钩子函数，传参为0表示不执行弹出的钩子函数，所以可以看到cleanup:1语句没有被输出，而且执行顺序是按照挂载的顺序逆序执行

# 线程的取消

一个线程在执行的过程中一定会用到线程的取消，多线程可以把任务分摊开，比如有一个深度为1W的二叉树，要在这颗二叉树中查找一个指定节点，从第三层开始用四个线程每个线程负责找一颗子树，则会有一个线程找到指定的那个节点，那么其他线程就没有必要继续找下去，则这个时候就需要把其他三个线程给取消了，如果是一个正在运行的线程是没有办法收尸的，所以就需要先取消再收尸

>NAME
>
>> pthread_cancel - send a cancellation request to a thread
>
>SYNOPSIS
>
>> #include <pthread.h>
>>
>> int pthread_cancel(pthread_t thread);
>>
>> Compile and link with -pthread.
>
>1. int pthread_cancel(pthread_t thread)：给一个线程发送取消请求，参数就是指定线程

1. 先使用pthread_cancel函数取消线程，再使用pthread_join进行线程资源的回收

有以下代码：

```c
fd1 = open();
if(fd1 < 0)
{
    perror();
	exit(1);
}
//      -----------> cleanup_push -> close(fd1)

fd2 = open();
if(fd2 < 0)
{
    perror();
	exit(1);
}
//      -----------> cleanup_push -> close(fd2)

close(fd1);
close(fd2);
```

这是一个线程，当前线程在执行打开两个文件的操作，当执行完打开第一个文件的时候收到了cancel线程的请求，则到此为止fd1还没来得即close，解决办法就是通过pthread_cleanup_push挂载一个钩子函数，这个钩子函数的作用就是执行close操作，钩子函数会在执行pthread_cleanup_pop的时候被调用

但这是一种非常理想的情况，如果是在还没来得及挂载钩子函数的时候就被别人cancel了怎么办？

取消由取消选项来做主，取消有两种状态：允许、不允许，如果设置为允许取消的话，又分为异步取消和推迟取消，其中推迟取消是默认的设置，推迟指的是推迟到cancel点来进行响应。cancel点：posix定义的cancel点都是可能引发阻塞的系统调用，而非系统调用的内容是否是cancel点在不同平台上可能不同，所以刚才举的例子`还没来得及挂载钩子函数的时候就被别人cancel了`的情况是不存在的，即可以在挂载函数之前收到cancel信号，但是不会取消，因为cleanup_push不是cancel点，即cleanup_push不阻塞系统调用，所以会继续执行直到执行到下面的open函数，因为open是阻塞系统调用，所以cancel会在这个点去响应

# 线程-线程竞争实例

* 使用多线程实现筛选质数的功能

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>

#define LEFT 30000000
#define RIGHT 30000200
#define THRNUM (RIGHT - LEFT + 1)


static void *thr_prime(void *p)
{
    int i, j, mark;
    i = *(int *)p;
    mark = 1;
    for (j = 2; j < i / 2; j++)
    {
        if (i % j == 0)
        {
            mark = 0;
            break;
        }
    }
    if (mark)
        printf("%d is a primer\n", i);

    pthread_exit(NULL);
}

int main()
{
    int i, err;
    pthread_t tid[THRNUM];

    for (i = LEFT; i <= RIGHT; i++)
    {

        err = pthread_create(tid + (i - LEFT), NULL, thr_prime, &i);
        if (err)
        {
            fprintf(stderr, "pthread_create():%s\n", strerror(err));
            exit(1);
        }
    }

    for (i = LEFT; i <= RIGHT; i++)
        pthread_join(tid[i - LEFT], NULL);

    exit(0);
}

```

* 11：使用201个线程执行操作
* 33：main线程创建其兄弟线程去执行判断质数与否的功能
* 49-50：创建了多少线程就需要调用多少次join操作进行线程资源的回收

```shell
liangruuu@liangruuu-virtual-machine:~/study/linuxc/code/parallel/thread/posix$ ./primer_th 
    
# result
30000059 is a primer
30000059 is a primer
30000163 is a primer
30000163 is a primer
30000079 is a primer
30000037 is a primer
30000133 is a primer
30000163 is a primer
30000037 is a primer
```

这个执行结果出现的原因是线程间的竞争关系，可以看这行代码`pthread_create(tid + (i - LEFT), NULL, thr_prime, &i);`，在创建所有线程的时候给每个线程传的参数都是i的地址，也就是说thr_prime函数同时在运行这201份，这201个指针p实际上都指向了变量i，即还没有确认创建出来的第一个线程把i拿走就在创建第二个线程了，所以说错误就错在用地址传参，i本身就一块地址，然后找了201个指针来指向这块空间，肯定会出现竞争的

<img src="index4.assets/image-20220327100421468.png" alt="image-20220327100421468" style="zoom:80%;" />

如果问题出在201个指针公用一块的地址的话，那就不传地址`pthread_create(tid + (i - LEFT), NULL, thr_prime, i);`，用传值来替换，但是因为pthread_create函数的第四个参数的类型需要是地址，所以使用一种丑陋的方式强制类型转换`pthread_create(tid + (i - LEFT), NULL, thr_prime, (void *)i);`

```c
static void *thr_prime(void *p)
{
    int i, j, mark;
    i = (int)p;
    mark = 1;
    for (j = 2; j < i / 2; j++)
    {
        if (i % j == 0)
        {
            mark = 0;
            break;
        }
    }
    if (mark)
        printf("%d is a primer\n", i);

    pthread_exit(NULL);
}

int main()
{
    int i, err;
    pthread_t tid[THRNUM];

    for (i = LEFT; i <= RIGHT; i++)
    {
        err = pthread_create(tid + (i - LEFT), NULL, thr_prime, (void *)i);
        if (err)
        {
            fprintf(stderr, "pthread_create():%s\n", strerror(err));
            exit(1);
        }
    }

    for (i = LEFT; i <= RIGHT; i++)
        pthread_join(tid[i - LEFT], NULL);

    exit(0);
}

```

<img src="index4.assets/image-20220327101122374.png" alt="image-20220327101122374"  />

```shell
liangruuu@liangruuu-virtual-machine:~/study/linuxc/code/parallel/thread/posix$ ./primer_th 

# result
30000083 is a primer
30000059 is a primer
30000037 is a primer
30000049 is a primer
30000079 is a primer
30000041 is a primer
30000109 is a primer
30000001 is a primer
30000137 is a primer
30000133 is a primer
30000071 is a primer
30000199 is a primer
30000193 is a primer
30000023 is a primer
30000163 is a primer
30000167 is a primer
30000169 is a primer
30000149 is a primer
```

结果为什么会冲突，是因为201个指针指向了同一个i的地址，那其实如果201个指针指向的是201块空间不就好了，所以可以构造一个结构体

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>

#define LEFT 30000000
#define RIGHT 30000200
#define THRNUM (RIGHT - LEFT + 1)

struct thr_arg_st
{
    int n;
};

static void *thr_prime(void *p)
{
    int i, j, mark;
    i = ((struct thr_arg_st *)p)->n;
    // free(p);
    mark = 1;
    for (j = 2; j < i / 2; j++)
    {
        if (i % j == 0)
        {
            mark = 0;
            break;
        }
    }
    if (mark)
        printf("%d is a primer\n", i);

    pthread_exit(p);
}

int main()
{
    int i, err;
    pthread_t tid[THRNUM];
    struct thr_arg_st *p;
    void *ptr;

    for (i = LEFT; i <= RIGHT; i++)
    {
        p = malloc(sizeof(*p));
        if (p == NULL)
        {
            perror("malloc()");
            exit(1);
        }
        p->n = i;
        err = pthread_create(tid + (i - LEFT), NULL, thr_prime, p);
        if (err)
        {
            fprintf(stderr, "pthread_create():%s\n", strerror(err));
            exit(1);
        }
    }

    for (i = LEFT; i <= RIGHT; i++)
    {
        pthread_join(tid[i - LEFT], &ptr);
        free(ptr);
    }

    exit(0);
}

```

* 45：201个线程每一个都拿到一块独立的空间则需要考虑使用动态分配，而指针变量指向一个结构体(其实可以直接malloc一块int大小的空间，这里采用结构体是为了以后可能的功能拓展，两种方式均可)
* 22，34，53，64，65：因为需要把malloc申请的内从空间释放掉，通过pthread_exit函数把处理过的变量p返回，并且通过pthread_join接收，从而做到释放空间







