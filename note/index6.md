# 管道实例-池类算法

之前写过一个筛选质数的程序，其实使用之前的实现方法效率是比较低的，因为存放任务的num空间有点小，上游线程负责提供任务之后就进入阻塞等待直到num值变为0，下游线程等待num的值变为非0，不管是上游还是下游线程多数时间都处于等待中，等待num变成自己期望的那个状态。如果其中一个线程把num锁住之后，其他线程查看查看当前num状态不是所期望的值的话，首先unlock，然后scheled_yield出让调度器让其他线程有机会抢到这把锁来改变num的值，然后lock并且查看num的值，这种方式是查询法

再然后使用通知法之后，当下游线程查看num值为0即没有新任务的时候，使用条件变量进行pthread_cond_wait等待，等上有线程往下游下方一个任务之后即用pthread_cond_signal或者pthread_cond_broadcast把下游线程唤醒并执行任务，但其实这个程序的效率还没有很高，因为不管是通知法还是查询法，其实都是线程在等待，等待的原因是num这块空间太小

那么我们其实可以把任务池放大比如上游一次性往num这个池子里放10个任务，我们之间对num变量是定义成整型数，现在就可以设置为一个数组存放多个任务，再进一步我们可以把这个数组设计成队列来实现有前后关系，上游线程下发任务时往对尾放，下游线程在拿取任务的时候从队头拿，提到队列又会引出一个新的概念：管道

现在要做出这样的效果：一个进程当中假如说有多个线程来进行通信的话，原来用的是一个int类型的变量，但是这个变量有点小导致各个线程需要不停地等待，现在把这块空间放大，比如一个数组，也就是说在当前进程中实现了一个类似管道或者队列的机制

<img src="/home/liangruuu/.config/Typora/typora-user-images/image-20220330075658678.png" alt="image-20220330075658678" style="zoom:80%;" />

```c
#ifndef MYPIPE_H__
#define MYPIPE_H__

#define PIPESIZE 1024
#define MYPIPE_READ 0x00000001UL
#define MYPIPE_WRITE 0x00000002UL

typedef void mypipe_t;

mypipe_t *mypipe_init(void);

int mypipe_read(mypipe_t *, void *buf, size_t);

int mypipe_write(mypipe_t *, const void *buf, size_t);

int mypipe_register(mypipe_t *, int);

int mypipe_unregister(mypipe_t *, int);

int mypipe_destroy(mypipe_t *);

#endif


struct mypipe_st
{
    int head;
    int tail;
    char data[PIPESIZE];
    int datasize;
    int count_rd;
    int count_wr;
    pthread_mutex_t mut;
    pthread_cond_t cond;
};
```

* 4、8：定义管道的大小，像之前写过的代码一样需要把管道的数据结构隐藏起来在.c文件中定义
* 25-35：顺序存储的队列中有什么，这个管道结构体中就有什么，并且由于管道中的数据都是共用的，会涉及到竞争问题，所以设置互斥量来强调对于管道的操作应该是独占的。并且由于管道的特点，线程在获取管道里数据的时候如果发现没有数据就会阻塞并且等待，等到管道当中有内容为止，所以这一块还是要么是查询法，要么是通知法，所以按照之前的知识还需要设置一个条件变量来不使线程处于盲等状态
* 12、13：读管道与写管道，第一个参数就是指管道；第二个参数是把管道内数据读到缓冲区buf中；所读内容为size个字节大小，当然用户企图读size个字节大小的数据，但是管道中实际可能没那么多数据，所以返回值为两者取其小；写管道同理

```c
mypipe_t *mypipe_init(void)
{
    struct mypipe_st *me;

    me = malloc(sizeof(*me));
    if (me == NULL)
        return NULL;

    me->head = 0;
    me->tail = 0;
    me->datasize = 0;
    pthread_mutex_init(&me->mut, NULL);
    pthread_cond_init(&me->cond, NULL);

    return me;
}


int mypipe_destroy(mypipe_t *ptr)
{
    struct mypipe_st *me = ptr;

    pthread_mutex_destroy(&me->mut);
    pthread_cond_destroy(&me->cond);

    free(ptr);

    return 0;
}
```

