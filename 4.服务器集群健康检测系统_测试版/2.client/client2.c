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

double dyanmic_parity = 0;
char script_info[6][BUFFSIZE * 5];

pthread_mutexattr_t mattr;
pthread_condattr_t cattr;

void client_heartbeat(char *ip, int port, struct Share_Mem *msg, char *logpath) {
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
            write_log(logpath, "[pid = %d] [主动心跳进程] [error] : %s", getpid(), strerror(errno));
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

void recv_heartbeat(int port, struct Share_Mem *msg, char *logpath) {
    int listenfd = socket_create(port);
    if (listenfd < 0) { 
        write_log(logpath, "[pid = %d] [接受心跳进程] [error] : %s", getpid(), strerror(errno));
        return ;
    }
    while (1) {
        int newfd = accept(listenfd, NULL, NULL);
        pthread_mutex_lock(&msg->smutex);
        msg->share_cnt = 0;
        pthread_mutex_unlock(&msg->smutex);
        if (newfd < 0) {
            write_log(logpath, "[pid = %d] [接受心跳进程] [waring] : %s", getpid(), strerror(errno));
            continue;
        }
        printf("\033[31m❤ \033[0m ");
        fflush(stdout);
        close(newfd);
    }
    close(listenfd);
}

void send_data(int judgeport, int dataport, char *ip, struct Share_Mem *msg, char *logpath) {
    int listenfd = socket_create(judgeport);
    if (listenfd < 0) {
        write_log(logpath, "[pid = %d] [数据交互进程] [error] : %s", getpid(), strerror(errno));
        return ;
    }

    while (1) {
        int newfd;
        if ((newfd = accept(listenfd, NULL, NULL)) < 0) {
            write_log(logpath, "[pid = %d] [数据交互进程] [waring] : %s", getpid(), strerror(errno));
            continue;
        }
        for (int i = 0; i < 3; i++) {
            int ack = 0, sign = 0;
            int recvnum =  recv(newfd, &sign, sizeof(int), 0);
            if (recvnum <= 0) {
                write_log(logpath, "[pid = %d] [数据交互进程] [waring] : %s", getpid(), strerror(errno));
                close(newfd);
                continue;
            }
            char pathname[50] = {0};
            switch(sign) {
                case 101: {
                    sprintf(pathname, "./cpu.log");
                } break;
                case 102: {
                    sprintf(pathname, "./disk.log");
                } break;
                case 103 : {
                    sprintf(pathname, "./mem.log");
                } break;
 		case 104: {
                    sprintf(pathname, "./user.log");
                } break;
                case 105: {
                    sprintf(pathname, "./os.log");
                } break;
                case 106 : {
                    sprintf(pathname, "./malic_process.log");
                } break;
            }
            if (access(pathname, F_OK) < 0) {
                ack = 0;
                send(newfd, &ack, sizeof(int), 0);
                continue;
            }
            ack = 1;
            send(newfd, &ack, sizeof(int), 0);
            ack = 0;
            recvnum = recv(newfd, &ack, sizeof(int), 0);

            if (recvnum < 0 || ack != 1) {
                close(newfd);
                continue;
            }
            
            int sendfd = socket_connect(ip, dataport);
            FILE *rdfp = NULL;
            rdfp = fopen(pathname, "r");
            flock(rdfp->_fileno, LOCK_EX);
            char buff[BUFFSIZE] = {0};
            while (1) {
                int num_read = fread(buff, 1, BUFFSIZE, rdfp);
                if (send(sendfd, buff, num_read, 0) < 0) {
                    write_log(logpath, "[pid = %d] [数据交互进程] [error] : %s", getpid(), strerror(errno));
                    fclose(rdfp);
                    return ;
                }
                if (num_read == 0) break;
            }
            memset(buff, 0, BUFFSIZE);
            memset(pathname, 0, sizeof(pathname));
            fclose(rdfp);
            close(sendfd);
        }
        pthread_mutex_lock(&msg->smutex);
        msg->share_cnt = 0;
        pthread_mutex_unlock(&msg->smutex);
        close(newfd);
    }
    close(listenfd);
    return ;
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

void Autoselftest(struct Share_Mem *msg, int cnt, char *logpath) {
    int flag;
    flag = get_script_info("cpuinfo.sh", "./cpu.log", cnt, 0);
    flag &= get_script_info("diskinfo.sh", "./disk.log", cnt, 1);
    char buff[50] = {0};
    sprintf(buff, "meminfo.sh %.2lf", dyanmic_parity);
    flag &= get_script_info(buff, "./mem.log", cnt , 2);
    flag &= get_script_info("userinfo.sh", "./user.log", cnt, 3);
    flag &= get_script_info("osinfo.sh", "./os.log", cnt, 4);
    flag &= get_script_info("malic_process.sh", "./malic_process.log", cnt, 5);

    if (flag == 0) {
        write_log(logpath, "[pid = %d] [用户自检进程] [error] : %s", getpid(), strerror(errno));
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
        //recv_data(DataPort, share_msg);
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

