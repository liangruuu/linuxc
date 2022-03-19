# 标准I/O介绍

IO的实现包括两种，一种称之为标准IO(stdio)、一种称之为系统调用IO(sysio | 文件IO)。IO操作是一切实现的基础，在程序运行过程中产生的数据就有了保留价值，不管是永久的还是暂时，都可以把它们转存到文件中去，等某个时刻用到的话就可以把这些数据从文件中读取出来，然后进行使用或者计算等等操作，如果没了IO操作的话程序的结果想就相当于不存在了。

标准IO和系统调用IO该怎样进行区分呢？在可行的情况下，即在某种环境下两种IO都能够使用的话，优先使用标准IO：当程序处于用户态并且对话内核态的时候，当前的内核为我们提供了一组函数让我们能够去对话内核，这组函数被称作系统调用IO(sysio)；另外一种形式出现的关键是当内核不一样时，也就是说内核提供的一套系统调用IO不一样，这个时候给程序员造成了困扰，比如说linux和windows的内核显然是不一样的，linux提供了一套系统调用IO，windows也提供了一套系统调用IO，在进行任何实现的时候就要顾及到此时计算机是处于哪个环境中的。此时"标准就跳出来和稀泥了"，不管是在linux系统还是windows系统都想对内核进行对话，那么标准就提供了一套使用，即标准IO(stdio)。此时不仅能够通过sysio对话内核也能通过stdio来对话内核，标准IO是基于系统调用IO而实现的，大家都接触过printf这个函数，很多人在使用这个函数的时候压根没考虑过到底是在linux平台下使用的还是在windows平台下使用的，printf函数是典型的stdio中的一个函数，也就是说标准中规定了一套标准函数，不管各个环境平台的底层、系统调用IO或者说内核是如何实现的，只要标准IO实现的是打印、关闭、打开...就使用标准IO提供的函数。

标准IO的移植性好，并且能够合并系统调用，合并系统调用在绝大多数情况下都是有好处的，因为其提供了buffer和cahce的机制，能够为读写提供加速机制。不同stdio的函数依赖的系统调用函数是不太一样的，比如打开文件函数fopen，这是标准IO里的一个函数。fopen在linux环境下依赖的是open函数，而在windows环境下依赖的是openfile函数，以上两个函数就是内核提供的系统调用IO，而fopen就属于标准IO，在哪个平台都能使用fopen来实现打开一个文件的作用，但它依赖的系统调用IO是不一样的，所以在两个IO都能用的情况下优先考虑标准IO(stdio)。
![image-20220316220501231](index.assets/image-20220316220501231.png)

标准IO里有这么一系列函数：

1. fopen();   // 文件打开
2. fclose();   // 文件关闭

 // 二进制字符的读写

1. fgetc();    
2. fputc();

// 字符串读写

1. fgets();   
2. fputs();

// 二进制数据块的操作

1. fread();    
2. fwrite();



1. printf();
2. scanf();

// 文件位置指针操作

1. fseek(); 
2. ftell();
3. rewind();

// buffer&cache操作

1. fflush();

# fopen函数

标准IO当中涉及到一个类型贯穿始终：FILE，FILE是一个结构体，FILE由fopen函数产生

>  FILE *fopen(const char *pathname, const char *mode);
>
>  * const char *pathname : 打开文件地址
>  * const char *mode : 打开文件操作权限(读|写)
>  * return FILE * : 文件结构体起始位置指针
>
> 参数由const修饰，表明函数调用不会对文件path和mode进行改变
>
> 1) RETURN VALUE
>     Upon  successful completion fopen(), fdopen() and freopen() return a FILE pointer.  Otherwise, NULL is returned and errno is set to indicate the error.
>
>     * errno被定义的时候是一个全局变量的概念，如果此时出错了就会把出错的原因放到errno这个全局变量上。换句话说，当前进行完某个操作，如果出错需要马上打印或者勘误被放进errno中的出错信息。如果没有马上打印或者勘误而继续进行别的操作的话，则errno也会记录其他执行发生的错误。
>
>     * 从定义角度讲，errno本身是一个整型，这是其最早的定义，但是现在已经重构了这个概念，把它私有化之后errno是一个宏了。 
>
> 2. The argument mode points to a string beginning with one of the following sequences (possibly followed by additional characters, as described below)

**01**

```markdown
/usr/include/asm-generic/errno-base.h

/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
#ifndef _ASM_GENERIC_ERRNO_BASE_H
#define _ASM_GENERIC_ERRNO_BASE_H

#  宏名               宏值          errormessage

#define EPERM            1      /* Operation not permitted */
#define ENOENT           2      /* No such file or directory */
#define ESRCH            3      /* No such process */
#define EINTR            4      /* Interrupted system call */
#define EIO              5      /* I/O error */
#define ENXIO            6      /* No such device or address */
#define E2BIG            7      /* Argument list too long */
#define ENOEXEC          8      /* Exec format error */
#define EBADF            9      /* Bad file number */
#define ECHILD          10      /* No child processes */
...
```

```c
#include <errno.h>

errno;
```

这个程序没有main函数也就没了入口地址，因此是运行不起来的。但是可以执行`gcc -E test.c`预处理指令，以#开头的内容都是在预处理阶段完成的。

```markdown
# 37 "/usr/include/errno.h" 3 4
extern int *__errno_location (void) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));
# 52 "/usr/include/errno.h" 3 4

# 2 "test.c" 2

(*__errno_location ())
# 3 "test.c"
    ;
```

这是预处理的结果，如果能看到errno这个变量的话就说明它还是一个整型变量，可是errno已经私有化成为了(*__errno_location ())宏实现，然后把当前出错的内容映射到当前的地址空间上，所以不会和别的程序冲突

**02**

> r      Open text file for reading.  The stream is positioned at the beginning of the file.
>
> r+     Open for reading and writing.  The stream is positioned at the beginning of the file.
>
> w      Truncate file to zero length or create text file for writing.  The stream is positioned at the  beginning  of  the file.
>
> w+     Open for reading and writing.  The file is created if it does not exist, otherwise it is truncated.  The stream is positioned at the beginning of the file.
>           
> a      Open for appending (writing at end of file).  The file is created if it does not exist.  The stream is  positioned at the end of the file.
>
> a+     Open for reading and appending (writing at end of file).  The file is created if it does not exist.  Output is always appended to the end of the file.  POSIX is silent on what the initial read position is when using this  mode. For  glibc,  the initial file position for reading is at the beginning of the file, but for Android/BSD/MacOS, the initial file position for reading is at the end of the file.

* Truncate file to zero length or create text file for writing : 有则清空，无则创建
* beginning of the file : 文件当中第一个有效字符
* the end of the file : 文件最后一个有效字节的下一个位置。文件的读和写都发生在当前位置，如果the end of the file表示的是文件的最后一个字节，则会覆盖掉最后一个字节
* Open for reading and appending (writing at end of file) : 如果进行append操作的话，那么文件位置指针就会在文件末尾处，此时读操作就不好用了。因为如果以a+形式打开文件并读文件，但是文件指针在末尾处，就不好读了，所以以a+形式打开文件之后的指针位置取决于下一步要进行的操作
* 在这六种操作文件方式里，只有r和r+最特殊，因为其他四种方式都有着The file is created if it does not exist字样。r和r+在操作一个文件的时候必须保证文件存在，如果文件不存在则结束当前调用返并回出错信息
* 在其他一些参考书里讲到文件IO的时候，有着类似'r+b'，'w+b'，b表示二进制操作。在windows下区分两种流：文本流和二进制流这两种流在用程序进行控制的时候是不一样的，所以在windows环境下进行编程要指定是用r打开还是用r+b打开。但是在linux环境下不用指定，因为在linux环境下只有流(stream)的概念而不区分是二进制还是文本，除非是程序有可能移植到windows环境下才有必要加上b

