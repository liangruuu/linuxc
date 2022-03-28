#include<stdio.h>
#include<stdlib.h>

#include "queue.h"

#define NAMESIZE 32

struct score_st
{
    int id;
    char name[NAMESIZE];
    int math;
    int chinese;
};

struct node_st
{
    struct score_st data;
    struct node_st *l,*r;
};

struct score_st * find(struct node_st *root, int id)
{
    if(root == NULL)
        return NULL;

    if(id == root->data.id)
        return &root->data;
    
    if(id < root->data.id)
       return find(root->l, id);
    else
        return find(root->r, id);
}

static void print_s(struct score_st *r)
{
    printf("%d %s %d %d \n", r->id, r->name, r->math, r->chinese);
}

static void draw_(struct node_st *root, int level)
{
    if(root == NULL)
        return;
    draw_(root->r, level + 1);

    for(int i = 0; i < level; i++)
        printf("    ");

    print_s(&root->data);
    
    draw_(root->l, level + 1);   
}

static int get_num(struct node_st *root)
{
    if(root == NULL)
        return 0;

    return get_num(root->l) + 1 + get_num(root->r);
}

static struct node_st *find_min(struct node_st *root)
{
    if(root->l == NULL)
        return root;

    return find_min(root->l);
}

static struct node_st *find_max(struct node_st *root)
{
    if(root->r == NULL)
        return root;

    return find_min(root->r);
}

static void turn_left(struct node_st **root)
{
    struct node_st *cur = *root;

    *root = cur->r;
    cur->r = NULL;

    find_min(*root)->l = cur;
}

static void turn_right(struct node_st **root)
{
    struct node_st *cur = *root;

    *root = cur->l;
    cur->l = NULL;

    find_max(*root)->r = cur;

}

static void balance(struct  node_st **root)
{
    if(*root == NULL)
        return;
    int sub;
    while(1)
    {
        sub = get_num((*root)->l) - get_num((*root)->r);

        if(sub >= -1 && sub <= 1)
            break;

        if(sub < -1)
            turn_left(root);
        else
            turn_right(root);
    }

    balance(&(*root)->l);

    balance(&(*root)->r);
}


static void draw(struct node_st *root)
{
    draw_(root, 0);
}

static int insert(struct node_st **root, struct score_st *data)
{
    struct node_st *node;

    if(*root == NULL)
    {
        node = malloc(sizeof(*node));
        if(node == NULL)
            return -1;
        node->data = *data;
        node->l = NULL;
        node->r = NULL;
        *root = node;

        return 0;
    }
    
    if(data->id <= (*root)->data.id)
       return insert(&(*root)->l, data);
    
    return insert(&(*root)->r, data);
}

static void delete(struct node_st **root, int id)
{
    struct node_st **node = root;
    struct node_st *cur = NULL;

    while(*node != NULL && (*node)->data.id != id)
    {
        if(id <(*node)->data.id)
            node = &(*node)->l;
        else
            node = &(*node)->r;
    }
    
    if(*node == NULL)
        return;

    cur = *node;
    if(cur->l == NULL)
        *node = cur->r;
    else
    {
        *node = cur->l;
        find_max(cur->l)->r = cur->r;
    }
    
    free(cur);
}

static void travel(struct node_st *root, int type)
{
    if(root == NULL)
        return;
    
    if(type == 1)
    {
        //先序遍历
        print_s(&root->data);
        travel(root->l, type);
        travel(root->r, type);
    }
    else if(type == 2)
    {
        //中序遍历
        travel(root->l, type);
        print_s(&root->data);
        travel(root->r, type);
    }
    else
    {
        //后序遍历
        travel(root->l, type);
        travel(root->r, type);
        print_s(&root->data);
    }
}

static void travel_level(struct node_st *root)
{
    QUEUE *qu;
    struct node_st *cur;

    qu = qu_create(sizeof(struct node_st *));
    if(qu == NULL)
        return;

    qu_enqueue(qu, &root);
    int ret;
    while(1)
    {
        ret = qu_dequeue(qu, &cur);
        if(ret == -1)
            break;
        print_s(&cur->data);

        if(cur->l != NULL)
            qu_enqueue(qu, &cur->l);
        if(cur->r != NULL)
            qu_enqueue(qu, &cur->r);
    }

    qu_destroy(qu);
}

int main()
{
    struct node_st *tree = NULL;
    struct score_st tmp;
    struct score_st *datap;
    int arr[] = {1,2,3,7,6,5,9,8,4};

    for(int i = 0; i < sizeof(arr)/sizeof(*arr); i++)
    {
        tmp.id = arr[i];
        snprintf(tmp.name, NAMESIZE, "std_%d", arr[i]);
        tmp.math = rand() % 100;
        tmp.chinese = rand() % 100;        
        insert(&tree, &tmp);
    }
    
//    int tmpid = 2;
//    datap = find(tree, tmpid);
//    if(datap == NULL)
//        printf("Can not find id= %d\n", tmpid);
//    else
//        print_s(datap);

    draw(tree);

    printf("\n\n");

    balance(&tree);

    draw(tree);

    printf("\n\n");

    // int tmpid = 5;
    // delete(&tree, tmpid);

    // draw(tree);

    // travel(tree, 1);
    // printf("\n");
    // travel(tree, 2);
    // printf("\n");
    // travel(tree, 3);

    travel_level(tree);

    exit(0);
}
