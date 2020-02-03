/*************************************************************************
	> File Name: write.c
	> Author: szt
	> Mail: 253604653@qq.com
	> Created Time: 2019年07月29日 星期一 22时41分58秒
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

typedef struct _data {
    int a;
    char b[64];
} Data;

int main() {
    Data *addr;
    Data data = { 10, "Hello World\n" };
    int fd;

    fd = open("mmap_temp_file", O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        perror("open failed\n");
        exit(EXIT_FAILURE);
    }
    ftruncate(fd, sizeof(data));
    
    addr = (Data *)mmap(NULL, sizeof(data), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED) {
        perror("mmap failed!\n");
        exit(EXIT_FAILURE);
    }
    close(fd);

    memcpy(addr, &data, sizeof(data));
    munmap(addr, sizeof(data));
    return 0;
}
