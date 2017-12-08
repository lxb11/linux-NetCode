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
#include<errno.h>

#define _PORT_ 8080
#define _BACK_LOG_ 5
#define _MAX_FD_NUM_ 32
#define _FILE_NAME_MAX_SIZE_ 256
#define _BUFFSIZE_ 4096

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
    local.sin_port = htons(_PORT_);
    inet_aton("127.0.0.1", &local.sin_addr);

    //设置套结子选项避免地址使用错误
    int reuse = 1;
    if((setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse))) < 0)
    {
        perror("setsockopt");
        exit(1);
    }
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

//写，定长
int writen(int fd, const void *vptr, size_t n)
{
    ssize_t nleft;
    int nwriten;
    const char *ptr;
    
    ptr = (char*)vptr;
    nleft = n;
    while(nleft > 0)
    {
        nwriten = write(fd, ptr, nleft);
        if(nwriten <= 0)
        {
            if(nwriten < 0 && errno == EINTR)
            {
                nwriten = 0;//call write() again
            }
            else
            {
                return -1;//error
            }
        }
        nleft -= nwriten;
        ptr += nwriten;
    }
    return n;
}

int main()
{
    char file_name[_FILE_NAME_MAX_SIZE_];
    bzero(file_name, _FILE_NAME_MAX_SIZE_);
    printf("请输入你发送的文件名:");
    scanf("%s", file_name);
    printf("请输入要保存的新文件名:");
    char new_file_name[_FILE_NAME_MAX_SIZE_];
    scanf("%s", new_file_name);
    char buf[_BUFFSIZE_];
    bzero(buf, _BUFFSIZE_);
    printf("按回车监听\n");
    getchar();

    int sock = start_up();
    //printf("server socket is: %d\n",sock);
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
                if(nfd[0].revents & POLLIN)
                {
                    int new_sock = accept(nfd[0].fd, (struct sockaddr*)&client,&client_len);
                    //connect failed
                    if(new_sock < 0)
                    {
                        perror("accept");
                    }
                    else
                    {
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
                        printf("输入任意字符触发标准读入，使文件发送给所有客户端\n");
                    }
                }
                if(0 == nfd[1].fd && (nfd[1].revents & POLLIN) && (fgets(buf, sizeof(buf), stdin) != NULL))
                {
                    bzero(buf, _BUFFSIZE_);
                    ssize_t size = read(nfd[1].fd, buf, sizeof(buf));
                    if(size < 0)
                    {
                        printf("empty\n");
                        break;
                    }
                    else
                    {

                        //printf("stdin is readable\n");
                        //write file to all clients
                        for(int k = 2; k < _MAX_FD_NUM_; k++)
                        {
                            if(nfd[k].fd > 0 && nfd[k].revents & POLLOUT)
                            {
                                printf("%s\n",file_name);
                                FILE * fd = fopen(file_name, "rb");
                                if(NULL == fd)
                                {
                                    printf("文件 %s 没有找到!\n", file_name);
                                    int flag = -1;
                                    if(write(nfd[k].fd, &flag, sizeof(flag)))
                                    {
                                        perror("write flag error");
                                        continue;
                                    }
                                }
                                else
                                {
                                    ssize_t size = writen(nfd[k].fd, new_file_name, sizeof(new_file_name));
                                    if(size < 0)
                                    {
                                        perror("write");
                                        continue;
                                    }
                                    else
                                    {
                                        printf("新文件按名发送成功\n");
                                    }

                                }
                                bzero(buf, _BUFFSIZE_);
                                int length = 0;//每读取一段数据，便将其发送给客户端，循环直到文件读完为止  
                                while((length = fread(buf, sizeof(char), _BUFFSIZE_, fd)) > 0)
                                {
                                    printf("length: %d\n", length);

                                    if(writen(nfd[k].fd, buf, length) < 0)
                                    {
                                        perror("write error");
                                        break;
                                    }
                                    bzero(buf, _BUFFSIZE_);
                                }
                                char* flag = "end";//结束标志
                                if(writen(nfd[k].fd, flag, sizeof(flag)) < 0)//发送结束标志
                                {
                                    perror("writen");
                                }
                                fclose(fd);
                                printf("Transfer file fininshed\n");
                            }
                        }
                    }
                }
            }//default
            break;
        }//switch 
    }//while loop



    close(sock);
    return 0;
}
