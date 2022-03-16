#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <errno.h>

#include "anytimer.h"

static int inited = 0;
static struct sigaction older;
static struct itimerval olditv;


struct timer
{
    int sec;
    void *data;
    anyfunc *func;
};

struct timer *job[JOB_MAX];

static int anytimer_print(void)
{
    int i, ret;
    for (i = 0; i < JOB_MAX; i++)
    {
        if (job[i] != NULL && job[i]->sec == 0)
        {
            // 发送中断信号
            ret = kill(getpid(), SIGALRM);
            if(ret < 0)
                perror("kill()");
            return i;
        }
    }
    return -1;
}

static void alarm_handler()
{
    int i, ret;
    pid_t pid;

    for (i = 0; i < JOB_MAX; i++)
    {
        if (job[i] != NULL)
            job[i]->sec -= 1;
    }

    pid = fork();

    //child pid
    if (pid == 0)
    {
        ret = anytimer_print();
        if (ret != -1)
            job[ret]->func(job[ret]->data);
        else
        {
            sigaction(SIGALRM, &older, NULL);
        }

        exit(0);
    }

    wait(NULL);

    ret = anytimer_print();
}

static void moduler_unload(void)
{
    sigaction(SIGALRM, &older, NULL);
    setitimer(ITIMER_REAL, &olditv, NULL);
}

void moduler_load()
{
    struct sigaction act;
    struct itimerval itv;

    act.sa_handler = alarm_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGALRM, &act, &older);

    itv.it_interval.tv_sec = 1;
    itv.it_interval.tv_usec = 0;

    itv.it_value.tv_sec = 1;
    itv.it_value.tv_usec = 0;

    setitimer(ITIMER_REAL, &itv, &olditv);
    //钩子函数
    atexit(moduler_unload); 
}

static int find_arr() // 寻找空数组下标
{
    int i;

    for (i = 0; i < JOB_MAX; i++)
    {
        if (job[i] == NULL)
            return i;
    }

    return -1;
}

void anytimer_alarm(int sec, anyfunc *func, void *p)
{
    struct timer *me = NULL;
    int ret;
    if (inited == 0)
    {
        moduler_load();
        inited = 1;
    }

    me = malloc(sizeof(*me));

    if (me == NULL)
    {
        fprintf(stderr, "malloc is faild !\n");
        exit(1);
    }
    me->sec = sec;
    me->data = p;
    me->func = func;

    ret = find_arr(); // 寻找空数组下标
    if (ret == -1)
    {
        fprintf(stderr, "find arr is faild!\n");
        free(me);
        exit(-1);
    }

    job[ret] = me;
}

void anytimer_destroy(void)
{
    int i;

    for (i = 0; i < JOB_MAX; i++)
    {
        if (job[i] != NULL)
            free(job[i]);

        job[i] = NULL;
    }
}