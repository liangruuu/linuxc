#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

//ls -l file        查看文件逻辑大小

//du -c file     查看文件实际占用的存储块多少

//od -c file     查看文件存储的内容

int main(int argc, char **argv)
{
	int fd;
	if(argc < 2)
	{
		fprintf(stderr, "Usage:%s <filename>\n", argv[0]);
		exit(1);
	}

	fd = open(argv[1], O_WRONLY|O_CREAT|O_TRUNC, 0600);
	if(fd < 0)
	{
		perror("open()");
		exit(1);
	}
	// extend the size of file
	lseek(fd, 5LL * 1024LL * 1024LL * 1024LL - 1LL, SEEK_SET);
	// write '\0' to end file
	write(fd, "", 1);
	close(fd);
	
	exit(0);
}
