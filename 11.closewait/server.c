/*************************************************************************
	> File Name: server.c
	> Author: szt
	> Mail: 253604653@qq.com
	> Created Time: 2019年08月20日 星期二 19时25分59秒
 ************************************************************************/

#include "common.h"

#define MAXBUFF 4096

int main() {
    struct sockaddr_in cliaddr;
    socklen_t len;
    int listenfd, connfd;
    char buff[MAXBUFF];
    char str[MAXBUFF];
    int opt = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if ((listenfd = socket_create(8888)) < 0) {
        perror("socket_create");
        exit(1);
    }
    
    printf("Accepting connections ...\n");
    while (1) {
        len = sizeof(cliaddr);
        connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &len);
        int readnum = read(connfd, buff, MAXBUFF);
        if (readnum <= 0) {
            perror("read");
            break;
        }
        printf("received from %s at PORT %d\n", inet_ntop(AF_INET, &cliaddr.sin_addr, str, sizeof(str)), ntohs(cliaddr.sin_port));

        for (int i = 0; i < readnum; i++) {
            buff[i] = toupper(buff[i]);
        }
        write(connfd, buff, readnum);
    }
    return 0;
}
