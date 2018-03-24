/*************************************************************************
	> File Name: msg.c
	> Author: 
	> Mail: 
	> Created Time: 2018年03月22日 星期四 22时17分41秒
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<sys/msg.h>
#include<sys/types.h>
#include<unistd.h>

//创建一个唯一的key
#define MSG_FILE "/etc/passwd"

//消息结构
struct msg_form
{
    long mtype;
    char mtext[256];
};

int main()
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

    //打开消息队列
    if((msqid = msgget(key, IPC_CREAT | 0777)) == -1)
    {
        perror("msgget error");
        exit(1);
    }

    //打印消息队列ID及进程ID
    printf("my msqid is: %d.\n", msqid);
    printf("my pid is: %d.\n",getpid());

    //添加消息，消息类型为888
    msg.mtype = 888;
    sprintf(msg.mtext, "hello, i am client %d", getpid());
    msgsnd(msqid, &msg, sizeof(msg.mtext), 0);

    //读取类型为777的消息
    msgrcv(msqid, &msg, 256, 999, 0);
    printf("client: receive msg.mtext is: %s.\n", msg.mtext);
    printf("client: receive msg.mtype is: %ld.\n", msg.mtype);
    return 0;

}
