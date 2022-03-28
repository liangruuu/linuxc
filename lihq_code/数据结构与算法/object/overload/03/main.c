#include<stdio.h>
#include<stdlib.h>
#include<stdarg.h>

// 利用了 GCC 的内置函数，__builtin_types_compatible_p() 实现重载

#define gcc_type_overload(A)\
    gcc_type_overload_aux(\
        __builtin_types_compatible_p(typeof(A), struct s1) * 1\
        + __builtin_types_compatible_p(typeof(A), struct s2) * 2\
        , A)

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

void gcc_type_overload_aux(int typeval, ...)
{
    switch(typeval)
    {
        case 1:
        {
            va_list v;
            va_start(v, typeval);
 
            struct s1 s = va_arg(v, struct s1);
 
            va_end(v);
 
            gcc_overload_s1(s);
 
            break;
        }
 
        case 2:
        {
            va_list v;
            va_start(v, typeval);
 
            struct s2 s = va_arg(v, struct s2);
 
            va_end(v);
 
            gcc_overload_s2(s);
 
            break;
        }
 
        default:
        {
            printf("Invalid type to 'gcc_type_overload()'\n");
            exit(1);
        }
    }
}
 


int main()
{
    struct s1 a1 = {3, 6, 88.9};

    struct s2 a2 = {16.67, 88.9};

    gcc_type_overload(a1);

    gcc_type_overload(a2);

    exit(0);
}