* 7：这里不用perror报错的原因是，最好不要在.c实现或者在库中进行报错，防止跟用户的报错冲突

```c
int mypipe_read(mypipe_t *ptr, void *buf, size_t count)
{
    struct mypipe_st *me = ptr;
    size_t i;

    pthread_mutex_lock(&me->mut);

    while (me->datasize <= 0)
        pthread_cond_wait(&me->cond, &me->mut);

    for (i = 0; i < count; i++)
    {
        if (mypipe_readbyt_unlocked(me, buf + i) != 0)
            break;
    }

    pthread_cond_broadcast(&me->cond);
    pthread_mutex_unlock(&me->mut);

    return i;
}

static int mypipe_readbyt_unlocked(struct mypipe_st *me, char *datap)
{
    if (me->datasize <= 0)
        return -1;

    *datap = me->data[me->head];
    me->head = next(me->head);
    me->datasize--;

    return 0;
}
```

* 9：使用pthread_cond_wait一直等到有线程通知dataset发生了变化然后来打断这个wait，即总有某个时刻有写者去写管道，写者写完管道之后就应该发送一个通知打断wait，不管是pthread_cond_broadcast还是pthread_cond_signal；同理对于写管道操作时，如果管道中的数据是满的，那么写者也应该等待直到读者把管道读出一部分数据之后通知写者可以写了，所以write函数中也一定会有wait和signal、broadcast操作
* 23-33：mypipe_readbyt_unlocked调用一次读一个字节，循环了几次就代表读了几个字节

```c
while (me->datasize <= 0)
    pthread_cond_wait(&me->cond, &me->mut);
```

这一部分其实还缺少一个逻辑，即当当前管道空并且有写者的时候才有必要去等待，比如说管道中没有数据，但是现在一个写者都没有的话就没必要等待，即当当前管道空但是没有写者的时候读者就退出，如果这样来实现的话就应该对写者读者做一个计数

```c
struct mypipe_st
{
    int head;
    int tail;
    char data[PIPESIZE];
    int datasize;
    int count_rd;
    int count_wr;
    pthread_mutex_t mut;
    pthread_cond_t cond;
};

#define MYPIPE_READ 0x00000001UL
#define MYPIPE_WRITE 0x00000002UL

int mypipe_register(mypipe_t *, int);

int mypipe_unregister(mypipe_t *, int);
```

* 7-8：读者写者计数
* 15-17：注册或者身份函数，第二个参数是一个位图，表示是以什么样的身份进行操作的，如果是1的话就代表以读者身份对管道进行操作，如果是2的话就代表与写者身份对管道进行操作，返回值代表是否成功

```c
int mypipe_register(mypipe_t *ptr, int opmap)
{
    struct mypipe_st *me = ptr;

    pthread_mutex_lock(&me->mut);

    if (opmap & MYPIPE_READ)
        me->count_rd++;
    if (opmap & MYPIPE_WRITE)
        me->count_wr++;

    pthread_cond_broadcast(&me->cond);

    while (me->count_rd <= 0 || me->count_wr <= 0)
        pthread_cond_wait(&me->cond, &me->mut);

    pthread_mutex_unlock(&me->mut);

    return 0;
}


mypipe_t *mypipe_init(void)
{
    struct mypipe_st *me;

    me = malloc(sizeof(*me));
    if (me == NULL)
        return NULL;

    me->head = 0;
    me->tail = 0;
    me->datasize = 0;
    me->count_rd = 0;
    me->count_wr = 0;
    pthread_mutex_init(&me->mut, NULL);
    pthread_cond_init(&me->cond, NULL);

    return me;
}
```

