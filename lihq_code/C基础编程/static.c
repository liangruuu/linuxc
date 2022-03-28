#include<stdio.h>
#include<stdlib.h>

void func(){
    // static 定义的变量在全局数据区分配内存，
    // 始终驻留在全局数据区直到程序结束。
    // 多次调用函数中的static变量都指向第一次创建的同一个变量
    static int x = 0;
    x = x + 1;
    // C 内部预定义宏：
    // __LINE__     ：当前程序行的行号，表示为十进制整型常量
    // __FILE__     ：当前源文件名，表示字符串型常量
    // __DATE__     ：转换的日历日期，表示为Mmm dd yyyy 形式的字符串常量，Mmm是由asctime产生的。
    // __TIME__     ：转换的时间，表示"hh:mm:ss"形式的字符串型常量，是有asctime产生的。（asctime貌似是指的一个函数）
    // __FUNCTION__ ：当前程序所在函数
    printf("[%s]%p -> %d\n",__FUNCTION__,&x, x);

}

int main(){ 
    
    func();
    func();
    func();
    
    exit(0) ;

}




