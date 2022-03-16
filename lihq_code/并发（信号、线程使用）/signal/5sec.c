#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

static int loop = 1;

void fun1(void)
{
    time_t end;
    int64_t count = 0;
    end = time(NULL) + 5;

    while(time(NULL) < end)
        count++;

    printf("%ld\n", count);
}

static void alarm_handler(int s)
{
    loop = 0;
}

void fun2(void)
{
    int64_t count = 0;
    
    signal(SIGALRM, alarm_handler);
    alarm(5);

    while(loop)
        count++;

    printf("%ld\n", count);
}

int main()
{
    
    fun2();

    exit(0);
}
