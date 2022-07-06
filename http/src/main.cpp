#include <iostream>
#include <string.h>
#include "config.h"
#include "log.h"
#include "http.h"
#include "common.h"
#include "mdebug.h"
#include "webserver.h"

/**
 * @brief 
 * http 服务器的运行函数 main 函数
 * @param argc 
 * @param argv 
 * @return int 
 */


int main(int argc, char **argv) {

    char conf_path[FILENAME_MAX]={0};
    // 获取配置文件路径
    try {
        getconfigpath(conf_path);
        DebugPrint("conf_path: %s\n", conf_path);
    } catch (HttpException e) {
        std::cout << e.what() << std::endl;
        strcpy(conf_path, "/home/alone/work/project/server/conf");
    }

    // 初始化配置文件
    try {
        Config::getInstance()->init(conf_path);
    } catch (HttpException e) {
        std::cout << e.what() << std::endl;
    }
    
    // 初始化log日志文件
    try{
        Log::get()->init("http", false);
    } catch (HttpException e) {
        std::cout << e.what() << std::endl;
    }

    Log::get()->write(INFO_TYPE, "init complete.");

    // 创建服务器
    WebServer web_server;
    
    // 初始化服务器
    web_server.init_server(8080, false);

    // 初始化套接字以及epoll
    web_server.init_socket();

    // 开启服务器，循环监听
    web_server.epoll_loop();

    

    return 0;
}