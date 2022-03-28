#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>

#define FNAME "/tmp/out"


void mydup()
{
    int fd;
    // 关闭标准输出
    close(1);
    fd = open(FNAME,O_WRONLY|O_CREAT|O_TRUNC,0600);
    if(fd < 0)
    {
        perror("open()");
        exit(1);
    }
    
    puts("Hello!");
}

void mydup2()
{
    int fd;
    fd = open(FNAME,O_WRONLY|O_CREAT|O_TRUNC,0600);
    if(fd < 0)
    {
        perror("open()");
        exit(1);
    }
    //以下三句实现输出重定向
    close(1);
    // 非原子操作
    dup(fd);
    close(fd);
    
    puts("Hello you!");
}

void mydup3()
{
    int fd;
    fd = open(FNAME,O_WRONLY|O_CREAT|O_TRUNC,0600);
    if(fd < 0)
    {
        perror("open()");
        exit(1);
    }
    //dup2实现原子操作
    dup2(fd, 1);
    if(fd != 1)
        close(fd);
    
    puts("Hello you.......!");
}

int main()
{
    mydup3();
    exit(0);
}
