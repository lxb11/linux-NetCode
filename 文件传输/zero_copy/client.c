/*************************************************************************
	> File Name: client.c
	> Author: 
	> Mail: 
	> Created Time: 2017年11月18日 星期六 09时44分06秒
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<string.h>
#include<netinet/in.h>
#include<unistd.h>

#define _PORT_ 8080
#define _NEW_FILE_NAME_ 256
#define _BUFF_SIZE_ 4096

int start_up()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(-1 == sock)
    {
        perror("socket");
        return -1;
    }
    printf("client's sock is: %d\n",sock);

    struct sockaddr_in remote;
    remote.sin_family = AF_INET;
    remote.sin_port = htons(_PORT_);
    remote.sin_addr.s_addr = inet_addr("127.0.0.1");

    if(connect(sock, (struct sockaddr*)&remote, sizeof(remote)) < 0)
    {
        perror("connect");
        return -1;
    }
    return sock;
}

int main()
{
    int sock = start_up();

   
    char new_file_name[_NEW_FILE_NAME_];
    //read from server
    char buf[_BUFF_SIZE_];
    memset(buf, '\0', _BUFF_SIZE_);
    
    ssize_t size = read(sock, new_file_name, sizeof(new_file_name));
    if(size < 0)
    {
        perror("read");

    }
    else if(0 == size)
    {
        close(sock);
        exit(1);
    }
    else if(size > 0)
    {
        printf("new file name: %s\n", new_file_name);

    }

    
    size = 0;
    char path[_NEW_FILE_NAME_] = "./client_file/";
    strcat(path, new_file_name); // 把文件保存在当前目录下的client_file文件夹里
    FILE *fd = fopen(path, "wb");
    if(NULL == fd)
    {
        perror("fopen");
        exit(1);
    }
    bzero(buf, _BUFF_SIZE_);
    while((size = read(sock, buf, sizeof(buf))) > 0) 
    {

        printf("size %ld\n", size);
        if(size < 0)
        {
            perror("read");
            exit(1);
        }
        if(fwrite(buf, sizeof(char), sizeof(buf), fd) < 0)
        {
            perror("fwrite");
            exit(1);
        }
        if(0 == strncmp(buf, "end", 3))
        {
            break;
        }
        bzero(buf, _BUFF_SIZE_);

        printf("fwrite\n");
    }
    //read(sock, buf, sizeof(buf));
    //fwrite(buf, sizeof(char), sizeof(buf), fd);
    fclose(fd);
    printf("recv file from server successful!\n");
    
    close(sock);
    return 0;
}
