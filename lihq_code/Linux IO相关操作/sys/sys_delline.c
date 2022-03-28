#include<stdio.h>
#include<stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUFSIZE 1024
#define DELLINE 10

// r  O_RDONLY
// r+ O_RDWR
// w  O_WRONLY|O_CREAT|O_TRUNC
// w+ O_RDWR|O_CREAT|O_TRUNC

int main(int argc, char **argv)
{
    int fd1, fd2;
    int size = 0;
    int offsize = 0;
    int pos = 0;
    int ret = 0;
    int posr = 0;
    int posw = 0;
    int count = 0;
    int len = 0;
    char buf;
    char linebuf[BUFSIZE];

    if(argc < 2)
    {   
        fprintf(stderr, "Usage:%s <src_file>\n", argv[0]);
        exit(1);
    }

    fd1 = open(argv[1], O_RDONLY);
    if(fd1 < 0)
    {
        perror("open()");
        exit(1);
    }

    lseek(fd1, 0, SEEK_END);
    size = lseek(fd1, 0, SEEK_CUR);
    printf("file size: %d\n", size);
    lseek(fd1, 0, SEEK_SET);
    lseek(fd1, 0, SEEK_CUR);

    while(1)
    {
        len = read(fd1, &buf, 1);
        if(len < 0)
        {
            perror("read()");
            break;
        }
        if(len == 0)
            break;

        if(buf == '\n')
            count++;
        
        if(count == DELLINE)
            break;
    }
    posr = lseek(fd1, 0, SEEK_CUR);
    printf("read cur: %d\n", posr);

    fd2 = open(argv[1], O_RDWR);
    if(fd2 < 0)
    {
        close(fd1);
        perror("open()");
        exit(1);
    }

    len = 0;
    count = 0;
    while(1)
    {
        len = read(fd2, &buf, 1);
        if(len < 0)
        {
            perror("read()");
            break;
        }
        if(len == 0)
            break;

        if(buf == '\n')
            count++;
        
        if(count == DELLINE - 1)
            break;
    }
    posw = lseek(fd2, 0, SEEK_CUR);
    printf("write cur: %d\n", posw);

    len = 0;
    while(1)
    {
        len = read(fd1, linebuf, BUFSIZE);
        if(len < 0)
        {
            perror("read()");
            break;
        }
        if(len == 0)
            break;
        
        while(len > 0)
        {
            ret = write(fd2, linebuf + pos, len);
            if(ret < 0)
            {
                perror("write()");
                exit(1);
            }
            pos += ret;
            len -= ret;
        }
    }

    offsize = size - (posr - posw);
    if(truncate(argv[1], offsize) < 0)
        perror("truncate()");


    close(fd2);
    close(fd1);

    exit(0);
}