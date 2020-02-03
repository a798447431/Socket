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
#include <sys/mman.h>

struct sm_msg {
    char buff[1024];
    pthread_mutex_t sm_mutex;
    pthread_cond_t sm_ready;
};

int main() {
    int shmid, pid;

    pthread_mutexattr_t m_attr = PTHREAD_MUTEX_INITIALIZER;
    pthread_condattr_t c_attr = PTHREAD_COND_INITIALIZER;

    pthread_mutexattr_setpshared(&m_attr, PTHREAD_PROCESS_SHARED);
    pthread_condattr_setpshared(&c_attr, PTHREAD_PROCESS_SHARED);
     
    struct sm_msg *msg;
    msg = (struct sm_msg *)mmap(NULL, sizeof(struct sm_msg), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0);
    if (msg == MAP_FAILED) {
        perror("mmap failed!\n");
        exit(1);      
    }

    pthread_mutex_init(&(msg->sm_mutex), &m_attr);
    pthread_cond_init(&(msg->sm_ready), &c_attr);

    
    if ((pid = fork()) < 0) {
        perror("fork");
        exit(1);
    }
    
    if (pid == 0) {
        while (1) {
            scanf("%s", msg->buff);
            pthread_mutex_lock(&msg->sm_mutex);
            pthread_cond_signal(&msg->sm_ready);
            pthread_mutex_unlock(&msg->sm_mutex);
        }   
    }
    
    while (1) {
        pthread_mutex_lock(&msg->sm_mutex);
        pthread_cond_wait(&msg->sm_ready, &msg->sm_mutex);
        pthread_mutex_unlock(&msg->sm_mutex);
        printf("father-> : %s\n", msg->buff);
        memset(msg->buff, 0, sizeof(msg->buff));
    } 
    
    int ret = munmap(msg, sizeof(struct sm_msg));
    
    if (ret == -1) {
        perror("munmap failed\n");
        exit(EXIT_FAILURE);        
    }

    return 0;
}