> > The mode string can also include the letter 'b' either as a last character or as a character between  the  characters  in any of the two-character strings described above.  This is strictly for compatibility with C89 and has no effect; the 'b' is ignored on all POSIX conforming systems, including Linux.  (Other systems may treat text files and binary  files  differently, and adding the 'b' may be a good idea if you do I/O to a binary file and expect that your program may be ported to non-UNIX environments.)

```c
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int main()
{
    FILE *fp;

    fp = fopen("tmp", "r");
    if (fp == NULL)
    {
        fprintf(stderr, "fopen() failed! error = %d\n", errno);
        exit(1);
    }
    puts("ok!");

    exit(0);
}


# result
# fopen() failed! error = 2

# vim /usr/include/asm-generic/errno-base.h
# #define ENOENT           2      /* No such file or directory */
```

程序出错可能是由于方方面面的问题，比如说找不到文件、权限受限、路径有误...多种原因都会造成当前操作失败，所以说有必要查看出错原因，而当前使用该方法查看出错原因显然不是一个好的办法。这里提几个函数：

1. `perror()`  : 用来把errno转换成errmsg

```c
NAME
       perror - print a system error message

SYNOPSIS
       #include <stdio.h>

       void perror(const char *s);

       #include <errno.h>

       const char * const sys_errlist[];
       int sys_nerr;
       int errno;       /* Not really declared this way; see errno(3) */
```

* const char *s : 是人为规定的字符串，会在该字符串后面加上出错信息从而打印输出

```c
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int main()
{
    FILE *fp;

    fp = fopen("tmp", "r");
    if (fp == NULL)
    {
        // fprintf(stderr, "fopen() failed! error = %d\n", errno);
        perror("fopen()");

        exit(1);
    }
    puts("ok!");

    exit(0);
}


# result
# fopen(): No such file or directory
```

2. `strerror()` : 要包含string.h头文件 

```c
NAME
       strerror, strerror_r, strerror_l - return string describing error number

SYNOPSIS
       #include <string.h>

       char *strerror(int errnum);
```

```c
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

int main()
{
    FILE *fp;

    fp = fopen("tmp", "r");
    if (fp == NULL)
    {
        // fprintf(stderr, "fopen() failed! error = %d\n", errno);
        // perror("fopen()");
        fprintf(stderr, "fopen():%s\n", strerror(errno));

        exit(1);
    }
    puts("ok!");

    exit(0);
}


# result
# fopen():No such file or directory
```

# fclose和文件权限问题

fopen的返回值是一个指针，此时需要思考这个指针指向的空间是哪一块？
下面给出三种可能的情况1. 栈；2. 静态区；3. 堆

1. 如果返回指针所指空间存在于栈上(×)

```c
FILE *fopen(const char *pathname, const char *mode)
{
    # 1. 定义FILE类型变量
	FILE tmp；
    # 2. 给结构体内的成员变量赋值
    tmp.xxx = xxx;
    tmp.yyy = yyy;
    ......
    
    # 3. 返回指针
    return  &tmp;
}
```

这种方式是不对的，因为这段函数企图返回一个局部变量的地址，而这个局部变量tmp会在函数调用结束后比回收，从而使得tmp指针失效并且释tmp指针空间

2. 如果返回指针所指空间存在于静态区上(×)

```c
FILE *fopen(const char *pathname, const char *mode)
{
    # 1. 定义FILE类型静态变量
	static FILE tmp；
    # 2. 给结构体内的成员变量赋值
    tmp.xxx = xxx;
    tmp.yyy = yyy;
    ......
    
    # 3. 返回指针
    return  &tmp;
}
```

因为tmp变量被存放在了静态区上，所以当函数调用执行完时tmp空间还会被保留直到进程结束为止。但是此时就会有这样一个问题：保存在静态区上的变量有一个特点就是函数被重复调用的时候，被static修饰的变量只会被声明一次，也就是说即使调用10次fopen，给tmp变量分配的空间也只有一块。这就会导致打开第一个文件用FILE\*操作是正确的，然后打开第二个文件时依然会返回一个FILE\*，但是第二个文件填充的结构体就是第一个文件填充的结构体，也就是说第二次的结果会把第一次的结果给覆盖掉，即之前的文件就没法用了

3. 如果返回指针所指空间存在于堆上(√)

```c
FILE *fopen(const char *pathname, const char *mode)
{
    # 1. 定义FILE类型指针变量
	FILE *tmp = NULL；
    tmp = malloc(sizeof(FILE));
    
    # 2. 给结构体内的成员变量赋值
    tmp->xxx = xxx;
    tmp->yyy = yyy;
    ......
    
    # 3. 返回指针
    return  tmp;
}
```

fopen做了一步动态分配空间的操作即malloc，还有一个对应的函数fclose()，与malloc相对应的free函数就定义在fclose()中。如果一个函数的返回值是指针并且有一个与之对应的逆操作函数，比如fopen与fclose，则多半可以确定这个函数返回的指针就存放在堆上。如果没有互逆操作，则可能存放在堆上也有可能存放在静态区上，这需要代码去验证。

>int fclose(FILE *stream)
>
>* FILE *stream : 通过fopen成功打开的一个流
>
>1. 一般情况下fclose的返回值很少去校验，因为一般我们认为fclose不会失败
>2. RETURN VALUE
>    Upon successful completion, 0 is returned.  Otherwise, EOF is returned and errno is set to indicate the error.  In either case, any further access (including another call to fclose()) to the stream results in undefined behavior.

2. EOF是typedefine出来的一个宏，宏值一般情况下是-1，但即便告知了EOF的宏值一般情况为-1，也应该去验证EOF这个宏名，因为宏值未必是-1。有宏名用宏名，不要轻易用宏值

```c
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

int main()
{
    FILE *fp;

    fp = fopen("tmp", "w");
    if (fp == NULL)
    {

        fprintf(stderr, "fopen():%s\n", strerror(errno));

        exit(1);
    }
    puts("ok!");

    fclose(fp);

    exit(0);
}
```

以上程序经过了`打开文件->ok->释放文件`这几步操作

在linux编程领域里有几个概念需要强调：1. 谁打开谁关闭；2. 谁申请谁释放；3. 一切皆文件等设计原理，还有一个重要的理念就是`是资源就有上限`，当前可以在一个进程当中利用fopen打开文件，但是打开文件个数一定是有上限的。不管这个上限设计的有多大，100个也好，10个也好但都是有上限的。就比如在写递归函数的时候也要有一个上限的控制，那么现在来做一个实验看下一个进程的空间最多能打开多少个文件？

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int main(void)
{
    FILE *fp;
    int count = 0;

    while (1)
    {
        fp = fopen("tmp", "r");
        if (fp == NULL)
        {
            perror("fopen()");
            break;
        }
        count++;
    }

    printf("count = %d\n", count);

    exit(0);
}


