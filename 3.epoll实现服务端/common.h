/*************************************************************************
	> File Name: common.h
	> Author: suziteng
	> Mail: 253604653@qq.com
	> Created Time: 2019年06月20日 星期四 18时26分07秒
 ************************************************************************/
#ifndef _COMMON_H
#include <stdio.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <sys/ioctl.h>
#include <sys/epoll.h>

int socket_connect(int port, char *host);
int socket_create(int port);
int socket_connect_nonblock(char *ip, int port, long timeout);

#define _COMMON_H
#endif
