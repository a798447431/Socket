/*************************************************************************
	> File Name: common.h
	> Author: szt
	> Mail: 253604653@qq.com
	> Created Time: 2019年07月29日 星期一 16时43分36秒
 ************************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

#define PATHNAME "."
#define PROJ_ID 0x6666
 
int CreateShm(int size);
int DestroyShm(int shmid);
int GetShm(int size);


