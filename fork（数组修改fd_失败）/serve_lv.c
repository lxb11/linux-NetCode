/*************************************************************************
	> File Name: t_server.c
	> Author: 
	> Mail: 
	> Created Time: Sun 16 Jul 2017 09:19:34 AM CST
 ************************************************************************/

#include<stdio.h>
#include<string.h>
#include<sys/types.h>  /* See NOTES */
#include<sys/socket.h>
#include<stdlib.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<signal.h>
#define BUFFER_SIZE 1024 //消息的最大长度
//static int que[10]  = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};  //原本想用数组实现服务器链接多客户端，随意收发数据，实测行不通
//总结：fork函数实际拷贝了相关变量，父进程与子进程的数据并不是同一内存的数据，导致行不通

void mulchat(int conn, int que[])
{
    char buf[1024];
    char buf2[1024];
    pid_t pid;

    int child_id, father_id;
    for(int i = 0; i < 10; i++)
    {
        printf("%d\n",que[i]);
    }
    while(1)
    {

        pid = fork();
        if(pid == 0)
        {
            do
            {
                memset(buf,0,sizeof(buf));

                int ret = recv(conn,buf,sizeof(buf),0);
                if(ret == -1)
                {
                    perror("read");
                }
                if(ret == 0)
                {
                    printf("client close\n");
                    break;
                }
                printf("recv cli%d data[%d]:%s\n", conn,ret,buf);
                }
            while(strncmp(buf,"quit",4) != 0);
            putchar(10);
            exit(0);
        }
        else if(pid > 0)
        {
            father_id = getppid();
            do
            {
                memset(buf2,0,sizeof(buf));

                if(fgets(buf2,1024,stdin) == NULL)
                {
                    perror("fgets");
                    exit(1);
                }
                //for(int i = 0; i < 10; i++)
                //{
                    if(que[i] > 0)
                    {
                        if(send(que[i],buf2,strlen(buf2),0) == -1)
                        {
                            perror("send");
                            exit(1);
                        }

                    //}
                }
                /*if(send(4,buf2,strlen(buf2),0) == -1)
                {
                    perror("send");
                    exit(1);
                }
                if(send(5,buf2,strlen(buf2),0) == -1)
                {
                    perror("send");
                    exit(1);
                }*/
            }
            while(strncmp(buf2,"quit",4) != 0);
            putchar(10);
            if(kill(father_id,SIGKILL) == -1)
            {
                printf("error in quitt father_process");
            }
            else
            {
                printf("successd in quit father_process!");
            }
            exit(0);
        }
    }
}

int main()
{
    int listenfd;

    if((listenfd = socket(PF_INET,SOCK_STREAM,0)) < 0)
        perror("socket");

    struct sockaddr_in serveraddr;
    memset(&serveraddr,0,sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(5188);
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    //serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    //inet_aton("127.0.0.1",&serveraddr.sin_addr);

    int on = 1;

    if(setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on))< 0)//地址重复利用
       perror("setsockopt");
    if(bind(listenfd,(struct sockaddr*)&serveraddr,sizeof(serveraddr)) < 0)
       perror("bind");
    if(listen(listenfd,SOMAXCONN) < 0) //SOMAXCONN全连接队列大小
       perror("listen");

    struct sockaddr_in peeraddr;
    socklen_t peer_len = sizeof(peeraddr);

    int i = 0;
    int conn;
    pid_t pid;
    while(1)
    {

        conn = accept(listenfd,(struct sockaddr *)&peeraddr,&peer_len);

        pid = fork();
        /*if(pid == -1)
        {
            perror("fork");
        }*/
        //if(pid == 0)
        //{

            close(listenfd);
            for(i; i < 10; i++)
            {
                if(que[i] < 0)
                {
                    que[i] = conn;
                    break;
                }
            }

            mulchat(que[i], que);
            close(que[i]);
            exit(0);

        //}
        close(que[i]);        
        /*else
        {
            printf("conn = %d\n",conn);
            printf("IP = %s PORT = %d\n",inet_ntoa(peeraddr.sin_addr),ntohs(peeraddr.sin_port));
            
        }*/
    }
    return 0;
}
