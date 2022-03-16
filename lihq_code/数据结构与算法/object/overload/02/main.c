#include<stdio.h>
#include<stdlib.h>


// 利用了 GCC 的内置函数，__builtin_types_compatible_p()
// 和__builtin_choose_expr() 实现重载

// warning: dereferencing type-punned pointer will break strict-aliasing rules
#define gcc_overload(A)\
    __builtin_choose_expr(__builtin_types_compatible_p(typeof(A), struct s1),\
        gcc_overload_s1(*(struct s1 *)&A),\
    __builtin_choose_expr(__builtin_types_compatible_p(typeof(A), struct s2),\
        gcc_overload_s2(*(struct s2 *)&A),(void)0))

struct s1
{
    int a;
    int b;
    double c;
};
 
struct s2
{
    long long a;
    long long b;
};
 
void gcc_overload_s1(struct s1 s)
{
    printf("Got a struct s1: %d %d %f\n", s.a, s.b, s.c);
}
 
void gcc_overload_s2(struct s2 s)
{
    printf("Got a struct s2: %lld %lld\n", s.a, s.b);
}
 



int main()
{
    struct s1 a1 = {3, 6, 88.9};

    struct s2 a2 = {16.67, 88.9};

    gcc_overload(a1);

    gcc_overload(a2);

    exit(0);
}