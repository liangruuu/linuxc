#include<stdio.h>
#include<stdlib.h>
#include <unistd.h>

#define BUFSIZE 1024
#define DELLINE 10

// r  O_RDONLY
// r+ O_RDWR
// w  O_WRONLY|O_CREAT|O_TRUNC
// w+ O_RDWR|O_CREAT|O_TRUNC

int main(int argc, char **argv)
{
    FILE *fp1, *fp2;
    int fd;
    int size = 0;
    int offsize = 0;
    int pos = 0;
    int ret = 0;
    int posr = 0;
    int posw = 0;
    int count = 0;
    int len = 0;
    char buf[BUFSIZE];
    char *linebuf = NULL;
    size_t linesize = 0;

    if(argc < 2)
    {   
        fprintf(stderr, "Usage:%s <src_file>\n", argv[0]);
        exit(1);
    }

    fp1 = fopen(argv[1], "r");
    if(fp1 == NULL)
    {
        perror("fopen()");
        exit(1);
    }

    fseek(fp1,0,SEEK_END);
    size = ftell(fp1);
    printf("file size: %d\n", size);
    rewind(fp1);

    // while(fgets(buf, BUFSIZE, fp1) != NULL)
    // {
    //     if(++count == DELLINE)
    //         break;
    // }
    while(1)
    {
        if(getline(&linebuf, &linesize, fp1) < 0 || (++count == DELLINE))
            break;
    }
    posr = ftell(fp1);
    printf("read cur: %d\n", posr);

    fp2 = fopen(argv[1], "r+");
    if(fp2 == NULL)
    {
        fclose(fp1);
        perror("fopen()");
        exit(1);
    }
    count = 0;
    // while(fgets(buf, BUFSIZE, fp2) != NULL)
    // {
    //     if(++count == DELLINE - 1)
    //         break;
    // }
    while(1)
    {
        if(getline(&linebuf, &linesize, fp2) < 0 || (++count == DELLINE - 1))
            break;
    }
    posw = ftell(fp2);
    printf("write cur: %d\n", posw);

    while(1)
    {
        len = fread(buf, 1, BUFSIZE, fp1);
        if(len < 0)
        {
            perror("fread()");
            break;
        }
        if(len == 0)
            break;
        
        while(len > 0)
        {
            ret = fwrite(buf + pos, 1, len, fp2);
            if(ret < 0)
            {
                perror("fwrite()");
                exit(1);
            }
            pos += ret;
            len -= ret;
        }
    }

    //文件描述符转换为文件指针
    // fp1 = fdopen(fd1, "r");       
    // if (fp1 == NULL)
    //     perror("fd to fp error.");

    // 文件指针转换为文件描述符
    fd = fileno(fp2);    
    if (fd < 0)
        perror("fp to fd error.");

    offsize = size - (posr - posw);
    if(ftruncate(fd, offsize) < 0)
        perror("ftruncate()");

    free(linebuf);
    fclose(fp2);
    fclose(fp1);

    exit(0);
}
