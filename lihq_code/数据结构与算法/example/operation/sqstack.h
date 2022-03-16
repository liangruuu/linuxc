#ifndef _SQSTACK_H_
#define _SQSTACK_H_

#define MAXSIZE 32

typedef int datatype;

typedef struct node_st 
{
    datatype data[MAXSIZE];
    int top;
}sqstack;

int st_isempty(sqstack *);

int st_push(sqstack *, datatype *);

int st_pop(sqstack *, datatype *);

int st_top(sqstack *, datatype *);

sqstack * st_create(void);

void st_travel(sqstack *, int);

void st_destroy(sqstack *);


#endif
