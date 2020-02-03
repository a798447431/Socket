/*************************************************************************
	> File Name: common.h
	> Author: szt
	> Mail: 253604653@qq.com
	> Created Time: 2019年07月30日 星期二 23时32分00秒
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <sys/file.h>
#include <stdarg.h>
#include <errno.h>
#include <signal.h>
#include <sys/param.h>

int socket_create(int port);
int socket_connect(char *ip, int port);
int get_conf_value(char *file, char *key, char *value);

