#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <stdarg.h>
#include "log.h"
#include "config.h"
#include "mdebug.h"
#include "common.h"

using std::string;

bool Log::init(const char *filename, bool isclose, int max_line, int buff_size, int block_size) {
    // 判断是否开启异步日志, block_size>0表示开启异步日志
    if (block_size > 0) {
        DebugPrint("create thread\n");
        m_block = new BlockQueue<string>(block_size);
        if (m_block == nullptr) {   // 阻塞队列开启失败
            throw HttpException("block init failed.");
            m_isasync = false;  // 初始化阻塞队列失败，使用同步线程
        } else {
            // 开启成功，创建线程处理函数
            if (pthread_create(&m_tids, nullptr, logThread, nullptr) == 0) {
                // 让子线程与主线程分离
                pthread_detach(m_tids);
            }
            m_isasync = true;   // 开启异步线程
        }
    }

    // 初始化文件最大行数等等
    m_buff_size = buff_size;    // 缓冲区大小
    m_buff = new char[m_buff_size]; // 初始化缓冲区
    if (m_buff == nullptr) 
        return false;

    m_max_line = max_line;      // 文件的最大行数
    m_file_count = 1;           // 文件计数，默认从1开始
    m_count = 0;                // 文件当前的行数
    m_isclose = isclose;        // 是否开启日志

    // 处理创建 log 日志文件，获取路径和文件名
    char tmp_filename[FILENAME_MAX]={0};
    strncpy(tmp_filename, filename, FILENAME_MAX);
    char *find_dir = strrchr(tmp_filename, '/');
    if (find_dir == nullptr) {  // 表示当期那没有给定log目录，需要自己从配置文件读取
        auto conf = Config::getInstance()->getConfig();
        if (conf.find("log-path") == conf.end()) {
            // 配置文件中 log 日志文件路径不存在
            return false;
        }

        // 获取log 日志文件路径
        m_log_path = conf["log-path"];
        m_log_path += "/http";
        m_log_name = tmp_filename;
        DebugPrint("log filename: %s/%s\n", m_log_path.c_str(), m_log_name.c_str());       
    } else {    // 否则表示给定了当前log配置文件路径
        // 获取log日志文件目录和文件名
        *find_dir = '\0';
        m_log_path = tmp_filename;
        m_log_name = find_dir+1;
        DebugPrint("log filename: %s/%s\n", m_log_path.c_str(), m_log_name.c_str());       
    }

    // 判断日志目录是否存在
    DIR *dir;
    if ((dir = opendir(m_log_path.c_str())) == nullptr) {
        // 目录不存在，则创建该目录，递归创建
        createdir(m_log_path.c_str());            
    } else {
        closedir(dir);
    }

    // 格式化 log 日志文件名
    createFilename();

    // 拼接log日志的完整路径
    string real_path = m_log_path + "/" + m_tmp_log_name;
    DebugPrint("real_log_path: %s\n", real_path.c_str());

    // 打开 log 日志文件，需要判断当前日志文件是否达到最大行
    countFile(real_path);
    DebugPrint("m_file_count: %d\n", m_file_count);

    // 设置当前是哪一天
    setCurrentTime();
    DebugPrint("m_now_time: %d\n", m_now_day);
    return true;
}

void Log::createFilename() {
    // 格式化log日志文件名，显式的知道当前是那一天的日志
    m_tmp_log_name = m_log_name;

    // 获取当前时间
    time_t t = time(nullptr);
    // 转化当前时间为年月日时分秒
    struct tm *mid_time = localtime(&t);
    struct tm real_time = *mid_time;

    // 拼接当前时间 
    char tmp_strtime[BUFFER_MAX_SIZE]={0};
    sprintf(tmp_strtime, "%d_%02d_%02d_%02d.log", real_time.tm_year+1900, real_time.tm_mon, \
                                                  real_time.tm_mday, m_file_count);

    m_tmp_log_name += "_";
    m_tmp_log_name += tmp_strtime;
    // DebugPrint("m_tmp_log_name: %s\n", m_tmp_log_name.c_str());
}

void Log::countFile(const std::string &file) {
    // 统计当前使用的是哪个文件
    std::fstream out_file;

    DebugPrint("file: %s\n", file.c_str());
    if (createNewFile(file, out_file) == true) {
        DebugPrint("create new file is success.\n");
        return ;
    }

    // 文件存在读取文件行数
    char buff[BUFFER_MAX_SIZE];
    while (!out_file.eof()) {
        out_file.getline(buff, BUFFER_MAX_SIZE);
        m_count++;
    }

    if (m_count >= m_max_line) {
        // 调用下一个文件
        m_count=0;
        m_file_count++;
        createFilename();
        std::string real_paht = m_log_path + "/" + m_tmp_log_name;
        DebugPrint("log file: %s\n", real_paht.c_str());
        countFile(real_paht);
        out_file.close();
    } else {
        if (m_fp.is_open()) // 如果文件是打开的，则关闭它
            m_fp.close();
        
        m_fp.open(file, std::ofstream::app);
        out_file.close();
        --m_count;
        DebugPrint("m_cout: %d\n", m_count);
        return ;
    }
}

