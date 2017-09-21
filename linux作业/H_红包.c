/*************************************************************************
	> File Name: H_红包.c
	> Author: 
	> Mail: 
	> Created Time: 2017年09月19日 星期二 21时38分27秒
 ************************************************************************/

#include<stdio.h>
#include<pthread.h>
#include<unistd.h>
#include<sys/time.h>
#include<string.h>
#include<time.h>
#include<stdlib.h>


#define SB main()
int redpack = 20;
int N = 10;
pthread_t thread[3];
pthread_mutex_t mut;
int T = 1;
int count = 0;
int who  = 0;


int my_rand(int s, void * arg)
{
    int *p = (int *)arg;
    time_t ts;
    srand((unsigned int)time(&ts) + *p);//按照时间设定随机数种子
    int a = rand() % s;           //限定随机数的范围
    return a;
}


void *thread_0(void * agr)
{
    int *p = (int *)agr;
    
    //printf("thread_0: waiting\n");
    int num = 0;

    pthread_mutex_lock(&mut);
    if(N = 1)
    {

        num = 20 - count;
        for(int i = 0; i < 10; i++)
        {
            if(agr != thread[i])
                pthread_cancel(thread[i]);
        }
    }
    else
    {
        count += num = my_rand(3, p);
        N--;
    }
    printf("%d抢到的钱数%d\n",who++,num);
    pthread_mutex_unlock(&mut);
    pthread_exit(NULL);
}

void thread_wait(void)
{
    //等待线程结束
    for(int i = 0; i < 10; i++)
    {
        if(thread[i] != 0)
        {
            pthread_join(thread[i], NULL);
            //printf("线程%d已结束\n",i);
        }
    }
}
void thread_create(void)
{
    int temp;
    memset(&thread, 0, sizeof(thread));
    for(int i = 0; i < 10; i++)
    {


        if((temp = pthread_create(&thread[i], NULL, thread_0, &thread[i])) != 0)
        {
            //printf("线程%d创建失败\n",i);
        }
        else
        {
            //printf("线程%d被创建\n",i);
        }

    }
}



int SB
{
    
    pthread_mutex_init(&mut, NULL);
    printf("我是主函数，我正在创建线程\n");
    thread_create();
    printf("我是主函数，我正在等待线程完成任务\n");
    thread_wait();


    return 0;
}
