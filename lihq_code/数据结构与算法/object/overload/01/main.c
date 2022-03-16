
#include<stdio.h>
#include<stdlib.h>


// 函数指针实现的参数重载
void func_int(void * a)
{
    printf("%d\n",*(int*)a);  //输出int类型，注意 void * 转化为int
}
 
void func_double(void * b)
{
    printf("%.2f\n",*(double*)b);
}
 
typedef void (*ptr)(void *);  //typedef申明一个函数指针
 
void c_func(ptr p, void *param)
{
     p(param);                //调用对应函数
}
 
int main()
{
    int a = 23;
    double b = 23.23;
    c_func(func_int,&a);
    c_func(func_double,&b);
    exit(0);
}