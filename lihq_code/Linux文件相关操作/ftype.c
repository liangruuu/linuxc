#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

static int ftype(const char *fname)
{
    struct stat statres;
    if(stat(fname, &statres) < 0)
    {
        perror("stat()");
        exit(1);
    }

    if(S_ISREG(statres.st_mode))
        return '-';
    else if(S_ISDIR(statres.st_mode))
        return 'd';
    else if(S_ISSOCK(statres.st_mode))
        return 's';
    else
        return '?';
}

int main(int argc, char **argv)
{
    int fd;
	if(argc < 2)
	{
		fprintf(stderr, "Usage:%s <filename>\n", argv[0]);
		exit(1);
	}

	printf("%c\n", ftype(argv[1]));
	
	close(fd);
    exit(0);
}

