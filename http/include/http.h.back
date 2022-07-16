#ifndef __HTTP_H__
#define __HTTP_H__

/**
 * @file http.h
 * @author 910319432@qq.com
 * @brief 
 * @version 0.1
 * @date 2022-07-05
 * http服务器请求解析
 * @copyright Copyright (c) 2022
 * 
 */

#include <string>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <fstream>

#include "mmacro.h"

class HttpConn {
private:
    // 重命名
    typedef struct sockaddr_in SockAddrIn;
    
    // 定义请求的方法
    enum Method {
        GET=1,          // 请求指定的页面信息，并返回实体主体。
        POST,           // 向指定资源提交数据进行处理请求(例如提交表单或者上传文件)
        PUT,            // 从客户端向服务器传送的数据取代指定的文档的内容
        DELETE          // 请求服务器删除指定的页面
    };

    // 报文解析的结果
    enum HttpCode {
        GET_REQUEST,        // 表示获得了一个完整的客户端请求       
        BAD_REQUEST,        // 坏的请求表示客户端请求语法有问题             404
        NO_REQUEST,         // 表示请求不完整，需要继续读取客户端数据
        FORBIDDEN_REQUEST,  // 拒绝服务表示客户对资源没有足够的访问权限      403
        FILE_REQUEST,       // 请求文件                                   200
        INTERNAL_ERROR,     // 表示服务器内部错误                          500
        CLOSE_CONNECTION    // 表示客户端已经关闭连接了
    };

    // 主状态机的状态
    enum CheckState {
        STATE_LINE=0,   // 请求行
        STATE_HEADER,   // 请求头
        STATE_CONTENT   // 请求体
    };

    // 从状态机的状态
    enum LineStatus {
        LINE_OK=0,      // 解析完成
        LINE_OPEN,      // 未读取到完整的请求
        LINE_BAD        // 坏的解析
    };

public:
    // 构造和析构
    HttpConn(){}
    ~HttpConn(){}

private:
    /* 需要的相关变量 */
    bool m_isclose;     // 是否开启日志文件

    char m_read_buf[READ_BUFFER_MAX];   // read buff
    char m_write_buf[WRITE_BUFFER_MAX];     // write buff
    int m_read_idx;     // 当前读指针位置
    int m_write_idx;    // 当前写指针位置
    int m_check_idx;    // 解析行的当前位置
    int m_start_idx;    // 解析的开始位置 

    int m_epollfd;      // epoll 描述符

    SockAddrIn m_addrs;
    int m_sockfd;   // 客户端套接字

    int m_burst_mode;   // 触发模式，ET | IN or IN
    int m_check_state;  // 主状态机当前状态
    int m_connect_type;     // 连接类型，为1表示keep-alive，否则为 close

    Method m_method;                // 请求方式
    std::string m_url;              // 请求路径
    std::string m_version;          // htpp的版本
    std::string m_host;             // 请求的主机
    std::string m_connection;       // 连接方式
    std::string m_user_agent;       // 客户端使用的操作系统和浏览器的名称和版本
    std::string m_accept;           // 浏览器端可以接受的媒体类型
    std::string m_referer;          // 从哪个页面连接过来的
    std::string m_accept_encoding;  // 浏览器申明自己接收的编码方法
    std::string m_accept_language;  // 浏览器申明自己接收的语言
    std::string m_content;          // 请求体内容
    int m_contetn_len;              // 请求体长度
    std::string m_resp_content;     // 返回的内容

    std::fstream m_out_file;        // 读写文件

    std::string m_root;             // http服务器资源根路径

public:
    /*
     *  mode: 模式设置
     */
    void init(int sockfd, int isclose, const SockAddrIn &addr, const std::string &web_root, \
              int mode, int epollfd);

    void process();     // 读写总进程

    void closefd(bool real_bool=true);   // 关闭连接

    bool readData();    // 读取内容

    void writeData(HttpCode code);   // 写数据

private:
// public:
    /* 私有方法 */
    void init();        // 初始化

    /* 解析函数 */
    HttpCode processRead();     // 循环解析

    bool processWrite(HttpCode code);    // 写

    HttpCode parseRequestLine(char *request);   // 解析请求行

    HttpCode parseRequestHeader(char *request); // 解析请求头

    HttpCode parseRequestContent(char *request);    // 解析请求体

    LineStatus parseLine();     // 解析一行

    /* 添加响应数据 */
    bool addResponse(const char *format, ...);      // 添加响应行

    bool addContent(const char *content);       // 添加内容，对于 post put 和 delete

    bool addStatusLine(int status, const char *title);  // 添加状态

    bool addHeaders(int content_len);       // 添加响应头

    bool addContentType();          // 添加响应体的类型

    bool addContentLength(int content_len);    // 添加相应长度

    bool addConnectionType();           // 添加连接类型

    bool addBlockLine();        // 添加 \r\n

    HttpCode processData();         // 处理数据
};

#endif // __HTTP_H__