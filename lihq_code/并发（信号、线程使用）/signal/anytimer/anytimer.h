#ifndef _ANYTIMER_H_
#define _ANYTIMER_H_


#define JOB_MAX 1024

typedef void anyfunc(void *);

void anytimer_alarm(int, anyfunc *, void *);

void anytimer_destroy(void);

#endif