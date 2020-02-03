/*************************************************************************
	> File Name: shm.c
	> Author: szt
	> Mail: 253604653@qq.com
	> Created Time: 2019年07月29日 星期一 21时01分16秒
 ************************************************************************/

#include <stdio.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <fcntl.h>
#include <string.h>

struct sm_msg {
    char buff[1024];
    pthread_mutex_t sm_mutex;
    pthread_cond_t sm_ready;
};

int main() {
    int shmid, pid;
    char *share_memory = NULL;
    if ((shmid = shmget(IPC_PRIVATE, sizeof(sm_msg), 0666 | IPC_CREAT)) == -1) {
        perror("shmget");
        exit(1);
    }
    
    if ((share_memory = shmat(shmid, 0, 0)) == NULL) {
        perror("shmat");
        exit(1);
    }

    //char *buff = share_memory;
    
    struct sm_msg *msg = (struct sm_msg *)share_memory;


    if ((pid = fork()) < 0) {
        perror("fork");
        exit(1);
    }
    
    if (pid == 0) {
        printf("In Clild -> please input the string:");
        scanf("%s", buff);
        exit(0);
    }

    wait(NULL);
    printf("father-> : %s\n", share_memory);
    return 0;
}
