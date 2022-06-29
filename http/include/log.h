#ifndef __LOG_H__
#define __LOG_H__

/**
 * @brief 
 * 该头文件主要是log日志文件处理
 * Log 日志文件采用单例模式
 */

namespace http {
    class Log;
}

class Log {
private:
    // 单例模式隐藏构造和析构
    Log() {}
    virtual ~Log() {}

public:
    // 单例模式
    static Log *get() {
        static Log log;
        return &log;
    }

    // 线程处理函数
    static void *logThread(void *arg) {
        // 调用内部私有线程处理函数
        
        return nullptr;
    }

};

#endif // __LOG_H__