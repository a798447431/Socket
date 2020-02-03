/*************************************************************************
	> File Name: daemon1.c
	> Author: szt
	> Mail: 253604653@qq.com
	> Created Time: 2019年08月08日 星期四 11时07分18秒
 ************************************************************************/

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h> 
#include <sys/param.h>
#include <time.h>
 
void daemon_init() {
	/*
	创建守护进程
	*/
	int i;
	pid_t pid;
	signal(SIGINT,	SIG_IGN);// 终端中断  
	signal(SIGHUP,	SIG_IGN);// 连接挂断  
	signal(SIGQUIT, SIG_IGN);// 终端退出  
	signal(SIGPIPE, SIG_IGN);// 向无读进程的管道写数据	
	signal(SIGTTOU, SIG_IGN);// 后台程序尝试写操作	
	signal(SIGTTIN, SIG_IGN);// 后台程序尝试读操作	
	signal(SIGTERM, SIG_IGN);// 终止
	pid = fork(); 
	if(pid < 0) {  
		perror("fork error!");	
		exit(1);  
	} else if(pid > 0) {  
		exit(0);  
	} 
	setsid(); 
	
	char szPath[1024];	
	if(getcwd(szPath, sizeof(szPath)) == NULL) {   
		exit(1);  
	} else {  
		chdir(szPath);	
	}
		umask(0);
	for (i = 0; i < 3; ++i) {  
		close(i);  
	}
	signal(SIGCHLD,SIG_IGN); 
}
 
 
int main() 
{ 
	time_t now;
	char buf[128]= {0};
	int count =100;
	printf("pid = %d\n", getpid());
	daemon_init();
		
	//syslog(LOG_USER|LOG_INFO,"TestDaemonProcess! \n");
	system("echo \"TestDaemonProcess!\\n\" >> logspp");
	FILE* fd = fopen("./logspp", "r+");
	while(count--) { 
		sleep(2);
	    system("echo \"hahahahah!\\n\" >> logspp");
		time(&now); 
		//syslog(LOG_USER|LOG_INFO,"SystemTime: \t%s\t\t\n",ctime(&now));
		fprintf(fd, "SystemTime: \t%s\t\t.. pid : %d\n",ctime(&now), getpid());	
	} 
	fclose(fd);
   	return 0;
}