# result
# fopen(): Too many open files
# count = 8174
```

程序结果显示在该进程下最多能打开8174个文件，但其实不是。我们提到stream流的概念，在不更改当前环境的情况下，实际上有3个流在一个进程产生的时候就默认打开了：1. stdin；2. stdout；3. stderr，所以当前进程最多能打开的文件个数是8174+3=8177个
我们在程序目录下使用`ulimit -a`命令就能显示当前进程最多打开的文件个数

* 这里的最多文件打开数为什么是8192而不是8174暂且不知，应该是有其他的流在占用这文件打开数

* -a表示查看所有项

* ulimit -n 数字 : 更改最多文件打开数

```markdown
core file size          (blocks, -c) 0
data seg size           (kbytes, -d) unlimited
scheduling priority             (-e) 0
file size               (blocks, -f) unlimited
pending signals                 (-i) 31399
max locked memory       (kbytes, -l) 65536
max memory size         (kbytes, -m) unlimited
open files                      (-n) 8192
pipe size            (512 bytes, -p) 8
POSIX message queues     (bytes, -q) 819200
real-time priority              (-r) 0
stack size              (kbytes, -s) 8192
cpu time               (seconds, -t) unlimited
max user processes              (-u) 31399
virtual memory          (kbytes, -v) unlimited
file locks                      (-x) unlimite
```

执行`ulimit -n 1024`指令后的程序结果

```c
# result
# fopen(): Too many open files
# count = 1006
```

我们在调用fopen函数打开tmp文件时使用的时'w'形式，tmp文件的属性是'-rw-rw-r--'，即664。在创建文件的时候并没有提供参数或者接口去指定文件权限，所以文件权限并不是凭空出来的，文件权限遵循公式`0666 & ~umask`，这里的umask值为0002，umask存在的意义就是为了防止产生权限过松的文件，umask的值越大，文件权限值就会越低。比如说写一个进程，我怕这个进程出现bug被别人利用，我们就可以禁止产生文件或者是利用umask去产生权限很低的文件

```markdown
# /home/liangruuu/study/linuxc/code/io/stdio

drwxrwxr-x 2 liangruuu liangruuu  4096 Mar 17 10:23 ./
drwxrwxr-x 3 liangruuu liangruuu  4096 Mar 17 07:39 ../
-rw-rw-r-- 1 liangruuu liangruuu    26 Mar 17 07:50 errno.c
-rwxrwxr-x 1 liangruuu liangruuu 17016 Mar 17 10:12 fopen*
-rw-rw-r-- 1 liangruuu liangruuu   389 Mar 17 10:13 fopen.c
-rwxrwxr-x 1 liangruuu liangruuu 16832 Mar 17 10:23 maxfopen*
-rw-rw-r-- 1 liangruuu liangruuu   351 Mar 17 10:23 maxfopen.c
-rw-rw-r-- 1 liangruuu liangruuu     0 Mar 17 10:12 tmp
```

# fgetc和fputc

以后凡是碰到函数的返回值是指针的情况，要多问自己一句这个指针指向的内容是静态区里的一块地址还是堆上的一块地址

> // 二进制字符的读写
>
> 1. fgetc();    
> 2. fputc();
>
> // 字符串读写
>
> 1. fgets();   
> 2. fputs();
>
> // 二进制数据块的操作
>
> 1. fread();    
> 2. fwrite();

* getchar()

> SYNOPSIS
>
> > int fgetc(FILE *stream);
> >
> > char *fgets(char *s, int size, FILE *stream);
> >
> > int getc(FILE *stream);
> >
> > int getchar(void);
> 
> DESCRIPTION
>
> > fgetc()  reads the next character from stream and returns it as an unsigned char cast to an int, or EOF on end of file or error.
>>
> > getc() is equivalent to fgetc() except that it may be implemented as a macro which evaluates stream more than once.
> >
> > getchar() is equivalent to getc(stdin).
> >
> > fgets() reads in at most one less than size characters from stream and stores them into  the  buffer  pointed  to  by  s. Reading  stops  after  an EOF or a newline.  If a newline is read, it is stored into the buffer.  A terminating null byte ('\0') is stored after the last character in the buffer.
> 
> 1. 有三个标准流是被默认打开的：stdin、stdout、stderr，getchar()在功能上属于字符读入的函数，字符默认是从标准的输入设备上来的
> 2. getc() is equivalent to fgetc()：getc又相当于fgetc
> 3. getc和fgetc除了函数名不同其余都相同

2. getc获取的内容不仅仅局限于终端，它可以指定从任意成功打开的流中获取内容，它的返回值和getchar是一样的，返回的是读到字符的整型形式。因为读到的是一个unsinged char型数据，但是防止出错所以就把返回值用整型数据来代替，如果失败或者读到文件末尾了则返回EOF。

    > RETURN VALUE
    >
    > > fgetc(), getc() and getchar() return the character read as an unsigned char cast to an int or EOF on end of file  or  error.

 	3. 这两个函数一个会被定义成宏，一个会被定义成函数。getc()最原始的定义会被定义成宏使用，fgetc()被定义成函数。关于函数和宏的区别主要在于要写哪一部分内容。在数据结构中，内核链表通篇都是用宏来进行实现了，没有任何函数，因为内核在帮助节省一点一滴的时间，宏不占用调用时间，只占用编译时间，函数的调用则恰恰是相反的。但如果是在写应用态程序的话，建议以函数形式为主，因为要的是稳定安全

* putchar()

> SYNOPSIS
>
> >  int fputc(int c, FILE *stream);
> >
> >  int fputs(const char *s, FILE *stream);
> >
> >  int putc(int c, FILE *stream);
> >
> >  int putchar(int c);
> >
> >  int puts(const char *s);
>
> DESCRIPTION
>
> > fputc() writes the character c, cast to an unsigned char, to stream.
> >
> > fputs() writes the string s to stream, without its terminating null byte ('\0').
> >
> > putc() is equivalent to fputc() except that it may be implemented as a macro which evaluates stream more than once.
> >
> > putchar(c) is equivalent to putc(c, stdout).
> >
> > puts() writes the string s and a trailing newline to stdout.
> >
> > Calls  to  the  functions  described  here can be mixed with each other and with calls to other output functions from the stdio library for the same output stream.
>
> 1. putchar(c) is equivalent to putc(c, stdout) : 把指定的字符输出到stdout上，putchar相当于putc(c, stdout)的二次封装，而putc又相当于fputc
>
> 2. int putc(int c, FILE *stream) : 指的是指定一个输出项，但是它的走向是走到一个指定的流上，这个流可以是标准的输出、出错也可以是任何的成功打开的文件
>
> 3. fputc和putc跟fgetc和getc的区别是一样的

> 实现文件copy的功能，指令为`mycp src dest`

* 以下代码为程序骨架

```c
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    fopen();
    fopen();
    
    while(1)
    {
        fgetc();
        fputc();
    }
    
    fclose();
    fclose();
    
    exit(0);
}
```

1. 6-7&15-16：由于有两个文件，所以要调用两次fopen，然后理所应当谁打开谁关闭，所以都要调用两次fclose
2. 9-13：应该是从src中读一块往dest中写一块，用fgetc从源文件当中读一个字符，再往目标文件中写一个字符，而这个过程是需要放在一个循环当中的，即第一次写一次，并且有必要的话需要校验每一条语句的输出情况

* 完整代码

```c
# mycp.c

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    FILE *fps, *fpd;
    int ch;

    if (argc < 3)
    {
        fprintf(stderr, "Usage:%s <src_file> <dest_file>\n", argv[0]);
        exit(1);
    }

    fps = fopen(argv[1], "r");
    if (fps == NULL)
    {
        perror("fopen()");
        exit(1);
    }

    fpd = fopen(argv[2], "w");
    if (fpd == NULL)
    {
        perror("fopen()");
        fclose(fps);
        exit(1);
    }

    while (1)
    {
        ch = fgetc(fps);
        if (ch == EOF)
            break;
        fputc(ch, fpd);
    }

    fclose(fpd);
    fclose(fps);

    exit(0);
}
```

1. 11-15：在程序中写足够多的注释或者说明性的内容，只要用到命令行的传参则必须判断命令行的参数
2. 15-20：以只读方式打开源文件并且返回FILE\*，并且马上判断有无出错，选择r而不选择r+就是为了当源文件不存在时报错
3. 22-28：以只写方式打开目标文件并且返回FILE\*，并且马上判断有无出错
4. 38-39：首先关闭依赖别人的文件对象再关闭被依赖的文件对象
5. 7：ch被定义为int类型数据而不是char*，因为当getc出错时返回的值为-1，字符型数据的值没有负值
6. 30-36：fgetc从源文件读取数据，返回值为读取到的字符并且以整型接收然；EOF被判断为文件末尾；fputc把ch值写入目标文件
7. 28：当src文件成功打开，但是目标文件打开失败的话，按照之前代码的逻辑就会出现内存泄漏。因为当src文件成功打开时就有了一个合法的文件指针指向一个FILE结构体，如果之后文件读取失败的话，还没来得及关闭之前的文件指针。所以在判断fpd==null的代码体里需要加上对之前已打开文件的关闭逻辑

```c
# result

