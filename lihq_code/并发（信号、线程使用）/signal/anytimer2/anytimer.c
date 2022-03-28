#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <errno.h>
#include <wait.h>

#include "anytimer.h"

//闹钟系统初始标志位
static int init_mod;    
struct sigaction oldact;


struct alarm_st 
{
    int token;
    anytm_func *anyfunc;
    void *str;
};

static struct alarm_st *alarm_arr[BUFSIZE];

//闹钟系统关闭
static void __arr_destroy(void)
{
    sigaction(SIGALRM, &oldact, NULL);
    struct itimerval oldtime;
    oldtime.it_interval.tv_sec = 0;
    oldtime.it_interval.tv_usec = 0;
    oldtime.it_value.tv_sec = 0;
    oldtime.it_value.tv_usec = 0;
    setitimer(ITIMER_REAL, &oldtime, NULL);
    init_mod == 0;
}

//无闹钟判断
static void __ifarr_NULL(void)
{
    int i;
    for(i = 0; i < 1024 && alarm_arr[i] == NULL; i++);
    if(i == 1024)    {
        __arr_destroy();
    }
}

//中断
static void alarm_handler(int s)
{
    int i;
    for(i = 0; i < 1024; i++)    {
        if(alarm_arr[i] != NULL)    {
            alarm_arr[i]->token--;
            if(alarm_arr[i]->token <= 0)    {
                alarm_arr[i]->anyfunc(alarm_arr[i]->str);
                free(alarm_arr[i]);
                alarm_arr[i] = NULL;
            }
        }
    }
    __ifarr_NULL();
}

//闹钟系统初始化
static void __alarm_init(void)
{
    struct itimerval newtime;            //时间设定
    newtime.it_interval.tv_sec = 1;
    newtime.it_interval.tv_usec = 0;
    newtime.it_value.tv_sec = 1;
    newtime.it_value.tv_usec = 0;

    struct sigaction act;                //中断设定
    act.sa_handler = alarm_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    setitimer(ITIMER_REAL, &newtime, NULL);
    sigaction(SIGALRM, &act, &oldact);
    init_mod = 1;                        //已初始化
}

//设置闹钟
int anytimer_alarm(int sec, anytm_func *anytm, const void *p)
{
    if(0 == init_mod)                    //初始化判定
        __alarm_init();
    
    int i;
    struct alarm_st *me = NULL;
    
    for(i = 0; i < 1024; i++)    {        //查找空位
        if(alarm_arr[i] == NULL)
            break;
    }
    if(i >= 1024)
        return -1;

    me = malloc(sizeof(*me));
    if(NULL == me)
        return -1;

    me->token = sec;
    me->anyfunc = anytm;
    me->str = (void*)p;
    alarm_arr[i] = me;
    return i;
}

//定时关闭
void anytimer_close(int n)
{
    //清除相应闹钟,未考虑原子性
    if(alarm_arr[n] != NULL)    {        
        free(alarm_arr[n]);
        alarm_arr[n] = NULL;
    }
    __ifarr_NULL();
}

//全定时关闭
void anytimer_destroy(void)
{
    __arr_destroy();
    for(int i = 0; i < 1024; i++)    {
        if(alarm_arr[i] != NULL)    {
            free(alarm_arr[i]);
            alarm_arr[i] = NULL;
        }
    }
}