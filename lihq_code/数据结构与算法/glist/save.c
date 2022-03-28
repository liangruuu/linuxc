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

static int insert(struct node_st **root, int data)
{
    struct node_st *node;

    if(*root == NULL)
    {
        node = malloc(sizeof(*node));
        if(node == NULL)
            return -1;
        node->data = data;
        node->l = NULL;
        node->r = NULL;
        *root = node;

        return 0;
    }
    
    if(data <= (*root)->data)
       return insert(&(*root)->l, data);
    
    return insert(&(*root)->r, data);
}

static int save_(struct node_st *root, FILE *fp)
{
    fputc('(', fp);

    if(root == NULL)
    {
        fputc(')', fp);
        return 0;
    }

    fputc(root->data, fp);

    save_(root->l, fp);
    save_(root->r, fp);


    fputc(')', fp);
}

static int mkdir_(const char *spath)  
{  
    char DirName[256];  
    strcpy(DirName, spath);  
    int i;
    int len = strlen(DirName);
    for(i=1; i<len; i++)  
    {  
        if(DirName[i]=='/')  
        {  
            DirName[i] = 0; 
            if(access(DirName, 0) == -1)  
            {  
                if(mkdir(DirName, 0755)==-1)  
                    return -1;   
            }  
            DirName[i] = '/';  
        }  
    } 

    return 0;  
} 

static int save(struct node_st *root, const char *path)
{
    FILE *fp;

    if(access(path,0) == -1)
    {
        if(mkdir_(path) == 0)
            printf("created the directory:\n%s\n" , path);
        else
        {
            printf("can not creat the directoty:\n%s\n", path);
            return -1;
        }
    }

    fp = fopen(path, "w");  
    if(fp == NULL)
    {
        printf("can not open file:\n%s\n", path);
        return -1;
    }

    save_(root, fp);

    printf("save file success.\n");

    
    return 0; 
}
int main()
{
    struct node_st *tree = NULL;
    char arr[] = "cefadjbh";

    for(int i = 0; i < sizeof(arr)/sizeof(*arr)-1; i++)
    {  
        insert(&tree, arr[i]);
    }
    

    draw(tree);

    save(tree, FNAME);

    exit(0);
}