# ./mycp /etc/services /tmp/out
# diff /etc/services /tmp/out
# diff指令无输出则表明两个文件相同

# ./mycp
# Usage:./mycp <src_file> <dest_file>
```

> 用fpetc实现小功能：文件有属性，其中有一个size值代表文件的大小，指的是文件当中的有效字符个数，当我们有了之前介绍的几个函数之后，我们就可以测试一个文件有多少个有效字符

```markdown
total 84
drwxrwxr-x 2 liangruuu liangruuu  4096 Mar 17 13:20 ./
drwxrwxr-x 3 liangruuu liangruuu  4096 Mar 17 07:39 ../
-rw-rw-r-- 1 liangruuu liangruuu    26 Mar 17 07:50 errno.c
-rwxrwxr-x 1 liangruuu liangruuu 17016 Mar 17 10:12 fopen*
-rw-rw-r-- 1 liangruuu liangruuu   389 Mar 17 10:13 fopen.c
-rwxrwxr-x 1 liangruuu liangruuu 16832 Mar 17 10:23 maxfopen*
-rw-rw-r-- 1 liangruuu liangruuu   351 Mar 17 10:23 maxfopen.c
-rwxrwxr-x 1 liangruuu liangruuu 17000 Mar 17 13:20 mycp*
-rw-rw-r-- 1 liangruuu liangruuu   634 Mar 17 12:47 mycp.c
-rw-rw-r-- 1 liangruuu liangruuu     0 Mar 17 10:12 tmp
```

```c
# fgetc.c

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    FILE *fp;
    int count = 0;

    if (argc < 2)
    {
        fprintf(stderr, "Usage:%s <src_file>\n", argv[0]);
        exit(1);
    }

    fp = fopen(argv[1], "r");
    if (fp == NULL)
    {
        perror("fopen()");
        exit(1);
    }

    while (fgetc(fp) != EOF)
    {
        count++;
    }

    printf("count = %d\n", count);

    fclose(fp);

    exit(0);
}
```

1. 9：计数器count，表示读取了多少个字符，假设当前用来测试的文件不会超过整型的最大返回，如果要测试大文件的话数据类型可能会溢出，这时就需要考虑用long类型或者long long类型来进行接收
2. 24-27：不需要判断读取到的具体字符，只需要直到读到的是否是EOF，即文件末尾，如果不是的话就代表读取到的字符是有效字符

```c
# result

# ./fgetc ./fopen.c
#count = 389
```

# fread和fwrite

有一个函数叫做gets，gets()很危险不应该去使用，因为其不检查缓冲区的溢出

>NAME
>
>> gets - get a string from standard input (DEPRECATED) 
>
>BUGS
>
>> Never  use  gets().  Because it is impossible to tell without knowing the data in advance how many characters gets() will read, and because gets() will continue to store characters past the end of the buffer, it is extremely dangerous to  use. It has been used to break computer security.  Use fgets() instead.
>
>1. Use fgets() instead.

1. 查看fgets()函数具体内容

> SYNOPSIS
>
> > char *fgets(char *s, int size, FILE *stream);
>
> 1. gets()危险的地方在于函数传参只约定了一个地址，从终端上接收内容，这些内容没有放到指定的地址空间上去，而是放到了输入缓冲区中，只有当用户键入回车键的时候这些内容才会被放到所指定的地址上去，这个输入缓冲区我们是不知道其大小的，因此当我们从键盘或者其他IO设备输入过大的数据时可能会导致数据大小大于输入缓冲区大小，从而导致缓冲区溢出
> 2. fget()指定输入内容被存放至指针s所指向的地址空间上去，并且最多能接受size个字符，数据是从指定的流stream上来的，返回值char也代表了指针s指向的地址本身
> 3. fgets()的正常结束有两种情况：
>     * 读到了size-1个有效字节，剩余的一个字节是留给'\0'的
>     * 读到了'\n'

3. 我们使用fgets()来读取字符

```markdown
# define SIZE 5

char buf[SIZE];
fgets(buf, SIZE, stream);

1. 第一种情况读到size-1个字符
2. 第二种情况读到'\n'

# 内容是abcdef
1. 第一次去读的话，buf空间里分别存储的是a b c d '\0'，当前文件中文件的位置指针是定位在字符e，因为下一个要读取的字符为e，即读取size-1=4个字符再加上一个'\0'。

# 内容是ab
1. 即使是最后一行的内容，句末也会有一个换行符。举一个不是很贴切的例子，windows环境下的word，word一旦打开如果把显示换行标记的设置打开，就会看到新建一个word文档什么还没有输入的时候其实这个word文档就默认有了一个换行符。再比如用vim打开一个文档，只要按一下i进入编辑模式，其实第一行默认就是一个换行符，只不过是没有有效的字符内容罢了，有效的字节和字节是两个概念，很多函数在统计文件大小的时候是统计有效的字符个数，而不是字符个数，即不去统计诸如空格，回车等，这点需要区分。
2. 如果用fgets(buf, SIZE, stream)去读取内容的时候，buf空间里存储的是a b '\n' '\0'，并且只读取了3个字符，'\0'不能算做读取的字符

# 内容是abcd
1. 如果用fgets(buf, SIZE, stream)去读取该内容，需要几次读完？
2. 因为每一次读取都只读取size-1个字符，所以第一次读取内容a b c d 再加一个'\0'，此时的文件位置指针指向的是串尾，即字符d的下一个字符，因为每一行的字符串末尾都有一个换行符'\n'
3. 第二次读取内容'\n' '\0'

```

还有一个函数叫做puts()

> NAME
>
> > fputc, fputs, putc, putchar, puts - output of characters and strings
>
> SYNOPSIS
>
> > int fputc(int c, FILE *stream);
> >
> > int fputs(const char *s, FILE *stream);
> >
> > int putc(int c, FILE *stream);
> >
> > int putchar(int c);
> >
> > int puts(const char *s);
>
> 1. int puts(const char *s)：从起始地址开始输出直到遇到'\0'为止
> 2. int fputs(const char *s, FILE *stream)：把指定的串不一定输出到stdout上，可以输出到任意一个打开的流上去
>
> RETURN VALUE
>
> > fgets() returns s on success, and NULL on error or when end of file occurs while no characters have been read.
>
> 

> 用fgets()和fputs()实现mycpy

```c
#include <stdio.h>
#include <stdlib.h>

#define BUFSIZE 1024

int main(int argc, char **argv)
{
    FILE *fps, *fpd;
    char *buf[BUFSIZE];

    if (argc < 3)
    {
        fprintf(stderr, "Usage:%s <src_file> <dest_file>\n", argv[0]);
        exit(1);
    }

    fps = fopen(argv[1], "r");
    if (fps == NULL)
    {
        perror("fopen()");
        exit(1);
    }

    fpd = fopen(argv[2], "w");
    if (fpd == NULL)
    {
        perror("fopen()");
        fclose(fps);
        exit(1);
    }

    while (fgets(buf, BUFSIZE, fps) != NULL)
    {
        fputs(buf, fpd);
    }

    fclose(fpd);
    fclose(fps);

    exit(0);
}

