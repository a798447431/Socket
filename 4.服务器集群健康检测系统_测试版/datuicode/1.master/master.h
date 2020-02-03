/*************************************************************************
	> File Name: master.h
	> Author: szt
	> Mail: 253604653@qq.com
	> Created Time: 2019年08月02日 星期五 11时32分36秒
 ************************************************************************/

#include "common.h"
#define BUFFSIZE 4096
#define MAXCLIENT 10000

typedef struct Node {
    struct sockaddr_in addr;
    int sockfd;
    struct Node *next;
} Node, *LinkedList;

struct Heart {
    int *sum;
    LinkedList *linkedlist;
    int ins;
    long timeout;
};

struct Data {
    LinkedList head;
    int ind;
    int port;
};

int insert(LinkedList head, Node *node);
void print_linkedlist(LinkedList head);
void *do_heart(void *arg);
int find_min(int *sum, int ins);
int check_connect(struct sockaddr_in addr, long timeout);
void *do_data(void *arg);
int listen_epoll(int listenfd, LinkedList *linkedlist, int *sum, int ins, int heartport);

int insert(LinkedList head, Node *node) {
    Node *p = head;
    while (p->next != NULL) {
        p = p->next;
    }
    p->next = node;
    return 0;
}

void print_linkedlist(LinkedList head) {
    Node *p = head;
    int cnt = 0;
    while (p->next != NULL) {
        printf("<%d> : %s\n", cnt++, inet_ntoa(p->next->addr.sin_addr));
        p = p->next;
    }
}

int find_min(int *sum, int ins) {
    int sub = 0;
    for (int i = 0; i < ins; i++) {
        if (*(sum + i) < *(sum + sub)) {
            sub = i;               
        }    
    }
    return sub;
}

int check_connect(struct sockaddr_in addr, long timeout) {
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        return 0;
    }
    int len = sizeof(int);
    unsigned long ul = 1;
    ioctl(sockfd, FIONBIO, &ul);

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = timeout;
    
    fd_set wfds;
    FD_ZERO(&wfds);
    FD_SET(sockfd, &wfds);
    
    int error = -1;
    int ret = 0;

    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        if (select(sockfd + 1, NULL, &wfds, NULL, &tv) > 0) {
            getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, (socklen_t *)&len);
            if (error == 0) {
                ret = 1;
            } else {
                ret = 0;
            }
        }  
    }
    close(sockfd);
    return ret;
}

void *do_heart(void *arg) {
    int cnt = 1;
    printf("cnt = %d\n", cnt);

    struct Heart *harg = (struct Heart *)arg;
    while (1) {
        for (int i = 0; i < harg->ins; i++) {
            Node *p = harg->linkedlist[i];
            while (p->next != NULL) {
                if (!check_connect(p->next->addr, harg->timeout)) {
                    printf("<%s:%d> \033[31m用户不在线!\033[0m\n", inet_ntoa(p->next->addr.sin_addr), ntohs(p->next->addr.sin_port));
                    Node *q = p->next;
                    p->next = p->next->next;
                    free(q);
                    harg->sum[i] -= 1;
                } else {
                    printf("<%s:%d> \033[32m用户在线!\033[0m\n", inet_ntoa(p->next->addr.sin_addr), ntohs(p->next->addr.sin_port));
                    p = p->next;
                }
            }
        }
        sleep(5);
        printf("\n");
    }
} 

void *do_data(void *arg) {
    struct Data *dataarg = (struct Data *)arg;
    char log[50] = {0};
    sprintf(log, "./%d.log", dataarg->ind);
    int epollfd = epoll_create(MAXCLIENT);
    struct epoll_event ev, events[MAXCLIENT];
    while (1) {
        FILE *fp = fopen(log, "a+");
        Node *p = dataarg->head;
        while (p->next != NULL) {
            int client_sockfd = socket(AF_INET, SOCK_STREAM, 0);
            if (client_sockfd < 0) {
                perror("sockfd socket()");
                continue;
            }
            unsigned long ul = 1;
            ioctl(client_sockfd, FIONBIO, &ul);
            struct sockaddr_in addr;
            addr.sin_family = AF_INET;
            addr.sin_addr = p->next->addr.sin_addr;
            addr.sin_port = htons(dataarg->port);
            connect(client_sockfd, (struct sockaddr *)&addr, sizeof(struct sockaddr));
            fprintf(fp, "%s:%d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
            ev.events = EPOLLOUT;
            ev.data.fd = client_sockfd;
            if (epoll_ctl(epollfd, EPOLL_CTL_ADD, client_sockfd, &ev) == -1) {
                perror("epoll_create");
                continue;
            }
            p->sockfd = client_sockfd;
            p = p->next;
        }
        int nfds = epoll_wait(epollfd, events, MAXCLIENT, -1);
        if (nfds == -1) {
            perror("epoll_wait");
            fclose(fp);
            return NULL;
        } else if (nfds == 0) {
            continue;
        }
        for (int i = 0; i < nfds; i++) {
            if (events[i].events & EPOLLOUT) {
                int data_sockfd = events[i].data.fd;
                unsigned long ul = 0;
                ioctl(data_sockfd, FIONBIO, &ul);
                struct epoll_event em;
                char buff[BUFFSIZE] = {0};
                send(data_sockfd, "Give Me Data!", strlen("Give Me Data!"), 0);
                int recv_num = recv(data_sockfd, buff, BUFFSIZE, 0);
                if (recv_num > 0) {
                    printf("%s", buff);
                }
                em.data.fd = data_sockfd;
                epoll_ctl(epollfd, EPOLL_CTL_DEL, data_sockfd, &em);
                close(data_sockfd);
            }
        }
        fclose(fp);
        sleep(5);
    }
}

int listen_epoll(int listenfd, LinkedList *linkedlist, int *sum, int ins, int heartport) {
    unsigned int ul = 1;
    ioctl(listenfd, FIONBIO, &ul);
    struct epoll_event ev, events[MAXCLIENT];
    int epollfd = epoll_create(MAXCLIENT);
    if (epollfd < 0) {
        perror("epoll_create()");
        return -1;
    }
    ev.data.fd = listenfd;
    ev.events = EPOLLIN;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &ev) < 0) {
        perror("epoll_ctl()");
        return -1;
    }

    while (1) {
        int nfds = epoll_wait(epollfd, events, MAXCLIENT, -1);
        if (nfds < 0) {
            perror("epoll_wait()");
            return -1;
        } else if(nfds == 0) {
            continue;
        }

        for (int i = 0; i < nfds; i++) {
            if (events[i].data.fd == listenfd && events[i].events & EPOLLIN) {
                struct sockaddr_in newaddr;
                socklen_t len = sizeof(newaddr);
                int newsock = accept(listenfd, (struct sockaddr *)&newaddr, &len);
                if (newsock < 0) {
                    perror("accept()");
                    return -1;
                }
                int sub = find_min(sum, ins);
                Node *p = (Node *)malloc(sizeof(Node));
                newaddr.sin_port = htons(heartport);
                p->addr = newaddr;
                p->sockfd = newsock;
                p->next = NULL;
                insert(linkedlist[sub], p);
                printf("insert %s:%d is success!\n", inet_ntoa(newaddr.sin_addr), htons(newaddr.sin_port));
                sum[sub]++;
            }
        }
    }
}
