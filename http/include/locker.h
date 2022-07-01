#ifndef __LOCKER_H__
#define __LOCKER_H__

#include <pthread.h>
#include <semaphore.h>
#include "mexception.h"

/**
 * @brief 
 * 互斥锁、条件变量、信号量的封装
 * user: aloneisbestes
 * email: 910319432@qq.com
 */

namespace http {
    class Locker;
    class Cond;
    class Sem;
}

/**
 * @brief 
 * 互斥锁的封装
 */
class Locker {
private:
    pthread_mutex_t m_mutex;

public:
    // 构造和析构
    Locker() {
        pthread_mutex_init(&m_mutex, nullptr);
    }
    ~Locker() {
        pthread_mutex_destroy(&m_mutex);
    }

    // 加锁
    bool locker() {
        return pthread_mutex_lock(&m_mutex) == 0 ? true : false;
    }

    bool trylocker() {
        return pthread_mutex_trylock(&m_mutex) == 0 ? true : false;
    }

    // 解锁
    bool unlocker() {
        return pthread_mutex_unlock(&m_mutex) == 0 ? true : false;
    }

    // 获取锁
    pthread_mutex_t *getmutex() { 
        return &m_mutex;
    }
};

/**
 * @brief 
 * 条件变量的封装
 */
class Cond {
private:
    pthread_cond_t m_cond;

public:
    // 构造与析构
    Cond() {
        if (pthread_cond_init(&m_cond, nullptr) != 0) {
            throw HttpException("Condition variable initialization failed");
        }
    }
    ~Cond() {
        pthread_cond_destroy(&m_cond);
    }

public:
    // 唤醒所有线程
    bool broadcast() {
        return pthread_cond_broadcast(&m_cond) == 0 ? true : false;
    }

    // 至少唤醒一个线程
    bool signal() {
        return pthread_cond_signal(&m_cond) == 0 ? true : false;
    }

    // 等待条件变量到达
    bool wait(pthread_mutex_t *mutex) {
        return pthread_cond_wait(&m_cond, mutex) == 0 ? true : false;
    }

    // 带有超时时间的等待条件变量到达
    bool timedwait(pthread_mutex_t *mutex, const timespec *t) {
        return pthread_cond_timedwait(&m_cond, mutex, t) == 0 ? true : false;
    }
};

/**
 * @brief 
 * 信号量的封装
 */
class Sem {
private:
    sem_t m_sem;

public:
    // 构造与析构
    Sem(int n=10) {
        if (sem_init(&m_sem, 0, n) != 0) {
            throw HttpException("Failed to initialize semaphore");
        }
    }
    ~Sem() {
        sem_destroy(&m_sem);
    }

public:
    // 信号量减一
    bool wait() {
        return sem_wait(&m_sem) == 0 ? true : false;
    }
    bool trywait() {
        return sem_trywait(&m_sem) == 0 ? true : false;
    }

    // 信号量加1
    bool post() {
        return sem_post(&m_sem) == 0 ? true : false;
    }
};

#endif // __LOCKER_H__