* 14-17：管道必须凑齐读写双方才能建立成功，否则当前管道是没有建立成功的，比如一下子注册了10个读者，但是一个写者都没有，那么这10个读者来读管道又有什么意义？即至少要有一个读者和写者
* 12：比如说作为读者注册到了管道上，同时并没有写者注册，则会被阻塞等待，先后来了10个读者则10个读者都处于等待状态，等待写者到来，终于有一个时刻写者到来了，因为不满足14行的循环条件所以不执行wait操作，但是这个写者需要执行pthread_cond_broadcast或者pthread_cond_signal把处于等待的读者线程唤醒

```c
int mypipe_unregister(mypipe_t *ptr, int opmap)
{
    struct mypipe_st *me = ptr;

    pthread_mutex_lock(&me->mut);

    if (opmap & MYPIPE_READ)
        me->count_rd--;
    if (opmap & MYPIPE_WRITE)
        me->count_wr--;

    pthread_cond_broadcast(&me->cond);

    pthread_mutex_unlock(&me->mut);

    return 0;
}
```

* 12：打断wait等待

根据以上的逻辑更改read和write的代码

```c
int mypipe_read(mypipe_t *ptr, void *buf, size_t count)
{
    struct mypipe_st *me = ptr;
    size_t i;

    pthread_mutex_lock(&me->mut);

    while (me->datasize <= 0 && me->count_wr > 0)
        pthread_cond_wait(&me->cond, &me->mut);

    if (me->datasize <= 0 && me->count_wr <= 0)
    {
        pthread_mutex_unlock(&me->mut);
        return 0;
    }

    for (i = 0; i < count; i++)
    {
        if (mypipe_readbyt_unlocked(me, buf + i) != 0)
            break;
    }

    pthread_cond_broadcast(&me->cond);
    pthread_mutex_unlock(&me->mut);

    return i;
}
```

* 8、11：当前管道没有数据但是有写者才读者等待，同理当管道满并且有读者时写者才等待，即当管道空以及有写者的时候才去等待
* 11-15：当wait被打断了发现循环条件不满足了，未必是因为有数据而是因为写者数量变为0了，所以应该判断如果是因为写者数量变为0的问题而导致了跳出循环，那就不等直接退出
* 9：将来有两个条件的改变会导致发送一个通知来打断这个wait，一个是datasize的值变了，另外一个条件是读写者的计数变了，因为这两个值的变化可能导致循环条件不满足，不管结果是否真的能跳出循环，都需要打断等待进行判断，所以打断这个等待的broadcast函数在mypipe_unregister函数的第12行书写

上面的代码已经体现了管道的使用特点，为之后的进程间通信内容做铺垫

<img src="/home/liangruuu/.config/Typora/typora-user-images/image-20220330093553887.png" alt="image-20220330093553887" style="zoom:80%;" />

# 进程间通信详解

1. 管道
2. XSI ——> SysV
3. 网络套接字socket：前两点的进程都是处于同一台计算机上，如果两个用于通信的进程处于不同的计算机上，比如说QQ服务器和手机QQ不是在同一个计算机上，这种跨主机的进程间通信就属于网络套接字部分



管道由内核提供；并且是单工通信，一端为读端，一端为写端；自同步机制，管道永远是迁就比较慢的一方，比如管道的两端作为写端速度快，但是读端速度比较慢，那么会导致管道的数据会迅速变满从而被阻塞，如果管道被写满之后写者就会阻塞一直等到有读者把管道清空一些空间为止，反之亦然

匿名管道：我们之前学过一个用于创建临时文件的函数`tmpfile`，我们需要一个临时文件来存储信息，`tmpfile`直接返回一个FILE\* ，所以我们并不关心这个临时文件名，这里的匿名管道也是同一个概念，虽然我们不知道这个管道叫什么名字，我们可以通过P\*来控制这个管道，只是我们被办法在磁盘上看到这个管道

命名管道：如果两个进程没有亲缘关系，那么是不能用匿名管道来进行通信的

