/*************************************************************************
	> File Name: producer_consumer.c
	> Author: szt
	> Mail: 253604653@qq.com
	> Created Time: 2019年08月01日 星期四 10时34分50秒
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define MAX_STOCK 20 //容量
char g_storage[MAX_STOCK];
size_t g_stock = 0; //当前库存
pthread_mutex_t g_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t g_full = PTHREAD_COND_INITIALIZER;
pthread_cond_t g_empty = PTHREAD_COND_INITIALIZER;

void show(const char *who, const char *op, char prod) {
    printf("%s:", who);
    size_t i;
    for (i < 0; i < g_stock; i++) {
        printf("%c", g_storage[i]);
    }
    printf("%s%c\n", op, prod);
}

void *producer(void *arg) {  
    const char * who = (const char *)arg;
    while (1) {
        pthread_mutex_lock(&g_mtx);
        while (g_stock >= MAX_STOCK) {
            printf("%s:full\n", who);
            pthread_cond_wait(&g_full, &g_mtx);
        }
        char prod = 'A' + rand() % 26;
        show(who, "<-", prod);
        g_storage[g_stock++] = prod;
        pthread_cond_broadcast(&g_empty);
        pthread_mutex_unlock(&g_mtx);
        usleep((rand() % 100) * 1000);
    }
    return NULL;
}

void *customer(void *arg) {
    const char * who = (const char *)arg;
    while (1) {
        pthread_mutex_lock(&g_mtx);
        while (!g_stock) {
            printf("%s:空仓\n", who);
            pthread_cond_wait(&g_empty, &g_mtx);
        }
        char prod = g_storage[--g_stock];
        show(who, "->", prod);
        pthread_cond_broadcast(&g_full);
        pthread_mutex_unlock(&g_mtx);
        usleep((rand() % 100) * 1000);
    }
    return NULL;
}

int main() {
    srand(time(NULL));
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_t tid;
    pthread_create(&tid, &attr, producer, "prodecer1");
    pthread_create(&tid, &attr, producer, "prodecer2");
    pthread_create(&tid, &attr, customer, "customer1");
    pthread_create(&tid, &attr, customer, "customer2");
    getchar();
}
