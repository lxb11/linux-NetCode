/*************************************************************************
	> File Name: server.c
	> Author: 
	> Mail: 
	> Created Time: 2017年11月19日 星期日 09时34分51秒
 ************************************************************************/

#include"common.c"
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/epoll.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<string.h>

int main()
{
    int a[_MAX_NUM_];//存放描述符的数组
    memset(a, -1, _MAX_NUM_);
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

    //注册标准输入
    _ev.events = EPOLLIN;
    _ev.data.fd = 0;
    if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, 0, &_ev) < 0)
    {
        perror("epoll_ctl");
        close(epoll_fd);
    }

    //int timeout = 5000;//5 second
    int count = 0; //链接总数
    char buf[1024 * 5];
    int ready_num = -1;


    while(1)
    {
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

                printf("ready_num: %d\n", ready_num);
                int i = 0;
                //如果新监测到一个SOCKET用户连接到了绑定的SOCKET端口，建立新的连接
                for(i = 0; i < ready_num; i++)
                {
                    int new_sock = -1;
                    int fd = _ev_out[i].data.fd;
                    if((fd == sock) && (_ev_out[i].events & EPOLLIN))//有新的链接
                    {
                        //sleep(3);
                        new_sock = accept(fd, (struct sockaddr*)&client, &client_len);
                        if(new_sock < 0)
                        {
                            perror("accept");
                        }
                        else
                        {
                            printf("get a connect...,count:%d\n",++count);

                            for(int l = 0;l < 50; l++)//添加到数组
                            {
                                if(a[l] == -1)
                                {
                                    a[l] = new_sock;
                                    break;
                                }
                            }

                            set_nonblock(new_sock);//设置为非阻塞方式
                            _ev.events = EPOLLIN ;// EPOLLOUT;//可以读
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
                    else if(fd > 0 && fd != new_sock && _ev_out[i].events & EPOLLIN)//如果是已经连接的用户，有可读数据
                    {
                        //sleep(3);
                        memset(buf, '\0', sizeof(buf));
                        ssize_t size = read(fd, buf, sizeof(buf));
                        //if(size = read(fd, buf, sizeof(buf) < 0)  错误写法。。。。。。。。找了好久，bug
                        if(size < 0)
                        {
                            perror("read");
                            break;
                        }
                        else if(0 == size)//客户端关闭，删除其fd
                        {
                            for(int k = 0; k < count; ++k)
                            {
                                if(fd == a[k])
                                {
                                    a[k] = -1;
                                }
                                for(int m = k; m < count; ++m)
                                {
                                    a[m] = a[m + 1];//数组往前移一位
                                }
                            }
                            printf("close connect..count:%d\n",--count);
                            //delete fd
                            epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
                            close(fd);
                        }
                        else//读出来，显示在stdout
                        {
                            printf("%s\n", buf);
                        }
                    }
                    else if(0 == fd && _ev_out[i].events & EPOLLIN)//如果有数据发送
                    {
                        //sleep(2);
                        memset(buf, '\0', sizeof(buf));
                        printf("stdin is readable\n");
                        fgets(buf, sizeof(buf), stdin);
                        //write to all clients
                        for(int k = 0; k < count; k++)
                        {
                            if(a[k] > 0)
                            {
                                ssize_t size = writen(a[k], buf, strlen(buf));
                                if(size < 0)
                                {
                                    perror("writen");
                                    continue;
                                }
                                else if(0 == size)
                                {
                                    printf("empty!\n");
                                    continue;
                                }
                                else
                                {
                                    printf("write size %ld fd is %d\n", size, a[k]);
                                }
                            }
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
