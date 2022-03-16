#ifndef _ANYTIMER_H_
#define _ANYTIMER_H_

#define BUFSIZE 1024
typedef void anytm_func(void *);

//设置可重复使用的闹钟数组
int anytimer_alarm(int, anytm_func *, const void *);

//关闭闹钟
void anytimer_close(int n);

//关闭全部闹钟
void anytimer_destroy(void);

#endif