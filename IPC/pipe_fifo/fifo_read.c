/*************************************************************************
	> File Name: fifo_read.c
	> Author: 
	> Mail: 
	> Created Time: 2018年03月21日 星期三 21时53分33秒
 ************************************************************************/

#include<stdio.h>
#include<unistd.h>
#include<sys/stat.h>
#include<stdlib.h>
#include<fcntl.h>
#include<time.h>
#include<errno.h>

int fifo_oread()
{
    int fd;
    int len;
    char buf[1024];

    if(mkfifo("fifo1", 0666) < 0 && errno != EEXIST)//创建FIFO管道
    {
        perror("Create fifo failed");
    }

    if((fd = open("fifo1", O_RDONLY)) < 0) //以du打开一个FIFO
    {
        perror("open fifo failed\n");
        exit(1);
    }

    while((len = read(fd, buf, 1024)) > 0) //读取fifo管道
    {
        printf("read message: %s", buf);
    }
    close(fd);//关闭fifo文件
    return 0;
}

int main(void)
{
    fifo_oread();
    return 0;
}
