#pragma once
#include "TaskQueue.h"
#include <pthread.h>

struct ThreadPool
{
private:
    /* data */
    TaskQueue *taskQueue;

    pthread_t managerID;       // 管理者线程ID
    pthread_t *threadIDs;      // 工作线程ID
    int minNum;                // 最小线程数量
    int maxNum;                // 最大线程数量
    int busyNum;               // 繁忙线程数量
    int liveNum;               // 存活的线程数量
    int exitNum;               // 需要销毁的线程数量
    pthread_mutex_t mutexPool; // 锁整个线程池
    // pthread_mutex_t mutexBusy; // 锁busyNum
    pthread_cond_t notEmpty;   // 条件变量,任务队列是不是空了

    int shutdown; // 是否销毁线程池

    static const int NUMBER =2;

public:
    ThreadPool(int min, int max);

    ~ThreadPool();

    // 给线程池添加任务
    void addTask(Task t);
    // 获取线程池中工作的线程的个数
    int getBusyNum();
    // 获取线程池中活着的线程的个数
    int getAliveNum();

private:
    //////////////////////
    // 工作的线程(消费者线程)任务函数
    static void *worker(void *arg);
    // 管理者线程任务函数
    static void *manager(void *arg);
    // 单个线程退出
    void threadExit();
};
