#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

static void *func(void *p)
{
    while(1)
        pause();
    pthread_exit(NULL);
}

int main()
{
    pthread_t tid;
    pthread_attr_t attr;
    int i, err;
    
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 1024*1024);


    for(i = 0; ;i++)
    {
        err =  pthread_create(&tid, &attr, func, NULL);
        if(err)
        {
            fprintf(stderr, "pthread_create():%s\n", strerror(err));
            break;
        }
    }
    
    printf("max thread num:%d\n",i);

    pthread_attr_destroy(&attr);

    pthread_join(tid, NULL);

    
    exit(0);
}
