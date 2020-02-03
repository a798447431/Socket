/*************************************************************************
	> File Name: daemon.c
	> Author: szt
	> Mail: 253604653@qq.com
	> Created Time: 2019年08月08日 星期四 10时53分15秒
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <time.h>
#include <syslog.h>
#include <unistd.h>

#define BUFFMAX 4096

int init_daemon() {
    int pid;
    int i;

    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

    pid = fork();
    
    if (pid > 0) {
        exit(0);
    }
    else if(pid < 0) {
        return -1;
    }
    setsid();
    pid = fork();
    if (pid > 0) {
        exit(0);
    }
    else if (pid < 0) {
        return -1;
    }

    for (i = 0; i < NOFILE; close(i++));

    chdir("/");

    umask(0);

    signal(SIGCHLD, SIG_IGN);
    
    return 0;
}

int main() {
    time_t now;
    char buf[BUFFMAX] = {0};
    init_daemon();
    int count = 100;

    system("echo \"TestDaemonProcess!\\n\" >> logspp");
    FILE *fd = fopen("./logspp", "r+");
    while (count--) {
        sleep(2);
        time(&now);
        fprintf(fd, "SyetemTime : \t%s\t\t\n", ctime(&now));
    }
    fclose(fd);
    return 0;
}
