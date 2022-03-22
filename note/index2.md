# glob函数解析

glob函数提供目录的解析功能

>NAME
>
>> glob, globfree - find pathnames matching a pattern, free memory from glob()
>
>SYNOPSIS
>
>> #include <glob.h>
>>
>> int glob(const char \*pattern, int flags, int (\*errfunc) (const char *epath, int eerrno), glob_t \*pglob);
>> void globfree(glob_t \*pglob);
>
>1. find pathnames matching a pattern：glob函数的目的是帮助分析一个pattern，这个pattern被翻译成模式或者通配符，flag
>
>

1. main函数的参数跟命令行shell来解析的通配符有什么样的关系？

```c
// main.c

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    printf("argc = %d\n", argc);

    exit(0);
}
```

```shell
> ./main

# result
argc = 1

> ./main hello world 123 900

# result
argc = 5

> ./main *.c

# result
argc = 5
```

`./main *.c`命令的结果是是5而不是2，其实执行这一段指令就相当于执行`./main big.c main.c fsize.c ftype.c`，也就是说从shell环境下是获取不到\*号的，这个\*号会被当前shell解析掉，这个\*被称为通配符

// TODO

# 进程终止方式

进程环境

1. main函数
2. 进程的终止
3. 命令行参数的分析
4. 环境变量
5. C程序的存储空间布局
6. 库
7. 函数跳转
8. 资源的获取与控制



main函数目前来讲是两个参数的形式`int main(int argc, char *argv[])`

进程终止分为正常终止和异常终止

正常终止：

* 从main函数返回：return 0
* 调用exit
* 调用\_exit或\_Exit
* 最后一个线程从其启动例程返回：我们可以把进程看做一个容器，在这个容器当中如果最后一个线程都结束了的话就表示当前进程空间内再也没有线程在运行了，启动例程代表线程本身
* 最后一个线程调用了pthread_exit函数：这个函数相当于线程环境下的exit函数

异常终止：

* 调用abort函数：发送一个sigabrt_abort信号给当前进程，然后结束当前进程顺便得到一个call dword文件
* 接收到一个信号并终止，比如说有一个死循环的程序，用crtl+c结束掉了，这实际上就是接收到一个终止信号
* 最后一个线程对其取消请求作出响应：这种形式本质上来说也是被迫结束，因为线程本身跑得好好的，因为需要所以被迫取消

如果是正常终止则会刷新各种各样的流调用、钩子函数等等。但如果是异常终止，则相当于什么都不做，并且如果程序最后确定是异常终止的话，那么就会导致一些操作没有正常结束，比如关闭文件、释放资源、进程锁

# 钩子函数

>NAME
>
>> exit - cause normal process termination
>
>SYNOPSIS
>
>> #include <stdlib.h>
>>
>> void exit(int status);
>
>1. void exit(int status)：exit(status)的意思是向父进程返回一个值。比如exit(1)意思是向父进程返回值为1的整型数据，
>
>> All functions registered with atexit(3) and on_exit(3) are called, in the reverse order of their  registration.
>
>2. 如果调用exit函数使得进程正常结束，那么被atexit(3) and on_exit(3)函数注册过的函数将会被逆序调用
>
>

1. 虽然这个status是一个整型值，也就是说范围是-2^31~2^31-1，但其实并不能返回这么多种情况。exit所能返回的值是`status & 0xFF`，这个步骤就相当于保留了整型数值的后8位，而原来的整型数本身就是有符号的，所以按位与之后得到的数值也是有符号的，也就是说是一个有符号的char型大小数据，说白了exit能返回256种数据，其值范围是-128~127

> DESCRIPTION
>
> > The  exit()  function  causes  normal  process  termination and the value of status & 0xFF is returned to the parent (see wait(2))

2. 钩子函数调用者：atexit()，钩子函数能解决之前进程结束时候出现的内存来不及释放、进程来不及关闭等等问题

>NAME
>
>> atexit - register a function to be called at normal process termination
>
>SYNOPSIS
>
>> #include <stdlib.h>
>>
>> int atexit(void (*function)(void));
>
>* 注册一个函数，这个函数将会在进程正常终止时被调用
>
>* int atexit(void (*function)(void))：函数参数就是之前说的钩子函数，钩子函数的调用次序是以当初声明顺序的逆序，返回值指的是是否成功挂载钩子函数
>
>

```c
// atexit.c

#include <stdio.h>
#include <stdlib.h>

static void f1(void)
{
    puts("f1() is working");
}

static void f2(void)
{
    puts("f2() is working");
}

static void f3(void)
{
    puts("f3() is working");
}

int main()
{

    puts("Begin......");

    atexit(f1);
    atexit(f2);
    atexit(f3);
    puts("End......");

    exit(0);
}

```

* 26-28：把三个函数挂在钩子上，并没有去调用，直到要执行exit函数之前才会被调用

