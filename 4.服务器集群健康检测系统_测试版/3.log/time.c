/*************************************************************************
	> File Name: time.c
	> Author: szt
	> Mail: 253604653@qq.com
	> Created Time: 2019年08月06日 星期二 17时11分59秒
 ************************************************************************/

#include<stdio.h>
#include <string.h>
#include <time.h>

int main(int argc,char **argv)
{
        time_t time_now;
        struct tm *curr_time = NULL;
        time(&time_now);
        printf("time_now:[%ld]\n",time_now);

        curr_time = gmtime(&time_now);
        printf("gmtime[%02d:%02d:%02d]\n",curr_time->tm_hour, curr_time->tm_min, curr_time->tm_sec);

        curr_time = localtime(&time_now);
        printf("localtime[%d.%02d.%02d %02d:%02d:%02d]\n",(1900 + curr_time->tm_year),(1 + curr_time->tm_mon),curr_time->tm_mday,curr_time->tm_hour,curr_time->tm_min,curr_time->tm_sec);

        return 0;

}
