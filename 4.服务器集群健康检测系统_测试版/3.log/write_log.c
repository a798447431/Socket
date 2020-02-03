/*************************************************************************
	> File Name: write_log.c
	> Author: szt
	> Mail: 253604653@qq.com
	> Created Time: 2019年08月06日 星期二 16时52分22秒
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/file.h>
#include <error.h>
#include <string.h>
#include <stdarg.h>

int write_log(char *pathname, const char *format, ...) {
    FILE *fp = fopen(pathname, "a+");
    flock(fp->_fileno, F_OK);
    va_list arg;
    int done;
    va_start(arg, format);
    time_t time_now  = time(NULL);
    struct tm *curr_time = localtime(&time_now);
    fprintf(fp, "%d.%02d.%02d %02d:%02d:%02d", (1900 + curr_time->tm_year), (1 + curr_time->tm_mon), curr_time->tm_mday, curr_time->tm_hour, curr_time->tm_min, curr_time->tm_sec);    
    done = vfprintf(fp, format, arg);
    va_end(arg);
    fprintf(fp, "\n");
    fclose(fp);
    return done;
}


int main() {
    char pathname[20] = "./daily.log";
    int num = 0;
    num = write_log(pathname, "变参函数测试");
    return 0;
}
