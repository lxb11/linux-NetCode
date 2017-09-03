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

int main()
{
    pid_t  child;
    int child_id,father_id;
    int sock;
    char sendbuff[1024];
    char recvbuff[1024];

    if((sock = socket(PF_INET,SOCK_STREAM,0)) < 0)
        perror("socket");

    struct sockaddr_in serveraddr;
    memset(&serveraddr,0,sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(5188);
    serveraddr.sin_addr.s_addr = inet_addr("127.0.0.2");
    if(connect(sock,(struct sockaddr*)&serveraddr,sizeof(serveraddr))< 0)
    {
        perror("connect");
    }

    printf("\n");
    child = fork();
    if(child == 0)
    {
        child_id = getppid();
        do
        {
            memset(sendbuff,0,sizeof(sendbuff));
            if(fgets(sendbuff,1024,stdin) == NULL)
            {
                perror("fget");
                exit(1);
            }
            if(send(sock,sendbuff,strlen(sendbuff),0) == -1)
            {
                perror("send");
                exit(1);
            }
        }while(strncmp(sendbuff,"quit",4) != 0);
        printf("\n");
        if(kill(child_id,SIGKILL) == -1)
        {
            printf("error in quit child_process!");
        }
        else
        {
            printf("Successed in quit child_process");
        }
        exit(0);
        
    }
    else if(child > 0)
    {
        do
        {
            memset(recvbuff,0,sizeof(recvbuff));

            if((recv(sock,recvbuff,1024,0)) == -1)
            {
                perror("recv");
                exit(1);
            }
            printf("recv:%s\n",recvbuff);
        }while(strncmp(recvbuff,"quit",4) != 0);
        printf("\n");
        exit(0);
    }
    else
    {
        perror("fork");
        exit(0);
    }
    close(sock);
    return 0;
}
