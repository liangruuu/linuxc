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















