#include <stdio.h>
#include <stdlib.h>
#include <wait.h>
#include <sys/types.h>
#include <unistd.h>
#include "anytimer.h"

static void f1(void *s)
{
        printf("f1():%s", (char *)s);
            fflush(NULL);
}

static void f2(void *s)
{
        printf("f2():%s", (char *)s);
            fflush(NULL);
}


int main(void)
{

    anytimer_alarm(4, f1, "hello");
    anytimer_alarm(2, f2, "world");
    anytimer_alarm(7, f1, "apue");

    while (1)
    {
        write(1, "*", 1);
        sleep(1);

    }

    anytimer_destroy();

    return 0;
}