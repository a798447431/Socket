/*************************************************************************
	> File Name: client.c
	> Author: szt
	> Mail: 253604653@qq.com
	> Created Time: 2019年07月27日 星期六 09时40分21秒
 ************************************************************************/

#include "common.h"
#define MAX_SIZE 1024

void sendfile(int sockfd, char *path) {
    FILE *fd = fopen(path, "r");
    if (fd < 0) {
        printf("fopen: %s\n", strerror(errno));
        return ;
    }
    int num_read;
    char data[MAX_SIZE] = {0};
    while(1) {
        num_read = fread(data, 1, MAX_SIZE, fd);
        if (send(sockfd, data, strlen(data), 0) < 0) {
            printf("send: %s\n", strerror(errno));
            fclose(fd);
            return ;
        }
        if (num_read == 0) break;
    }
    fclose(fd);
}

int main(int argc, char *argv[]) {
    int sockfd, sock_file, ack;
    char filename[50] = {0};
    if (argc != 4) {
        printf("Usage:./client ip port filename\n");
        exit(1);
    }
    char *p, *ip = argv[1];
    int port = atoi(argv[2]);
    char *path = argv[3];

    if (access(path, R_OK) != 0) {
        printf("access %s\n", strerror(errno));
        exit(2);
    }
   
    strcpy(filename, (p = strrchr(path, '/')) ? p + 1 : path);

    if ((sockfd = socket_connect(port, ip)) < 0) {
        printf("socket_connect: %s\n", strerror(errno));
        exit(3);
    }
    
    if ((send(sockfd, filename, strlen(filename), 0)) < 0) {
        printf("send error: %s\n", strerror(errno));
        exit(4);
    }    
    
    if (recv(sockfd, &ack, sizeof(ack), 0) < 0) {
        printf("recv error: %s\n", strerror(errno));
        exit(5);
        
    }
    
    if (ack == 1) {
        if ((sock_file = socket_connect(8090, ip)) < 0) {
            printf("socket_connect: %s\n", strerror(errno));
            exit(6);
        }
        printf("socket_connect success\n");
        sendfile(sock_file, path);
        close(sock_file);
    }

    close(sockfd);
    return 0;
}

