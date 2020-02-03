/*************************************************************************
	> File Name: common.c
	> Author: szt
	> Mail: 253604653@qq.com
	> Created Time: 2019年08月02日 星期五 10时43分07秒
 ************************************************************************/

#include "common.h"

int get_conf_value(char *file, char *key, char *value) {
   	FILE *fp = NULL;
    char *line = NULL, *substr = NULL;
    size_t n = 0, len = 0;
    ssize_t read;
    if (key == NULL || value == NULL) {
        printf("error\n");
        return -1;
    }
    fp = fopen(file, "r");
    if (fp == NULL) {
        printf("Open config file error!\n");
        return -1;
    }
    while ((read = getline(&line, &n, fp)) != -1) {
        substr = strstr(line, key);
        if (substr == NULL) continue;
        else {
            len = strlen(key);
            if (line[len] == '=') {
                strncpy(value, &line[len + 1], (int)read - len - 2);
                break;
            } else {
                continue;
            }
        }
    }
    if (substr == NULL) {
        printf("%s Not found in %s!\n", key, file);
        fclose(fp);
        free(line);
        return -1;
    }
    fclose(fp);
    free(line);
    return 0;
}

/*
int socket_create(int port) {
    int listenfd;
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        return -1;
    }
    
    int flag = 1, len = sizeof(int);
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &flag, len) < 0) {
        close(listenfd);
        perror("setsockopt");
        return -1;
    }

    struct sockaddr_in naddr;
    memset(&naddr, 0, sizeof(struct sockaddr));
    naddr.sin_family = AF_INET;
    naddr.sin_addr.s_addr = inet_addr(INADDR_ANY);
    naddr.sin_port = htons(port);
    
    if (bind(listenfd, (struct sockaddr *)&naddr, sizeof(naddr)) < 0) {
        perror("socket bind");
        return -1;
    }
    
    
    if (listen(listenfd, 2000) < 0) {
        perror("socket listen");
        return -1;
    }

    return listenfd;
}
*/
int socket_create(int port) {
    int listenfd;
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    if (bind(listenfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(1);
    }
    if (listen(listenfd, 2000) < 0) {
        perror("listen");
        exit(1);
    }
	return listenfd;
}



int socket_connect(char *ip, int port) {
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        return -1;
    }

    struct sockaddr_in naddr;
    memset(&naddr, 0, sizeof(struct sockaddr));
    naddr.sin_family = AF_INET;
    naddr.sin_addr.s_addr = inet_addr(ip);
    naddr.sin_port = htons(port);

    if (connect(sockfd, (struct sockaddr *)&naddr, sizeof(naddr)) < 0) {
        perror("connect");
        return -1;
    }

    return sockfd;
}
