/*************************************************************************
	> File Name: server.c
	> Author: 
	> Mail: 
	> Created Time: 一 11/24 21:02:22 2016
 ************************************************************************/

#include<stdio.h>
#include<sys/select.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<string.h>
#include<netinet/in.h>
#include<unistd.h>
#include<stdlib.h>

int main()
{
	int read_fd = 0;
	int write_fd = 1;
	int max_fd = 0;
	fd_set read_set;
	fd_set write_set;

	printf("sizeof fd_set: %lu\n",sizeof(fd_set));

	int sock = socket(AF_INET,SOCK_STREAM,0);
	if(sock == -1)
	{
		perror("socket");
		return -1;
	}
	printf("client's sock is : %d\n",sock);

	struct sockaddr_in remote;
	remote.sin_family = AF_INET;
	remote.sin_port = htons(8888);
	remote.sin_addr.s_addr = inet_addr("127.0.0.1");

	if(connect(sock,(struct sockaddr*)&remote,sizeof(remote)) < 0)
	{
		perror("connect");
		return -1;
	}

	max_fd = sock > read_fd ? sock : read_fd;

	while(1)
	{
		FD_ZERO(&read_set);
		FD_ZERO(&write_set);
        FD_SET(sock,&read_set);
		FD_SET(read_fd,&read_set);
		FD_SET(sock,&write_set);

		struct timeval timeout = {2,0};
		switch(select(max_fd + 1,&read_set,&write_set,NULL,&timeout))
		{
			case 0://timeout
				printf("select timeout\n");
				break;
			case -1://error
				perror("select");
				break;
			default://correct
				{
					//  int  FD_ISSET(int fd, fd_set *set);
                    if(FD_ISSET(read_fd,&read_set))
					{
						char buf[1024];
						memset(buf,'\0',sizeof(buf));
						ssize_t size = read(read_fd,buf,sizeof(buf) - 1);
						if(size < 0)
						{
							perror("read");
							break;
						}
						buf[size] = '\0';
						if(FD_ISSET(sock,&write_set))
						{
							ssize_t write_size = write(sock,buf,strlen(buf));
							if(write_size < 0)
							{
								perror("write");
								break;
							}
						}
                    }
                    /*if(FD_ISSET(sock,&write_set))
                    {
                        char buf[1024];
                        memset(buf,'\0',sizeof(buf));
                        size_t size = write(sock,buf,strlen(buf));
                        if(size < 0)
                        {
                            perror("write");
                            break;
                        }
                    }*/
                    if(FD_ISSET(sock,&read_set))
                    {
                        printf("sock is readable\n");
                        char buffer[1024];
                        memset(buffer,'\0',1024);
                        size_t read_size=read(sock,buffer,1024);
                        if(read_size < 0)
                        {
                            perror("read");
                            exit(0);
                        }
                        else if(read_size > 0)
                        {

                            printf("recv size %lu buffer is : %s\n",read_size,buffer);    
                        }
                    }
				}
				break;
		}
	}
	close(sock);
	return 0;
}
