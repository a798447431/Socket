/*************************************************************************
	> File Name: client.c
	> Author: szt
	> Mail: 253604653@qq.com
	> Created Time: 2019年07月31日 星期三 15时20分48秒
 ************************************************************************/

#include "client.h"
#define BUFFSIZE 4096

/********************************************************************
101 CPU
102 MEM 
103 DISK 
104 USER 
105 OS 
106 PRO
********************************************************************/

struct Share_Mem {
    int share_cnt;
    pthread_mutex_t smutex;
    pthread_cond_t sready;
};

struct Script {
    char *logpath;
    char message[BUFFSIZE * 5];
};

double dyanmic_parity = 0;
char script_info[6][BUFFSIZE * 5];

pthread_mutexattr_t mattr;
pthread_condattr_t cattr;

void client_heartbeat(char *ip, int port, struct Share_Mem *msg) {
    int delaytime = 10;
    printf("\n \033[32m 心跳进程已开启!\033[0m");
    while (1) {
        if(msg->share_cnt < 5) {
            printf("\033[31m 心跳进程中止!\033[0m");
            break;
        }
        int sockfd = socket_connect(ip, port);
        if (sockfd < 0) {
            close(sockfd);
            printf("\n client_heartbeat connect error, try again!\n");
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

void recv_heartbeat(int port, struct Share_Mem *msg) {
    int listenfd = socket_create(port);
    if (listenfd < 0) {
        printf("recv_heartbeat listenfd create error!\n");
        return ;
    }
    while (1) {
        int newfd = accept(listenfd, NULL, NULL);
        pthread_mutex_lock(&msg->smutex);
        msg->share_cnt = 0;
        pthread_mutex_unlock(&msg->smutex);
        if (newfd < 0) {
            perror("recv_heartbeat accept()");
            continue;
        }
        printf("\033[31m❤ \033[0m ");
        fflush(stdout);
        close(newfd);
    }
    close(listenfd);
}

void recv_data(int DataPort, struct Share_Mem *msg) {
    int listenfd;
    if ((listenfd = socket_create(DataPort)) < 0) {
        perror("listenfd socket_create()");
        return ;
    }
    while (1) {
        int newfd;
        if ((newfd = accept(listenfd, NULL, NULL)) < 0) {
            perror("accept");
            continue;
        }
        char buff[BUFFSIZE] = {0};
        int recv_num = recv(newfd, buff, BUFFSIZE, 0);
        if (recv_num < 0) {
            perror("recv");
            close(newfd);
            continue;
        }
        printf("\n%s\n", buff);
        strcpy(buff, "recv data\n");
        send(newfd, buff, strlen(buff), 0);
        pthread_mutex_lock(&msg->smutex);
        msg->share_cnt = 0;
        pthread_mutex_unlock(&msg->smutex);
        close(newfd);
    }
    close(listenfd);
}

int get_script_info(char *bsname, char *destfile, int cnt, int ind) {
    FILE *pfile = NULL, *fp;
    char filename[100] = {0};
    sprintf(filename, "bash %s", bsname);
    pfile = popen(filename, "r");
    fp = fopen(destfile, "a+");
    if (!pfile) {
        fprintf(fp, "Error : Script run failed\n");
        return 0;
    }
    char buff[BUFFSIZE] = {0};
    if (ind == 2) {
        if (fgets(buff, BUFFSIZE, pfile) != NULL) {
            strcat(script_info[ind], buff);
        }
        if (fgets(buff, BUFFSIZE, pfile) != NULL) {
            dyanmic_parity = atof(buff);
        }
    } else {
        while (fgets(buff, BUFFSIZE, pfile) != NULL) {
            strcat(script_info[ind], buff);
        }
    }
    if (cnt == 5) {
        fprintf(fp, "%s", script_info[ind]);
        memset(script_info[ind], 0, sizeof(script_info[ind]));
    }
    fclose(fp);
    pclose(pfile);
    return 1;
}

void Autoselftest(struct Share_Mem *msg, int cnt) {
    int flag;
    flag = get_script_info("cpuinfo.sh", "./cpu.log", cnt, 0);
    flag &= get_script_info("diskinfo.sh", "./disk.log", cnt, 1);
    char buff[50] = {0};
    sprintf(buff, "./meminfo.sh %.2lf", dyanmic_parity);
    flag &= get_script_info(buff, "./mem.log", cnt , 2);
    flag &= get_script_info("userinfo.sh", "./user.log", cnt, 3);
    flag &= get_script_info("osinfo.sh", "./os.log", cnt, 4);
    flag &= get_script_info("malic_process.sh", "./malic_process.log", cnt, 5);

    if (flag == 0) {
        printf("self_test failed\n");
        return ;
    } else {
        printf("\033[32mOK\033[0m ");
        fflush(stdout);
    }
    sleep(2);

    if (msg->share_cnt >= 5) return ;
    pthread_mutex_lock(&msg->smutex);
    msg->share_cnt += 1;
    if (msg->share_cnt >= 5) {
        pthread_cond_signal(&msg->sready);
    }
    pthread_mutex_unlock(&msg->smutex);
}

int main() {
    char config[50] = "./pihealthd_client.conf";
    int client_listenfd;
    int HeartPort, DataPort, AddUserPort;
    char tmp[20] = {0};
    char ip[20] = {0};
    
    get_conf_value(config, "IP", tmp);
    strcpy(ip, tmp);
    memset(tmp, 0, sizeof(tmp));
    printf("IP = %s\n", ip);
    
    get_conf_value(config, "HeartPort", tmp);
    HeartPort = atoi(tmp);
    memset(tmp, 0, sizeof(tmp));
    printf("HeartPort = %d\n", HeartPort);
    
    get_conf_value(config, "DataPort", tmp);
    DataPort = atoi(tmp);
    memset(tmp, 0, sizeof(tmp));
    printf("DataPort = %d\n", DataPort);
    
    get_conf_value(config, "AddUserPort", tmp);
    AddUserPort = atoi(tmp);
    memset(tmp, 0, sizeof(tmp));
    printf("AddUserPort = %d\n", AddUserPort);
    
    get_conf_value(config, "DyanmicParity", tmp);
    dyanmic_parity = atof(tmp);
    memset(tmp, 0, sizeof(tmp));
    printf("DyanmicParity = %lf\n", dyanmic_parity);
    
    struct Share_Mem *share_msg = NULL;
    pthread_mutexattr_init(&mattr);
    pthread_condattr_init(&cattr);
    pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
    pthread_condattr_setpshared(&cattr, PTHREAD_PROCESS_SHARED);

    int shmid = shmget(IPC_PRIVATE, sizeof(struct Share_Mem), IPC_CREAT | 0666);
    if (shmid < 0) {
        printf("申请共享内存失败!\n");
        exit(1);
    }
    share_msg = (struct Share_Mem *)shmat(shmid, NULL, 0);
    if (share_msg == (void *)-1 ) {
        printf("获取共享内存地址失败!\n");
        exit(1);
    }
    pthread_mutex_init(&(share_msg->smutex), &mattr);
    pthread_cond_init(&(share_msg->sready), &cattr);
    
    pid_t pid = fork();
    if (pid > 0) {
        recv_data(DataPort, share_msg);
        shmdt((void *)share_msg);
        wait(&pid);
    } else if(pid == 0) {
        pid_t npid = fork();
        if (npid > 0) {
            recv_heartbeat(HeartPort, share_msg);
            wait(&npid);
            exit(0);
        }
        if (npid == 0) {
            pid_t mpid = fork();
            if (mpid > 0) {
                while (1) {
                    pthread_mutex_lock(&share_msg->smutex);
                    pthread_cond_wait(&share_msg->sready, &share_msg->smutex);
                    pthread_mutex_unlock(&share_msg->smutex);
                    client_heartbeat(ip, AddUserPort, share_msg);
                    pthread_mutex_lock(&share_msg->smutex);
                    share_msg->share_cnt = 0;
                    pthread_mutex_unlock(&share_msg->smutex);
                }
                wait(&mpid);
                exit(0);
            } else if (mpid == 0) {
                int cnt = 0;
                while (1) {
                    cnt++;
                    Autoselftest(share_msg, cnt);
                    if (cnt >= 5) cnt = 0;
                }
                exit(0);
            } 
        }
    }
    return 0;
}