```markdown
# result

Begin......
End......
f3() is working
f2() is working
f1() is working
```

用伪码来表示钩子函数大的用途：我们之前写过文件打开的代码，如果fd2打开失败的话就需要做close(fd1)操作，假设后面要打开100个文件，当打开到fd100时失败了，则需要关闭从fd1到fd99一共99个文件

```c
fd1 = open();
if(fd1 < 0)
{
    perror();
    exit(1);
}

fd2 = open();
if(fd2 < 0)
{
	close(fd1);
    perror();
    exit(1);
}
// ...


fd100 = open();
if(fd100 < 0)
{
    // close(fd1);
    // close(fd2);
    // ...
    // close(fd99);
    
    perror();
    exit(1);
}
```

所以我们需要使用到钩子函数，我们在每一个文件打开之后都调用一个atexit来调用钩子函数，钩子函数的作用是close(fd)。此时如果fd2打开失败了的话就不用手动调用close(fd1)了，因为在fd2打开失败调用exit(1)的时候会主动逆序调用所有钩子函数。同理如果fd2能成功打开的话就马上挂载一个钩子函数

```c
fd1 = open();
if(fd1 < 0)
{
    perror();
    exit(1);
}

atexit();  // ------> close(fd1)

fd2 = open();
if(fd2 < 0)
{
    perror();
    exit(1);
}

atexit();  // ------> close(fd2)


fd100 = open();
if(fd100 < 0)
{
    perror();
    exit(1);
}

atexit();  // ------> close(fd100)
```

# 进程-进程概念和fork

进程基本知识

1. 进程标识符pid
2. 父子进程的产生fork
3. 进程的消亡及释放资源
4. esec函数族
5. 用户权限及组权限
6. system函数：相当于2，3，4点的封装
7. 进程会计：统计进程的所占资源量
8. 进程时间
9. 守护进程
10. 系统日志



进程标识符的类型叫做pid_t，其是一个有符号的16位整型数，也就是说能同时产生的进程个数位3万多个，但是因为pid_t是一个type define出来的类型，所以具体所占位数在每个不同的机器上都有可能不同

这里有个常用的命令是：ps

>NAME
>
>> ps - report a snapshot of the current processes.
>
>SYNOPSIS
>
>>  ps [options]
>
>EXAMPLES
>
>> ps -e
>>
>> pe -au
>>
>> ......
>
>1. ps axf
>
>2. ps axm：以详细信息进行查看

1. ​		

```shell
> ps axf

# result
PID TTY      STAT   TIME COMMAND
2 ?        S      0:00 [kthreadd]
3 ?        I<     0:00  \_ [rcu_gp]
4 ?        I<     0:00  \_ [rcu_par_gp]
6 ?        I<     0:00  \_ [kworker/0:0H-events_highpri]
7 ?        I      0:00  \_ [kworker/0:1-mm_percpu_wq]
9 ?        I<     0:00  \_ [mm_percpu_wq]
....

```

* PID：进程号

* TTY：所占据终端
* STAT：进程状态
* TIME：消耗时间
* COMMAND：哪个命令触发的该进程

文件描述符的使用策略是优先使用当前可用范围内最小的整型数，而进程标识符pid是顺次向下使用，比如当前进程号为10001，则下一个进程号为10002....即使前面有释放的进程，当前进程号并不会回头去找，而是继续往下查找，把当前进程号消耗一遍，消耗完之后再从头开始

> NAME
>
> > getpid, getppid - get process identification
>
> SYNOPSIS
>
> > #include <sys/types.h>
> > #include <unistd.h>
> >
> > pid_t getpid(void);
> > pid_t getppid(void);
>
> 1. pid_t getpid(void)：获取当前进程的进程号
> 2. pid_t getppid(void)：获取当前进程父进程的进程号
>
> ERRORS
>
> > These functions are always successful.
>
> RETURN VALUE
>
> > On success, the PID of the child process is returned in the parent, and 0 is returned in the child.  On  failure,  -1  is returned in the parent, no child process is created, and errno is set appropriately.
>
> 3. 如果创建成功则返回进程的pid给父进程，如果失败则返回-1，这个pid就是用做分支语句的判断。并且返回给子进程0值

进程的产生fork

>NAME
>
>> fork - create a child process
>
>SYNOPSIS
>
>> #include <sys/types.h>
>> #include <unistd.h>
>>
>> pid_t fork(void);
>
>1. pid_t fork(void)：通过复制当前进程来创建新进程
>
>

1. fork是典型的执行一次返回两次的函数，它的返回是从两个不同的进程中返回，从理论上来讲fork之后一定跟的是一条分支语句，即如果返回值是第一种情况该如何，是第二种情况该如何

> DESCRIPTION
>
> > fork()  creates  a  new process by duplicating the calling process.
>
> 

