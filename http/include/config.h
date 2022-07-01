#ifndef __CONFIG_H__
#define __CONFIG_H__

/**
 * @brief 
 * Config类，用来读取配置文件，使用单例模式实现
 * 在第一次调用 init 函数时会自动读取配置文件，也就是会自动调用 read() 函数

 */

#include <map>
#include <string>

namespace http{
    class Config;
}

class Config {
private:
    std::map<std::string, std::string> m_conf;      // 读取的配置文件内容以键值对的形式存储到 map 中
    std::string m_conf_path;                        // 配置文件路径

private:
    Config() {}
    ~Config() {}
    bool read();                            // 读取配置文件

public:
    bool init(const char *conf_path);       // 初始化 config 类
    const std::map<std::string, std::string> &getConfig() { return m_conf; }  // 获取配置文件信息

public:
    // 单例模式
    static Config *getInstance() {
        static Config conf;
        return &conf;
    }
};

#endif // __CONFIG_H__