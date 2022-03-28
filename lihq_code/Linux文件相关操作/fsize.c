#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

static off_t fsize(const char *fname)
{
    struct stat statres;
    if(stat(fname, &statres) < 0)
    {
        perror("stat()");
        exit(1);
    }

    return statres.st_size;
}

int main(int argc, char **argv)
{
    if(argc < 2)
    {
        fprintf(stderr, "Usage:%s <filename>\n",argv[0]);

    }

    printf("file size:%ld\n", fsize(argv[1]));
    exit(0);
}
