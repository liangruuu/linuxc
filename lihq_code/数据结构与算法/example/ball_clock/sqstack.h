#ifndef _SQSTACK_H_
#define _SQSTACK_H_

#define SIZE 32

typedef int type;

typedef struct node_st 
{
    type data[SIZE];
    int top;
}sqstack;

int st_isempty(sqstack *);

int st_push(sqstack *, type *);

int st_pop(sqstack *, type *);

int st_top(sqstack *, type *);

sqstack * st_create(void);

void st_travel(sqstack *);

void st_destroy(sqstack *);


#endif