复制意味着副本跟原本一模一样，一模一样到什么程度，连执行到的代码位置都一样，也就是说创建出来的子进程不会从头再把程序复制一遍；注意理解关键字duplicating，意味着拷贝、一模一样

<img src="index2.assets/image-20220322080728942.png" alt="image-20220322080728942" style="zoom:80%;" />

执行fork函数之后父子进程的区别：fork的返回值不同，pid不同，ppid也不同(因为父进程也是由父进程的父进程fork出来的，所以也是有ppid的)，未决信号和文件锁不继承，资源利用量清零

init进程：实际上当前的启动过程从init进程产生，它起了一个分水岭似的地位，在init进程产生之前内核是相当于一个程序在执行，产生之后就想到与一个库守在后台，即每次出现异常的时候出来解决这个异常；init进程是所有进程的祖先进程，并且进程标识符为1号

# 进程-fork实例

* 实现fork

```c
// fork1.c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
    pid_t pid;

    printf("[%d]:Begin...\n", getpid());

    pid = fork();
    if (pid < 0)
    {
        perror("fork()");
        exit(1);
    }

    if (pid == 0)		// child
    {
        printf("[%d]:Child is working...\n", getpid());
    }
    else				// parent
    {
        // sleep(1);
        printf("[%d]:Father is working...\n", getpid());
    }

    printf("[%d]:End...\n", getpid());
    
    getchar();

    exit(0);
}

```

* 14-18：在创建子进程之后做的第一个工作就是进行条件判断，如果出错当前进程就没有继续的必要
* 13：一旦执行fork语句产生父子进程关系，就相当于代码一式两份了，连执行到的位置都一样，父子进程从fork语句开始分裂，因此子进程不会执行15行之前的代码，父子进程开始分别对16行以后的代码进行判断执行
* 32：让程序等待字符输入，不要让程序结束，以此来观察父子进程的关系

<img src="index2.assets/image-20220322090425818.png" alt="image-20220322090425818" style="zoom:80%;" />

出现倒数二行结果的原因是因为当前所写函数并不完善，终端命令行相较于子进程结果先打印出来了；父进程的进程号为1508，子进程的进程号为1509，在打印出Begin之后有可能看到父进程先打印也有可能看到子进程先打印。永远不要凭空猜测父子进程谁先被调度，因为调度情况是由调度器的调度策略来决定哪个进程先运行

假设一定要让子进程先运行，因为现在调度器决定的是父进程先运行，那就让父进程待一会，最简单的方式就是让父进程执行sleep函数

<img src="index2.assets/image-20220322091027321.png" alt="image-20220322091027321" style="zoom:80%;" />

* 观察父子进程的关系

```c
...
printf("[%d]:End...\n", getpid());

// 让程序等待字符输入，不要让父子程序结束，以此来观察父子进程的关系
getchar();

exit(0);
...
```

<img src="index2.assets/image-20220322091544792.png" alt="image-20220322091544792" style="zoom:80%;" />

调用`ps axf`命令观察进程关系，这个关系是呈现一种阶梯状关系的，当前shell创建fork1进程，在fork1执行过程中又创建一个子进程，这个子进程连名字都是一样的，因为正如之前说的子进程是父进程的一模一样的拷贝。这种阶梯状关系的源头就是init进程，所以并不是init进程直接fork出来所有进程，而是呈一种阶梯状关系不断地fork下去

<img src="index2.assets/image-20220322091834618.png" alt="image-20220322091834618" style="zoom:80%;" />

大家会发现这样的现象，之前代码的结果在终端上只会输出一个begin语句和两个end语句，这是理所应当的，但是当把输出换做是文件而不是终端的话就会出错

```shell
> ./fork1 > /tmp/out
> vim /tmp/out 

# result
[12395]:Begin...
[12395]:Father is working...
[12395]:End...
[12395]:Begin...
[12396]:Child is working...
[12396]:End...
```

而且如果把第11行的换行符删掉的话在终端也会出现输出两个begin语句的现象，对于这个现象的解释是因为没有换行符，所以导致文本就被放到了输入缓冲区中，实际上也不完全是这样，因为即使begin语句后面跟着换行符，在输出到文件去的时候也会导致输出两个begin语句，所以加上换行符并没有解决这个问题

<img src="index2.assets/image-20220322093639364.png" alt="image-20220322093639364" style="zoom:80%;" />

所以解决这类问题的方法就是在fork之前加一句fflush函数来刷新所有成功打开的流，所以这行非常重要。因为加一个换行符只不过是往终端输出，终端是一个标准的输出设备，而标准的输出设备是行缓冲模式，所以换行符会刷新缓冲区，而文件默认是全缓冲模式，在全缓冲模式下换行符已经不代表刷新缓冲区了，只是一个换行的作用。也就是说在begin放入了缓冲区中，还没来得及写入文件的时候就执行fork语句，那么父子进程的缓冲区里就各自有一个begin语句，所以才会输出两次。因为这个begin语句是由父进程创建的，所以即使是在子进程输出的语句，最前面的进程号也显示的是父进程的进程号

