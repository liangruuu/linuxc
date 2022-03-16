#include<stdio.h>
#include<stdlib.h>

struct simple_st
{
    int i;
    char ch;
    float f;
};


struct simple_st2
{
    int i;
    char ch;
    float f;
}__attribute__((packed)); // 不要进行位对齐

int main(){
    struct simple_st st = {1, 'a', 3.1};
    struct simple_st2 st2 = {2, 'b', 4.1};
    
    printf("%d %c %f\n",st.i, st.ch, st.f);
    printf("%d %c %f\n",st2.i, st2.ch, st2.f);
    printf("struct size = %lu\n",sizeof(st));
    printf("struct2 size = %lu\n",sizeof(st2));

    printf("int size = %lu\n",sizeof(st.i));
    printf("char size = %lu\n",sizeof(st.ch));
    printf("float size = %lu\n",sizeof(st.f));



    exit(0) ;
}
