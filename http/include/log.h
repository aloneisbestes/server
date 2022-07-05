#ifndef __LOG_H__
#define __LOG_H__

/**
 * @brief 
 * 该头文件主要是log日志文件处理
 * Log 日志文件采用单例模式
 */

#include <string>
#include <fstream>
#include "locker.h"
#include "mmacro.h"
#include "mblock.h"


class Log {
private:
    bool m_isasync;     // 是否开启异步线程
    bool m_isclose;     // 是否开启log日志

    int m_buff_size;    // log buff 的长度
    int m_count;        // log 文件当前行数
    int m_max_line;     // log 文件的最大行数
    int m_file_count;   // log 当前使用的文件个数
    int m_now_day;      // 记录当前的天数
    
    char *m_buff;   // log buff
    std::string m_log_path;         // log 日志文件路径
    std::string m_log_name;         // log 日志文件名
    std::string m_tmp_log_name;     // 临时的日志文件名

    std::ofstream m_fp;     // log 日志文件指针

    pthread_t m_tids;        // 线程 id 
    // int m_tids_size;          // 线程数组的大小

    BlockQueue<std::string> *m_block;    // 阻塞队列
    Locker m_mutex;     // 互斥锁
    Cond m_cond;        // 条件变量

private:
    // 单例模式隐藏构造和析构
    Log() {}
    virtual ~Log() {
        delete [] m_buff;
    }

    // 处理log日志文件名
    void createFilename();

    // 创建新文件
    bool createNewFile(const std::string &file, std::fstream &out_file);

    // 统计当前的 log 日志文件是否到达最大行
    void countFile(const std::string &file);

    // 设置当前是那天
    void setCurrentTime();

    // 内部私有的线程处理函数
    void *asyncWriteLog();

public:
    // 单例模式
    static Log *get() {
        static Log log;
        return &log;
    }

    // 线程处理函数
    static void *logThread(void *arg) {
        // 调用内部私有线程处理函数
        Log::get()->asyncWriteLog();
        return (void *)nullptr;
    }

public:
    /**
     * filename: 文件名或文件路径                       isclose: 是否开启日志 
     * max_line: 文件的最大长度                         buff_size: 日志缓冲区的大小
     * block_size: 阻塞队列的长度                       thread_size: 线程个数
     */
    bool init(const char *filename, bool isclose, int max_line=FILE_MAX_COUNT, int buff_size=BUFFER_MAX_SIZE, \
              int block_size=BLOCK_QUEUE_SIZE);

    /* 写日志到文件 */
    void write(int level, const char *format, ...);
};

#endif // __LOG_H__