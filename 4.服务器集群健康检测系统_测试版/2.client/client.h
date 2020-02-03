/*************************************************************************
	> File Name: client.h
	> Author: szt
	> Mail: 253604653@qq.com
	> Created Time: 2019年07月31日 星期三 15时27分58秒
 ************************************************************************/
#include "common.h"
#define BUFFSIZE 4096

struct Share_Mem {
    int share_cnt;
    pthread_mutex_t smutex;
    pthread_cond_t sready;
};

double dyanmic_parity = 0;
char script_info[6][BUFFSIZE * 5];

void client_heartbeat(char *ip, int port, struct Share_Mem *msg, char *logpath);
void recv_heartbeat(int port, struct Share_Mem *msg, char *logpath);
void send_data(int judgeport, int dataport, char *ip, struct Share_Mem *msg, char *logpath);
int get_script_info(char *bsname, char *destfile, int cnt, int ind);
void Autoselftest(struct Share_Mem *msg, int cnt, char *logpath);

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
        write_log(logpath, "[pid = %d] [主动心跳进程] [success] !");
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
            write_log(logpath, "[pid = %d] [接受心跳进程] [warning] : %s", getpid(), strerror(errno));
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
            write_log(logpath, "[pid = %d] [数据交互进程] [warning] : %s", getpid(), strerror(errno));
            continue;
        }
        for (int i = 0; i < 6; i++) {
            int ack = 0, sign = 0;
            int recvnum =  recv(newfd, &sign, sizeof(int), 0);
            if (recvnum <= 0) {
                write_log(logpath, "[pid = %d] [数据交互进程] [warning] : %s", getpid(), strerror(errno));
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
    flag = get_script_info("cpuinfo.sh", "./cpu.log", cnt, 0, logpath);
    flag &= get_script_info("diskinfo.sh", "./disk.log", cnt, 1, logpath);
    char buff[50] = {0};
    sprintf(buff, "meminfo.sh %.2lf", dyanmic_parity);
    flag &= get_script_info(buff, "./mem.log", cnt , 2, logpath);
    flag &= get_script_info("userinfo.sh", "./user.log", cnt, 3, logpath);
    flag &= get_script_info("osinfo.sh", "./os.log", cnt, 4, logpath);
    flag &= get_script_info("malic_process.sh", "./malic_process.log", cnt, 5, logpath);

    if (flag == 0) {
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
