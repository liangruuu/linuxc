/*===========================================================
* File Name   : jose.c
* Author      : 
* Created time: 2021-01-14 18:12
* Description :
===========================================================*/

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

#define JOSE_NUM 8

struct node_st
{
    int data;
    struct node_st *next;
};

struct node_st* jose_create(int n)
{
    struct node_st *me, *newnode, *cur;
    //create first node
    me = malloc(sizeof(*me));
        if(me == NULL)
            return NULL;
        me->data = 1;
        me->next = me;

    cur = me;
    for (int i = 2; i <= n; i++)
    {
        newnode = malloc(sizeof(*newnode));
        if(newnode == NULL)
            return NULL;
        newnode->data = i;
        newnode->next = me;

        cur->next = newnode;
        cur = newnode;
    }

    return me;
}

void jose_kill(struct node_st **me,int n)
{
    struct node_st *cur = *me;
    struct node_st *node;
    int i = 1;
    while(cur != cur->next)
    {
        while(i < n)
        {
            node = cur;
            cur = cur->next;
            i++;
        }

        printf("%d ", cur->data);
        node->next = cur->next;
        free(cur);
        cur = node->next;
        i = 1;
    }
    printf("\n");
    *me = cur;
}

void jose_show(struct node_st *me)
{
    struct node_st *list = me;

    do
    {
        printf("%d ", list->data);
        // sleep(1);
        // fflush(NULL);
        list = list->next;
    }while(list != me);
    printf("\n");
}

int main(){

    struct node_st *list;

    list = jose_create(JOSE_NUM);
    if(list == NULL)
        exit(1);

    jose_show(list);

    printf("kill:\n");

    jose_kill(&list, 3);

    jose_show(list);


    exit(0);
}
