#include <stdio.h>
#include <stdlib.h>
#include <glob.h>

#define PAT "/etc/a*.conf"
#define PAT2 "/etc/*"

#if 0
static int errfunc_(const char *errpath, int eerrno)
{
    puts(errpath);
    fprintf(stderr, "ERROR MSG:%s\n", strerror(eerrno));

    return 0;
}
#endif

int main()
{
    glob_t globres;
    int err;

    err = glob(PAT2, 0, NULL, &globres);
    if(err)
    {
        printf("error code = %d\n", err);
        exit(1);
    }
   
    for(int i = 0; i < globres.gl_pathc; i++)
        puts(globres.gl_pathv[i]);

    globfree(&globres);
    exit(0);
}
