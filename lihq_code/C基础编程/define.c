#include<stdio.h>
#include<stdlib.h>

#define MAX(a, b)  ({typeof(a) A = (a), B = (b); (A > B ? A : B);})

// 宏在预编译时候已经替换到代码中，占用编译时间
// 函数是运行时候才被调用，需要压栈保存当前环境，运行完后弹栈，所以占用运行时间
// 在对时间要求苛刻的项目中，使用宏有优势
#if 0
int max(int a, int b){
    return a > b ? a : b;
}
#endif

int main(void){

    int i = 5, j = 8;

    printf("%d\n",MAX(i++, j++));
    
    exit(0);
}
