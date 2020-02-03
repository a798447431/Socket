/*************************************************************************
	> File Name: common.h
	> Author: szt
	> Mail: 253604653@qq.com
	> Created Time: 2019年08月02日 星期五 10时36分56秒
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
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <pwd.h>

int get_conf_value(char *file, char *key, char *value);
int socket_create(int port);
int socket_connect(char *ip, int port);

