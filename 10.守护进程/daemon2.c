/*************************************************************************
	> File Name: daemon2.c
	> Author: szt
	> Mail: 253604653@qq.com
	> Created Time: 2019年08月08日 星期四 11时12分13秒
 ************************************************************************/

#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <stdio.h>
#include <stdbool.h>

static bool flag = true;
void create_daemon();
void handler(int);
 
int main()
{
	time_t t;
	int fd;
	create_daemon();
	struct sigaction act;
	act.sa_handler = handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	if(sigaction(SIGQUIT, &act, NULL))
	{
		printf("sigaction error.\n");
		exit(0);
	}
	while(flag)
	{
		fd = open("./logspp", O_WRONLY | O_CREAT | O_APPEND, 0644);
		if(fd == -1)
		{
			printf("open error\n");
		}
		t = time(0);
		char *buf = asctime(localtime(&t));
		write(fd, buf, strlen(buf));
		close(fd);
		sleep(60);
	}
	return 0;
}
void handler(int sig)
{
	printf("I got a signal %d\nI'm quitting.\n", sig);
	flag = false;
}
void create_daemon()
{
	pid_t pid;
	pid = fork();
	
	if(pid == -1)
	{
		printf("fork error\n");
		exit(1);
	}
	else if(pid)
	{
		exit(0);
	}
 
	if(-1 == setsid())
	{
		printf("setsid error\n");
		exit(1);
	}
 
	pid = fork();
	if(pid == -1)
	{
		printf("fork error\n");
		exit(1);
	}
	else if(pid)
	{
		exit(0);
	}
 
	chdir("/");
	int i;
	for(i = 0; i < 3; ++i)
	{
		close(i);
	}
	umask(0);
	return;
}
