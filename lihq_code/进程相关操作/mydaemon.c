#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>

#define FNAME "/tmp/out"


static int daemonize(void)
{
    int fd;
    pid_t pid;
    pid = fork();
    if(pid <0)
    {
        perror("fork()");
        exit(1);
    }
    if(pid >0)
        exit(0);

    fd = open("/dev/null", O_RDWR);
    if(fd < 0)
    {
        perror("open()");
        return -1;
    }
    // 标准流重定向到“黑洞”，即在终端不显示
    dup2(fd, 0);
    dup2(fd, 1);
    dup2(fd, 2);
    if(fd > 2)
        close(fd);

    setsid();
    chdir("/");
    // umask(0);

    return 0;
}

int main()
{
    FILE *fp;
    if(daemonize())
        exit(1);

    fp = fopen(FNAME, "w");
    if(fp == NULL)
    {
        perror("fopen()");
        exit(1);
    }

    for(int i = 0; ; i++ )
    {
        fprintf(fp, "%d\n", i);
        fflush(fp);
        sleep(1);
    }


    exit(0);
}