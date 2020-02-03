/*************************************************************************
	> File Name: read.c
	> Author: szt
	> Mail: 253604653@qq.com
	> Created Time: 2019年07月29日 星期一 22时54分34秒
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

typedef struct _data {
    int a;
    char b[64];
} Data;

int main() {
    Data *addr;
    int fd;

    fd = open("mmap_temp_file", O_RDONLY);
    if (fd == -1) {
        perror("open failed\n");
        exit(EXIT_FAILURE);
    }

    addr = (Data *)mmap(NULL, sizeof(Data), PROT_READ, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED) {
        perror("mmap failed!\n");
        exit(EXIT_FAILURE);
    }
    close(fd);
    printf("read from mmap: a = %d, b = %s\n", addr->a, addr->b);
    munmap(addr, sizeof(Data));
    return 0;
}
