/*************************************************************************
	> File Name: server.c
	> Author: 
	> Mail: 
	> Created Time: 2017年11月19日 星期日 09时34分51秒
 ************************************************************************/

#include<stdio.h>
#include<unistd.h>
#include<sys/epoll.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<string.h>
#include<stdlib.h>
#include<fcntl.h>

#define IPADDR "127.0.0.1"
#define PORT 8080

#define _MAX_NUM_ 64
#define _BACK_LOG_ 5

int start_up()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock == -1)
    {
        perror("socket");
        exit(1);
    }
    setnonblocking(sock);//把socket设置为非阻塞方式
    
    struct sockaddr_in local;
    local.sin_family = AF_INET;
    local.sin_port = htons(PORT);
    inet_aton(IPADDR, &local.sin_addr);

    //avoid port reuse
    int flag = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int));

    if(bind(sock, (struct sockaddr*)&local, sizeof(local)) == -1)
    {
        perror("bind");
        exit(1);
    }

    if(listen(sock, _BACK_LOG_) == -1)
    {
        perror("listen");
        exit(1);
    }
    return sock;

}

int set_nonblock(int fd)
{
    //int fontl(int fd, int cmd, .../*arg*/)
    int old_opt = fcntl(fd, F_GETFL);//read the file status flags
    if(old_opt < 0)
    {
        perror("fcntl");
        return -1;
    }
    fcntl(fd, old_opt | O_NONBLOCK);
    return old_opt;
}


int main()
{
    int sock = start_up();
    printf("server socket is: %d\n", sock);

    int epoll_fd = epoll_create(256);//生成用于处理accept的专用的文件描述符
    if(epoll_fd < 0)
    {
        perror("epoll_fd");
        exit(1);
    }
    printf("epoll_fd: %d\n", epoll_fd);

    struct sockaddr_in client;
    socklen_t client_len = sizeof(client);

    struct epoll_event _ev, _ev_out[_MAX_NUM_];//声明epoll_event结构体的变量,_ev用于注册事件,数组用于回传要处理的事件
    _ev.events = EPOLLIN;//设置要处理的类型
    _ev.data.fd = sock;//设置与要处理的事件相关的文件描述符

    //int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
    if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock, &_ev) < 0)//注册epoll事件
    {
        perror("epoll_ctl");
        close(epoll_fd);
    }


    //int timeout = 5000;//5 second
    int count = 0; //链接总数
    char buf[1024 * 5];
    int ready_num = -1;

    char user_buf[1024 * 4];

    while(1)
    {
        //int epoll_wait(int epfd, struct epoll_event * events, int maxevents, int timeout);
        switch(ready_num = epoll_wait(epoll_fd, _ev_out, _MAX_NUM_, -1))//等待epoll事件的发生
        {
            case 0://timeout
                printf("epoll timeout..\n");
                break;
            case -1://error
                perror("epoll_wait");
                break;
            default:
            {

                printf("ready_num: %d\n",ready_num);
                int i = 0;
                //如果新监测到一个SOCKET用户连接到了绑定的SOCKET端口，建立新的连接
                for(i = 0; i < ready_num; i++)
                {
                    int fd = _ev_out[i].data.fd;
                    if((fd == sock) && (_ev_out[i].events & EPOLLIN))//有新的链接
                    {
                        int new_sock = accept(fd, (struct sockaddr*)&client, &client_len);
                        if(new_sock < 0)
                        {
                            perror("accept");
                        }
                        else
                        {
                            printf("get a connect...,count:%d\n",++count);

                            set_nonblock(new_sock);//设置为非阻塞方式
                            _ev.events = EPOLLIN ;// EPOLLOUT;//可以读写
                            _ev.data.fd = new_sock;
                            //epoll的注册函数，注册_ev
                            if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_sock, &_ev) < 0)
                            {
                                perror("epoll_ctl");
                                close(new_sock);
                                continue;
                            }
                            printf("Insert new_sock success, new_sock is : %d\n", new_sock);
                            //continue;
                        }
                    }
                    if(_ev_out[i].events & EPOLLIN)//如果是已经连接的用户，有可读数据
                    {
                        memset(buf, '\0', sizeof(buf));
                        ssize_t size = read(fd, buf, sizeof(buf));
                        if(0 == size)//客户端关闭，删除其fd
                        {
                            printf("close connect..count = %d\n",--count);
                            //delete fd
                            epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
                            close(fd);
                        }
                        printf("%s\n", buf);


                        //echo to client
                        ssize_t size_write = write(fd, buf, strlen(buf));
                        if(size_write < 0)
                        {
                            perror("write");
                            break;
                        }
                        else if(size_write == strlen(buf))//全部发送，无剩余
                        {
                            printf("write size %ld  fd is %d\n", size_write, fd);
                            _ev.events &= ~EPOLLOUT;//不再关注写
                            epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &_ev);
                        }
                        else if(size_write < strlen(buf))//关注写事件，下次触发是继续发送剩余的
                        {
                            memset(user_buf, 0, sizeof(user_buf));
                            memcpy(user_buf, buf + size_write, strlen(buf) - size);
                            _ev.events = EPOLLOUT;//关注写事件
                            epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &_ev);
                        }
                    }
                    if(_ev_out[i].events & EPOLLOUT)//处理上次没发送完的数据
                    {
                        ssize_t size = write(fd, user_buf, strlen(user_buf));
                        if(size < 0)
                        {
                            perror("write");
                            break;
                        }
                        else if(size == strlen(buf))//全部发送，无剩余
                        {
                            printf("second write size %ld  fd is %d\n", size, fd);
                            _ev.events &= ~EPOLLOUT;//不再关注写
                            epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &_ev);
                        }
                        else if(size < strlen(buf))//关注写事件，下次触发是继续发送剩余的
                        {
                            memset(user_buf, 0, sizeof(user_buf));
                            memcpy(user_buf, buf + size, strlen(buf) - size);
                            _ev.events = EPOLLOUT;//关注写事件
                            epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &_ev);
                        }
                    }     
                }
            }//default
            break;
        }//switch
    }//while

    close(sock);
    close(epoll_fd);
    return 0;
}