```markdown
...
printf("[%d]:Begin...\n", getpid());

fflush(NULL);

pid = fork();
...
```

fork案例的完整代码如下

```c
// fork1.c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
    pid_t pid;

    printf("[%d]:Begin...\n", getpid());

    fflush(NULL);

    pid = fork();
    if (pid < 0)
    {
        perror("fork()");
        exit(1);
    }

    if (pid == 0)
    {
        printf("[%d]:Child is working...\n", getpid());
    }
    else
    {
        printf("[%d]:Father is working...\n", getpid());
    }

    printf("[%d]:End...\n", getpid());

    // getchar();

    exit(0);
}

```

加了fflush之后就可以正常只输出一个begin语句了

```shell
# result

[13761]:Begin...
[13761]:Father is working...
[13761]:End...
[13762]:Child is working...
[13762]:End...
```

* 一个进程实现筛选质数的功能

```c
// primer.c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define LEFT 30000000
#define RIGHT 30000200

int main()
{
    int i, j, mark;

    for (i = LEFT; i <= RIGHT; i++)
    {
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
    }

    exit(0);
}

```

```shell
# result

30000001 is a primer
30000023 is a primer
30000037 is a primer
30000041 is a primer
30000049 is a primer
30000059 is a primer
30000071 is a primer
30000079 is a primer
30000083 is a primer
30000109 is a primer
30000133 is a primer
30000137 is a primer
30000149 is a primer
30000163 is a primer
30000167 is a primer
30000169 is a primer
30000193 is a primer
30000199 is a primer

> ./primer | wc -l		# 查看输出结果有多少行
# result
18

> time ./primer			# 查看进程执行时间
# result
real    0m0.534s
user    0m0.525s
sys     0m0.009s

```

* 多个进程处理筛选质数功能

```c
// primer2.c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define LEFT 30000000
#define RIGHT 30000200

int main()
{
    int i, j, mark;
    pid_t pid;

    for (i = LEFT; i <= RIGHT; i++)
    {

        pid = fork();
        if(pid < 0)
        {
            perror("fork()");
            exit(1);
        }
        if(pid == 0)
        {
            mark = 1;
            for (j = 2; j < i / 2; j++)
            {
                if(i%j == 0)
                {
                    mark = 0;
                    break;
                }
            }
            if(mark)
                printf("%d is a primer\n",i);

        }
    }


    exit(0);
}

```

20：父进程执行i++，并且只管fork出子进程，而子进程执行具体的筛选操作

以上程序在一般的计算机上一定会造成资源报警，因为这段代码其实不是fork了201个子进程，而是当父进程fork出子进程之后，因为子进程的代码和父进程代码是一模一样的，所以也要执行循环语句，因此子进程也会fork出子进程的子进程，这个操作同样要执行200次，以此类推......

```markdown
1 -> 200
	1 -> 199
2 -> 199
	1 -> 198
......
```

所以需要给子进程一个退出的标记，也即调用exit函数

```c
// ...
if(mark)
    printf("%d is a primer\n",i);

exit(0);
// ...
```

添加exit函数之后执行一遍程序，可以观察到显示结果变得无序；执行time命令观察执行时间

```shell
# result
...
30000199 is a primer
30000109 is a primer
30000133 is a primer
30000137 is a primer
30000049 is a primer
30000149 is a primer
...

> time ./primer2	# 多进程执行

# result
real    0m0.063s
user    0m0.000s
sys     0m0.048s

> time ./primer		# 单进程执行

# reuslt
real    0m0.534s
user    0m0.521s
sys     0m0.012s
```

因为涉及到创建、切换进程导致程序执行时间变慢，又因为由于是多进程在处理这段程序所以导致程序执行时间加快，总的来说执行效率是变快的，所以可以看到的是sys时间变慢，因为涉及到进程操作，而real时间也就是总体时间变快

# 进程-init进程和vfork

针对之前获取质数的代码，观察父子进程关系的现象

```c
// primer2.c

int main()
{
    int i, j, mark;
    pid_t pid;

    for (i = LEFT; i <= RIGHT; i++)
    {

        pid = fork();
        if(pid < 0)
        {
            perror("fork()");
            exit(1);
        }
        if(pid == 0)
        {
            mark = 1;
            for (j = 2; j < i / 2; j++)
            {
                if(i%j == 0)
                {
                    mark = 0;
                    break;
                }
            }
            if(mark)
                printf("%d is a primer\n",i);
            
            sleep(1000);
			exit(0);
        }
    }


    exit(0);
}

```

* 31：加上sleep函数

