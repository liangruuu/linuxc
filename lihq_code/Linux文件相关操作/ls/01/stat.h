#ifndef __STAT_H_                                                                                                                                                                    
#define __STAT_H_
#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include<pwd.h>
#include<grp.h>
#include<errno.h>
#include<time.h>
#include<dirent.h>
#include<glob.h>
#include<string.h>

#define BUFSIZE 100
#define BUFFSIZE 1024

void statjudge(char *path);
void listl(char *path);
void listi(char *path);
void lista(char *path);
void listh(char *path);
 

#endif