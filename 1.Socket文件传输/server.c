/*************************************************************************
	> File Name: server.c
	> Author: szt
	> Mail: 253604653@qq.com
	> Created Time: 2019年07月27日 星期六 10时08分51秒
 ************************************************************************/

#include "common.h"
#define MAX_SIZE 1024

int main(int argc, char *argv[]) {
    int port, server_listen, server_fd, pid;
    int server_file_listen, server_file;
    char buff[MAX_SIZE] = {0};
    char pathname[MAX_SIZE] = {0};

    if (argc != 2) {
        printf("Usage:./server port\n");
        exit(1);
    }

    port = atoi(argv[1]);
    
    if ((server_listen = socket_create(port)) < 0) {
        printf("socket_create: %s\n", strerror(errno));
        exit(1);
    }
   
    if ((server_file_listen = socket_create(8090)) < 0) {
        printf("socket_create: %s\n", strerror(errno));
        exit(1);
    }

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t len = sizeof(client_addr);
        char ip[20] = {0};
        if ((server_fd = accept(server_listen, (struct sockaddr *)&client_addr, &len)) < 0) {
            printf("accept error: %s\n", strerror(errno));
            exit(1);
        }
        
        strcpy(ip, inet_ntoa(client_addr.sin_addr));
        printf("%s\n", ip);
        
        if (NULL == opendir(ip)) {
            mkdir(ip, 0755);
        }
        
        if ((pid = fork()) < 0) {
            perror("fork");
            exit(1);
        }
        
        if (pid == 0) {
            //close(server_listen);
            if (recv(server_fd, buff, sizeof(buff), 0) > 0) {
                printf("recv filename = %s\n", buff);
            }  
            printf("accept success %d\n", server_fd);
            sprintf(pathname, "%s/%s", ip, buff);
            int ack = 1;
            if (send(server_fd, &ack, sizeof(ack), 0) < 0) {
                printf("send: %s\n", strerror(errno));
                exit(1);
            }
            
            if ((server_file = accept(server_file_listen, NULL, NULL)) < 0) {
                printf("accept error: %s\n", strerror(errno));
                exit(1);
            }
            
            FILE *fp = fopen(pathname, "w");
            while (1) {
                int size;
                memset(buff, 0, sizeof(buff));
                if ((size = recv(server_file, buff, sizeof(buff), 0)) > 0) {
                    fwrite(buff, 1, size, fp);
                }    
                if (size <= 0) {
                    close(server_file);
                    break;
                }
            }
            fclose(fp);
        }
        close(server_fd);
    }
    close(server_fd);
    close(server_listen);
    return 0;
}

