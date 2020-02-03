/*************************************************************************
	> File Name: client.c
	> Author: szt
	> Mail: 253604653@qq.com
	> Created Time: 2019年08月20日 星期二 19时01分46秒
 ************************************************************************/

#include "common.h"

#define MAXBUFF 4096

int main(int argc, char *argv[]) {
    int sockfd;
    char buff[MAXBUFF];
    char *str;
    
    if (argc != 2) {
        printf("client: ./client massage!\n");
        exit(1);
    }
    
    str = argv[1];
    
    char ip[20] = "192.168.2.58";

    if ((sockfd = socket_connect(ip, 8888)) < 0) {
        perror("socket_connect");
    }
    
    write(sockfd, str, strlen(str));

    int readnum = read(sockfd, buff, MAXBUFF);
    printf("Response from server:\n");
    write(STDOUT_FILENO, buff, readnum);
    write(STDOUT_FILENO, "\n", 1);
    
    close(sockfd);
    return 0;
}
