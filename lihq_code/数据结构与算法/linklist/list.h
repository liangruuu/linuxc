#ifndef _LIST_H_
#define _LIST_H_

typedef int datatype;

typedef struct node_st
{
    datatype data;
    struct node_st *next;
}list;

list* list_create();

int list_insert_at(list *, int i, datatype *);

int list_order_insert(list *, datatype *);

int list_delete_at(list *, int i, datatype *);

int list_delete(list *, datatype *);

int list_isempty(list *);

void list_display(list *);

void list_destroy(list *);


#endif


