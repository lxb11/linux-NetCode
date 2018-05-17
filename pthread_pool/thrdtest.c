#define THREAD 8
#define QUEUE  32

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>

#include "threadpool.c"

int tasks = 0, done = 0;
pthread_mutex_t lock;

//执行任务
void dummy_task(void *arg) {
    usleep(10000);
    pthread_mutex_lock(&lock);
    /* 记录成功完成的任务数 */
    done++;
    pthread_mutex_unlock(&lock);
}

int main(int argc, char **argv)
{
    threadpool_t *pool;

    /* 初始化互斥锁 */
    pthread_mutex_init(&lock, NULL);

    /* 断言线程池创建成功 */
    assert((pool = threadpool_create(THREAD, QUEUE, 0)) != NULL);
    fprintf(stderr, "Pool started with %d threads and "
            "queue size of %d\n", THREAD, QUEUE);

    /* 只要任务队列还没满，就一直添加 */
    /*while(threadpool_add(pool, &dummy_task, NULL, 0) == 0) {
        pthread_mutex_lock(&lock);
        tasks++;
        printf("tasks = %d\n", tasks);
        pthread_mutex_unlock(&lock);
    }*/
    for (int i = 0; i < 32; ++i)
    {
        assert(threadpool_add(pool, &dummy_task, NULL, 0) == 0);
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
