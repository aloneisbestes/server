#ifndef __HTTP_H__
#define __HTTP_H__

/**
 * @file http.h
 * @author 910319432@qq.com
 * @brief 
 * @version 0.1
 * @date 2022-07-05
 * http服务器相关实现
 * @copyright Copyright (c) 2022
 * 
 */

#include <sys/socket.h> // socket 套接字头文件
#include <netinet/in.h>
#include "mmacro.h"

#include <string>

class HttpSocket {
private:
    // 定义别名
    typedef struct sockaddr_in  SockAddrIn;
    typedef struct sockaddr     SockAddr;

private:
    int m_http_fd;              // http的socket
    SockAddrIn m_http_addr;     // 监听套接字的结构
    char *m_read_buff;          // 读缓冲区
    char *m_write_buff;         // 写缓冲区
    int m_buff_size;
    int m_client_fd;            // 客户端套接字
    bool m_isclose;              // 是否关闭日志系统

public:
    // 构造和析构
    HttpSocket(int version=INET_4, int buff_size=BUFFER_MAX_SIZE, bool isclose=false);
    ~HttpSocket();

    // 开始
    void start();
    
// private:
    // 写数据
    void writeData();

    // 读数据
    void readData();

    // 开始监听
    int startListen(int max_listen);

    // 连接队列中取出已连接套接字
    int startAccept(SockAddrIn *addr=nullptr, socklen_t *len=nullptr);
};

#endif // __HTTP_H__