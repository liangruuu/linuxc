#include<stdio.h>
#include<stdlib.h>

#include "nohead.h"

int main()
{
   struct node_st *list = NULL;
   struct score_st tmp;
   
   int ret;
   for(int i = 0; i < 7; i++)
   {
       tmp.id = i;
       snprintf(tmp.name, NAMESIZE, "stu_%d",i);
       tmp.math = rand() % 100;
       tmp.chinese = rand() % 100;
       
       ret = list_insert(&list, &tmp);
       if(ret != 0)
           exit(1);
   }
   
   list_show(list);

   list_delete(&list);

   list_show(list);
    
   int id = 3;
   struct score_st *s = list_find(list, &id);
   if(s == NULL)
        printf("can not find id= %d\n", id);
   else
        printf("%d %s %d %d\n",s->id, s->name, s->math, s->chinese);

    list_destroy(list);
   
   exit(0);
}
