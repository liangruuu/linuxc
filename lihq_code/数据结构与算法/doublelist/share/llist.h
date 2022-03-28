#ifndef _LLIST_H_
#define _LLIST_H_


#define LLIST_FORWARD 1
#define LLIST_BACKWARD 2

//define some callback function 
typedef void llist_op(const void *);

typedef int llist_cmp(const void *, const void *);

struct llist_node_st
{
    struct llist_node_st *prve;
    struct llist_node_st *next;
    //char类型占位符，用于实现变长结构体
    char data[1];
};

typedef struct llist_head
{
    int size;
    struct llist_node_st head;
    //使用函数指针来实现方法的封装,实现类的方法
    int (*insert)(struct llist_head *, const void *, int);
    void * (*find)(struct llist_head *, const void *, llist_cmp *);
    int (*delete)(struct llist_head *, const void *, llist_cmp *);
    int (*fetch)(struct llist_head *, const void *, llist_cmp *, void *);
    void (*travel)(struct llist_head *, llist_op *);

}LLIST;

LLIST *llist_create(int initsize);
void llist_destroy(LLIST *);

#endif
