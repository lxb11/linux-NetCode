/*************************************************************************
	> File Name: client.c
	> Author: 
	> Mail: 
	> Created Time: 2017年05月01日 星期一 17时18分37秒
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<netdb.h>
#include<unistd.h>
#include<arpa/inet.h>

//服务器监听的端口号
#define PORT 4000
//我们第一次所能接受到的最大子结束
#define MAXDATASIZE 100

int main(int argc, char *argv[])
{
    //套接子描述
    int sockfd, numbytes;

    char buf[MAXDATASIZE];

    struct hostent *he;
    //链接者的信息
    struct sockaddr_in their_addr;

    if(argc !=2)
    {
        fprintf(stderr, "usage: client hostname\n");
        exit(1);
    }

    //取的主机信息
    if((he = gethostbyname(argv[1])) == NULL)
    {
        herror("gethostbyname");

        exit(1);
    }

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        //如果socket()调用出现错误则显示错误信息并退出
        perror("socket");
        exit(1);
    }
    //主机字节顺序
    their_addr.sin_family = AF_INET;

    //网络字节顺序，端正行
    their_addr.sin_port = htons(PORT);
    their_addr.sin_addr = *((struct in_addr *)he->h_addr);

    //将结构剩下的部分清零
    bzero(&(their_addr.sin_zero), 8);

    if(connect(sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr))== -1)
    {
        perror("connect");
        exit(1);
    }
    if((numbytes = recv(sockfd, buf,MAXDATASIZE,0)) == -1)
    {
        perror("recv");
        exit(1);
    }

    buf[numbytes] = '\0';
    printf("Received: %s\n", buf);
    close(sockfd);

    return 0;
}
