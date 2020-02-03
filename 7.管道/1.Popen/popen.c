/*************************************************************************
	> File Name: popen.c
	> Author: szt
	> Mail: 253604653@qq.com
	> Created Time: 2019年07月30日 星期二 15时13分25秒
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFSIZE 4096

int cnt = 0;
char check_info[10][BUFFSIZE] = {0};

int get_script_info(char *bsname, char *destfile, int ind) {
    FILE *pfile = NULL, *fp;
    while (1) {
        char filename[100] = {0};
        sprintf(filename, "bash %s", bsname);
        pfile = popen(filename, "r");
        fp = fopen(destfile, "a+");
        if (!pfile) {
            fprintf(fp, "Error : Script run failed\n");
            return 0;    
        }
        char buff[BUFFSIZE];
        while(fgets(buff, BUFFSIZE, pfile) != NULL) {
            strcat(check_info[ind], buff);
        }
        if (cnt == 5) {
            fprintf(fp,"%s", check_info[ind]);
            memset(check_info[ind], 0, sizeof(check_info[ind]));
            cnt = 0;
        }
        cnt++;
        sleep(1);
    }
    fclose(fp);
    pclose(pfile);
    return 1;
}

int main() {
    char *script_name = "./cpuinfo.sh";
    char *script_info = "./cpuinfo.info";
    get_script_info(script_name, script_info, 0);
    return 0;
}