```

* 9：因为fgets第一个参数需要一个缓冲区间的地址，所以定义一个字符缓冲区间
* 32-34：读一块写一块，并且需要进行跳出死循环的判断。因为fgets读取成功返回fps指针，读取失败或者读取到文件尾则返回NULL，此时返回的不是EOF

```c
# result

# ./mycp_fgets /etc/services /tmp/out
# diff /etc/services /tmp/out

```

* fread()和fwrite()

> NAME
>
> > fread, fwrite - binary stream input/output
>
> SYNOPSIS
>
> > size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
> >
> > size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);
>
> 1. size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream)：从一个指定的stream流中读取数据并且放到指针ptr所指向的地址空间去，读nmemb个数据对象，每个数据对象大小为size。也就是说ptr这块起始位置的地址总大小为size * nmemb
> 2. size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)：用const修饰ptr指针表示fwrite函数对源数据只读不写，其余参数同fread
>
> 3. 为什么很多人喜欢用这两个函数？

3. 比如有这么一种情况，要实现一个学校的人员数据管理系统，每个学生或老师要存放的数据就是固定那么多，比如姓名、年龄、籍贯......多半时候可以把它抽象成一个定长的结构体。那么把这些数据写到文件中去，那么就可以使用fwrite函数这么做：指针ptr指向元素数据，每个学生数据的结构体大小为size，要往文件stream里写nmemb=100个学生数据，使用fread同理，非常好操作，因此很多人比较倾向于在处理成块的数据时使用这两个函数。但是这两个函数有着致命的缺陷：并没有验证数据边界，比如说学生结构体大小为40个字节，那么每次操作都是在读写40个字节大小数据，这个过程能不能允许数据传输出错的，中间只要有1个字节出错那么整个数据传输过程都会出错。

> RETURN VALUE
>
> > On  success,  fread()  and  fwrite()  return the number of items read or written.  This number equals the number of bytes transferred only when size is 1.  If an error occurs, or the end of the file is reached, the return value is a short item count (or zero).
>
> 1. return the number of items read or written：如果当前读或者写不够一个对象(nmemb)的话，则当前读或写返回值就为0或者为失败 	

```markdown
# fread(buf, size, nmemb, fp);

# 1. 假设buf是10个字节大小的char型数组，并且当前文件当中的数据量非常足够，也就是绝对超过10个字节
# 第一种读取方式
fread(buf, 1, 10, fp);
# fread返回值为10，因为能够成功读取10个对象，每个对象大小为1字节

# 第二种读取方式
fread(buf, 10, 1, fp);
# fread返回值为1，一个对象大小为10字节


# 2. 文件大小只有5个字节
# 第一种读取方式
fread(buf, 1, 10, fp); 
# 文件只有5个字节，也就是说有5个对象存在，那么fread返回值为5，即5个对象

# 第二种读取方式
fread(buf, 10, 1, fp);
# 文件大小不够一个读取对象，所以fread返回值为0
```

fread和fwrite这两个函数只能去执行数据块工工整整的操作，还不太保险，文件位置指针是要稍微偏移一个位置，后面的全部跟着偏移一个位置，fread和fwrite保险起见永远采用单字节来操作，也就是把它当做fgetc和fputc来使用

> 用fread和fwrite实现my_cpy

```c
#include <stdio.h>
#include <stdlib.h>

#define BUFSIZE 1024

int main(int argc, char **argv)
{
    FILE *fps, *fpd;
    char buf[BUFSIZE];
    int n = 0;

    if (argc < 3)
    {
        fprintf(stderr, "Usage:%s <src_file> <dest_file>\n", argv[0]);
        exit(1);
    }

    fps = fopen(argv[1], "r");
    if (fps == NULL)
    {
        perror("fopen()");
        exit(1);
    }

    fpd = fopen(argv[2], "w");
    if (fpd == NULL)
    {
        perror("fopen()");
        fclose(fps);
        exit(1);
    }

    while ((n = fread(buf, 1, BUFSIZE, fps)) > 0)
    {
        fwrite(buf, 1, n, fpd);
    }

    fclose(fpd);
    fclose(fps);

    exit(0);
}
```

* 33-36：很多人会把这部分的代码写成如下形式，即读BUFSIZE个对象，写BUFSIZE个对象。这是肯定错误的，因为无法保证用fread函数满打满算读取BUFSIZE个或者BUFSIZE整倍数个的对象，fread也好、fwrite也好，它们的返回值是能正常读取或者写入的对象个数，所以这里应该是读到n个对象，写入n个对象，并且加上判断逻辑

    ```c
    while (fread(buf, 1, BUFSIZE, fps))
    {
        fwrite(buf, 1, BUFSIZE, fpd);
    }
    ```

* 10：定义n值



# 标准IO-printf和scanf函数族

> SYNOPSIS
>
> > #include <stdio.h>
> >
> > int printf(const char *format, ...);
> >
> > int fprintf(FILE *stream, const char *format, ...);
> >
> > int sprintf(char *str, const char *format, ...);
> >
> > int snprintf(char *str, size_t size, const char *format, ...);
>
> 1. int printf(const char *format, ...)：作用是把省略号部分的输出项所对应的值按照前面format当中约定的格式输出到stdout流上
> 2. int fprintf(FILE *stream, const char *format, ...)：当前流往哪里输出就往哪走向，最好不要把所有的输出都放在stdout流上，把省略号部分的输出项按照format指定格式放到指定的FILE *stream上去
> 3. 在标准IO中有一个类型是贯穿始终的，那就是FILE
> 4. int sprintf(char *str, const char *format, ...)：把省略号部分代表的内容按照format格式输出到字符串str当中去
> 5. sprintf可以把不同格式的字符拼接起来并且存入到一个字符串中
> 6. int snprintf(char *str, size_t size, const char *format, ...)：把省略部分的输出项按照format格式输出到字符串str当中去，str指针所指地址空间大小为size

2. 类似于与mycpy.c文件中，当命令行传参不正确的时候就用fprintf来报错，并把出错信息往stderr流上输出。stdin也就是标准输入，比如键盘鼠标等设备，stdout、stderr都指向目前的标准的输出设备，比如显示屏。这些都是默认的设置，我们也可以根据需要去改变它们，比如说现在有需要把stderr不再指向终端，而是指向一个打开文件，这就类似于linux里重定向echo的概念。所以经常使用fprintf把报错信息给输出到stderr上，因为stderr不仅能够输出到显示器上，也能通过设置从而输出重定向到一个文件里而不是终端上，有点类似于记录报错日志的感觉，fprintf首字母f代表的就是一个流(FILE*)

    ```c
    if (argc < 3)
    {
        fprintf(stderr, "Usage:%s <src_file> <dest_file>\n", argv[0]);
        exit(1);
    }
    ```

    <img src="index.assets/image-20220318123026437.png" alt="image-20220318123026437" style="zoom:80%;" />

4. atoi()：把一个串转换成一个整型数

> SYNOPSIS
>
> > #include <stdlib.h>
> >
> > int atoi(const char *nptr);
>
> 1. int atoi(const char *nptr)：传入参数是一个字符串，返回值是整型值。如果字符串中夹杂着字符的话，就会拿到字母或者尾0为止

```c
# atoi.c

#include <stdio.h>
#include <stdlib.h>

int main(void)
{

    char str[] = "123456";
    printf("%d\n", atoi(str));
    char str2[] = "123a56";
    printf("%d\n", atoi(str2));

    exit(0);
}
```

```c
# result 
# 123456
# 123
```

5. 比如说把一些信息综合起来，而最终得到的完整信息是有id、串、结构体、float...换句话说需要一种综合的数据类型

```c
# atoi.c

#include <stdio.h>
#include <stdlib.h>

