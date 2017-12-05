/*************************************************************************
	> File Name: common.c
	> Author: 
	> Mail: 
	> Created Time: 2017年12月03日 星期日 21时26分59秒
 ************************************************************************/

#include"common.h"
#include<stdio.h>
#include<unistd.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<fcntl.h>
#include<errno.h>

int set_nonblock(int fd)//把fd设置为非阻塞方式
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

int start_up()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock == -1)
    {
        perror("socket");
        exit(1);
    }
    set_nonblock(sock);//把socket设置为非阻塞方式
    
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

//读，定长
ssize_t readn(int fd, void *vptr, ssize_t n)
{
    size_t nleft;
    ssize_t nread;
    char *ptr;

    ptr = (char*)vptr;
    nleft = n;
    while(nleft > 0)
    {
        nread = read(fd, ptr, nleft);
        if(nread < 0)
        {
            if(errno == EINTR)
            {
                nread = 0;//call read() again
            }
            else
            {
                return -1;//maybe errno == EAGAIN
            }
        }
        else if(0 == nread)
        {
            break;//EOF
        }
        nleft -= nread;
        ptr += nread;
    }
    return(n - nleft);//return >= 0
}

//写，定长
ssize_t writen(int fd, const void *vptr, size_t n)
{
    size_t nleft;
    ssize_t nwriten;
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
