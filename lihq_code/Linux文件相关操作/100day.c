#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define BUFSIZE 1024

int main()
{
    char buf[BUFSIZE];
    time_t stamp;
    struct tm *tm;
    
    stamp = time(NULL);
    tm = localtime(&stamp);

    strftime(buf, BUFSIZE, "Now:%Y-%m-%d", tm);
    puts(buf);
    
    tm->tm_mday += 100;
    (void)mktime(tm);
    strftime(buf, BUFSIZE, "100 days later:%Y-%m-%d", tm);
    puts(buf);

    exit(0);
}
