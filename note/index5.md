# 数据中继原理解析

在IO层面按照一开始整理的思路来讲包括标准IO、系统调用IO、文件系统、并发，有了这些内容之后就可以回过头去再看IO，IO是所有实现的基础

高级IO：非阻塞IO——阻塞IO

阻塞IO意味必须把某个任务完成，读不完内容就不结束，非阻塞IO意味着做任务都是尝试着去做，如果不能做的话就返回

open函数的man手册中会有一些ERRORS，ERRORS就是当前可能会发生错误的情况，也就是在设置errno之后代表的是哪些出错情况

>ERRORS
>
>> EINTR  
>> While  blocked  waiting to complete an open of a slow device (e.g., a FIFO; see fifo(7)), the call was interrupted by a signal handler; see signal(7).
>>
>> EAGAIN 
>> The file descriptor fd refers to a file other than a socket and has been marked nonblocking (O_NONBLOCK), and  the read would block.  See open(2) for further details on the O_NONBLOCK flag.
>
>1. EINTR指的是当前如果在操作一个比较慢的设备的话，那么这个操作会被信号打断
>2. EAGAIN指的是当前操作如果是非阻塞的方式，那么就有可能发生EAGAIN假错

1. 除了阻塞情况下会有一个假错，假错不是因为操作失败而是因为当前的操动作非常慢，是一个阻塞的系统调用，总做不完要完成的那个任务，这时有信号过来就会打断一个正在阻塞的系统调用
2. 比如说读打印机，如果用阻塞的方式去读的话，那么就意味着读打印机必须读到内容为止；用非阻塞的方式的话读打印机，一读，有内容拿回来，没内容也回来，但是这个回来会被当做一个出错的返回，如果查看errno的话，这个errno值会是EAGAIN。也就是说这不是真正意义上的出错，是因为做这件事情尝试去做了，但是没有成功，只是因为现在没有数据而不是当前read函数的问题

高级IO这一部分主要是来研究非阻塞IO，在讲非阻塞IO的时候会介绍一种编程思路：有限状态机

1. 非阻塞IO
2. IO多路转接
3. 其他读写函数
4. 储存映射IO
5. 文件锁

假设有两个正常打开的设备，现在要用这两个设备进行数据交换(数据中继)：把左边设备L的数据拿到右边的设备R去，数据可能在右边的设备进行加工，再把右边的数据拿回到左边的设备中区去，有以下几种方式

1. 从任务的角度讨论，读L写R，然后读R写L，这样转一圈相当于四个工作，假设用阻塞的形式实现的话就会出现一些问题，比如读左边设备的数据，但是一直没有数据出现，那么就一定会阻塞在读L这样的动作上，这时如果有信号来打断的话，一判断是加假错，然后继续读，但是右边的设备不断地会有数据到来，其实如果先读R写L，再读L写R的话这个循环就能正常执行，结果一直在读L，L一直没数据，那就没办法读R写L。对于刚才的这一套流程是只针对一个线程或者进程而言的，总而言之没有协同操作，只是一个人在完成
2. 分为两个线程或进程，一个用来读L并且写R，一个用来读R并且写L，那么哪一端有数据那么哪一端就可以先执行，用第二种方式还可以勉强去考虑使用
3. 如果换成非阻塞方式只用一个线程或进程也能实现这个读写操作：企图读L，没数据，那么就换R去读，如果R没数据再去读L，再没数据就又去读R，左右横跳以此类推

<img src="index5.assets/image-20220328221459881.png" alt="image-20220328221459881" style="zoom:80%;" />

# 有限状态机编程原理

有限状态机这种编程思想可以用来解决复杂流程的问题

简单流程：如果一个程序的自然流程是结构化的，那么就是简单流程

复杂流程：如果一个程序的自然流程不是结构化的，那么就是复杂流程，比如执行某一个语句体，然后有个条件判断，如果为真则执行xx，如果为假则继续进行判断，再为真的话则仍然继续判断，如果为真则返回去执行第一步的真操作，如果为假的话......即用简单的顺序选择循环不能够完全把这个流程描述出来，可能会用到一个跳转goto.....这个流程是非结构化的，复杂流程很常见，比如网络协议，网络协议通常不是简单流程，比如之前提到过一个概念：口令的随机校验，在登录账号，输入密码，第一次输入正确然后报错输入错误请重新输入，它不会给你报"请再次输入"，第二次再输入成功，也就是只有连续两次输入同一个口令才会让用户正常登陆，这个两步口令校验是为了防止脚本攻击。就这么一个小的功能模块的流程图也不会是结构化的，因为跟人打交道这件事本身就不容易

