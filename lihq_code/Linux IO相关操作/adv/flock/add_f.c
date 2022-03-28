#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define PROCNUM 20
#define LINESIZE 1024
#define FNAME "/tmp/out"


static void func_add(void)
{
    FILE *fp;
    char linebuf[LINESIZE];

    fp = fopen(FNAME, "r+");
    if(fp == NULL)
    {
        perror("fopen()");
        exit(1);
    }

    int fd = fileno(fp);
    lockf(fd, F_LOCK, 0);

    fgets(linebuf, LINESIZE, fp);
    fseek(fp, 0, SEEK_SET);
    sleep(1);
    fprintf(fp, "%d\n", atoi(linebuf) + 1);
    fflush(fp);
    lockf(fd, F_ULOCK, 0);
    fclose(fp);

    return;
}

int main()
{

    int i;
    pid_t pid;

    for(i = 0; i < PROCNUM; i++)
    {
        pid = fork();
        if(pid < 0)
        {
            perror("fork()");
            exit(1);
        }

        if(pid == 0)
        {
            func_add();
            exit(0);
        }
    }

    for(i = 0; i < PROCNUM; i++)
        wait(NULL);
    
    exit(0);
}