void Log::write(int level, const char *format, ...) {
    // 写入 log 日志文件
    // 获取当前时间
    time_t t = time(nullptr);
    struct tm *mid_time = localtime(&t);
    struct tm real_time = *mid_time;
    std::string prompt;     // 写入日志提示
    
    // 写入日志前缀提示
    switch (level) {
        case DEBUG_TYPE:
            prompt = "[Debug]: ";
            break;
        case INFO_TYPE:
            prompt = "[Info ]: ";
            break;
        case WARN_TYPE: 
            prompt = "[Warn ]: ";
            break;
        case ERROR_TYPE:
            prompt = "[Error]: ";
            break;
        default:
            prompt = "[Info ]: ";
            break;
    }

    m_mutex.locker();

    // 判断是否是当前日志或者文件达到最大行数
    if (m_now_day != real_time.tm_mday || m_count >= m_max_line) {
        // 如果条件成立，则需要重新创建文件
        if (m_now_day != real_time.tm_mday) {   // 按天分类日志文件
            setCurrentTime();
            m_file_count = 1;   // 重置当天的当前是第一个文件
        }

        // 重新创建文件
        m_file_count++;
        createFilename();
        std::fstream out_file;
        std::string new_file = m_log_path + "/" + m_tmp_log_name;
        if (createNewFile(new_file, out_file) == false) {
            out_file.close();   // 关闭 out_file 管理的对象
        }
    }

    // 行数计数+1
    ++m_count;
    m_mutex.unlocker();


    /* 格式化写入日志格式 */
    m_mutex.locker();

    va_list vlst;
    va_start(vlst, format);
    std::string write_str;

    // 格式化时间
    int m= sprintf(m_buff, "%d-%02d-%02d %02d:%02d:%02d %s", real_time.tm_year+1900, real_time.tm_mon, real_time.tm_mday, \
                                                             real_time.tm_hour, real_time.tm_min, real_time.tm_sec, prompt.c_str());

    int n = vsnprintf(m_buff+m, m_buff_size-m, format, vlst);
    m_buff[m+n] = '\n';
    m_buff[m+n+1] = '\0';
    DebugPrint("log: %s", m_buff);

    va_end(vlst);

    write_str = m_buff;
    m_mutex.unlocker();

    // 写入日志
    if (!m_isclose) {   // 总日志开关
        if (m_isasync && !m_block->isfull()) {
            // 异步日志
            m_block->push(write_str);
        } else {
            m_mutex.locker();
            // 同步日志
            m_fp.write(write_str.c_str(), write_str.size());
            m_fp.flush();
            m_mutex.unlocker();
        }
    }
}

void Log::setCurrentTime() {
    // 设置当前时间
    time_t t = time(nullptr);

    struct tm *mid_time = localtime(&t);
    struct tm real_time = *mid_time;

    m_now_day = real_time.tm_mday;
}

// 创建新文件
bool Log::createNewFile(const std::string &file, std::fstream &out_file) {
    out_file.open(file, std::ofstream::in);
    if (!out_file.is_open()) {
        // 判断文件是否存在，不存在就创建
        std::ofstream tmp_file(file);
        // 创建文件的时间
        time_t t = time(nullptr);
        struct tm *mid_time = localtime(&t);
        struct tm real_time = *mid_time;
        char tmp_time[BUFFER_MAX_SIZE]={0};
        sprintf(tmp_time, "create file time: %d-%02d-%02d %02d:%02d:%02d\n", real_time.tm_year+1900, real_time.tm_mon, \
                                                                             real_time.tm_mday, real_time.tm_hour, \
                                                                             real_time.tm_min, real_time.tm_sec);
        // DebugPrint("tmp_time: %s\n", tmp_time);
        tmp_file << tmp_time;
        tmp_file.close();

        if (m_fp.is_open())
            m_fp.close();

        m_fp.open(file, std::ofstream::app);
        m_count=0;
        return true;
    }

    return false;
}

// 内部私有的线程处理函数
void *Log::asyncWriteLog() {

    std::string write_str;

    int count = 0;

    while (m_block->pop(write_str)) {
        // 写入日志
        m_mutex.locker();
        m_fp.write(write_str.c_str(), write_str.size());
        m_fp.flush();
        m_mutex.unlocker();
    }
    
    return (void*)nullptr;
}