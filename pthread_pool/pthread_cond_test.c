/*************************************************************************
	> File Name: pp.c
	> Author: 
	> Mail: 
	> Created Time: 2018年05月16日 星期三 08时55分16秒
 ************************************************************************/

#include<stdio.h>
#include<pthread.h>
#include<unistd.h>


struct msg {
    struct msg *m_next;
};

struct msg *workq;

pthread_cond_t qready = PTHREAD_COND_INITIALIZER;

pthread_mutex_t qlock = PTHREAD_MUTEX_INITIALIZER;


void process_msg(void)
{
    struct msg *mp;

    for (;;){
        pthread_mutex_lock(&qlock);
        while(workq == NULL){
            printf("hi\n");
            pthread_cond_wait(&qready, &qlock);
            printf("hello\n");
        }
        mp = workq;
        workq = mp->m_next;
        pthread_mutex_unlock(&qlock);
    }

}

void enqueue_msg(struct msg *mp)
{
    sleep(1);
    printf("msg\n");
    pthread_mutex_lock(&qlock);
    mp->m_next = workq;
    workq = mp;
    pthread_cond_signal(&qready);
    pthread_mutex_unlock(&qlock);
}

int main(void)
{

    struct msg *mp;
    mp->m_next = NULL;
    pthread_t a;
    void *arr;
    pthread_create(&a, NULL, &process_msg, NULL);
    enqueue_msg(mp);
    sleep(5);
    return 0;
}
