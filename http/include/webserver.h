#ifndef __WEB_SERVER_H__
#define __WEB_SERVER_H__

/**
 * @brief 
 * Web server come true.
 */

#include <netinet/in.h>
#include <sys/epoll.h>
#include <string>

#include "mmacro.h"

class WebServer {
private:
    typedef struct sockaddr_in SockAddrIn;
    typedef struct sockaddr SockAddr;
    typedef struct epoll_event epoll_event;

private:
    int m_serverfd;     // 服务器监听套接字
    int m_clientfd;     // 客户端套接字
    int m_epollfd;      // 使用 epoll
    int m_port;         // 服务器监听的端口
    int m_isclose;      // 是否关闭日志
    

    SockAddrIn m_server_addr;   // 服务端socket结构
    SockAddrIn m_client_addr;   // 客户端socket结构
    socklen_t m_client_len;     // 客户端socket结构长度
    socklen_t m_server_len;     // 服务端socket结构长度

    std::string m_root;         // web服务器资源，也就是html+css+js所在位置

    epoll_event m_events[EPOLL_EVEMT_MAX];      // 内核事件

public:
    WebServer();
    ~WebServer();

    // 初始化服务器
    void init_server(int port, int isclose);

    // 网络编程基础步骤
    void init_socket();

    // 开启监听
    void epoll_loop();

    // 测试
    void test(int fd);

    // 测试1
    void test1(int fd);
};

#endif // __WEB_SERVER_H__