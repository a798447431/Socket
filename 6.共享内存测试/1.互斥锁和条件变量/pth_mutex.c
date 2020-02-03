/*************************************************************************
	> File Name: pth_mutex.c
	> Author: szt
	> Mail: 253604653@qq.com
	> Created Time: 2019年07月29日 星期一 19时39分48秒
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

char *buf[5];
int pos;

//定义互斥量
pthread_mutex_t mutex;

void *task(void *p) {
    //使用互斥锁进行加锁
    pthread_mutex_lock(&mutex);
    
    buf[pos] = (char *)p;
    sleep(1);
    pos++;
    
    //使用互斥量进行解锁
    pthread_mutex_unlock(&mutex);
}

int main() {
    pthread_mutex_init(&mutex, NULL);

    pthread_t tid, tid2;
    pthread_create(&tid, NULL, task, (void *)"zhangfei");
    pthread_create(&tid2, NULL, task, (void *)"guanyu");

    pthread_join(tid, NULL);
    pthread_join(tid2, NULL);

    pthread_mutex_destroy(&mutex);

    int i = 0;
    printf("字符指针数组的内容是: ");
    for (i = 0; i < pos; i++) {
        printf("%s ", buf[i]);
    }
    printf("\n");
    return 0;
}
