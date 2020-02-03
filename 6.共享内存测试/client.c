/*************************************************************************
	> File Name: client.c
	> Author: szt
	> Mail: 253604653@qq.com
	> Created Time: 2019年07月29日 星期一 16时45分20秒
 ************************************************************************/

#include "common.h"
int main() {
	int shmid = GetShm(4096);
	sleep(1);
	char *addr = shmat(shmid,NULL,0);
	sleep(2);
	int i = 0;
	while(i < 26) {
		addr[i] = 'A' + i;
		i++;
		addr[i] = 0;
		sleep(1);
	}
	shmdt(addr);
	sleep(2);
	return 0;
}
