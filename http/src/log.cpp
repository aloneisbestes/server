#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "log.h"
#include "config.h"
#include "mdebug.h"

using std::string;

bool Log::init(const char *filename, bool isclose, int max_line, int buff_size, int block_size, int thread_size) {
    // 判断是否开启异步日志, block_size>0表示开启异步日志
    if (block_size > 0) {
        m_block = new BlockQueue<string>(block_size);
        if (m_block == nullptr) {   // 阻塞队列开启失败
            throw HttpException("block init failed.");
            m_isasync = false;  // 初始化阻塞队列失败，使用同步线程
        } else {
            // 开启成功，创建线程处理函数
            int count=0;
            m_tids = new pthread_t[thread_size];
            for (int i = 0; i < thread_size; i++) {
                // 创建线程
                pthread_t tmp_tid;
                if (pthread_create(&tmp_tid, nullptr, logThread, nullptr) == 0) {
                    m_tids[count++] = tmp_tid;
                    // 让子线程与主线程分离
                    pthread_detach(tmp_tid);
                }
            }
            m_tids_size = count;    // 正在开启线程的个数
            m_isasync = true;   // 开启异步线程
        }
    }

    // 初始化文件最大行数等等
    m_buff_size = buff_size;    // 缓冲区大小
    m_buff = new char[m_buff_size]; // 初始化缓冲区
    if (m_buff == nullptr) 
        return false;

    m_max_line = max_line;      // 文件的最大行数
    m_file_count = 0;           // 文件计数
    m_count = 0;                // 文件当前的行数
    m_isclose = isclose;        // 是否开启日志

    // 处理创建 log 日志文件
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
        m_log_name = tmp_filename;
        DebugPrint("log filename: %s/%s\n", m_log_path.c_str(), m_log_name.c_str());       

        // 判断日志目录是否存在
        DIR *dir;
        if ((dir = opendir(m_log_path.c_str())) == nullptr) {
            // 目录不存在，则创建该目录
            mkdir(m_log_path.c_str(), 0770);                       
        }
        closedir(dir);

        // 创建 http 日志目录
        m_log_path += "/http";
        DebugPrint("log filename: %s/%s\n", m_log_path.c_str(), m_log_name.c_str());       
        if ((dir = opendir(m_log_path.c_str())) == nullptr) {
            // 目录不存在，则创建该目录
            mkdir(m_log_path.c_str(), 0770);                       
        }
        closedir(dir);


    } else {    // 否则表示给定了当前log配置文件路径
        DebugPrint("loag filename: %s\n", filename);
    }

    return true;
}