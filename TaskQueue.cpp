#include "TaskQueue.h"

TaskQueue::TaskQueue(/* args */)
{
    pthread_mutex_init(&m_mutex, NULL);
}

TaskQueue::~TaskQueue()
{
    pthread_mutex_destroy(&m_mutex);
}

void TaskQueue::addTask(Task task)
{
    pthread_mutex_lock(&m_mutex);
    m_taskQueue.push(task);
    pthread_mutex_unlock(&m_mutex);
}

void TaskQueue::addTask(void (*callback)(void *arg), void *arg)
{
    Task t(callback, arg);
    pthread_mutex_lock(&m_mutex);
    m_taskQueue.push(t);
    pthread_mutex_unlock(&m_mutex);
}

Task TaskQueue::getTask()
{
    Task t;
    pthread_mutex_lock(&m_mutex);

    if (m_taskQueue.empty())
    {
        return Task();
    }
    t = m_taskQueue.front();
    m_taskQueue.pop();

    pthread_mutex_unlock(&m_mutex);
    return t;
}