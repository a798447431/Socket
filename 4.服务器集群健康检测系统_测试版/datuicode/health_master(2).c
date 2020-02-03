/*************************************************************************
	> File Name: health_master.c
	> Author: victoria 
	> Mail: 1105847344@qq.com
	> Created Time: 2019年07月25日 星期四 09时36分15秒
 ************************************************************************/

#include <pthread.h>
#include "health_master.h"

int main() {
    int listenfd;
    char *config = "./health.conf";
    int Port, Ins, ClientHeart_port, CtlPort;
    long Timeout;
    char FromIp[20] = {0}, ToIp[20] = {0};
    pthread_t pth_heart;


    char tmp[20] = {0};
    get_conf_value(config, "INS", tmp);
    Ins = atoi(tmp);

    memset(tmp, 0, sizeof(tmp));
    get_conf_value(config, "ClientHeart", tmp);
    ClientHeart_port = atoi(tmp);

    memset(tmp, 0, sizeof(tmp));
    get_conf_value(config, "MasterPort", tmp);
    Port = atoi(tmp);
    
    memset(tmp, 0, sizeof(tmp));
    get_conf_value(config, "CtlPort", tmp);
    CtlPort = atoi(tmp);

    memset(tmp, 0, sizeof(tmp));
    get_conf_value(config, "TimeOut", tmp);
    Timeout = atol(tmp);
    
    memset(tmp, 0, sizeof(tmp));
    get_conf_value(config, "From", tmp);
    strcpy(FromIp, tmp);

    memset(tmp, 0, sizeof(tmp));
    get_conf_value(config, "To", tmp);
    strcpy(ToIp, tmp);
    
    LinkList *linklist = (LinkList*) malloc (sizeof(LinkList) * Ins);
    int *sum = (int*) malloc (sizeof(int) * Ins);
    memset(sum, 0, sizeof(int) * Ins);
    
    //初始化链表数组头结点
    struct sockaddr_in initaddr;
    initaddr.sin_family = AF_INET;
    initaddr.sin_port = htons(ClientHeart_port);
    initaddr.sin_addr.s_addr = inet_addr("0.0.0.0");
    for (int i = 0; i < Ins; i++) {
        Node *p = (Node*) malloc (sizeof(Node));
        p->addr = initaddr;
        p->fd = -1;
        p->next = NULL;
        linklist[i] = p;
    }
     
    unsigned int sip, eip;
    sip = ntohl(inet_addr(FromIp));
    eip = ntohl(inet_addr(ToIp));
    for (unsigned int i = sip; i <= eip; i++) {
        if (i % 256 == 0 || i % 256 == 255) continue;
        initaddr.sin_addr.s_addr = htonl(i);
        Node *p = (Node*) malloc (sizeof(Node));
        p->addr = initaddr;
        p->fd = -1;   
        p->next = NULL;
        int sub = find_min(sum, Ins);
        if (insert(linklist[sub], p)) {
            sum[sub] += 1;
        }
    }
    
    for (int i = 0; i < Ins; i++) {
        printf("Ins <%d> :\n", i);
        output(linklist[i]);
    }
     
    Heart heartArg;
    heartArg.sum = sum;
    heartArg.link = linklist;
    heartArg.ins = Ins;
    heartArg.timeout = Timeout;
    pthread_create(&pth_heart, NULL, do_heart, (void*)&heartArg);
   
    struct Work workarg[Ins];
    pthread_t pth[Ins];
    for (int i = 0; i < Ins; i++) {
        workarg[i].link = linklist[i];
        workarg[i].index = i;
        workarg[i].ctlport = CtlPort;
        pthread_create(&pth[i], NULL, do_work, (void*)&workarg[i]);
    }
    
    if ((listenfd = socket_create(Port)) < 0) {
        perror("socket_create");
        exit(0);
    }
    
    listen_epoll(listenfd, linklist, sum, Ins, ClientHeart_port);
    
    for (int i = 0; i < Ins; i++) {
        pthread_join(pth[i], NULL);
    }    
    pthread_join(pth_heart, NULL);
    return 0;
}

