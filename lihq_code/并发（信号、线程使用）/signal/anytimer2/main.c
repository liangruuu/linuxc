#include <stdio.h>
#include <stdlib.h>
#include <wait.h>
#include <sys/types.h>
#include <unistd.h>

#include "anytimer.h"

static void any1(void *s)
{
    printf("%s", (char *)s);
    fflush(NULL);
}

int main(void)
{
    int i = 0;
    int alarm_arr_id;
    anytimer_alarm(2, any1, "hello2");
    anytimer_alarm(4, any1, "hello4");
    alarm_arr_id = anytimer_alarm(6, any1, "hello6");
    anytimer_alarm(8, any1, "hello8");
    anytimer_alarm(10, any1, "hello10");
    anytimer_alarm(12, any1, "hello12");

    while(1)    {
        write(1, ".", 1);
        sleep(1);
        i++;
        //关闭第三次alarm,不执行hello6输出
        if(i == 5)                                
            anytimer_close(alarm_arr_id);
        //第四次alarm后,全关闭alarm数组
        if(i == 9)                                
            anytimer_destroy();

    }

    return 0;
}