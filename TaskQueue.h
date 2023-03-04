#pragma once
#include<queue>
#include <pthread.h>

using namespace std;

//任务数据结构体
struct Task
{
    Task(){
        m_function = nullptr;
        m_arg = nullptr;
    }
    Task(void (*callback) (void* arg) , void* arg){
        m_function = callback;
        m_arg = arg;
    }
    /* data */
    void (*m_function) (void* arg); //任务回调函数
    void* m_arg;
};

/******************************************************************/
class TaskQueue
{
private:
    /* data */
    queue<Task> m_taskQueue;
    pthread_mutex_t m_mutex; //操作队列时的锁
public:
    TaskQueue(/* args */);
    ~TaskQueue();

    // 添加任务
    void addTask(Task task);
    void addTask(void (*callback) (void* arg) ,void* arg);
    // 获取一个任务
    Task getTask();
    // 获取任务个数
    inline int taskNumber()
    {
        return m_taskQueue.size();
    }

};

