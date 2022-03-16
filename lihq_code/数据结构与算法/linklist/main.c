#include <stdio.h>
#include <stdlib.h>

#include "list.h"


int main()
{
    list* l;
    datatype arr[] = {12, 9, 23, 2, 34, 6, 45};


    l = list_create();
    if(l == NULL)
        exit(1);

    for(int i = 0;i < (sizeof(arr)/sizeof(*arr));i++)
    {
        if(list_order_insert(l, &arr[i]))
            exit(1);
    }

    list_display(l);
    
    int v = 34;
    list_delete(l, &v);
    list_display(l);

    datatype b;
    int err = list_delete_at(l, 2, &b);
    printf("err = %d del num = %d\n", err, b);
    if(err)
        exit(1);

    list_display(l);

    list_destroy(l);


    exit(0);

}
