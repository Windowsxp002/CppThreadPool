#include "ThreadPool.h"
#include <iostream>
#include <string.h>
#include <string>
#include <unistd.h>

ThreadPool::ThreadPool(int min, int max)
{
    do
    {
        taskQueue = new TaskQueue;
        if (taskQueue == nullptr)
        {
            std::cout << "new taskQueue 失败!" << std::endl;
            break;
        }

        threadIDs = new pthread_t[max];
        if (threadIDs == nullptr)
        {
            std::cout << "初始化线程ID失败" << std::endl;
            break;
        }
        memset(threadIDs, 0, sizeof(pthread_t) * max);

        minNum = min;
        maxNum = max;
        busyNum = 0;
        liveNum = 0;
        exitNum = 0;

        if (pthread_mutex_init(&mutexPool, NULL) != 0 ||
            pthread_cond_init(&notEmpty, NULL) != 0)
        {
            std::cout << "初始化锁失败!" << std::endl;
            break;
        }

        shutdown = false;

        // 创建线程
        pthread_create(&managerID, NULL, manager, this);
        for (size_t i = 0; i < min; ++i)
        {
            pthread_create(&threadIDs[i], NULL, worker, this);
        }

    } while (0);
    // 释放资源
    if (threadIDs)
    {
        delete[] threadIDs;
    }
    if (taskQueue)
    {
        delete taskQueue;
    }
}

ThreadPool::~ThreadPool()
{
    // 关闭线程池
    shutdown = 1;
    // 阻塞回收管理者线程
    pthread_join(managerID, NULL);
    // 唤醒阻塞的消费者线程
    for (int i = 0; i < liveNum; ++i)
    {
        pthread_cond_signal(&notEmpty);
    }
    // 释放堆内存
    if (taskQueue)
    {
        delete taskQueue;
    }
    if (threadIDs)
    {
        delete[] threadIDs;
    }

    pthread_mutex_destroy(&mutexPool);
    pthread_cond_destroy(&notEmpty);
}

void ThreadPool::addTask(Task t)
{
    if (shutdown)
    {
        return;
    }
    // 添加任务
    taskQueue->addTask(t);
    pthread_cond_signal(&notEmpty);
}

int ThreadPool::getBusyNum()
{
    pthread_mutex_lock(&mutexPool);
    int busyNum = busyNum;
    pthread_mutex_unlock(&mutexPool);
    return busyNum;
}

int ThreadPool::getAliveNum()
{
    pthread_mutex_lock(&mutexPool);
    int aliveNum = liveNum;
    pthread_mutex_unlock(&mutexPool);
    return aliveNum;
}

void *ThreadPool::worker(void *arg)
{
    ThreadPool *pool = static_cast<ThreadPool *>(arg);

    while (1)
    {
        pthread_mutex_lock(&pool->mutexPool);
        // 当前任务队列是否为空
        while (pool->taskQueue->taskNumber() == 0 && !pool->shutdown)
        {
            // 阻塞工作线程
            pthread_cond_wait(&pool->notEmpty, &pool->mutexPool);

            // 判断是不是要销毁线程
            if (pool->exitNum > 0)
            {
                pool->exitNum--;
                if (pool->liveNum > pool->minNum)
                {
                    pool->liveNum--;
                    pthread_mutex_unlock(&pool->mutexPool);
                    pool->threadExit();
                }
            }
        }

        // 判断线程池是否被关闭了
        if (pool->shutdown)
        {
            pthread_mutex_unlock(&pool->mutexPool);
            pool->threadExit();
        }

        // 从任务队列中取出一个任务
        Task task = pool->taskQueue->getTask();
        // 解锁
        pool->busyNum++;
        pthread_mutex_unlock(&pool->mutexPool);

        std::cout << "thread" << to_string(pthread_self()) << "start working..." << std::endl;

        task.m_function(task.m_arg);
        delete task.m_arg;
        task.m_arg = NULL;

        std::cout << "thread" << to_string(pthread_self()) << "end working..." << std::endl;
        pthread_mutex_lock(&pool->mutexPool);
        pool->busyNum--;
        pthread_mutex_unlock(&pool->mutexPool);
    }
    return NULL;
}

void *ThreadPool::manager(void *arg)
{
    ThreadPool *pool = static_cast<ThreadPool *>(arg);
    while (!pool->shutdown)
    {
        // 每隔3s检测一次
        sleep(3);

        // 取出线程池中任务的数量和当前线程的数量
        pthread_mutex_lock(&pool->mutexPool);
        int queueSize = pool->taskQueue->taskNumber();
        int liveNum = pool->liveNum;
        int busyNum = pool->busyNum;
        pthread_mutex_unlock(&pool->mutexPool);

        // 添加线程
        // 任务的个数>存活的线程个数 && 存活的线程数<最大线程数
        if (queueSize > liveNum && liveNum < pool->maxNum)
        {
            pthread_mutex_lock(&pool->mutexPool);
            int counter = 0;
            for (int i = 0; i < pool->maxNum && counter < NUMBER && pool->liveNum < pool->maxNum; ++i)
            {
                if (pool->threadIDs[i] == 0)
                {
                    pthread_create(&pool->threadIDs[i], NULL, worker, pool);
                    counter++;
                    pool->liveNum++;
                }
            }
            pthread_mutex_unlock(&pool->mutexPool);
        }
        // 销毁线程
        // 忙的线程*2 < 存活的线程数 && 存活的线程>最小线程数
        if (busyNum * 2 < liveNum && liveNum > pool->minNum)
        {
            pthread_mutex_lock(&pool->mutexPool);
            pool->exitNum = NUMBER;
            pthread_mutex_unlock(&pool->mutexPool);
            // 让工作的线程自杀
            for (int i = 0; i < NUMBER; ++i)
            {
                pthread_cond_signal(&pool->notEmpty);
            }
        }
    }
    return NULL;
}

void ThreadPool::threadExit()
{
    pthread_t tid = pthread_self();
    for (int i = 0; i < this->maxNum; ++i)
    {
        if (threadIDs[i] == tid)
        {
            threadIDs[i] = 0;
            std::cout << "threadExit() called " << tid << "exiting..." << std::endl;
            break;
        }
    }
    pthread_exit(NULL);
}