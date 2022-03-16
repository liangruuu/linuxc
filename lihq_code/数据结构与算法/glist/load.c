#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>



#include "queue.h"

#define FNAME "tmp/out"

struct node_st
{
    char data;
    struct node_st *l,*r;
};

static void draw_(struct node_st *root, int level)
{
    if(root == NULL)
        return;
    draw_(root->r, level + 1);

    for(int i = 0; i < level; i++)
        printf("    ");

    printf("%c\n", root->data);
    
    draw_(root->l, level + 1);   
}


static void draw(struct node_st *root)
{
    draw_(root, 0);
}

static struct node_st *load_(FILE *fp)
{
    int c;
    struct node_st *root;

    c = fgetc(fp);
    if(c != '(')
    {
        fprintf(stderr, "fgetc():error.\n");
        exit(1);
    }

    c = fgetc(fp);
    if(c == ')')
        return NULL;

    root = malloc(sizeof(*root));
    if(root == NULL)
        exit(1);

    root->data = c;
    root->l = load_(fp);
    root->r = load_(fp);
    fgetc(fp);

    return root;
}


static struct node_st *load(const char *path)
{
    FILE *fp;

    fp = fopen(path, "r");  
    if(fp == NULL)
    {
        printf("can not open file:\n%s\n", path);
        return NULL;
    }

    struct node_st *root;

    root = load_(fp);

    fclose(fp);
    return root; 
}

int main()
{
    
    struct node_st *root = NULL;

    root = load(FNAME);
    
    draw(root);

    exit(0);
}


