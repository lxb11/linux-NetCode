#define THREAD 8//线程池线程数量
#define QUEUE  32//任务队列大小

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>

#include "threadpool.c"

#define MIN_VALUE 1e-8
double redpacket = 100;//红包金额


int tasks = 0, done = 0;
pthread_mutex_t lock;

double my_rand(int s)
{
    time_t ts;
    srand((unsigned int)time(NULL));//按照时间设定随机数种子
    double a = 0;
    while(a == 0)
    {
        a = rand() % s;           //限定随机数的范围
    }

    return a + 0.1;
}




//执行任务
void dummy_task(void *arg) {
    int *p = (int*)arg;
    usleep(10000);
    pthread_mutex_lock(&lock);
    /* 记录成功完成的任务数 */
    done++;


    if(redpacket < MIN_VALUE)
    {
        printf("money is run out!\n");
    }
    else if(done == *p)
    {
        printf("抢到的红包数 = %f\n", redpacket);
    }
    else
    {
        int base = redpacket / *p;
        double my_rand_red = my_rand(1.8 * base);
        printf("抢到的红包数 = %f\n", my_rand_red);
        redpacket -= my_rand_red;
    }
    pthread_mutex_unlock(&lock);
}

int main(int argc, char **argv)
{
    threadpool_t *pool; //线程池指针

    /* 初始化互斥锁 */
    pthread_mutex_init(&lock, NULL);

    /* 断言线程池创建成功 */
    assert((pool = threadpool_create(THREAD, QUEUE, 0)) != NULL);
    fprintf(stderr, "Pool started with %d threads and "
            "queue size of %d\n", THREAD, QUEUE);


    int redpacket_num = 0;
    printf("请红包个数\n");
    scanf("%d", &redpacket_num);


    for (int i = 0; i < redpacket_num; ++i)
    {
        assert(threadpool_add(pool, &dummy_task, &redpacket_num, 0) == 0);
        pthread_mutex_lock(&lock);
        tasks++;
        pthread_mutex_unlock(&lock);
    }
    fprintf(stderr, "Added %d tasks\n", tasks);

    /* 等任务执行完，否则休眠  */
    while(tasks > done) {
        usleep(10000);
    }
    /* 这时候销毁线程池,0 代表 immediate_shutdown */
    assert(threadpool_destroy(pool, 0) == 0);
    fprintf(stderr, "Did %d tasks\n", done);

    return 0;
}