>NAME
>
>> pipe, pipe2 - create pipe
>
>SYNOPSIS
>
>> #include <unistd.h>
>>
>>
>> int pipe(int pipefd[2]);
>
>1. int pipe(int pipefd[2])：创建一个管道，参数是一个数组，数组元素为管道两端对应的文件描述符，通过pipe创建了一个管道之后就会把管道两端的文件描述符回填到这个数组中；pipefd[0]作为读端，pipefd[1]作为写端
>2. 注意pipe函数创建的是一个匿名管道，磁盘上是看不到这个管道的
>
>

比如说一个进程函数中调用了pipe函数，就会创建一个管道，管道两端各自对应一个文件描述符，0端作为读端，1端作为写端，这两个文件描述符会保存到当前的进程空间中。匿名管道最方便的就是可以用在具有亲缘关系的进程间通信，先调用pipe函数创建匿名管道，再调用fork函数创建了一个子进程，子进程会复制父进程的文件描述符信息，也就是说在子进程也会记录这两个文件描述符。那么父子进程进行通信的话，比如父进程用1端写，子进程用0端读；或者子进程用1端写，父进程用0端读

<img src="/home/liangruuu/.config/Typora/typora-user-images/image-20220330104114499.png" alt="image-20220330104114499" style="zoom:80%;" />

```c
// pipe.c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define BUFSIZE 1024

int main()
{
    pid_t pid;
    int pd[2];
    int len;
    char buf[BUFSIZE];

    if (pipe(pd) < 0)
    {
        perror("pipe()");
        exit(1);
    }

    pid = fork();
    if (pid < 0)
    {
        perror("fork()");
        exit(1);
    }

    if (pid == 0)
    {
        close(pd[1]);
        len = read(pd[0], buf, BUFSIZE);
        write(1, buf, len);
        write(1, "\n", 1);
        close(pd[0]);
        exit(0);
    }
    else
    {
        close(pd[0]);
        write(pd[1], "Hello!", 6);
        close(pd[1]);
        wait(NULL);
        exit(0);
    }
}
```

一定要先执行pipe函数然后再执行fork函数，因为要先创建管道，然后父进程创建子进程，子进程就能继承这个管道以及管道两端对应的文件描述符

* 30-38：指定子进程读管道，关闭写端，从读端获取数据到缓冲区中，然后从缓冲区把数据显示到终端
* 41-46：指定父进程写管道，关闭读端，从写端获取数据
* 44：给子进程收尸

```shell
liangruuu@liangruuu-virtual-machine:~/study/linuxc/code/ipc/pipe$ ./pipe 

# result
Hello!
```

之后要实现的流媒体就需要使用管道来进行父子进程间的数据通信

<img src="/home/liangruuu/.config/Typora/typora-user-images/image-20220330135511180.png" alt="image-20220330135511180" style="zoom:80%;" />

```c
// player.c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <wait.h>
#include <errno.h>

#define BUFSIZE 1024

int main()
{
    int pd[2];
    int fd, sd;
    int len;
    pid_t pid;
    char buf[BUFSIZE];

    if (pipe(pd) < 0)
    {
        perror("pipe()");
        exit(1);
    }

    pid = fork();
    if (pid < 0)
    {
        perror("fork()");
        exit(1);
    }

    if (pid == 0)
    {
        close(pd[1]); //关闭写端

        dup2(pd[0], 0); //将stdin关闭，重定向到管道读端 mpg123 - "-" 表示从stdin读取, 即文件描述符0指代的是管道的读端，							而不再是标准输入stdin了；即标准输入端直接接入子进程中管道的读端，用于视频数据的解码播放
        close(pd[0]);

        fd = open("/dev/null", O_RDWR); //屏蔽其他标准输出
        dup2(fd, 1);
        dup2(fd, 2);

        int ret = execl("/usr/bin/mpg123", "mpg123", "-", NULL);
        if (ret < 0)
        {
            perror("execl()");
            exit(1);
        }

        close(fd);
    }
    else
    {
        close(pd[0]); //关闭读端
        // 父进程从网上收取数据，并往管道中写
        sd = open("/home/ray/Music/heroes.mp3", O_RDONLY);
        if (sd < 0)
        {
            perror("open()");
            exit(1);
        }

        while (1)
        {
            len = read(sd, buf, BUFSIZE);
            if (len < 0)
            {
                if (errno == EINTR)
                    continue;
                close(pd[1]);
                close(sd);
                exit(1);
            }
            if (len == 0)
            {
                break;
            }
            write(pd[1], buf, len);
        }

        close(pd[1]);
        close(sd);
        wait(NULL);
    }

    exit(0);
}
```

