/*************************************************************************
	> File Name: common.c
	> Author: suziteng
	> Mail: 253604653@qq.com
	> Created Time: 2019年06月26日 星期三 18时58分41秒
 ************************************************************************/

#include "common.h"

int socket_connect(int port, char *host) {
	int sockfd;
	struct sockaddr_in dest_addr;
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket() error");
        return -1;
	}

	memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(port);
	dest_addr.sin_addr.s_addr = inet_addr(host);

	if (connect(sockfd, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
		perror("connect() error");
		return -1;
	}
	return sockfd;
}

int socket_create(int port) {
    int sockfd;
    struct sockaddr_in my_addr;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket() error");
        return -1;
    }
    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(port);
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    socklen_t socklen = sizeof(struct sockaddr);
    if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(my_addr)) < 0) {
        perror("bind() error");
        return -1;
    }
    if (listen(sockfd, 20) < 0) {
        perror("listen() error");
        return -1;
    }
    return sockfd;
}

int socket_connect_nonblock(char *ip, int port, long timeout) {
    int sockfd, ret;
    struct sockaddr_in server;
    struct timeval tm;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return -1;
    }
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(ip);
    
    printf("connect to %s:%d\n", ip, port);

    unsigned long ul = 1;
    ioctl(sockfd, FIONBIO, &ul);
    
    fd_set set;
    FD_ZERO(&set);
    FD_SET(sockfd, &set);
    
    tm.tv_sec = 0;
    tm.tv_usec = timeout;
    
    int error = -1;
    int len = sizeof(error);

    if (connect(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        if (select(sockfd + 1, NULL, &set, NULL, &tm) > 0) {
            if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, (socklen_t *)&len) < 0) {
                perror("getsockopt");
                ret = -1;
            }
            if (error == 0) {
                ret = 0;
            } else {
                ret = -1;
            }
        }
    }
    close(sockfd);
    return ret;
}
