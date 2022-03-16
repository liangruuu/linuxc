#include <stdio.h>
#include <stdlib.h>

#include "sqstack.h"


int st_isempty(sqstack *st)
{

    return (st->top == -1);
}

int st_push(sqstack *st, datatype *data)
{
    if(st->top == (MAXSIZE -1))
        return -1;

    st->data[++st->top] = *data;
    
    return 0;
}

int st_pop(sqstack *st, datatype *data)
{
    if(st_isempty(st))
       return -1;
    
    *data = st->data[st->top--];
    return 0;
}

int st_top(sqstack *st, datatype *data)
{
    if(st_isempty(st))
       return -1;
    
    *data = st->data[st->top];
    return 0;
}

sqstack * st_create(void)
{
    sqstack *st;
    st = malloc(sizeof(*st));
    if(st == NULL)
        return NULL;

    st->top = -1;
    return st;
}

void st_travel(sqstack *st, int isnum)
{
    
    if(st_isempty(st))
    {
        printf("\n");
        return;
    }
    for(int i=0; i<= st->top; i++)
    {
        if(isnum)
            printf("%d ", st->data[i]);
        else
            printf("%c ", st->data[i]);
    }
    printf("\n");
}

void st_destroy(sqstack *st)
{
    free(st);
}

