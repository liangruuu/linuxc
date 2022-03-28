#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<pthread.h>
#include <unistd.h>
#include<string.h>
#include<fcntl.h>
#include"relayer.h"

#define   BUFSIZE  1024

static struct rel_job_st* rel_job[REL_JOBMAX];
static pthread_mutex_t mut_rel_job = PTHREAD_MUTEX_INITIALIZER;
static pthread_once_t init_once = PTHREAD_ONCE_INIT;

enum {
	//几种状态
	STATE_R,
	STATE_W,
	STATE_EX,
	STATE_T
};

enum {
	STATE_RUNNING = 1,
	STATE_CANCELED,
	STATE_OVER
};

//状态机
struct rel_fsm_st {
	int state;//记录状态
	int sfd;//源文件
	int dfd;//目的文件
	char buf[BUFSIZE];//中间缓冲区
	int len;//读到的长度
	int pos;//写的过程如果一次没有写完，记录上次写的位置
	char* err;//错误信息
	int64_t count; //输出字符数量
};


//每一对终端结构体
struct rel_job_st{
	//两个终端
	int fd1;
	int fd2;
	//该对终端状态STATE_RUNNING，STATE_CANCELED, STATE_OVER
	int job_state;
	//两个终端的状态机结构体
	struct rel_fsm_st fsm12, fsm21;
	//用来退出复原状态
	int fd1_save, fd2_save;

};

//状态转移函数
static void fsm_driver(struct rel_fsm_st* fsm) {
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
		ret = write(fsm->dfd, fsm->buf + fsm->pos, fsm->len);
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


static void *thr_relayer(void *p) {
	int i;
	while (1) {
		pthread_mutex_lock(&mut_rel_job);
		for (i = 0; i < REL_JOBMAX; i++) {
			if (rel_job[i] != NULL) {
				if (rel_job[i]->job_state == STATE_RUNNING) {
					fsm_driver(&rel_job[i]->fsm12);
					fsm_driver(&rel_job[i]->fsm21);
					if (rel_job[i]->fsm12.state == STATE_T && rel_job[i]->fsm21.state == STATE_T)
						rel_job[i]->job_state = STATE_OVER;
				}
			}
		}
		pthread_mutex_unlock(&mut_rel_job);
	}
	
}

static void module_load(void) {
	int err;
	pthread_t tid_relayer;
	err = pthread_create(&tid_relayer, NULL, thr_relayer, NULL);
	if (err) {
		fprintf(stderr, "pthread_create():%s\n", strerror(err));
		exit(1);
	}

}

static int get_free_pos_unlocked() {
	int i;
	for (i = 0; i < REL_JOBMAX; i++) {
		if (rel_job[i] == NULL)
			return i;
	}
	return -1;
}

int rel_addjob(int fd1, int fd2) {
	struct rel_job_st *me;
	int pos;

	pthread_once(&init_once, module_load);
	me = malloc(sizeof(*me));
	if (me == NULL)   //空间问题
		return -ENOMEM;
	me->fd1 = fd1;
	me->fd2 = fd2;
	me->job_state = STATE_RUNNING;//该对终端设置正在运行

	me->fd1_save = fcntl(me->fd1, F_GETFL);
	fcntl(me->fd1, F_SETFL, me->fd1_save | O_NONBLOCK);  //非阻塞	打开

	me->fd2_save = fcntl(me->fd2, F_GETFL);
	fcntl(me->fd2, F_SETFL, me->fd2_save | O_NONBLOCK);//非阻塞	打开

	me->fsm12.sfd = me->fd1;
	me->fsm12.dfd = me->fd2;
	me->fsm12.state = STATE_R;

	me->fsm21.sfd = me->fd2;
	me->fsm21.dfd = me->fd1;
	me->fsm21.state = STATE_R;

	pthread_mutex_lock(&mut_rel_job);
	pos = get_free_pos_unlocked();
	if (pos < 0) {
		pthread_mutex_unlock(&mut_rel_job);
		fcntl(me->fd1, F_SETFL, me->fd1_save);
		fcntl(me->fd2, F_SETFL, me->fd2_save);
		free(me);
		return -ENOSPC;
	}
	rel_job[pos] = me;
	pthread_mutex_unlock(&mut_rel_job);
	return pos;

}


int rel_canceljob(int id);
/*
	return == 0			成功，指定任务成功取消
		   == -EINVAL	失败，参数非法
		   == -EBUSY    失败，任务早已被取消
*/



int rel_waitjob(int id, struct rel_stat_st*);




int rel_statjob(int id, struct rel_stat_st*);