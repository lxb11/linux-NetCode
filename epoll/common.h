/*************************************************************************
	> File Name: common.h
	> Author: 
	> Mail: 
	> Created Time: 2017年12月03日 星期日 11时05分42秒
 ************************************************************************/

#ifndef _COMMON_H
#define _COMMON_H

#include<stdio.h>
#define IPADDR "127.0.0.1"
#define PORT 8080
#define _MAX_NUM_ 64
#define _BACK_LOG_ 5

int set_nonblock(int fd);

int start_up();

//读，定长
ssize_t readn(int fd, void *vptr, ssize_t n);

//写，定长
ssize_t writen(int fd, const void *vptr, size_t n);

#endif
