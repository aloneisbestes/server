#ifndef __M_BLOCK_H__
#define __M_BLOCK_H__

/**
 * @brief 
 * 实现阻塞队列，用于线程异步线程
 * user: aloneisbestes
 * email: 910319432qq.com
 */

#include <list>
#include "mmacro.h"
#include "locker.h"

template<class T> 
class BlockQueue{
private:
    T   *m_block;
    int m_size;         // 当前队列的个数
    int m_size_max;     // 队列的总个数
    int m_front;
    int m_back;

    // 条件变量和互斥锁
    Cond    m_cond;
    Locker  m_mutex;

public:
    BlockQueue(int size=BLOCK_QUEUE_SIZE) {
        m_back = -1;
        m_front = -1;
        m_size = 0;
        m_size_max = size;
        m_block = new T[m_size_max];
    }

    ~BlockQueue() {
        if (m_block) delete [] m_block;
    }

public:
    // 清空队列
    void clear() {
        m_mutex.locker();
        m_back = -1;
        m_front = -1;
        m_size = 0;
        m_mutex.unlocker();
    }

    // 队列是否为空
    bool isempty() {
        m_mutex.locker();
        if (m_size == 0) {
            m_mutex.unlocker();
            return true;
        }
        m_mutex.unlocker();
        return false;
    }

    // 获取队列当前长度
    int size() {
        m_mutex.locker();
        int size = m_size;
        m_mutex.unlocker();
        return size;
    }

    // 队列是否为满
    bool isfull() {
        m_mutex.locker();
        if (m_size == m_size_max) {
            m_mutex.unlocker();
            return true;
        }
        m_mutex.unlocker();
        return false;
    }

    // 返回队列的最大长度
    int sizemax() {
        m_mutex.locker();
        int sizemax = m_size_max;
        m_mutex.unlocker();
        return sizemax;
    }

    // 返回队首元素
    bool front(T &value) {
        m_mutex.locker();

        if (isempty()) {
            m_mutex.unlocker();
            return false;
        }

        value = m_block[m_front];
        m_mutex.unlocker();
        return true;
    }

    // 返回队尾元素
    bool back(T &value) {
        m_mutex.locker();

        if (isempty()) {
            m_mutex.unlocker();
            return false;
        }

        value = m_block[m_back];
        m_mutex.unlocker();
        return true;
    }

    // 出队
    bool push(const T &value) {
        m_mutex.locker();
        if (m_size == m_size_max) { // 判断阻塞队列是否为满
            m_cond.broadcast();
            m_mutex.unlocker();
            return false;
        }

        m_back = (m_back + 1) % m_size_max;
        m_block[m_back] = value;
        ++m_size;

        // 发送条件成立
        m_cond.broadcast();
        m_mutex.unlocker();
        return true;
    }

    // 出队
    bool pop(T &value) {
        m_mutex.locker();
        while (m_size <= 0) {
            if(!m_cond.wait(m_mutex.getmutex())) {
                m_mutex.unlocker();
                return false;
            }
        }

        // 出队
        m_front = (m_front + 1) % m_size_max;
        value = m_block[m_front];
        --m_size;

        m_mutex.unlocker();
        return true;
    }
};

#endif // __M_BLOCK_H__