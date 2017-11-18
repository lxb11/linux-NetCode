/*************************************************************************
	> File Name: server.c
	> Author: 
	> Mail: 
	> Created Time: 2017年11月14日 星期二 11时25分49秒
 ************************************************************************/

#include<stdio.h>
#include<poll.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>

#define _BACK_LOG_ 5
#define _MAX_FD_NUM_ 32

int start_up()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock == -1)
    { 
        perror("sock");
        exit(1);
    }
    struct sockaddr_in local;
    local.sin_family = AF_INET;
    local.sin_port = htons(8080);
    inet_aton("127.0.0.1", &local.sin_addr);

    if(bind(sock, (struct sockaddr*)&local, sizeof(local)) == -1)
    {
        perror("bind");
    }

    if(listen(sock, _BACK_LOG_) == -1)
    {
        perror("listen");
        exit(1);
    }
    return sock;
}

int main()
{
    int sock = start_up();
    printf("server socket is: %d\n",sock);
    struct sockaddr_in client;
    socklen_t client_len = sizeof(client);

    struct pollfd nfd[_MAX_FD_NUM_];
    //add fd
    nfd[0].fd = sock;
    nfd[0].events = POLLIN;
    //init struct pollfd
    int i = 0;
    int j = 0;
    for(i = 2; i < _MAX_FD_NUM_; i++)
    {
        nfd[i].fd = -1;
    }
    nfd[1].fd = 0;//stdin
    nfd[1].events = POLLIN;

    int maxi = 0;

    while(1)
    {
        switch(poll(nfd, maxi + 1, -1))
        {
            case 0://timeout
                printf("server timeout\n");
                break;
            case -1://error
                perror("select");
                break;
            default:
            {
                //accept client's connect
                //else if((nfd[i].fd == sock) && (nfd[i].revents & POLLIN))
                if(nfd[0].revents & POLLIN)
                {
                    int new_sock = accept(nfd[0].fd, (struct sockaddr*)&client,&client_len);
                    //connect failed
                    if(new_sock < 0)
                    {
                        perror("accept");
                    }
                    printf("Get a connect...\n");
                    //insert new_fd
                    for(j = 2; j < _MAX_FD_NUM_; j++)
                    {
                        if(nfd[j].fd == -1)
                        {
                            nfd[j].fd = new_sock;
                            nfd[j].events = POLLIN | POLLOUT;
                            break;//setting new_sock, then exit
                        }
                        //maxi++;
                    }
                    if(j > maxi)
                    {
                        maxi = j;
                    }
                    if(j == _MAX_FD_NUM_)
                    {
                        printf("socket array is full\n");
                        close(new_sock);
                        continue;
                    }
                    printf("insert new_sock is: %d\n",new_sock);
                }
                //write to all clientt
                if(0 == nfd[1].fd && (nfd[1].revents & POLLIN))
                {
                    char buf[1024];
                    memset(buf,'\0', sizeof(buf));
                    ssize_t size = read(nfd[1].fd, buf, sizeof(buf) - 1);
                    if(size < 0)
                    {
                        printf("empty\n");
                        break;
                    }
                    else
                    {
                        printf("stdin is readable\n");
                        //write to all client
                        for(int k = 2; k < _MAX_FD_NUM_; k++)
                        {
                            if(nfd[k].fd > 0 && nfd[k].revents & POLLOUT)
                            {
                                ssize_t size = write(nfd[k].fd, buf, strlen(buf));
                                if(size < 0)
                                {
                                    perror("write"); 
                                    break;
                                }
                                else if(size > 0)
                                {
                                    printf("write size %ld fd is %d\n", size, nfd[k].fd);
                                }
                            }
                        }
                    }
                }
                for(i = 2; i < _MAX_FD_NUM_; i++)
                {
                    if(nfd[i].fd < 0)
                    {
                        continue;
                    }
                    //check all socket for data
                    if(nfd[i].fd > 0 && (nfd[i].revents & POLLIN))
                    {
                        char buf[1024];
                        memset(buf, '\0', sizeof(buf));
                        ssize_t size = read(nfd[i].fd, buf, sizeof(buf) - 1);
                        if(0 == size)
                        {
                            printf("client release\n");
                            close(nfd[i].fd);
                            nfd[i].fd = -1;
                            maxi--;
                        }
                        else if(size > 0)
                        {
                            buf[size] = '\0';
                            printf("client %d: %s", nfd[i].fd, buf);
                        }
                    }

                }//default_for
            }//default
            break;
        }//switch 
    }//while loop



    close(sock);
    return 0;
}