现在有201个子进程，每一个子进程拿到一个待计算的i值，等计算完之后不管是否输出都需要sleep一下，即在这个程序中一定会让父进程先结束，现在执行以下程序primer2并且调用`ps axf`来看下执行结果，应该是有201个子进程，它们的状态都是S(sleep)，并且如果当前进程的树状进程关系是顶格来写的话，那么表示当前进程的父进程是init

```shell
> ps axf
# result

...
20779 pts/2    S      0:00  \_ ./primer2
20780 pts/2    S      0:00  \_ ./primer2
20781 pts/2    S      0:00  \_ ./primer2
20782 pts/2    S      0:00  \_ ./primer2
20783 pts/2    S      0:00  \_ ./primer2
20784 pts/2    S      0:00  \_ ./primer2
20785 pts/2    S      0:00  \_ ./primer2
20786 pts/2    S      0:00  \_ ./primer2
...
```

此时更改sleep函数的位置，把它放到父进程exit前面，那么现在是父进程在执行完程序之前需要sleep等待1000s，而子进程是每一个执行完就关闭

```c
// primer2.c

int main()
{
    int i, j, mark;
    pid_t pid;

    for (i = LEFT; i <= RIGHT; i++)
    {

        pid = fork();
        if(pid < 0)
        {
            perror("fork()");
            exit(1);
        }
        if(pid == 0)
        {
            mark = 1;
            for (j = 2; j < i / 2; j++)
            {
                if(i%j == 0)
                {
                    mark = 0;
                    break;
                }
            }
            if(mark)
                printf("%d is a primer\n",i);

            exit(0);
        }
    }

    sleep(1000);
    exit(0);
}

```

可以发现现在的子进程状态就不是顶格写了，而是以primer2作为父进程。当前父进程的状态是S态，而每个子进程的状态是Z态，在进程关系当中，出现Z态是非常正常的。我们一直在强调一个观点：谁打开谁关闭，谁申请谁释放，要在当前环境下把子进程理解为父进程创建的资源，所以理所应当父进程把资源创建出来最后应该及时释放资源。所以父子进程的关系需要有个"收尸"的过程，如果父进程不收尸的话，那么子进程就像僵尸一样待在内存空间，收尸环节需要利用到wait函数，这个我们之后再讲。等父进程sleep(1000)结束之后，父进程将会正常退出，那么这些僵尸进程就会变成孤儿进程(父亲没了可不就是孤儿了吗)，那么孤儿进程全部都由init进程接管并且释放资源

```shell
> ps axf
# result

...
34405 pts/2    S+     0:00  |   |   |   |   |   \_ ./primer2
34406 pts/2    Z+     0:00  |   |   |   |   |       \_ [primer2] <defunct>
34407 pts/2    Z+     0:00  |   |   |   |   |       \_ [primer2] <defunct>
34408 pts/2    Z+     0:00  |   |   |   |   |       \_ [primer2] <defunct>
34409 pts/2    Z+     0:00  |   |   |   |   |       \_ [primer2] <defunct>
34410 pts/2    Z+     0:00  |   |   |   |   |       \_ [primer2] <defunct>
34411 pts/2    Z+     0:00  |   |   |   |   |       \_ [primer2] <defunct>
34412 pts/2    Z+     0:00  |   |   |   |   |       \_ [primer2] <defunct>
34413 pts/2    Z+     0:00  |   |   |   |   |       \_ [primer2] <defunct>
34414 pts/2    Z+     0:00  |   |   |   |   |       \_ [primer2] <defunct>
...
```

如果程序中出现僵尸态，这个僵尸进程应该是一闪即逝的，因为可能是由于操作系统或者是父进程因为忙于其他处理事务而没来得及给僵尸子进程收尸，但是肯定有一时刻是处于空闲状态的，那么在这个时候就可以给僵尸进程收尸了。僵尸进程大量存在其实占不了多少内存，因为一个僵尸进程其实就是一个结构体，这个结构体中只有诸如PID、退出状态等一系列状态标识，这些状态标识也就几个字节大小

我们再回到第一个sleep函数在循环体内的程序，也就是父进程不执行sleep，而子进程执行sleep，即父进程执行完程序就立马退出，而这些执行了sleep函数的子进程统统变成了孤儿进程等待init进程收尸。但是init进程也在等，等这群孤儿进程sleep执行完，再去执行exit才能去收尸，也就是说不管是父进程还是子进程，只要是一个正常在运行的程序是没有办法被收尸的，如果要强制收尸的话，那就意味着异常终止。所以到目前为止还差一个收尸的手段：父进程创建子进程，子进程干完活之前父进程在等子进程结束任务再去把子进程收尸释放资源，收尸主要做两件事，1. 是否关心子进程的退出状态，如果关心的话那就应该有个能获取子进程状态的功能；2.释放PID，因为这个资源非常宝贵，以为PID之前说过是一个16位的整型数据，是有一个上限的，如果僵尸进程一直占用着PID资源，那么极端点来讲其他进程就有可能获取不到PID了 

