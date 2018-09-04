/*************************************************************************
	> File Name: ipc.c
	> Author: 
	> Mail: 
	> Created Time: 2018年03月21日 星期三 20时33分02秒
 ************************************************************************/

#include<stdio.h>
#include<unistd.h>
#include<sys/stat.h>
#include<stdlib.h>
#include<fcntl.h>
#include<time.h>
#include<errno.h>
#include<string.h>
#include<sys/msg.h>

/*1.管道*/

int pipe_test()
{
    int fd[2];//两个文件描述符
    pid_t pid;
    char buff[20];

    if(pipe(fd) < 0) //创建管道
    //int pipe(int fd[2]);   成功返回0, 失败返回-1,
    //当一个管道建立时，它会创建两个文件描述符：fd[0]为读而打开，fd[1]为写而打开      
    {
        printf("Create Pipe Error!\n");
    }
    if((pid = fork()) < 0)//创建子进程
    {
        printf("Fork Error!\n");
    }
    else if(pid > 0)//父进程
    {
        close(fd[0]); //关闭读端
        write(fd[1], "hello world\n",12);
    }
    else
    {
        close(fd[1]);//关闭写端
        read(fd[0], buff, 20);
        printf("%s", buff);
    }
    return 0;
}

/*2.FIFO 命名管道，它是一种文件类型
1、特点
FIFO可以在无关的进程之间交换数据，与无名管道不同。
FIFO有路径名与之相关联，它以一种特殊设备文件形式存在于文件系统中。
2、原型
1 #include <sys/stat.h>
2 // 返回值：成功返回0，出错返回-1
3 int mkfifo(const char *pathname, mode_t mode);*/


int fifo_owrite()
{
    int fd;
    int n, i;
    char buf[1024];
    time_t tp;

    printf("i am %d process.\n", getpid());//说明进程ID

    if((fd = open("fifo1", O_WRONLY)) < 0) //以写打开一个FIFO
    {
        perror("open fifo failed\n");
        exit(1);
    }

    for(i = 0; i < 10; ++i)
    {
        time(&tp); //取系统当前时间
        n = sprintf(buf, "Process %d's time is %s",getpid(), ctime(&tp));
        printf("send message: %s", buf); //打印
        if(write(fd, buf, strlen(buf)) < 0)//写到FIFO中
        {
            perror("write FIFO filed!");
            close(fd);
            exit(1);
        }
        sleep(1);
    }
    close(fd);//关闭fifo文件
    return 0;
}

/* #include <sys/msg.h>
// 创建或打开消息队列：成功返回队列ID，失败返回-1
int msgget(key_t key, int flag);
// 添加消息：成功返回0，失败返回-1
int msgsnd(int msqid, const void *ptr, size_t size, int flag);
// 读取消息：成功返回消息数据的长度，失败返回-1
int msgrcv(int msqid, void *ptr, size_t size, long type,int flag);
// 控制消息队列：成功返回0，失败返回-1
int msgctl(int msqid, int cmd, struct msqid_ds *buf);*/

//用于创建一个唯一的key
#define MSG_FILE "/etc/passwd" 

//消息结构
struct msg_form
{
    long mtype;
    char mtext[256];
};

int msg()
{
    int msqid;
    key_t key;
    struct msg_form msg;
    
    //获取key值
    if((key = ftok(MSG_FILE, 'z')) < 0)
    {
        perror("ftok error");
        exit(1);
    }

    //打印key值
    printf("message queue - server key is: %d.\n", key);


    // 创建消息队列
    if ((msqid = msgget(key, IPC_CREAT|0777)) == -1)
    {
        perror("msgget error");
        exit(1);
    }

    // 打印消息队列ID及进程ID
    printf("My msqid is: %d.\n", msqid);
    printf("My pid is: %d.\n", getpid());

    // 循环读取消息
    for(;;) 
    {
        msgrcv(msqid, &msg, 256, 888, 0);// 返回类型为888的第一个消息
        printf("Server: receive msg.mtext is: %s.\n", msg.mtext);
        printf("Server: receive msg.mtype is: %ld.\n", msg.mtype);

        msg.mtype = 999; // 客户端接收的消息类型
        sprintf(msg.mtext, "hello, I'm server %d", getpid());
        msgsnd(msqid, &msg, sizeof(msg.mtext), 0);
    }
    return 0;
}




int main(void)
{

    msg();
    return 0;
}