# 进程间通信-消息队列详解

IPC——>Inter-Process Communication

XSI IPC中有三种机制：

1. Message Queues消息队列
2. Semaphore Arrays信号量数组
3. Shared Memory共享内存

```shell
liangruuu@liangruuu-virtual-machine:~$ ipcs

------ Message Queues --------
key        msqid      owner      perms      used-bytes   messages    

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      
0x00000000 10         liangruuu  600        524288     2          dest         
0x00000000 11         liangruuu  600        524288     2          dest         
0x00000000 14         liangruuu  600        7786496    2          dest         
0x00000000 15         liangruuu  600        7786496    2          dest         
0x00000000 18         liangruuu  600        524288     2          dest         
0x00000000 21         liangruuu  600        524288     2          dest         
0x00000000 26         liangruuu  600        73728      2          dest         
0x00000000 27         liangruuu  600        73728      2          dest         
0x00000000 28         liangruuu  600        45056      2          dest         
0x00000000 29         liangruuu  600        45056      2          dest         
0x00000000 32         liangruuu  600        524288     2          dest         

------ Semaphore Arrays --------
key        semid      owner      perms      nsems 
```

这三种机制可以用于有亲缘关系的进程间通信，也可以用于没有亲缘关系的进程间通信

key是为了确定通信双方拿到同一个通信机制来进行实现，可以理解成通信地点，确定了这个通信地点之后，通信双方就在约定的痛心地点通过管道进行通信。得到同一个key值是为了创建某一个实例，创建实例拿到id，通信双方拿到id之后

>NAME
>
>> ftok - convert a pathname and a project identifier to a System V IPC key
>
>SYNOPSIS
>
>> #include <sys/types.h>
>> #include <sys/ipc.h>
>>
>> key_t ftok(const char *pathname, int proj_id);
>
>1. key_t ftok(const char *pathname, int proj_id)：pathname指的就是通信双方约定的通信地点

![image-20220330142244685](/home/liangruuu/.config/Typora/typora-user-images/image-20220330142244685.png)

Message Queues

>NAME
>
>> msgget - get a System V message queue identifier
>
>SYNOPSIS
>
>> #include <sys/types.h>
>> #include <sys/ipc.h>
>> #include <sys/msg.h>
>>
>> int msgget(key_t key, int msgflg);
>
>1. int msgget(key_t key, int msgflg)：获取一个消息队列的实例，以消息队列的id值作为返回值

>NAME
>
>> msgrcv, msgsnd - System V message queue operations
>
>SYNOPSIS
>
>> #include <sys/types.h>
>> #include <sys/ipc.h>
>> #include <sys/msg.h>
>>
>> int msgsnd(int msqid, const void *msgp, size_t msgsz, int msgflg);
>> ssize_t msgrcv(int msqid, void *msgp, size_t msgsz, long msgtyp, int msgflg);
>
>1. 消息队列一端作为发送端msgsnd发送消息；一端作为接收端msgrcv接收消息，消息队列是双工的，双方都能往消息队列写数据，也都能从消息队列中拿数据

>NAME
>
>> msgctl - System V message control operations
>
>SYNOPSIS
>
>> #include <sys/types.h>
>> #include <sys/ipc.h>
>> #include <sys/msg.h>
>>
>> int msgctl(int msqid, int cmd, struct msqid_ds *buf);
>
>1. msgctl：对某一个消息队列执行某一个操作cmd，buf是cmd的参数
>
>Valid values for cmd are:
>
>IPC_STAT
>
>IPC_SET
>
>IPC_RMID：删除当前消息队列

使用消息队列实现一个小例子