int main(void)
{

    int year = 2020, month = 12, day = 30;
    printf("%d-%d-%d\n", year, month, day);

    exit(0);
}

```

```c
# result 
# 2020-12-30
```

* 10：这行代码能做出一种假象即输出了“2020-12-30”这行字符串，但是却没办法当做一个完整的串来处理。sprintf函数却能把多种不同的数据类型按照特定一对一的格式把它放在字符串中

```c
# atoi.c

#include <stdio.h>
#include <stdlib.h>

int main(void)
{

    char buf[1024];
    int year = 2020, month = 12, day = 30;
    sprintf(buf, "%d-%d-%d", year, month, day);
    puts(buf);

    exit(0);
}
```

* 12：现在就拿到了一个综合起来的串用来承载所要求的的年月日输出，并且是以一个完整的串输出的

6. 在谈到gets函数的时候提到了用fgets来替代它，因为gets是不检查缓冲区大小的，即不知道buffer有多大，所以很容易造成缓冲区溢出。同理sprintf依然存在相同的问题：str所指向的空间大小未知，所以就用snprintf来解决这个缓冲区溢出的问题，所以在写程序的时候就要有意识的去注意内存溢出或者内存泄露的问题。str所指地址空间所能容纳的数据大小为size-1个字节，因为要给尾0预留一个字节。

> SYNOPSIS
>
> > #include <stdio.h>
> >
> > int scanf(const char *format, ...);
> > int fscanf(FILE *stream, const char *format, ...);
> > int sscanf(const char *str, const char *format, ...);
>
> 1. int scanf(const char *format, ...)：从终端获取内容并且按照format格式放到省略号代表的一个个地址上去
> 2. int fscanf(FILE *stream, const char *format, ...)：从一个指定流中获取，当然这个流包括stdin，按照format格式放到地址当中去
> 3. int sscanf(const char *str, const char *format, ...)：数据来源是一个字符串，其余同理
> 4. 在scanf一系列的函数中要慎重使用%s这个格式，因为在format部分设置%s，则需要在省略号部分给出地址，但是这一步非常危险，因为在终端输入或者文件取数据的时候是不清楚有待拿的字符串是有多长的，所以依然是看不到目标位置有多大，这是scanf在使用时候最大的缺陷之一

# 标准IO-文件位置函数和缓冲区刷新函数

操作文件位置指针

> NAME
>
> > fgetpos, fseek, fsetpos, ftell, rewind - reposition a stream
>
> SYNOPSIS
>
> > #include <stdio.h>
> >
> > int fseek(FILE *stream, long offset, int whence);
> >
> > long ftell(FILE *stream);
> >
> > void rewind(FILE *stream);
> >
>
> 1. int fseek(FILE *stream, long offset, int whence)
>     * FILE *stream：指定要操作的FILE流
>     * long offset：偏移量，即要偏移多大的字符
>     * int whence：相对位置，意为从哪个位置开始偏移。位置有三种选项，分别是：SEEK_SET,  SEEK_CUR, SEEK_END
>     * return int：使用feek()函数成功的话返回值为0，否则返回值为-1并且设置errno
> 2. long ftell(FILE *stream)：反映当前文件位置指针在哪
>     * FILE *stream：指定要操作的FILE流
> 3. void rewind(FILE *stream)
>     * FILE *stream：指定要操作的FILE流
> 4. fseek也可以帮助完成一个空洞文件
>
> DESCRIPTION
>
> > The fseek() function sets the file position indicator for the stream pointed to by stream.  The new position, measured in bytes, is obtained by adding offset bytes to the position specified by whence.  If whence is set to  SEEK_SET,  SEEK_CUR, or  SEEK_END,  the  offset  is relative to the start of the file, the current position indicator, or end-of-file, respecively.  A successful call to the fseek() function clears the end-of-file indicator for the stream and undoes any effects of the ungetc(3) function on the same stream.
>
> RETURN VALUE
>
> > The  rewind() function returns no value.  Upon successful completion, fgetpos(), fseek(), fsetpos() return 0, and ftell() returns the current offset.  Otherwise, -1 is returned and errno is set to indicate the error.
>
> 

在讲到fopen函数的时候说到过，由于打开文件方式不同导致文件位置指针不同，有的在文件首，有的在文件尾，而且还有一个概念叫做当前位置，文件的读和写都是发生在当前位置。

```c
// 假设需要这样的实现：

// 打开文件
fp = fopen();

// 利用fputc()往指针fp指向的文件每次写入一个字符，并且执行10次
fputc(fp) * 10

// 想要读出写入文件的10个字符
fgetc(fp) * 10
```

我们会认为想要读出写入文件的10个字符就用`fgetc(fp) * 10`代码去实现，其实不是这样的。因为文件当中有个文件位置指针，文件位置指针就特别像眼睛一样，比如说眼睛在读报纸的实收是顺序依次向后看的，没有人在读报纸的时候是永远在读第一个字节的，所以在读写文件的时候也是同样的情况，读/写完第一个字节然后第二个第三个...文件位置指针所在的位置被称为当前位置，读和写一定是发生在当前位置的，所以之前给出的伪代码是显然无法实现读取10个被写入相同文件中的字符的，因为在利用fputc写进10个字符的时候，文件位置指针已经向后偏移了，现在是指针第11个字符的位置，如果在这个位置读取10次的话，其实文件位置指针是向后移动再读取10个。

```c
// 打开文件
fp = fopen();

// 利用fputc()往指针fp指向的文件每次写入一个字符，并且执行10次
fputc(fp) * 10
    
fclose();

fp = fopen();

// 想要读出写入文件的10个字符
fgetc(fp) * 10
```

如果没有其他机制介入的话，实现该读写操作的方法就是关闭再打开，让文件位置指针重新定位到文件开头。而且假设这10个字符是在文件的中间位置写的，那么即使关掉文件再打开也无法实现该功能，因为无法找到刚才的位置，而本节介绍的函数就是来解决这些问题的，所以他们的功能被叫做reposition a stream，即重新定位一个流。当然我们会有一些相对位置，是要定位到文件首还是文件尾，还是说当前位置向前多少、向后多少...

1. SEEK_SET表示文件头，SEEK_CUR代表文件当前位置，SEEK_END表示文件尾。如果想要使用fseek实现读取写入文件的10个字节的话，伪代码如下：

```markdown
fp = fopen();

fputc(fp) * 10
    
# 把指针定义到文件开头：偏移量为0，然后偏移的位置设置为SEEK_SET
fseek(fp, 0, SEEK_SET);
# 如果字符不是从文件首开始写的，向前偏移10个字节就能读到刚才写的10个字节了
fseek(fp, -10, SEEK_CUR);


fgetc(fp) * 10
```

2. ftell经常和fseek放在一起使用，即其实我们可以用另外一种方式获取文件大小。在之前的代码中我们其实写过一份代码用来获得文件大小，现在我们使用feek和ftell来获取文件大小，代码见文件`flen.c`

```c
// fget.c

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    FILE *fp;
    int count = 0;

    if (argc < 2)
    {
        fprintf(stderr, "Usage:%s <src_file>\n", argv[0]);
        exit(1);
    }

    fp = fopen(argv[1], "r");
    if (fp == NULL)
    {
        perror("fopen()");
        exit(1);
    }

    while (fgetc(fp) != EOF)
    {
        count++;
    }

    printf("count = %d\n", count);

    fclose(fp);

    exit(0);
}
```

```c
#include <stdio.h>
#include <stdlib.h>

#define BUFSIZE 1024

