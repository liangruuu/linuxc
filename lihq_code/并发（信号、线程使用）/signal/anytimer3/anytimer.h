#ifndef _ANYTIMER_H_
#define _ANYTIMER_H_

#define JOB_MAX 1024
typedef void at_jobfunc_t(void *);

//设置可重复使用的闹钟数组
int at_addjob(int, at_jobfunc_t *, void *);

//关闭闹钟
int at_canceljob(int);

int at_waitjob(int);

int at_addjob_repeat(int , at_jobfunc_t *, void *);

// int at_pausejob();

// int at_resumejob();

#endif