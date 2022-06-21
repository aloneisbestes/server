#include <exception>
#include "config.h"


void Config::init(const char *conf_path) {
    // 设置配置文件路径
    if (conf_path == nullptr) 
        throw std::exception();

    m_conf_path = conf_path;

    // 读取配置文件信息
    read();
}

// 内部读取配置文件函数
void Config::read() {

}