自然流程：作为人类而言解决问题最直接最直观的思路，比如说大象装冰箱分几步？第一步把冰箱门打开，第二步把大象放进去，第三步把冰箱门关上

<img src="index5.assets/image-20220328215046674.png" alt="image-20220328215046674" style="zoom:80%;" />

* 用有限状态机来实现之前两个设备数据交换的功能

```c
#define TTY1 "/dev/tty8"
#define TTY2 "/dev/tty9"
```

* 1-2：设置两个设备 

```c
int main()
{
    int fd1, fd2;

    fd1 = open(TTY1, O_RDWR);
    if (fd1 < 0)
    {
        perror("open()");
        exit(1);
    }
    write(fd1, "TTY1\n", 5);

    fd2 = open(TTY2, O_RDWR | O_NONBLOCK);
    if (fd2 < 0)
    {
        perror("open()");
        exit(1);
    }
    write(fd2, "TTY2\n", 5);

    relay(fd1, fd2);

    close(fd2);
    close(fd1);

    exit(0);
}

static void relay(int fd1, int fd2)
{
    
}
```

* main函数主要用来打开文件调用数据中继的函数，然后用有限状态机的方式来进行编程
* 5：当前使用非阻塞IO方式来实现程序，但是即便使用非阻塞IO，也不能够确定用户一定是用非阻塞方式打开文件的，main函数是用来模拟用户的操作的，所以应该在main函数之外进行所有工作之前要保证是以非阻塞方式进行操作的，这时就可以用到`fcontrol`函数，这个函数针对文件描述符的属性、状态的改变进行了封装操作，我们假设用户在打开文件fd1时忘记了以非阻塞方式打开文件，即以O_RDWR方式打开
* 13：第二个用户使用了非阻塞方式打开文件fd2，所以是以O_NONBLOCK方式打开
* 21：调用数据中继函数

把前一节画的数据交换图用有限状态机的形式表示：

①的任务是读L写R，②的任务是读R写L，这两个过程是一样的，所以只拿出其中一个进行分析：首先不管初始的思路是什么，一定会有一个start的状态，这个start状态一定是读态，如果读取失败了，如果是真错那么就报错，把报错这个环节称为EX态(异常处理态)，报完错之后进程结束进入terminate进程终止态；如果读取是假错，即在非阻塞情况下读文件，但是文件中并没有数据造成的错误，那么就重新进入读态，即如果是ERRAGAIN的话就继续进入读态；如果发现读取是最后一次读了，那么读完之后就进入结束状态；如果读取能成功的话就进入写态，如果写成功了就进入读态，写操作可能会出错，如果是真错那么进入EX态，如果是假错则重新进入写态，还有一种情况是，比如说当前的写没有失败，但是读到了10个字节，写另外的一个设备只写了3个字节，那剩余的7个字节要坚持写够，所以在这个情况下还要继续进入写态。写操作不能作为主导，写操作不会导致程序正常终止，也就是说不能从写态连接到终止态度

<img src="index5.assets/image-20220328223340851.png" alt="image-20220328223340851" style="zoom:80%;" />

甲方改需求只不过是在这张图的某个地方添加一个圈，并且分析这个圈和其他圈的连线是什么，并且在程序中只需要把这些圈作为点，然后从每个点出发，把从每个点出发的线用代码表示好就可以了

```c
static void relay(int fd1, int fd2)
{
    int fd1_save, fd2_save;

    fd1_save = fcntl(fd1, F_GETFL);
    fcntl(fd1, F_SETFL, fd1_save | O_NONBLOCK);

    fd2_save = fcntl(fd2, F_GETFL);
    fcntl(fd2, F_SETFL, fd2_save | O_NONBLOCK);

}
```

* 6-7、9-10：F_GETFL意为获取文件fd1的文件状态，文件状态是一个32位的整型数；使用F_SETFL选项对fd1进行文件状态的设置，设置的内容是fd1_save，即原有的属性或者是O_NONBLOCK状态，如果文件本身就有O_NONBLOCK状态的话，那只不过上在位图上重复置1，如果先前没有指定O_NONBLOCK，那么现在的文件状态就是非阻塞的了。也就是说在状态机的状态转移之前需要将两个文件以非阻塞方式打开

