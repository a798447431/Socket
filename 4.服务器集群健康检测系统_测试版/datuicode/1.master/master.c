/*************************************************************************
	> File Name: master.c
	> Author: szt
	> Mail: 253604653@qq.com
	> Created Time: 2019年08月02日 星期五 11时30分56秒
 ************************************************************************/

#include "master.h"

int main() {
    int listenfd;
    char config[50] = "./conf_master.conf";
    char StartIp[20] = {0};
    char EndIp[20] = {0};
    int ins, heartport, dataport, listenport;
    long timeout;
    char reval[20] = {0};

    get_conf_value(config, "INS", reval);
    ins = atoi(reval);
    memset(reval, 0, sizeof(reval));
     
    get_conf_value(config, "HeartPort", reval);
    heartport = atoi(reval);
    memset(reval, 0, sizeof(reval));

    get_conf_value(config, "DataPort", reval);
    dataport = atoi(reval);
    memset(reval, 0, sizeof(reval));
    printf("dataport = %d\n", dataport);
    get_conf_value(config, "ListenPort", reval);
    listenport = atoi(reval);
    memset(reval, 0, sizeof(reval));
    
    get_conf_value(config, "TimeOut", reval);
    timeout = atol(reval);
    memset(reval, 0, sizeof(reval));
    
    get_conf_value(config, "StartIp", reval);
    strcpy(StartIp, reval);
    memset(reval, 0, sizeof(reval));
    
    get_conf_value(config, "EndIp", reval);
    strcpy(EndIp, reval);
    memset(reval, 0, sizeof(reval));
    
    LinkedList *linkedlist =  (LinkedList *)malloc(sizeof(LinkedList) * ins);
    
    int *sum = (int *)malloc(sizeof(int) *ins);
    memset(sum, 0, sizeof(int) * ins);
    
    struct sockaddr_in initaddr;
    initaddr.sin_family = AF_INET;
    initaddr.sin_addr.s_addr = inet_addr("0.0.0.0");
    initaddr.sin_port = htons(0);

    for (int i = 0; i < ins; i++) {
        Node *p = (Node *)malloc(sizeof(Node));
        p->addr = initaddr;
        p->sockfd = -1;
        p->next = NULL;
        linkedlist[i] = p;
    }
    
    unsigned int begin, end;
    begin = ntohl(inet_addr(StartIp));
    end = ntohl(inet_addr(EndIp));

    for (unsigned int i = begin; i <= end; i++) {
        if (i % 256 == 0 || (i + 1) % 256 == 0) {
            continue;
        }
        initaddr.sin_addr.s_addr = htonl(i);
        initaddr.sin_port = htons(heartport);
        Node *p = (Node *)malloc(sizeof(Node));
        p->addr = initaddr;
        p->sockfd = -1;
        p->next = NULL;
        int sub = find_min(sum, ins);
        insert(linkedlist[sub], p);
        sum[sub] += 1;
    }
    
    for (int i = 0; i < ins; i++) {
        printf("Ins<%d>\n", i);
        print_linkedlist(linkedlist[i]);
    }
    
    pthread_t pth_heart, pth_data[ins];
    
    struct Heart heart;
    heart.ins = ins;
    heart.sum = sum;
    heart.linkedlist = linkedlist;
    heart.timeout = timeout;

    pthread_create(&pth_heart, NULL, do_heart, (void *)&heart);
    
    sleep(5);

    struct Data darg[ins];

    for (int i = 0; i < ins; i++) {
        darg[i].head = linkedlist[i];
        darg[i].ind = i;
        darg[i].port = dataport;
        pthread_create(&pth_data[i], NULL, do_data, (void *)&darg[i]);
    }
    
    if ((listenfd = socket_create(listenport)) < 0) {
        perror("socket_create()");
        exit(1);
    }

    listen_epoll(listenfd, linkedlist, sum, ins, heartport);
    
    pthread_join(pth_heart, NULL);
    
    for (int i = 0; i < ins; i++) {
        pthread_join(pth_data[i], NULL);
    }
    
    return 0;
}

