#include <stdio.h>
#include <stdlib.h>

#include "sqstack.h"

static int get_pri(int op)
{
    switch(op)
    {
        case '(':
            return 0;
        case '+':
        case '-':
            return 1;
        case '*':
        case '/':
            return 2;
    }
}

static void compute(sqstack *snum, datatype *op)
{
    datatype n1, n2, n;
    st_pop(snum, &n2);
    st_pop(snum, &n1);
    switch (*op)
    {
    case '+':
        n = n1 + n2;
        break;
    case '-':
        n = n1 - n2;
        break;
    case '*':
        n = n1 * n2;
        break;
    case '/':
        n = n1 / n2;
        break;
    default:
        printf("error operator char: %c\n",*op);
        exit(1);
    }
    st_push(snum, &n);
}

static void deal_bracket(sqstack *snum, sqstack *sop)
{
    datatype old_op;
    
    st_top(sop, &old_op);

    while(old_op != '(')
    {
        st_pop(sop, &old_op);
        compute(snum, &old_op);
        st_top(sop, &old_op);
    }
    st_pop(sop, &old_op);
}

static void deal_op(sqstack *snum, sqstack *sop, int op)
{
    datatype old_op;

    if(st_isempty(sop) || op == '(')
    {
        st_push(sop, &op);
        return;
    }
    
    st_top(sop, &old_op);

    if(get_pri(op) > get_pri(old_op))
    {
        st_push(sop, &op);
        return;
    }

    while(get_pri(op) <= get_pri(old_op))
    {
        st_pop(sop, &old_op);
        compute(snum, &old_op);

        if(st_isempty(sop))
            break;
        st_top(sop, &old_op);
    }
    st_push(sop, &op);
}

int main()
{
    
    sqstack *snum, *sop;

    char str[] ="(125-25*5+35)*6";
    // char str[] ="(11+3)*2-5";
    int i = 0;
    int value = 0;
    int flag = 0;

    snum = st_create();
    if(snum == NULL)
        exit(1);
    
    sop = st_create();
    if(sop == NULL)
        exit(1);
 
    while(str[i] != '\0')
    {
        
        // single char is number
        if(str[i] > '0' && str[i] < '9')
        {
            value = value * 10 + str[i] - '0';
            flag = 1;
        }
        else
        {
            if(flag)
            {
                st_push(snum, &value);
                flag = 0;
                value = 0;
            }
            // single char is right bracket
            if(str[i] == ')')
            {
                deal_bracket(snum, sop);
            }
            // single char is operator
            else
            {
                deal_op(snum, sop, str[i]);
            }
            
        }
        i++;
    }

    datatype _op;

    if(flag)
        st_push(snum, &value);
   
    
    while(!st_isempty(sop))
    {
        st_pop(sop, &_op);
        compute(snum, &_op);
        st_pop(snum, &_op);
    }

    printf("result = %d\n", _op);

    st_destroy(snum);
    st_destroy(sop);
    
    exit(0);
}
