#ifndef __HREAD_POOL_H__
#define __HREAD_POOL_H__

/**
 * @brief 
 * user: aloneisbestes
 * email: 910319432@qq.com
 * 该头文件主要实现线程池，用于处理 http server 的线程池
 */

#include <list>

template<class T>
class ThreadPool {
private:
    std::list<T> m_queue;       // 线程资源队列


public:
    ThreadPool()
    ~ThreadPool();

public:
    bool push(const T &value);
    bool pop(T &value)
};

#endif // __HREAD_POOL_H__
