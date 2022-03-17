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

```markdown
r      Open text file for reading.  The stream is positioned at the beginning of the file.

r+     Open for reading and writing.  The stream is positioned at the beginning of the file.

w      Truncate file to zero length or create text file for writing.  The stream is positioned at the  beginning  of  the file.

w+     Open for reading and writing.  The file is created if it does not exist, otherwise it is truncated.  The stream is positioned at the beginning of the file.
          
a      Open for appending (writing at end of file).  The file is created if it does not exist.  The stream is  positioned at the end of the file.

a+     Open for reading and appending (writing at end of file).  The file is created if it does not exist.  Output is always appended to the end of the file.  POSIX is silent on what the initial read position is when using this  mode. For  glibc,  the initial file position for reading is at the beginning of the file, but for Android/BSD/MacOS, the initial file position for reading is at the end of the file.
```

* Truncate file to zero length or create text file for writing : 有则清空，无则创建
* beginning of the file : 文件当中第一个有效字符
* the end of the file : 文件最后一个有效字节的下一个位置。文件的读和写都发生在当前位置，如果the end of the file表示的是文件的最后一个字节，则会覆盖掉最后一个字节
* Open for reading and appending (writing at end of file) : 如果进行append操作的话，那么文件位置指针就会在文件末尾处，此时读操作就不好用了。因为如果以a+形式打开文件并读文件，但是文件指针在末尾处，就不好读了，所以以a+形式打开文件之后的指针位置取决于下一步要进行的操作
* 在这六种操作文件方式里，只有r和r+最特殊，因为其他四种方式都有着The file is created if it does not exist字样。r和r+在操作一个文件的时候必须保证文件存在，如果文件不存在则结束当前调用返并回出错信息
* 在其他一些参考书里讲到文件IO的时候，有着类似'r+b'，'w+b'，b表示二进制操作。在windows下区分两种流：文本流和二进制流这两种流在用程序进行控制的时候是不一样的，所以在windows环境下进行编程要指定是用r打开还是用r+b打开。但是在linux环境下不用指定，因为在linux环境下只有流(stream)的概念而不区分是二进制还是文本，除非是程序有可能移植到windows环境下才有必要加上b

```markdown
The mode string can also include the letter 'b' either as a last character or as a character between  the  characters  in any of the two-character strings described above.  This is strictly for compatibility with C89 and has no effect; the 'b' is ignored on all POSIX conforming systems, including Linux.  (Other systems may treat text files and binary  files  differently, and adding the 'b' may be a good idea if you do I/O to a binary file and expect that your program may be ported to non-UNIX environments.)

```

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