写时拷贝技术：

fork是通过复制父进程的方式来产生一个子进程，假设父进程从数据库中读取30w条记录，需要子进程帮忙打印一串“hello world”然后退出。这边关联了30w条数据，那么在执行fork操作产生子进程的时候就相当于memcpy了一遍，辛辛苦苦拷贝了30w条数据然后就只打印一串字符然后还没用上，所以就可以看到fork的成本实际来说是有点高的，但是需要知道的是，这只是fork的一种原始实现：

父进程一旦fork就产生一了一个子进程，父子进程是通过memcpy实现的，比如说现在有一份页表(虚拟地址->物理地址的映射)和一块物理地址，假设父进程当中用到了一个物理块，并且指针指向其中一个页表项，这个指针所关联的物理地址也是其中一块物理地址。然后一fork出子进程的话，子进程里面的数据也指向其中一个页表项，然后在对应的物理地址上也memcpy了一块空间给子进程中的数据

<img src="index2.assets/image-20220322131923075.png" alt="image-20220322131923075" style="zoom:80%;" />

然后就遇到了我们刚才说的情况，所以现在在引入了写实时拷贝技术后的fork函数在父进程调用fork产生子进程的时候，父子进程共用同一块真实的物理地址，如果父子进程对这块空间是只读不写的话，那么还是共用一块；但是如果有一个进程企图去写这块物理空间的话，假设是父进程要去改变里面的数据，那么就把这块物理地址上的数据memcpy一份到别的地址上去，让页表项的对应数据变为另一块物理块的地址，然后去改新数据地址上的数据，该操作的准则是：谁写谁拷贝，父子进程之间除了一开始创建和收尸的关系之外，其他的资源一律不共享(进程是资源分配的最小单位)，两者是完全独立的个体，不是说父亲进程能去子进程里获取资源

<img src="index2.assets/image-20220322132833585.png" alt="image-20220322132833585" style="zoom: 67%;" />

# 进程-wait和waitpid

进程的消亡以资源释放

* wait();
* waitpid();

>NAME
>
>> wait, waitpid, waitid - wait for process to change state
>
>SYNOPSIS
>
>> #include <sys/types.h>
>> #include <sys/wait.h>
>>
>> pid_t wait(int *wstatus);
>>
>> pid_t waitpid(pid_t pid, int *wstatus, int options);
>
>1. wait for process to change state：等待进程状态发生变化
>2. pid_t wait(int *wstatus)：wstatus表示的是进程状态，要的是一个整型指针类型数据，这个wait操作是把当前进程子进程收尸回来的状态值放到这个wstatus变量当中去，因为是要把值找一个变量存储起来，所以传参是指针类型
>3. pid_t waitpid(pid_t pid, int *wstatus, int options)：指定一个PID回收，把退出码存放在wstatus指定的存储空间中，在收尸阶段的配置在options字段设置

在之前我们执行primer函数的时候，结果是命令行先显示出来然后才是逐渐打印结果。本来所有的程序理应是先出结果再出命令行，本节讲解的wait操作其实也就是在解释这个现象

<img src="index2.assets/image-20220322142515872.png" alt="image-20220322142515872" style="zoom:67%;" />

2. wait函数如果成功的话返回的是终止的子进程ID，如果失败则返回-1

>RETURN VALUE
>
>> wait(): on success, returns the process ID of the terminated child; on error, -1 is returned.

其实可以给status传值NULL，因为可以只收尸不关注返回回来的状态，linux提供了若干宏来检测当前进程的退出状态

>DESCRIPTION
>
>> WIFEXITED(wstatus)：子进程是否正常结束，如果正常结束则返回true，否则返回false
>>
>> WEXITSTATUS(wstatus)：返回子进程结束时的状态，这个宏值必须要在WIFEXITED()返回值为真的时候才能检测，即首先得保证子进程是正常结束，然后这个宏就能打印子进程的退出码，即return()或者exit()括号里的值，比如常见的return 0; exit(0)，则退出码就是0
>>
>> WIFSIGNALED(wstatus)：如果子进程是由一个信号终止的，则返回true，也就是说子进程不是正常结束而是被一个信号叫停
>>
>> WTERMSIG(wstatus)：如果WIFSIGNALED为真，则可进一步使用。把导致当前子进程结束的signal number的编号返回
>>
>> ......
>
>

wait函数是没有指向的，即参数中并没有指定回收哪个子进程，只有等到把子进程回收之后通过返回值才知道其具体的PID，所以waitpid就是来解决这个不足的

