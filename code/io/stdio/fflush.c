#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    int i;

    // printf("before while(1)\n");
    printf("before while(1)");
    // 1. 要么单独传入某一个流
    fflush(stdout);

    while (1)
        ;
    // printf("after while(1)\n");
    printf("after while(1)");
    // 2. 要么就传空参数
    fflush(NULL);

    exit(0);
}