int main(int argc, char **argv)
{
    FILE *fp;

    if (argc < 2)
    {
        fprintf(stderr, "Usage:%s <file>\n", argv[0]);
        exit(1);
    }

    fp = fopen(argv[1], "r");
    if (fp == NULL)
    {
        perror("fopen()");
        exit(1);
    }

    fseek(fp, 0, SEEK_END);
    printf("%ld\n", ftell(fp));

    fclose(fp);

    exit(0);
}

```

* 23-24：不管文件之前的位置指针在哪，即不管用什么方式打开的，都需要把第三个参数设置为文件末尾，然后利用ftell()，它会反映文件位置指针的位置，此时ftell的返回值就代表着文件内容字节数

```markdown
# result

 # ./flen flen.c
 > 414
 
 # ls -l flen.c
 > -rw-rw-r-- 1 liangruuu liangruuu 414 Mar 18 18:39 flen.c
```

3. rewind()不需要返回值，也就是我们人为rewind()函数不会出错，它的功能等同于代码`(void) fseek(stream, 0L, SEEK_SET)`，所以rewind完成的功能是不管文件位置指针在哪，只要在程序中执行了rewind，那么文件位置指针一定指向了文件开始处。所以在01给出的代码可以以rewind的形式给出

```markdown
fp = fopen();

fputc(fp) * 10
    
rewind(fp);


fgetc(fp) * 10
```

4. 空洞文件中全部或者一部分充斥的是字符0，这里的字符0不是指用单引号括起来的'0'，而是字符的ASCII码值为0的哪个特殊的字符，也就是指的是空字符。大家都用过下载工具比如说迅雷，用这些下载工具下载一部2GB大小的电影，会在建立下载任务之后在磁盘上马上创建一个文件，那么基本上下载工具产生的那个文件一被创建就应该是源文件的大小，而不是从1个字节大小慢慢涨到2GB大小，因为下载工具不会在下载到一半的时候然后告知空间不足，它会在建立下载任务的时候磁盘容量是多少、当前文件有多大等等，类似于在steam上下载游戏时的弹窗界面显示信息。那么空洞文件就是刚刚建立下载任务时被创建的文件，所要产生的效果就是先占有磁盘空间，当创建一个文件的时候，文件大小本应该为0，下载工具在文件产生的时候立马调用fseek，把产生文件打开并且给它的大小从SEEK_SET开始直接延伸到2G大小的磁盘空间，这2G个空间里存放的全部都是字符0，然后把这2G空间切成片，用多进程或多线程进行每一小块的下载。

> NAME
>
> > fflush - flush a stream
>
> SYNOPSIS
>
> > #include <stdio.h>
> >
> > int fflush(FILE *stream);
>
> 1. int fflush(FILE *stream)：刷新流
>     

1. 给一个小例子

```c
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    int i;

    printf("before while(1)");
    while(1);
    printf("after while(1)");

    exit(0);
}

```

直觉上来看最后程序的结果是只输出"before while(1)"，在死循环后被定义的打印不出来，可事实是两个都打印不出来

<img src="index.assets/image-20220318200239323.png" alt="image-20220318200239323" style="zoom:80%;" />
printf一系列的函数往标准终端进行输出的时候，标准输出是典型的行缓冲模式，即碰到换行或者是一行内容满了的情况下刷新缓冲区，所以在没有特殊格式要求的情况下printf上要加上\n

<img src="index.assets/image-20220318200628352.png" alt="image-20220318200628352" style="zoom:80%;" />

还有一种方式就是使用fflush函数强制刷新缓冲区

> DESCRIPTION
>
> > If the stream argument is NULL, fflush() flushes all open output streams.
>
> 1. 如果有多个流需要刷新的话就只需要写一句fflush()，并且传参为空即可

```c
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    int i;

    // printf("before while(1)\n");
    printf("before while(1)");
    // 1. 要么单独传入某一个流
    fflush(stdout);

    while(1);
    // printf("after while(1)\n");
    printf("after while(1)");
    // 2. 要么就传空参数
    fflush(NULL);

    exit(0);
}
```

<img src="index.assets/image-20220318201046677.png" alt="image-20220318201046677" style="zoom:80%;" />

缓冲区的作用：大多数情况下是好事，最大的作用就是合并系统作用
缓冲的模式：

* 行缓冲：换行的时候刷新缓冲区数据、缓冲区满了的时候刷新或者强制刷新(使用fflush)，典型的例子比如说标准输入stdout
* 全缓冲：缓冲区满了的时候刷新或者强制刷新，只要不是终端设备默认是全缓冲模式，stdout是行缓冲模式就是因为它是终端设备
* 无缓冲：需要立即输出的内容，如stderr，因为一旦出错立马输出，什么条件都不等待，也不等待缓冲区是否满了等情况

# 标准IO-fseeko和ftello

刚才我们说过printf和scanf一族函数的缺陷是什么，也说了对于文件位置指针如何使用以及缓冲区的使用方法

包括之前讲到的输入输出函数实际上都会有问题，比如说没有一个函数能够做到从任何一个流中想取多少取多少，如完整的取到一行内容。

文件位置指针定位的几个函数中会存在这么几个问题：fseek和ftell函数"奇丑无比"，这两个函数中的long类型特别的"恶劣"

```c
int fseek(FILE *stream, long offset, int whence);

long ftell(FILE *stream);
```

从ftell的角度来分析，它能反映一个文件位置指针的位置，而这个long型是不知道多大的，因为long型在不同的平台上要看字自身机器字长来定义，即使在标准C当中对于类型没有一个严格的定义，比如整型大概一个字长、float和double类型的关系...long型在32位的环境中占32位也就是4个字节，而在ftell函数中没有特殊的说明，那么这就是个有符号的long型，有符号的long型代表着的是(-2^31)~(2^31 - 1)，也就是从-2G~2G - 1。而在ftell函数中不可能反映出文件的位置是一个负值，所以ftell的返回值只能是个正数(0~2G - 1)，所以为了迁就ftell，在fessk当中的offset参数能有正有负，即能够从当前位置向前定义2G，也能从当前位置向后定义2G，那么加起来这个文件有4G大小。但如果结合ftell来看的话，就会发现这个文件压根就不敢达到4G的长度，因为ftell用不了负值部分，所以fseek和ftell共同进行工作的时候就会遇到这样的问题：当前文件大小不会超过2G,也就是只能定义2G大小的文件

> NAME
>
> > fseeko, ftello - seek to or report file position
>
> SYNOPSIS
>
> > #include <stdio.h>
> >
> > int fseeko(FILE *stream, off_t offset, int whence);
> >
> > off_t ftello(FILE *stream);
>
> 1. fseeko和ftello在实现的时候就把刚才说的有争议的两个参数替换为了一个type define过的类型，unix定义的标准名字都是很规范的，什么什么类型都被叫_t，t代表的是type，off_t代表的位数随着机器字长而改变。如果让我们去设计类似于fseek函数的话，其实可以想到把数据类型高度抽象，将来如果这个类型不够用的话那么就可以把另外的一个数据类型放过去就行了
>
> DESCRIPTION
>
> > On some architectures, both off_t and long are 32-bit types, but defining _FILE_OFFSET_BITS with the value 64 (before including any header files) will turn off_t into a 64-bit type.
>
> 2. off_t如果不加上`# define _FILE_OFFSET_BITS 64`声明的话，那么off_t的长度是不是32位是很难说的，是不确定的。如果加上了这个声明，则off_t一定表示的是64位的整型数

既然fseeko和ftello函数中的文件大小已经不受限制了，那么我们是否可以抛弃原来的feek和ftell呢？答案是不能的，因为fseeko和ftello是方言，遵循的是POSIX，而fseek和ftell是遵循C88，C99的，意味着可移植性要好。所以如果文件真的超过2G并且要求可移植性好，那么就只能想别的办法了，而别去使用fseeko和ftello

> CONFORMING TO
>
> > POSIX.1-2001, POSIX.1-2008, SUSv2.