```c
// proto.h

#ifndef PROTO_H__
#define PROTO_H__

#define KEYPATH "/etc/services"
#define KEYPROJ 'g'
#define NAMESIZE 1024

struct msg_st
{
    long mtype;
    char name[NAMESIZE];
    int math;
    int chinese;
};

#endif
```

定义通信进程双方数据传输协议

* 6-7：通信双方需要拿到同一个msgid，为了拿到同一个msgid则需要拿到同一个key值，要想拿到同一个key值则需要拿到同一个pathname和proj
* 10-16：定义传输内容的格式，接收方按同样的格式解析
* 12：代表消息的类型，值必须大于0

代码骨架如下：

```c
// snder.c

int main()
{


    ftok();

    msgget();

    msgsnd();

    msgctl();	// 销毁消息队列
    
    exit(0);

}
```

```c
// rcver.c

int main()
{


    ftok();

    msgget();

    msgrcv();

    msgctl();	// 销毁消息队列

    exit(0);

}
```

区分消息队列两端角色是主动端还是被动端，主动端是先发数据包的一方，被动端是先收数据包的一方，并且一定是被动端先运行程序，所以说rcver先运行

完整程序如下：

```c
// rcver.c

#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <unistd.h>
#include "proto.h"

int main()
{
    key_t key;
    int msgid;
    struct msg_st rbuf;

    key = ftok(KEYPATH, KEYPROJ);
    if (key < 0)
    {
        perror("ftok()");
        exit(1);
    }

    msgid = msgget(key, IPC_CREAT | 0666);
    if (msgid < 0)
    {
        perror("msgget()");
        exit(1);
    }

    while (1)
    {
        if (msgrcv(msgid, &rbuf, sizeof(rbuf) - sizeof(long), 0, 0) < 0)
        {
            perror("msgrcv()");
            exit(1);
        }

        fprintf(stdout, "name : %s ", rbuf.name);
        fprintf(stdout, "chinese : %d ", rbuf.chinese);
        fprintf(stdout, "math : %d\n", rbuf.math);
        fflush(NULL);
    }
    //销毁当前实例
    msgctl(key, IPC_RMID, NULL);

    exit(0);
}

```

* 14：主动端和被动端代码的区别在于主动端不用再重复使用IPC_CREAT，反之不能由主动端调用IPC_CREAT参数
* 33：msgrcv函数的第一个参数表示消息队列ID，第二个参数表示获取数据存入的缓冲区，也第三个参数表示接收的数据类型有效的大小，这里因为msg_st结构体中的long型的mtype属性暂时没用，不属于消息中的内容，不在有效大小之内，mtype之后的变量才属于消息，所以需要把这个属性的大小删去
* 45：实际上这一个正常终止时销毁实例的函数永远不会被执行，因为之前的接收操作是在一个死循环里执行的，但是我们可以把这个销毁实例的操作放入信号处理函数中定义，并且在源程序中加入信号机制，这样在收到信号的时候就可以被打断去执行信号处理函数了

```c
// snder.c

#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <unistd.h>
#include <string.h>

#include "proto.h"

int main()
{

    key_t key;
    int msgid;
    struct msg_st sbuf;

    key = ftok(KEYPATH, KEYPROJ);
    if (key < 0)
    {
        perror("ftok()");
        exit(1);
    }

    msgid = msgget(key, 0);
    if (msgid < 0)
    {
        perror("msgget()");
        exit(1);
    }

    sbuf.mtype = 1;
    strcpy(sbuf.name, "Alan");
    sbuf.chinese = rand() % 100;
    sbuf.math = rand() % 100;

    if (msgsnd(msgid, &sbuf, sizeof(sbuf) - sizeof(long), 0) < 0)
    {
        perror("msgrcv()");
        exit(1);
    }

    puts("ok!");

    exit(0);
}
```

<img src="/home/liangruuu/.config/Typora/typora-user-images/image-20220330153803335.png" alt="image-20220330153803335" style="zoom:80%;" />

# 消息队列-ftp实例

// TODO





















