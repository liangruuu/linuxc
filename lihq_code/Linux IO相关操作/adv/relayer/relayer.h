#ifndef RELAYER_H__
#define RELAYER_H__

#include <stdint.h>

#define   REL_JOBMAX  10000


struct rel_stat_st {
	int state;
	int fd1;
	int fd2;
	int64_t count12, count21;//1向2发送了多少字节，2向1发送了多少字节
};


int rel_addjob(int fd1, int fd2);
/*
	return >= 0			成功，返回当前任务ID
		   == -EINVAL	失败，参数非法
		   == -ENOSPC   失败，任务数组满
		   == -ENOMEM   失败，内存分配有误
*/



int rel_canceljob(int id);
/*
	return == 0			成功，指定任务成功取消
		   == -EINVAL	失败，参数非法
		   == -EBUSY    失败，任务早已被取消
*/



int rel_waitjob(int id, struct rel_stat_st*);
/*
	return == 0			成功，指定任务已终止并返回状态
		   == -EINVAL	失败，参数非法
		
*/



int rel_statjob(int id, struct rel_stat_st*);
/*

	return == 0			成功，指定任务状态返回
		   == -EINVAL	失败，参数非法

*/
#endif