/*************************************************************************
	> File Name: client.c
	> Author: szt
	> Mail: 253604653@qq.com
	> Created Time: 2019年07月31日 星期三 15时20分48秒
 ************************************************************************/

#include "client.h"

/********************************************************************
101 CPU
102 MEM 
103 DISK 
104 USER 
105 OS 
106 PRO
********************************************************************/

pthread_mutexattr_t mattr;
pthread_condattr_t cattr;

int main() {
    char config[50] = "./pihealthd_client.conf";
    int client_listenfd;
    int HeartPort, JudgePort, DataPort, AddUserPort;
    char logpath[50] = {0};
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
    
    get_conf_value(config, "JudgePort", tmp);
    JudgePort = atoi(tmp);
    memset(tmp, 0, sizeof(tmp));
    printf("JudgePort = %d\n", JudgePort);
    
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

    get_conf_value(config, "LogPath", tmp);
    strcpy(logpath, tmp);
    memset(tmp, 0, sizeof(tmp));
    printf("LogPath = %s\n", logpath);
    
    //daemon_init();

    struct Share_Mem *share_msg = NULL;
    pthread_mutexattr_init(&mattr);
    pthread_condattr_init(&cattr);
    pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
    pthread_condattr_setpshared(&cattr, PTHREAD_PROCESS_SHARED);

    int shmid = shmget(IPC_PRIVATE, sizeof(struct Share_Mem), IPC_CREAT | 0666);
    if (shmid < 0) {
        write_log(logpath, "[pid = %d] [共享内存] [error] : %s", getpid(), strerror(errno));
        exit(1);
    }
    share_msg = (struct Share_Mem *)shmat(shmid, NULL, 0);
    if (share_msg == (void *)-1 ) {
        write_log(logpath, "[pid = %d] [共享内存] [error] : %s", getpid(), strerror(errno));
        exit(1);
    }
    pthread_mutex_init(&(share_msg->smutex), &mattr);
    pthread_cond_init(&(share_msg->sready), &cattr);
    
    pid_t pid = fork();
    if (pid > 0) {
        send_data(JudgePort, DataPort, ip, share_msg, logpath);
        shmdt((void *)share_msg);
        wait(&pid);
    } else if(pid == 0) {
        pid_t npid = fork();
        if (npid > 0) {
            recv_heartbeat(HeartPort, share_msg, logpath);
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
                    client_heartbeat(ip, AddUserPort, share_msg, logpath);
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
                    Autoselftest(share_msg, cnt, logpath);
                    if (cnt >= 5) cnt = 0;
                }
                exit(0);
            } 
        }
    }
    return 0;
}