3. 因为waitpid函数中有pid，所以就可以指定回收具体的子进程，其实waitpid这个函数好用的地方并不在于PID这个参数，而在于options参数。之前的wait函数是死等，即子进程结束之后，进程状态发生了改变，则会有个状态码status来通知父进程，随后父进程才去收尸，如果子进程状态出了问题永远也通知不了父进程的话，那么父进程就会一直处于等待的状态；但如果是waitpid的话就有着options可以设置某些参数来改变死等的状态

> DESCRIPTION
>
> > The value of options is an OR of zero or more of the following constants:
> >
> > WNOHANG     return immediately if no child has exited.
> >
> > WUNTRACED   also return if a child has stopped (but not traced via ptrace(2)).  Status for  traced  children  which  have stopped is provided even if this option is not specified.
> >
> > WCONTINUED (since Linux 2.6.10)
> > also return if a stopped child has been resumed by delivery of SIGCONT.
>
> 

* options是一个位图

* WNOHANG：如果添加了WNOHANG选项的话，即使当前没有子进程结束运行退出，也要立即退出，相当于WNOHANG选项把waitpid这个操作从阻塞变为非阻塞。wait操作死等收一个子进程的尸然后发现返回值，才能直到回收的是哪个子进程；waipid中如果options字段为0就相当于wait操作，如果options字段不为空，比如说是WNOHANG，那么如果指定进程仍在运行也会立刻返回不死等，如果确实已经结束了才会回收子进程。只有在子进程状态发生变化的时候才能执行收尸操作取出退出码然后释放资源，一个进程如果在正常运行是无法收尸的，有了options字段waitpid函数可以是非阻塞的，但是wait函数一定是阻塞的

>DESCRIPTION
>
>> The value of pid can be:
>>
>> < -1   meaning wait for any child process whose process group ID is equal to the absolute value of pid.
>>
>> -1     meaning wait for any child process.
>>
>> 0      meaning  wait  for any child process whose process group ID is equal to that of the calling process at the time of the call to waitpid().
>>
>> 0    meaning wait for the child whose process ID is equal to the value of pid.

PID的值其实也没有这么简单，PID如果大于0则意味着要回收的进程就是函数参数PID指定的进程；如果为0表示回收同组中的其他任意一个子进程；如果为-1则可以回收任意一个子进程；如果小于-1，则回收PID为对应值绝对值的哪个进程，比如返回值为-5，则回收PID为5的子进程

wait就相当于`waitpid(-1, &wstatus, 0)`的封装，-1表示回收任意一个子进程，状态码放入wstatus地址空间，没有options配置

* 添加收尸手段之后的primer程序

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define LEFT 30000000
#define RIGHT 30000200

int main()
{
    int i, j, mark;
    pid_t pid;

    for (i = LEFT; i <= RIGHT; i++)
    {

        pid = fork();
        if (pid < 0)
        {
            perror("fork()");
            exit(1);
        }
        if (pid == 0)
        {
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

            // sleep(1000);
            exit(0);
        }
    }

    for (i = LEFT; i <= RIGHT; i++)
        // wait(&st);
        wait(NULL);
    
    // sleep(1000);

    exit(0);
}

```

* 43-45：如果想要获取状态码则把状态写入st地址，如果不关心状态，则可以传入NULL

添加了收尸手段之后程序的结构就发生变化了，父进程fork了201个子进程，则等待子进程执行完毕，并且等待201次。当201个子进程陆续的exit回来，父进程就等在那边一个个回收

```shell
liangruuu@liangruuu-virtual-machine:~/study/linuxc/code/process_basic$ ./primer2
# result
30000001 is a primer
30000023 is a primer
30000041 is a primer
30000037 is a primer
30000049 is a primer
30000071 is a primer
30000079 is a primer
30000109 is a primer
30000059 is a primer
30000163 is a primer
30000137 is a primer
30000133 is a primer
30000167 is a primer
30000083 is a primer
30000193 is a primer
30000149 is a primer
30000199 is a primer
30000169 is a primer
```

# 进程-进程分配之交叉分配法实现

之前的筛选质数程序其实是有一点问题的，201个待计算的数值对应着创建了201个进程进行计算，其实这个设计思路不太好，因为假设在程序当中把left和right的值不停地放大，那么待计算的数也会一直增大到很大的数值，如果每一个待计算的数值都要用一个子进程来做的话就会出现一个问题：因为pid_t类型是有一个大小上限的，所以到底能否创建这么多个子进程？因此需要从别的角度去设计代码

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define LEFT 30000000
#define RIGHT 30000200

int main()
{
    int i, j, mark;
    pid_t pid;

    for (i = LEFT; i <= RIGHT; i++)
    {

        pid = fork();
        if (pid < 0)
        {
            perror("fork()");
            exit(1);
        }
        if (pid == 0)
        {
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

            exit(0);
        }
    }

    for (i = LEFT; i <= RIGHT; i++)
        wait(NULL);

    exit(0);
}

```

不管现在有201个质数还是多少个质数都统一找N个进程去执行，N是任意非负整数，并且确定系统能创建这么多个进程

