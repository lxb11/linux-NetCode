/*************************************************************************
	> File Name: sema.c
	> Author: 
	> Mail: 
	> Created Time: 2018年03月24日 星期六 09时22分55秒
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<sys/sem.h>
#include<unistd.h>

/*信号量（semaphore）与已经介绍过的 IPC结构不同，它是一个计数器。
 信号量用于实现进程间的互斥与同步，而不是用于存储进程间通信数据
#include <sys/sem.h>
// 创建或获取一个信号量组：若成功返回信号量集ID，失败返回-1
int semget(key_t key, int num_sems, int sem_flags);
// 对信号量组进行操作，改变信号量的值：成功返回0，失败返回-1
int semop(int semid, struct sembuf semoparray[], size_t numops);  
// 控制信号量的相关信息
int semctl(int semid, int sem_num, int cmd, ...);

当semget创建新的信号量集合时，必须指定集合中信号量的个数（即num_sems），通常为1； 如果是引用一个现有的集合，则将num_sems指定为 0 。
在semop函数中，sembuf结构的定义如下：

struct sembuf 
{
    short sem_num; // 信号量组中对应的序号，0～sem_nums-1
    short sem_op;  // 信号量值在一次操作中的改变量
    short sem_flg; // IPC_NOWAIT, SEM_UNDO
}
*/

//联合体，用于semctl初始化
union semun
{
    int val;//for setval
    struct semid_ds *buf;
    unsigned short *array;
};

//初始化信号量
int init_sem(int sem_id, int value)
{
    union semun tmp;
    tmp.val = value;
    if(semctl(sem_id, 0, SETVAL, tmp) == -1)
    {
        perror("Init semaphore error");
        return -1;
    }
    return 0;
}


//P操作：
//若信号量值为1,获取资源并将信号量值-1
//若信号量值为0;进程呢个挂起等待

int sem_p(int sem_id)
{
    struct sembuf sbuf;
    sbuf.sem_num = 0;//序号
    sbuf.sem_op = -1;//P操作
    sbuf.sem_flg = SEM_UNDO;

    if(semop(sem_id, &sbuf, 1) == -1)
    {
        perror("P operation error");
        return -1;
    }
    return 0;
}

//V操作
//释放资源并将信号量值+1
//如果有进程中呢个在挂起等待，则唤醒他们

int sem_v(int sem_id)
{
    struct sembuf sbuf;
    sbuf.sem_num = 0;//序号
    sbuf.sem_op = 1;//V操作
    sbuf.sem_flg = SEM_UNDO;
    
    if(semop(sem_id, &sbuf, 1) == -1)
    {
        perror("V operation error");
        return -1;
    }
    return 0;
}

//删除信号量集
int del_sem(int sem_id)
{
    union semun tmp;
    if(semctl(sem_id, 0, IPC_RMID, tmp) == -1)
    {
        perror("delete semaphore error");
        return -1;
    }
    return 0;
}

int main(void)
{
    int sem_id;//信号量集ID
    key_t key;
    pid_t pid;


    //获取key值
    if((key = ftok(".", 'z')) < 0)
    {
        perror("ftok error");
        exit(1);
    }
    //创建信号量，其中只有一个信号量
    if((sem_id = semget(key, 1, IPC_CREAT|0666)) == -1)
    {
        perror("semget error");
        exit(1);
    }

    //初始化，处置设为0资源被占用
    init_sem(sem_id, 0);

    if((pid = fork()) == -1)
    {
        perror("fork error");
    }
    else if(pid == 0)//子进程
    {
        sleep(2);
        printf("process child: pid = %d\n", getpid());
        sem_v(sem_id); //释放资源
    }
    else //父进程
    {
        sem_p(sem_id);//等待资源
        printf("process father: pid = %d\n", getpid());
        sem_v(sem_id);//释放资源
        del_sem(sem_id);//删除信号量集
    }


    return 0;
}

