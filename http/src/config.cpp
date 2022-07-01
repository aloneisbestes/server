#include <fstream>
#include <string.h>
#include "mexception.h"
#include "config.h"
#include "mmacro.h"
#include "mdebug.h"
#include "common.h"

using std::ofstream;

bool conf_flags=false;

bool Config::init(const char *conf_path) {
    // 设置配置文件路径
    if (conf_path == nullptr) 
        throw std::exception();

    m_conf_path = conf_path;
    m_conf_path += '/';
    m_conf_path += CONF_FILE_NAME;

    DebugPrint("Config::init->m_conf_path: %s\n", m_conf_path.c_str());

    // 读取配置文件信息
    return read();
}

// 内部读取配置文件函数
bool Config::read() {
    // 读取配置文件
    std::ifstream readfile;
    readfile.open(m_conf_path, std::ofstream::in);
    if (!readfile.is_open()) {  // 判断配置文件是否打开成功
        return false;
    }

    // 读取配置文件
    char tmp_buff[FILENAME_MAX];
    while (!readfile.eof()) {
        // 读取文件
        memset(tmp_buff, 0, FILENAME_MAX);
        readfile.getline(tmp_buff, FILENAME_MAX);
        strclrs(tmp_buff, " \",");
        DebugPrint("Config::read-> tmp_buff: %s\n", tmp_buff);
        // 以:切割该字符串
        char *tmp = strrchr(tmp_buff, ':');
        if (tmp == nullptr) // 表示该行和没有配置
            continue;
        
        // 将读取的配置存入 map中
        char first[FILENAME_MAX]={0};
        char second[FILENAME_MAX]={0};
        strncpy(first, tmp_buff, tmp-tmp_buff);
        strcpy(second, tmp+1);
        // 如果在配置文件 map 中没有该字段，则添加
        if (m_conf.find(first) == m_conf.end()) {
            m_conf[first] = second;
        }
    }
    readfile.close();   // 关闭连接

    return true;
}