# 标准IO-getline

没有任何一个现行的函数能够完整地取得一行，因为一定会有大小限制。如果自己去实现这个功能的话该如何实现呢？一个办法是使用动态内存来实现：首先申请一块内存，这块内存有10个字节大小，往这10个字节空间里写数据，当快写满的时候再去申请10个字节大小的空间并以此类推，直到满足空间需求为止，那么这就可以借助malloc一族函数去实现

> NAME
>
> SYNOPSIS
>
> > #include <stdlib.h>
> >
> > void *malloc(size_t size);
> > void *realloc(void *ptr, size_t size);
>
> 1. 首先使用malloc(size_t size)去获得一块内存，然后如果还有需要的话再通过realloc对一块已有的内存进行重复的申请

但为了实现这个功能还有一个更好的方法，就是geline，实际上这个函数就是把刚才说的过程给封装了一遍

> NAME
>
> > getline, getdelim - delimited string input
>
> SYNOPSIS
>
> > #include <stdio.h>
> >
> > ssize_t getline(char **lineptr, size_t *n, FILE *stream);
>
> 1. ssize_t getline(char **lineptr, size_t *n, FILE *stream)：这个函数实现的功能或者说封装起来的过程就是刚才提到的那样。首先malloc一块空间用一用，不够用了再去扩展以此类推，即第一次用的是malloc，之后用的都是realloc
>     * char **lineptr：虽然参数类型是二级指针，但是这个参数实际上表示的是一级指针的地址(char\*)\*lineptr
>     * size_t *n：用malloc申请的内存块的大小，因为要把值回填给n，所以是一个整型数据的地址
>     * FILE *stream：从stream中读取一行内容
>
> DESCRIPTION
>
> > getline()  reads  an  entire  line from stream, storing the address of the buffer containing the text into *lineptr.  The
> > buffer is null-terminated and includes the newline character, if one was found.
>
> 2. 把保存文本数据的缓冲区的地址存一级指针lineptr当中去，另外即使给出一个串的起始位置，但依然不知道这块空间有多大，所以用size_t定义的整型值去限制以lineptr指针指向的地址开头空间的整个大小为n。因为要回填存储空间大小，所以size_t *n也不代表一级指针，而代表的是整型数的地址
>
> RETURN VALUE
>
> > On success, getline() and getdelim() return the number of characters read, including the delimiter character, but not including the terminating null byte ('\0').  This value can be used to handle embedded null bytes in the line read.
> >
> > Both functions return -1 on failure to read a line (including end-of-file condition).  In the event of an error, errno is
> > set to indicate the cause.
>
> 

1. 实现：利用getline返回文件中每一行有多少个文件字符

```c
// getline.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{

    // mtrace();
    FILE *fp;
    char *linebuf = NULL;
    size_t linesize = 0;

    if (argc < 2)
    {
        fprintf(stderr, "Usage:%s <src_file>\n", argv[0]);
        exit(1);
    }

    fp = fopen(argv[1], "r");
    if (fp == NULL)
    {
        perror("fopen()");
        exit(1);
    }

    while (1)
    {
        if(getline(&linebuf, &linesize, fp) < 0)
            break;
        printf("%d\n", strlen(linebuf));
        printf("%d\n", linesize);
    }

    fclose(fp);

    
    exit(0);
}
```

* 10-11：两个指针必须初始化，不然会出现段错误

以自定义的makefile文件为例，该文件第一行的字符个数为45个(包括一个换行符\n)，第二航只有一个换行符，所以用`strlen(linebuf)`打印出来的结果就是45和1，但是因为linesize表示的是malloc申请的内存块大小，所以可以看到每次用malloc申请的块大小为120个字节，因为该文文本有两行，所以调用了两次getline函数，因此也相当于调用了2次malloc函数

<img src="index.assets/image-20220319122919133.png" alt="image-20220319122919133" style="zoom:80%;" />

<img src="index.assets/image-20220319123002266.png" alt="image-20220319123002266" style="zoom:80%;" />

当前行数据大小将要超过120字节时，就会调用realloc，内存块大小有可能是120，120...的增长，也有可能是120，60，60...或者其他情况，可以来看一个例子，结果是用malloc申请的内存块大小为240，也就是说当内存块数据快要到达120字节的时候又使用realloc以120字节长度再次申请，即最后使得getline一次申请了240字节大小的内存空间

```shell
> ./getline /etc/services
```

<img src="index.assets/image-20220319123709116.png" alt="image-20220319123709116" style="zoom:80%;" />

`getline.c`里的代码其实有一点不妥当，其实这个程序已经造成了可控的内存泄漏，这段程序实现了通过malloc和realloc申请了一段内存空间，但是并没有给出互逆操作：使用malloc函数之后并没有是用诸如free函数来释放掉内存空间，内存泄漏指的是&linebuf的空间没来得及释放掉，其实getline这个函数也没有提供一个比如getlinefree的函数。那为什么说是可控的内存泄漏呢？如果在服务器上运行的程序长时间不能宕机，每一段时间都会产生一点内存泄漏的碎片，那么长此以往就会有问题了，这段程序泄露的只有从lineptr地址开始n大小的空间。

也许会有人想既然存在内存泄露，那么在程序结束的时候加上free函数不就好了吗？理论上是可以的，但是不建议这么做，因为这里写free是以原函数使用malloc为前提的，即知道原函数的封装细节，但是内存动态申请的函数并不是只有malloc和free这一对，还有诸如c++里的new和delete，万一函数里使用的是new，那么就不能用free来释放内存空间了。这里提个醒，在做动态内存分配的时候一定记着提供类似于close、free等释放内存的操作

```markdown
fclose(fp);

free(linebuf);

exit(0);
```

* 这里提供一个自己去实现而不是系统提供的getline函数，以为getline是典型的的遵循GNU标准的方言，只存在于libc库中，但是getline在实际生产中具有十分意义的函数，因为没有一个函数能完成取一行，不管这行有多大字节，总能够通过malloc不断扩充而获取到数据

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include<mcheck.h>

// void cleanup(char** pointer) {
//   free(*pointer);
//   *pointer = NULL;
// }

// 自行实现getline函数
ssize_t mygetline(char **line, size_t *n, FILE *fp)
{
    char *buf = *line;
    // c来存储字符，i来记录字符串长度
    ssize_t c = 0;
    ssize_t i = 0;

    // buf为空或n为0时动态分配空间
    if (buf == NULL || *n == 0)
    {
        *line = malloc(10);
        buf = *line;
        *n = 10;
    }

    while ((c = fgetc(fp)) != '\n')
    {
        if (c == EOF)
            return -1;
        //留2个空间给'\n'和'\0'
        if (i < *n - 2)
            *(buf + i++) = c;
        else
        {
            //空间不足,需要扩展空间，重新进行分配
            *n = *n + 10;
            buf = realloc(buf, *n);
            *(buf + i++) = c;
        }
    }

    *(buf + i++) = '\n';
    *(buf + i) = '\0';
    return i;
}

int main(int argc, char **argv)
{

    // mtrace();
    FILE *fp;
    char *linebuf = NULL;
    size_t linesize = 0;

    if (argc < 2)
    {
        fprintf(stderr, "Usage:%s <src_file>\n", argv[0]);
        exit(1);
    }

    fp = fopen(argv[1], "r");
    if (fp == NULL)
    {
        perror("fopen()");
        exit(1);
    }

    while (1)
    {
        if (mygetline(&linebuf, &linesize, fp) < 0) // if(getline(&linebuf, &linesize, fp) < 0)
            break;
        printf("%lu\n", strlen(linebuf));
        printf("%lu\n", linesize);
    }

    fclose(fp);
    free(linebuf);
    exit(0);
}
```













