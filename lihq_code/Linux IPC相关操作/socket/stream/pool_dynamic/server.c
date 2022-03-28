#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdarg.h>
#include <time.h>
#include <signal.h>
#include <sys/mman.h>
#include <errno.h>

#include "proto.h"

#define IPSIZE 40
#define LINESIZE 80

#define MAXSPARESERVER 10
#define MINSPARESERVER 5
#define MAXCLIENTS 20

#define SIG_NOTIFY SIGUSR2

// 客户端执行以下命令
// while true; do (./client 127.0.0.1 &); sleep 1; done


enum
{
    STATE_IDLE = 0,
    STATE_BUSY,
};

struct server_st
{
    pid_t pid;
    int state;
    // int reuse;
};

static struct server_st *serverpool;
static int idle_count = 0, busy_count = 0;

//socket相关全局变量
static int sd;

static void server_job(int pos)
{
    int clientsd;
    int ppid;

    struct sockaddr_in raddr; //remote addr
    char ipstr[IPSIZE];
    char linebuf[LINESIZE];
    int len = 0;

    socklen_t raddr_len = sizeof(raddr);

    ppid = getppid(); //父进程

    while (1)
    {
        serverpool[pos].state = STATE_IDLE;
        kill(ppid, SIG_NOTIFY);

        clientsd = accept(sd, (void *)&raddr, &raddr_len); //接收客户端连接
        if (clientsd < 0)
        {
            if (errno == EINTR || errno == EAGAIN)
                continue;
            else
            {
                perror("accept()");
                exit(1);
            }
        }

        serverpool[pos].state = STATE_BUSY;
        kill(ppid, SIG_NOTIFY);
        inet_ntop(AF_INET, &raddr.sin_addr, ipstr, IPSIZE);
        // printf("[%d]Client:%s:%d\n", getpid(), ipstr, ntohs(raddr.sin_port));

        len = snprintf(linebuf, LINESIZE, FMT_STAMP, (long long)time(NULL));

        if (send(clientsd, linebuf, len, 0) < 0)
        {
            perror("send()");
            exit(1);
        }
        close(clientsd);
        sleep(5);
    }
}

static void usr2_handler(int s)
{
    return;
}

static int add_1_server(void)
{
    int slot;
    pid_t pid;

    if (idle_count + busy_count >= MAXCLIENTS)
    {
        return -1;
    }

    for (slot = 0; slot < MAXCLIENTS; slot++)
    {
        if (serverpool[slot].pid == -1)
        {
            break;
        }
    }
    serverpool[slot].state = STATE_IDLE;
    pid = fork();
    if (pid < 0)
    {
        perror("fork()");
        exit(1);
    }
    if (pid == 0) // 子进程
    {
        server_job(slot);
        exit(0);
    }
    else // 父进程
    {
        serverpool[slot].pid = pid;
        idle_count++;
    }
    return 0;
}

static int del_1_server(void)
{
    if (idle_count == 0)
    {
        return -1;
    }

    for (int i = 0; i < MAXCLIENTS; i++)
    {
        if (serverpool[i].pid != -1 && serverpool[i].state == STATE_IDLE)
        {
            kill(serverpool[i].pid, SIGTERM);
            serverpool[i].pid = -1;
            idle_count--;
            break;
        }
    }
    return 0;
}

static void scan_pool()
{
    int idle = 0, busy = 0;
    for (int i = 0; i < MAXCLIENTS; i++)
    {
        if (serverpool[i].pid == -1)
        {
            continue;
        }
        if (kill(serverpool[i].pid, 0))
        { //kill pid 0检测一个进程是否存在
            serverpool[i].pid = -1;
            continue;
        }
        //统计进程池的状态
        if (serverpool[i].state == STATE_IDLE)
            idle++;
        else if (serverpool[i].state == STATE_BUSY)
            busy++;
        else
        {
            fprintf(stderr, "Unknown state!\n");
            abort();
        }
    }
    idle_count = idle;
    busy_count = busy;
}

int main()
{
    int val = 1;
    struct sigaction sa, osa;
    struct sockaddr_in laddr;

    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_NOCLDWAIT;
    sigaction(SIGCHLD, &sa, &osa);

    sa.sa_handler = usr2_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIG_NOTIFY, &sa, &osa);

    //屏蔽信号
    sigset_t set,oset;
    sigemptyset(&set);
    sigaddset(&set, SIG_NOTIFY);
    sigprocmask(SIG_BLOCK, &set, &oset);

    serverpool = mmap(NULL, sizeof(struct server_st) * MAXCLIENTS,
                      PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (serverpool == MAP_FAILED)
    {
        perror("mmap()");
        exit(1);
    }

    //初始化进程池
    for (int i = 0;i < MAXCLIENTS;i++){
        serverpool[i].pid = -1;
    }

    sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sd < 0)
    {
        perror("socket()");
        exit(1);
    }

    if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) < 0)
    {
        perror("setsockopt()");
        exit(1);
    }

    laddr.sin_family = AF_INET;
    laddr.sin_port = htons(atoi(SERVERPORT));
    inet_pton(AF_INET, "0.0.0.0", &laddr.sin_addr);

    if (bind(sd, (void *)&laddr, sizeof(laddr)) < 0)
    {
        perror("bind()");
        exit(1);
    }

    if (listen(sd, 200) < 0)
    {
        perror("listen()");
        exit(1);
    }

    for (int i = 0; i < MINSPARESERVER; i++)
    {
        add_1_server();
    }

    while (1)
    {
        sigsuspend(&oset);
        scan_pool();
        if (idle_count > MAXSPARESERVER)
        {
            for (int i = 0; i < (idle_count - MAXSPARESERVER); i++)
            {
                del_1_server();
            }
        }
        else if (idle_count < MINSPARESERVER)
        {
            for (int i = 0; i < (MINSPARESERVER-idle_count); i++)
            {
                add_1_server();
            }
        }

        for (int i = 0; i < MAXCLIENTS; i++)
        {
            if(serverpool[i].pid == -1)
                putchar('o');
            else if(serverpool[i].state == STATE_IDLE)
                putchar('.');
            else
                putchar('x');
        }
        putchar('\n');
    }

    sigprocmask(SIG_SETMASK, &oset, NULL);

    exit(0);
}