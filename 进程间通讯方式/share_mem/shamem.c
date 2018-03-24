/*************************************************************************
	> File Name: shamem.c
	> Author: 
	> Mail: 
	> Created Time: 2018年03月24日 星期六 15时42分08秒
 ************************************************************************/

/*
// 创建或获取一个共享内存：成功返回共享内存ID，失败返回-1
int shmget(key_t key, size_t size, int flag);
// 连接共享内存到当前进程的地址空间：成功返回指向共享内存的指针，失败返回-1
void *shmat(int shm_id, const void *addr, int flag);
// 断开与共享内存的连接：成功返回0，失败返回-1
int shmdt(void *addr); 
// 控制共享内存的相关信息：成功返回0，失败返回-1
int shmctl(int shm_id, int cmd, struct shmid_ds *buf);

当用shmget函数创建一段共享内存时，必须指定其 size；
而如果引用一个已存在的共享内存，则将 size 指定为0 。
当一段共享内存被创建以后，它并不能被任何进程访问。
必须使用shmat函数连接该共享内存到当前进程的地址空间，
连接成功后把共享内存区对象映射到调用进程的地址空间，随后可像本地空间一样访问。
shmdt函数是用来断开shmat建立的连接的。注意，这并不是从系统中删除该共享内存，
只是当前进程不能再访问该共享内存而已。
shmctl函数可以对共享内存执行多种操作，根据参数 cmd 执行相应的操作。
常用的是IPC_RMID（从系统中删除该共享内存）。
*/

#include<stdio.h>
#include<stdlib.h>
#include<sys/shm.h>  // shared memory
#include<sys/sem.h>  // semaphore
#include<sys/msg.h>  // message queue
#include<string.h>   // memcpy

// 消息队列结构
struct msg_form {
    long mtype;
    char mtext;
};

// 联合体，用于semctl初始化
union semun
{
    int              val; /*for SETVAL*/
    struct semid_ds *buf;
    unsigned short  *array;
};

// 初始化信号量
int init_sem(int sem_id, int value)
{
    union semun tmp;
    tmp.val = value;
    if(semctl(sem_id, 0, SETVAL, tmp) == -1)
    {
        perror("Init Semaphore Error");
        return -1;
    }
    return 0;
}

// P操作:
    //  若信号量值为1，获取资源并将信号量值-1 
    //  若信号量值为0，进程挂起等待
    int sem_p(int sem_id)
    {
        struct sembuf sbuf;
        sbuf.sem_num = 0; /*序号*/
        sbuf.sem_op = -1; /*P操作*/
        sbuf.sem_flg = SEM_UNDO;

        if(semop(sem_id, &sbuf, 1) == -1)
        {
            perror("P operation Error");
            return -1;
        }
        return 0;
    }

// V操作：
//  释放资源并将信号量值+1
//  如果有进程正在挂起等待，则唤醒它们
int sem_v(int sem_id)
{
    struct sembuf sbuf;
    sbuf.sem_num = 0; /*序号*/
    sbuf.sem_op = 1;  /*V操作*/
    sbuf.sem_flg = SEM_UNDO;

    if(semop(sem_id, &sbuf, 1) == -1)
    {
        perror("V operation Error");
        return -1;
    }
    return 0;
}

// 删除信号量集
int del_sem(int sem_id)
{
    union semun tmp;
    if(semctl(sem_id, 0, IPC_RMID, tmp) == -1)
    {
        perror("Delete Semaphore Error");
        return -1;
    }
    return 0;
}

// 创建一个信号量集
int creat_sem(key_t key)
{
    int sem_id;
    if((sem_id = semget(key, 1, IPC_CREAT|0666)) == -1)
    {
        perror("semget error");
        exit(-1);
    }
    init_sem(sem_id, 1);  /*初值设为1资源未占用*/
    return sem_id;
}


int main()
{
    key_t key;
    int shmid, semid, msqid;
    char *shm;
    char data[] = "this is server";
    struct shmid_ds buf1;  /*用于删除共享内存*/
    struct msqid_ds buf2;  /*用于删除消息队列*/
    struct msg_form msg;  /*消息队列用于通知对方更新了共享内存*/

    // 获取key值
    if((key = ftok(".", 'z')) < 0)
    {
        perror("ftok error");
        exit(1);
    }

    // 创建共享内存
    if((shmid = shmget(key, 1024, IPC_CREAT|0666)) == -1)
    {
        perror("Create Shared Memory Error");
        exit(1);
    }

    // 连接共享内存
    if((shm = (char *)shmat(shmid, 0, 0)) < 0)
    {
        perror("Attach Shared Memory Error");
        exit(1);
    }


    // 创建消息队列
    if ((msqid = msgget(key, IPC_CREAT|0777)) == -1)
    {
        perror("msgget error");
        exit(1);
    }

    // 创建信号量
    semid = creat_sem(key);

    // 读数据
    while(1)
    {
        msgrcv(msqid, &msg, 1, 888, 0); /*读取类型为888的消息*/
        if(msg.mtext == 'q')  /*quit - 跳出循环*/ 
        {
            break;
        }
        if(msg.mtext == 'r')  /*read - 读共享内存*/
        {
            sem_p(semid);
            printf("%s\n",shm);
            sem_v(semid);
        }
    }

    // 断开连接
    if(shmdt(shm) < 0)
    {
        perror("shumdt error");
        exit(1);
    }
    
    /*删除共享内存、消息队列、信号量*/
    shmctl(shmid, IPC_RMID, &buf1);
    msgctl(msqid, IPC_RMID, &buf2);
    del_sem(semid);
    return 0;
}