现在该来建立数据结构了，即封装状态机的数据结构

```c
struct fsm_st
{
    int state;
    int sfd;
    int dfd;
    char buf[BUFSIZE];
    int len;
    int pos;
    char *errstr;
};

enum
{
    STATE_R = 1, // read
    STATE_W,     // write
    STATE_EX,    // error
    STATE_T      // over
};
```

* 3：当前状态机的状态，即图中的每一个圈圈，状态用enum结构体中的属性指代
* 4-5：两个文件的文件描述符
* 6：因为任务是读L写R，或者是读R写L，读完之后要暂时把数据放在一个缓冲区，然后写入的时候再从这个缓冲区里取数据

```c
static void relay(int fd1, int fd2)
{
    int fd1_save, fd2_save;
    struct fsm_st fsm12, fsm21;

    fd1_save = fcntl(fd1, F_GETFL);
    fcntl(fd1, F_SETFL, fd1_save | O_NONBLOCK);

    fd2_save = fcntl(fd2, F_GETFL);
    fcntl(fd2, F_SETFL, fd2_save | O_NONBLOCK);

    fsm12.state = STATE_R;
    fsm12.sfd = fd1;
    fsm12.dfd = fd2;

    fsm21.state = STATE_R;
    fsm21.sfd = fd2;
    fsm21.dfd = fd1;

    while (fsm12.state != STATE_T || fsm21.state != STATE_T)
    {
        fsm_driver(&fsm12);
        fsm_driver(&fsm21);
    }

    fcntl(fd1, F_SETFL, fd1_save);
    fcntl(fd2, F_SETFL, fd2_save);
}
```

* 4：fsm12代表读L写R时的状态机，fsm21代表读R写L时的状态机
* 12-14、16-18：初始化状态机，用到的文件描述符fd1和fd2其实一个是由阻塞方式打开，一个是由非阻塞方式打开，所以需要在用户调用完函数之后，即做完数据中继之后把文件描述符的状态复原，我们之前一直在强调要把我们实现的函数作为一个大工程里的小模块，函数里的实现不能影响外部模块，所以需要确保在进入模块和离开模块时整体状态是一致的
* 26-27：把文件描述符的状态恢复，所以一开始的时候设置了fd1_save和fd1_save两个参数
* 20-24：当状态机12的状态不是终止态或者状态机21的状态不是终止态的话那就调用函数fsm_driver执行状态机的状态转移

```c
static void fsm_driver(struct fsm_st *fsm)
{
    int ret;

    switch (fsm->state)
    {
    case STATE_R:
        fsm->len = read(fsm->sfd, fsm->buf, BUFSIZE);
        if (fsm->len == 0)
            fsm->state = STATE_T;
        else if (fsm->len < 0)
        {
            if (errno == EAGAIN)
                fsm->state = STATE_R;
            else
            {
                fsm->errstr = "read()";
                fsm->state = STATE_EX;
            }
        }
        else
        {
            fsm->pos = 0;
            fsm->state = STATE_W;
        }
        break;
    case STATE_W:
        ret = write(fsm->dfd, fsm->buf + fsm->pos, fsm->len);
        if (ret < 0)
        {
            if (errno == EAGAIN)
                fsm->state = STATE_W;
            else
            {
                fsm->errstr = "write()";
                fsm->state = STATE_EX;
            }
        }
        else
        {
            fsm->pos += ret;
            fsm->len -= ret;
            if (fsm->len == 0)
                fsm->state = STATE_R;
            else
                fsm->state = STATE_W;
        }
        break;
    case STATE_EX:
        perror(fsm->errstr);
        fsm->state = STATE_T;
        break;
    case STATE_T:
        break;
    default:
        abort();
        break;
    }
}
```

* 7-26：如果当前状态是读态的话就调用read函数进行文件读操作；len为实际读取的字节数，等于0则表示正常结束，那么当前状态机的状态就转移成终止态，如果len的值小于0，即表示读取失败，错分真错加措，所以如果是加错的话那么状态机的状态就应该回到读态，如果是真错，那么状态机的状态就应该转移至终止态度，如果读取成功，则状态机的状态就需转移到写态
* 27-48：逻辑和读操作类似，在写态一共有四条线，` if (ret < 0)`体中包括了真错和假错两条线，还有一条是没有写够len个字节进行的重返写态写够len个字节，还有一条是写够了len个字节然后转移到读态，`fsm->buf + fsm->pos`的意思是假如要写len个字节，可是实际上只写入了ret个字节，那么下一次就应该从buf+3的位置开始写len-ret个字节
* 49-52：真错态，只有一条链接到终止态的线，因为有读错也有写错，所以用fsm->errstr字段记录当前出错原因

