#include<stdio.h>
#include<stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include<errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <poll.h>

#define     TTY1       "/dev/tty11"
#define     TTY2       "/dev/tty12"
#define     BUFSIZE     1024
enum {
	//几种状态
	STATE_R,
	STATE_W,
	STATE_AUTO,
	STATE_EX,
	STATE_T
};

struct fsm_st {
	int state;//记录状态
	int sfd;//源文件
	int dfd;//目的文件
	char buf[BUFSIZE];//中间缓冲区
	int len;//读到的长度
	int pos;//写的过程如果一次没有写完，记录上次写的位置
	char* err;//错误信息
};


static void fsm_driver(struct fsm_st* fsm) {
	int ret;
	switch (fsm->state) {
	case STATE_R:
		fsm->len = read(fsm->sfd, fsm->buf, BUFSIZE);
		if (fsm->len == 0)
			fsm->state = STATE_T;
		else if (fsm->len < 0) {
			if (errno == EAGAIN)
				fsm->state = STATE_R;
			else {
				fsm->err = "read()";
				fsm->state = STATE_EX;
			}

		}
		else {
			fsm->pos = 0;
			fsm->state = STATE_W;
		}

		break;
	case STATE_W:
		ret = write(fsm->dfd, fsm->buf + fsm->pos, BUFSIZE);
		if (ret < 0) {
			if (errno == EAGAIN)
				fsm->state = STATE_W;
			else {
				fsm->err = "write()";
				fsm->state = STATE_EX;
			}

		}
		else {
			fsm->pos += ret;
			fsm->len -= ret;
			if (fsm->len == 0)
				fsm->state = STATE_R;//写完了再去读
			else
				fsm->state = STATE_W;//没写完继续写
		}
		break;
	case STATE_EX:
		perror(fsm->err);
		fsm->state = STATE_T;
		break;
	case STATE_T:
		/*  do smoething*/
		break;
	default:
		abort();
		break;

	}
}

static int max(int a, int b) {
	return a > b ? a : b;
}
static void relay(int fd1, int fd2) {
	struct fsm_st fsm12, fsm21;
	struct pollfd pfd[2];
	int fd1_save = fcntl(fd1, F_GETFL);
	fcntl(fd1, F_SETFL, fd1_save | O_NONBLOCK);		//非阻塞	打开


	int fd2_save = fcntl(fd2, F_GETFL);
	fcntl(fd2, F_SETFL, fd2_save | O_NONBLOCK);		//非阻塞	打开

	//初始状态
	fsm12.state = STATE_R;
	fsm12.sfd = fd1;
	fsm12.dfd = fd2;

	fsm21.state = STATE_R;
	fsm21.sfd = fd2;
	fsm21.dfd = fd1;



	pfd[0].fd = fd1;
	pfd[1].fd = fd2;
	while (fsm12.state != STATE_T || fsm21.state != STATE_T) {

		//布置监视任务
		pfd[0].events = 0;//位图清0
		if (fsm12.state == STATE_R)   //如果可读，或上读的事件
			pfd[0].events |= POLLIN;
		if (fsm21.state == STATE_W)
			pfd[0].events |= POLLOUT;

		pfd[1].events = 0;
		if (fsm12.state == STATE_W)
			pfd[1].events |= POLLOUT;
		if (fsm21.state == STATE_R)
			pfd[1].events |= POLLIN;
		
		//监视
		if (fsm12.state < STATE_AUTO || fsm21.state < STATE_AUTO) {					
			while (poll(pfd, 2, -1) < 0){			 //pollfd数组首地址， 数组长度， 阻塞形式
				if (errno == EINTR) {
					continue;
				}
				perror("poll()");
				exit(1);
			}
		}
		
		//查看监视结果
		if ( pfd[0].revents & POLLIN || pfd[1].revents & POLLOUT || fsm12.state > STATE_AUTO)//如果1可读2可写或者处于EX,T态
			fsm_driver(&fsm12);
		if (pfd[1].revents & POLLIN || pfd[0].revents & POLLOUT || fsm21.state > STATE_AUTO)//如果2可读1可写
			fsm_driver(&fsm21);
	}

	//复原退出
	fcntl(fd1, F_SETFL, fd1_save);
	fcntl(fd2, F_SETFL, fd2_save);
}

int main(int argc, char** argv) {
	int fd1, fd2;
	fd1 = open(TTY1, O_RDWR);//先以阻塞打开（故意先阻塞形式）
	if (fd1 < 0) {
		perror("open()");
		exit(1);
	}
	write(fd1, "TTY1\n", 5);
	fd2 = open(TTY2, O_RDWR | O_NONBLOCK);//非阻塞
	if (fd2 < 0) {
		perror("open()");
		exit(1);
	}
	write(fd2, "TTY2\n", 5);
	relay(fd1, fd2);		//核心代码

	close(fd2);
	close(fd1);
	
	exit(0);

}
