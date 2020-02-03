/*************************************************************************
	> File Name: client.c
	> Author: szt
	> Mail: 253604653@qq.com
	> Created Time: 2019年08月02日 星期五 16时44分41秒
 ************************************************************************/

#include "client.h"
#define BUFFSIZE 4096

void client_heartbeat(char *ip, int port) {
    int delaytime = 10;
    printf("\n \033[32m 心跳进程已开启!\033[0m");
    while (1) {
        int sockfd = socket_connect(ip, port);
        if (sockfd < 0) {
            close(sockfd);
            sleep(delaytime);
            if (delaytime < 100) {
                delaytime += 10;
            }
            continue;
        }
        printf("\n \033[32m 心跳成功!\033[0m");
        close(sockfd);
        break;
    }
}

void recv_heart(int port) {
	int listenfd;
    listenfd = socket_create(port);
    if (listenfd < 0) {
        perror("socket_create");
        return ;
	}

    while (1) {
        int newfd = accept(listenfd, NULL, NULL);
        if (newfd < 0) {
            perror("accept");
            continue;
        } 
        printf(" \033[31m -❤- \033[0m ");
        fflush(stdout);
        close(newfd);
    }
    close(listenfd);
    return ;
}

void recv_data(int port) {
    int listenfd;
    if ((listenfd = socket_create(port)) < 0) {
        perror("error socket_create()");
        return ;
    }
    while (1) {
        int newfd = accept(listenfd, NULL, NULL);
        if (newfd < 0) {
            perror("accept");
            continue;
        }
        char buff[BUFFSIZE] = {0};
        int recv_num = recv(newfd, buff, sizeof(buff), 0);
        if (recv_num <= 0) {
            perror("recv");
            close(newfd);
            continue;
        }
        printf("\n%s\n", buff);
        memset(buff, 0, sizeof(buff));
        strcpy(buff, "bye");
        send(newfd, buff, strlen(buff), 0);
        memset(buff, 0, sizeof(buff));
        close(newfd);
    }
    close(listenfd);
}

int main() {
    int heartport, dataport, loadport; 
    char tmp[20] = {0};
    char *config = "./conf_client.conf";
    char ip[20] = {0};
    
    get_conf_value(config, "HeartPort", tmp);
    heartport = atoi(tmp);
    memset(tmp, 0, sizeof(tmp));
    printf("heartport = %d\n", heartport);
    get_conf_value(config, "DataPort", tmp);
    dataport = atoi(tmp);
    memset(tmp, 0, sizeof(tmp));
    
    get_conf_value(config, "LoadPort", tmp);
    loadport = atoi(tmp);
    memset(tmp, 0, sizeof(tmp));
    
    get_conf_value(config, "IP", tmp);
    strcpy(ip, tmp);
    memset(tmp, 0, sizeof(tmp));

    pid_t pid;
    pid = fork();
    
    if (pid < 0) {
        perror("fork()");
        exit(1);
    } else if (pid == 0) {
        pid_t cpid = fork();
        if (cpid == 0) {
            client_heartbeat(ip, loadport);
        }
        recv_heart(heartport);
    }
    recv_data(dataport); 
    wait(&pid);
    return 0;
}