完整代码

```c
// relay.c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define TTY1 "/dev/tty8"
#define TTY2 "/dev/tty9"
#define BUFSIZE 1024

enum
{
    STATE_R = 1, // read
    STATE_W,     // write
    STATE_EX,    // error
    STATE_T      // over
};

struct fsm_st
{
    int state;
    int sfd;
    int dfd;
    char buf[BUFSIZE];
    int len;
    int pos;
    char *errstr;
};

static void fsm_driver(struct fsm_st *fsm)
{
    int ret;

    switch (fsm->state)
    {
        case STATE_R:
            fsm->len = read(fsm->sfd, fsm->buf, BUFSIZE);
            if (fsm->len == 0)
                fsm->state = STATE_T;
            else if (fsm->len < 0)
            {
                if (errno == EAGAIN)
                    fsm->state = STATE_R;
                else
                {
                    fsm->errstr = "read()";
                    fsm->state = STATE_EX;
                }
            }
            else
            {
                fsm->pos = 0;
                fsm->state = STATE_W;
            }
            break;
        case STATE_W:
            ret = write(fsm->dfd, fsm->buf + fsm->pos, fsm->len);
            if (ret < 0)
            {
                if (errno == EAGAIN)
                    fsm->state = STATE_W;
                else
                {
                    fsm->errstr = "write()";
                    fsm->state = STATE_EX;
                }
            }
            else
            {
                fsm->pos += ret;
                fsm->len -= ret;
                if (fsm->len == 0)
                    fsm->state = STATE_R;
                else
                    fsm->state = STATE_W;
            }
            break;
        case STATE_EX:
            perror(fsm->errstr);
            fsm->state = STATE_T;
            break;
        case STATE_T:
            break;
        default:
            abort();
            break;
    }
}

static void relay(int fd1, int fd2)
{
    int fd1_save, fd2_save;
    struct fsm_st fsm12, fsm21;

    fd1_save = fcntl(fd1, F_GETFL);
    fcntl(fd1, F_SETFL, fd1_save | O_NONBLOCK);

    fd2_save = fcntl(fd2, F_GETFL);
    fcntl(fd2, F_SETFL, fd2_save | O_NONBLOCK);

    fsm12.state = STATE_R;
    fsm12.sfd = fd1;
    fsm12.dfd = fd2;

    fsm21.state = STATE_R;
    fsm21.sfd = fd2;
    fsm21.dfd = fd1;

    while (fsm12.state != STATE_T || fsm21.state != STATE_T)
    {
        fsm_driver(&fsm12);
        fsm_driver(&fsm21);
    }

    fcntl(fd1, F_SETFL, fd1_save);
    fcntl(fd2, F_SETFL, fd2_save);
}

int main()
{
    int fd1, fd2;

    fd1 = open(TTY1, O_RDWR);
    if (fd1 < 0)
    {
        perror("open()");
        exit(1);
    }
    write(fd1, "TTY1\n", 5);

    fd2 = open(TTY2, O_RDWR | O_NONBLOCK);
    if (fd2 < 0)
    {
        perror("open()");
        exit(1);
    }
    write(fd2, "TTY2\n", 5);

    relay(fd1, fd2);

    close(fd2);
    close(fd1);

    exit(0);
}
```

<img src="index5.assets/image-20220329081715178.png" alt="image-20220329081715178" style="zoom:80%;" />

<img src="index5.assets/image-20220329081727734.png" alt="image-20220329081727734" style="zoom:80%;" />

因为111111这一行最后输入了ctrl+c，表示该行没有被输入到缓冲区中，也就没有被fd2获取，所以在终端显示TTY2信息的时候没有改行数据出现

<img src="index5.assets/image-20220329081914269.png" alt="image-20220329081914269" style="zoom:80%;" />

因为当前程序处于盲等状态，忙在假错(看一下没内容——>看一下没内容)*N，对应到状态机图上表现为一直读状态上转圈







