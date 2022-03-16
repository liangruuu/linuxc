#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <glob.h>

#define PATHSIZE 1024


static int path_noloop(const char *path)
{
    char *pos;

    pos = strrchr(path, '/');
    if(pos == NULL)
        exit(1);

    if(strcmp(pos+1, ".") == 0 || strcmp(pos+1, "..") == 0)
        return 0;

    return 1;
}

static int64_t mydu(const char *path)
{
    static struct stat statres;
    static char nextpath[PATHSIZE];
    int64_t sum = 0;
    glob_t globres;


    if(lstat(path, &statres) < 0)
    {
        perror("lstat()");
        exit(1);
    }
    
    if(!S_ISDIR(statres.st_mode))
        return statres.st_blocks;

    strncpy(nextpath, path, PATHSIZE);
    strncat(nextpath, "/*", PATHSIZE);
    glob(nextpath, 0, NULL, &globres);

    // include the hide files
    strncpy(nextpath, path, PATHSIZE);
    strncat(nextpath, "/.*", PATHSIZE);
    glob(nextpath, GLOB_APPEND, NULL, &globres);

    sum = statres.st_blocks;

    for(int i = 0; i < globres.gl_pathc; i++)
    {
        if(path_noloop(globres.gl_pathv[i]))
        {
            sum += mydu(globres.gl_pathv[i]);
            // puts(globres.gl_pathv[i]);
        }
    }

    
    globfree(&globres);
    return sum;
}

int main(int argc, char **argv)
{
    if(argc < 2)
    {
        fprintf(stderr, "Usage: %s <filename or directory>\n", argv[0]);
        exit(1);
    }
    
    printf("%ld\t%s\n", mydu(argv[1]) / 2, argv[1]);
    


    exit(0);
}
