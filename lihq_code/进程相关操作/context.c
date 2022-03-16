#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <unistd.h>

// 可以用来实现协程

void func1(ucontext_t *ctx, ucontext_t *ctx2)
{
    printf("func1 start...\n");
    printf("func1: swapcontext(&uctx_func1, &uctx_func2)\n");
    swapcontext(ctx, ctx2);
    printf("func1: returning\n");
}

void func2(ucontext_t *ctx, ucontext_t *ctx2)
{
    printf("func2 start...\n");
    printf("func2: swapcontext(&uctx_func2, &uctx_func1)\n");	
    swapcontext(ctx, ctx2);
    printf("func2: returning\n");
}

int main() {
    ucontext_t uctx_main, uctx_func1, uctx_func2;
    //注意在实际中要注意stack大小,否则可能会出现溢出.  
    char func1_stack[16384];
    char func2_stack[16384];

    //获取当前进程/线程上下文信息,存储到uctx_func1中
    getcontext(&uctx_func1);
        
    //uc_stack: 分配保存协程数据的堆栈空间
    uctx_func1.uc_stack.ss_sp = func1_stack;	//栈头指针
    uctx_func1.uc_stack.ss_size = sizeof(func1_stack);	//栈大小
    uctx_func1.uc_link = &uctx_main;	
    makecontext(&uctx_func1, (void (*)(void))func1, 2, &uctx_func1, &uctx_func2); //依改动得到一个新的centext

    getcontext(&uctx_func2);

    uctx_func2.uc_stack.ss_sp = func2_stack;
    uctx_func2.uc_stack.ss_size = sizeof(func2_stack);
    uctx_func2.uc_link = &uctx_func1;
    makecontext(&uctx_func2, (void (*)(void))func2, 2, &uctx_func2, &uctx_func1); 

    printf("main: swapcontext(&uctx_main, &uctx_func2)\n");

    //将当前上下文保存到uctx_main, 并切换到uctx_func2
    swapcontext(&uctx_main, &uctx_func2);

    printf("main: end...\n");

    exit(0);
}