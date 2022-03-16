#include<stdio.h>
#include<stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include<errno.h>
#include<string.h>
#include"relayer.h"


//两个设备为一对互传数据
#define     TTY1       "/dev/tty12"
#define     TTY2       "/dev/tty10"


#define     TTY3       "/dev/tty9"
#define     TTY4       "/dev/tty8"


int main(int argc, char** argv) {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                
	int fd1, fd2, fd3, fd4;
	int job1, job2;
	fd1 = open(TTY1, O_RDWR);//先以阻塞打开（故意先阻塞形式）
	if (fd1 < 0) {
		perror("open()");
		exit(1);
	}
	write(fd1, "TTY1\n", 5);
	fd2 = open(TTY2, O_RDWR|O_NONBLOCK);//非阻塞
	if (fd2 < 0) {
		perror("open()");
		exit(1);
	}
	write(fd2, "TTY2\n", 5);

	job1 = rel_addjob(fd1, fd2);//添加一对终端互传数据  
	if (job1 < 0) {
		fprintf(stderr, "rel_addjob():%s\n", strerror(-job1));
		exit(1);
	}
	


	fd3 = open(TTY3, O_RDWR);//先以阻塞打开（故意先阻塞形式）
	if (fd3 < 0) {
		perror("open()");
		exit(1);
	}
	write(fd3, "TTY3\n", 5);
	fd4 = open(TTY4, O_RDWR | O_NONBLOCK);//非阻塞
	if (fd4 < 0) {
		perror("open()");
		exit(1);
	}
	write(fd4, "TTY4\n", 5);

	job2 = rel_addjob(fd3, fd4);//添加一对终端互传数据  
	if (job2 < 0) {
		fprintf(stderr, "rel_addjob():%s\n", strerror(-job2));
		exit(1);
	}

	while (1)
		pause();

	close(fd4);
	close(fd3);
	close(fd2);
	close(fd1);
	
	exit(0);
}