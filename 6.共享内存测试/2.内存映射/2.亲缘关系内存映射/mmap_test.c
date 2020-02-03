/*************************************************************************
	> File Name: mmap_test.c
	> Author: szt
	> Mail: 253604653@qq.com
	> Created Time: 2019年07月29日 星期一 23时13分06秒
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>

int m_var = 100;

int main() {
    int *addr;
    pid_t child_pid;

    //以匿名方式创建内存映射区，适用于存在亲缘关系的进程间
    addr = (int *)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0);
    if (addr == MAP_FAILED) {
        perror("mmap failed!\n");
        exit(EXIT_FAILURE);
    }

    child_pid = fork();
    if (child_pid == 0) {
        *addr = 666;
        m_var = 200;
        printf("child process: *addr = %d, m_var = %d\n", *addr, m_var);
    } else {
        sleep(1);
        printf("parent process: *addr = %d, m_var = %d\n", *addr, m_var);
        wait(NULL);

        int ret = munmap(addr, sizeof(int));
        if (ret == -1) {
            perror("munmap failed\n");
            exit(EXIT_FAILURE);
        }
    }
    return 0;
}