1. 分块法：假如N是3，把201个待计算的数分成3份，每个进程来分担一部分数据的计算(就算待分数据个数无法被进程个数所整除也无所谓，无非就是某个进程多分一点数，某个进程少分一点数，其实无所谓)，看样子确实是把201个数给均分了，但其实每组任务是有轻重的，也就是说任务分配地不平均，直观来看进程1计算的都是小一点的数，而进程3计算的都是大一点的数，但其实是进程1的负载最重，因为比如说2~20之间的质数个数要比20~40之间的质数要多，并且20~40之间的质数要比200~400之间的质数要多，看出来了吗？质数一定是在小数范围内的分布多一些，因此会分配更多的质数给进程1，所以进程1的负载会是最重的，所以分块法并不是解决问题的最佳方法

<img src="index2.assets/image-20220322164036992.png" alt="image-20220322164036992" style="zoom:80%;" />

2. 交叉分配法：交叉分配法就涉及到一些随机性了，201个数会随机分配给这三个进程中的其中一个。在一般的模型中如果能用分块法或交叉分配法进行并发的话优先使用交叉分配法，但是在这个模型上不是，这个模型太特殊了，用1、2两种方法解决这个问题都不是特别好。因为除3余0的数一定分配给第一个进程，除3余1的数一定分配给第二个进程，除3余2的数一定分配给第三个进程，可以看到第一个进程所处理的数肯定不是质数，因此会导致某个进程一个质数都拿不到

<img src="index2.assets/image-20220322164731608.png" alt="image-20220322164731608" style="zoom: 67%;" />

3. 池类算法：上游有一个任务，下游有N个任务，中间建立一个通讯的机制，上游的任务是把201个待计算的数值以此往中间容器传送，下游的三个进程去抢任务，抢到质数的进程就去计算，没抢到质数的进程就把数"扔了"然后继续去抢。当然这个池跟进程池是不同的，这个池只是一个容器，是一个任务池，然后把任务不停往这放。这种方式的好处就是能者多劳，拿到非质数就多算几个，拿到质数就少算几个，这里的随机性就完全取决于进程调度器的调度策略。这里先不实现池类算法，因为会涉及到竞争，比如某个进程抢到一个数，那么其他进程是怎么知道这个数已经被抢了的呢？怎么让别的进程知道该进程做了什么任务，等到讲到进程中通信的时候在去实现

<img src="index2.assets/image-20220322165503769.png" alt="image-20220322165503769" style="zoom:67%;" />

* 用交叉分配法实现筛选质数

```c
// primer.c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define LEFT 30000000
#define RIGHT 30000200
#define N 3

int main()
{
    int i, j, n, mark;
    pid_t pid;

    for (n = 0; n < N; n++)
    {
        pid = fork();
        if (pid < 0)
        {
            perror("fork()");
            exit(1);
        }
        if (pid == 0)
        {
            for (i = LEFT + n; i <= RIGHT; i += N)
            {
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
                    printf("[%d]:%d is a primer\n", n, i);
            }
            exit(0);
        }
    }

    for (n = 0; n < N; n++)
        wait(NULL);

    exit(0);
}
```

* 20：父进程专注于fork子进程，子进程去执行筛选质数的任务
* 21-25：其实这段代码是不严谨的，之前提到过假设前两个fork操作成功，在执行到第3个fork操作的时候失败了，如果是父进程的角色的话报错之后先别调用exit结束，因为有两个子进程已经被创建了还没来得及回收，从而使得子进程变成了僵尸进程，最后变成了孤儿进程从而被init进程接管。所以严谨一点的话先报错然后调用一个循环去回收之前创建的子进程的资源然后再调用exit
* 28-44：exit(0)放到了循环体外，目的是为了让子进程把活干完，因为每一个循环都是对应着一个子进程需要完成的任务；循环起始位置也不再固定位LEFT，因为该交叉分配法的原理，不同的进程起始位置不同，并且每次循环所增加的步长也是N
* 46-47：fork了3次，所以要收3次尸

```shell
liangruuu@liangruuu-virtual-machine:~/study/linuxc/code/process_basic$ ./primerN 
[2]:30000023 is a primer
[1]:30000001 is a primer
[2]:30000041 is a primer
[1]:30000037 is a primer
[2]:30000059 is a primer
[1]:30000049 is a primer
[2]:30000071 is a primer
[1]:30000079 is a primer
[2]:30000083 is a primer
[1]:30000109 is a primer
[2]:30000137 is a primer
[1]:30000133 is a primer
[2]:30000149 is a primer
[1]:30000163 is a primer
[1]:30000169 is a primer
[2]:30000167 is a primer
[1]:30000193 is a primer
[1]:30000199 is a primer
```

可以看到所拿数值是3的倍数的第一个子进程永远拿不到任何质数

































