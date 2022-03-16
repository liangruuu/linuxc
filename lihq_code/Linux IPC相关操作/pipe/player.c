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

    if (pipe(pd) < 0){
        perror("pipe()");
        exit(1);
    }

    pid = fork();
    if (pid < 0){
        perror("fork()");
        exit(1);
    }
    
    if(pid == 0){
        close(pd[1]); //关闭写端

        dup2(pd[0],0);//将stdin关闭，重定向到管道读端 mpg123 - "-" 表示从stdin读取
        close(pd[0]);

        fd = open("/dev/null",O_RDWR);//屏蔽其他标准输出
        dup2(fd,1);
        dup2(fd,2);

        int ret = execl("/usr/bin/mpg123", "mpg123", "-", NULL);
        if(ret < 0)
        {
            perror("execl()");
            exit(1);
        }

        close(fd);
    }
    else
    {
        close(pd[0]);//关闭读端
        sd = open("/home/ray/Music/heroes.mp3",O_RDONLY);
        if(sd < 0)
        {
            perror("open()");
            exit(1);
        }

        while(1){
            len = read(sd, buf, BUFSIZE);
            if (len < 0){
                if (errno == EINTR)
                    continue;
                close(pd[1]);
                close(sd);
                exit(1);
            }
            if (len == 0){
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