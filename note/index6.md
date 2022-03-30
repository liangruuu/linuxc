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





