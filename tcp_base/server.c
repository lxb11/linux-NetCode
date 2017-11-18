/*************************************************************************
	> File Name: server.c
	> Author: 
	> Mail: 
	> Created Time: 2017年05月01日 星期一 16时53分44秒
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<string.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<sys/wait.h>
#include<arpa/inet.h>
#include<unistd.h>

//服务器要监听的本地端口号
#define MYPORT 4000

//能够同时接受多少链接
#define BACKLOG 10

int main()
{
    //在sock_id上进行监听， new_fd接受新的链接
    int sock_fd, new_fd;

    //自己的地址信息
    struct sockaddr_in my_addr;

    //链接这的地址信息
    struct sockaddr_in their_addr;

    int sin_size;
    //这里就是我们一直强调的检查错误

    if((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        {
            //输出错误提示，并提出
            perror("socket");
            exit(1);
        }
    
    //主机字节顺序
    my_addr.sin_family = AF_INET;

    //网络字节顺序，短整型
    my_addr.sin_port = htons(MYPORT);

    //将运行程序机器的IP填入s_addr
    my_addr.sin_addr.s_addr = INADDR_ANY;
    /*将此结构的其余空间清零*/
    bzero(&(my_addr.sin_zero), 8);
    /*这里是强调的检查错误*/
    if(bind(sock_fd,(struct sockaddr *) & my_addr, sizeof(struct sockaddr)) == -1)
    {
        /*失败，退出*/
        perror("bind");
        exit(1);
    }

    if(listen(sock_fd, BACKLOG) == -1)
    {
        perror("listen");
        exit(1);
    }

    while(1)
    {
        //这里是accept主循环
        sin_size == sizeof(struct sockaddr_in);

        //这里是强调的检查错误

        if((new_fd = accept(sock_fd, (struct sockaddr*)&their_addr, &sin_size)) == -1)
        {
            perror("accept");
            continue;
        }

        //服务器给出出现链接的信息
        printf("server: got connection from %s\n", inet_ntoa(their_addr.sin_addr));
        //这里将建立一个子进程和刚刚建立的套接子进行通讯
        if(!fork())
        //这里是子进程
        if(send(new_fd, "hello word!\n",14, 0) == -1)
        {
            perror("send");
            close(new_fd);
            exit(0);
        }

        //关闭new_fd代表的这个套接子链接
        close(new_fd);
    }

    while (waitpid(-1, NULL, WNOHANG) > 0);

    return 0;
}
