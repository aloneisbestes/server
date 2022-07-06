// 系统库
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

// 自定义头文件
#include "config.h"
#include "webserver.h"
#include "log.h"
#include "common.h"
#include "mdebug.h"

WebServer::WebServer() {
    // 初始化服务器资源位置,从配置文件中读取
    auto conf = Config::getInstance()->getConfig();
    if (conf.find("http-root") == conf.end()) {
        m_root = "/home/alone/work/project/server/www";
    } else {
        m_root = conf["http-root"];
    }
}

WebServer::~WebServer() {

}


// 初始化服务器
void WebServer::init_server(int port, int isclose) {
    // 初始化服务器 port, 先使用配置文件，后使用 port
    auto conf = Config::getInstance()->getConfig();
    if (conf.find("http-port") == conf.end()) { // 如果配置文件中没有，则使用参数
        m_port = port;
    } else {    // 否则使用配置文件 
        m_port = atoi(conf["http-port"].c_str());
    }

    // 初始化是否打开日志
    m_isclose = isclose;
}


// 网络编程基础步骤
void WebServer::init_socket() {
    // 创建socket套接字
    m_serverfd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_serverfd < 0) {
        LogError("socket: %s", strerror(errno));        
        exit(1);
    }

    /* Socket描述符选项[SOL_SOCKET]
     * SOL_SOCKET: socket 描述符选项
     * SO_REUSEADDR: 打开和关闭地址复用功能, flag参数不为0表示打开地址复用，否则地址不复用
     */
    int flag = 1;
    setsockopt(m_serverfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));

    // 初始化 socket 结构
    m_server_len = sizeof(m_server_addr);
    m_client_len = sizeof(m_client_addr);
    memset(&m_server_addr, 0, m_server_len);
    m_server_addr.sin_family = AF_INET;
    m_server_addr.sin_port = htons(m_port);
    m_server_addr.sin_addr.s_addr = inet_addr("0.0.0.0");

    // 绑定套接字
    if (bind(m_serverfd, (SockAddr *)&m_server_addr, m_server_len) < 0) {
        LogError("bind: %s", strerror(errno));
        exit(1);
    }

    // 监听套接字
    if (listen(m_serverfd, LISTEN_MAX) < 0) {
        LogError("listen: %s", strerror(errno));
        exit(1);
    }

    // 创建epoll
    m_epollfd = epoll_create(EPOLL_MAX);
    if (m_epollfd < 0) {
        LogError("epoll_create: %s", strerror(errno));
        exit(1);
    }

    // socket 套接字添加到该事件
    epoll_event ev;
    ev.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    ev.data.fd = m_serverfd;

    // 添加到内核事件中
    if (epoll_ctl(m_epollfd, EPOLL_CTL_ADD, m_serverfd, &ev) < 0) {
        LogError("epoll_ctl: %s", strerror(errno));
    }
}

// 开启监听
void WebServer::epoll_loop() {
    int nfds;
    epoll_event ev;
    for (;;) {
        nfds = epoll_wait(m_epollfd, m_events, EPOLL_EVEMT_MAX, -1);
        if (nfds < 0) {
            LogWarn("epoll_wait: %s", strerror(errno));
            continue;
        }
        
        for (int i = 0; i < nfds; ++i) {
            if (m_events[i].data.fd == m_serverfd){ // 如果是监听套接字，则添加客户端
                m_clientfd = accept(m_serverfd, (SockAddr *)&m_client_addr, &m_client_len);
                if (m_clientfd < 0) {
                    LogWarn("accept: %s", strerror(errno));
                    continue;
                }

                // 设置文件描述符为非阻塞
                setnonblocking(m_clientfd);

                LogInfo("connect ip: %s", inet_ntoa(m_client_addr.sin_addr));
                LogInfo("connect port: %d", ntohs(m_client_addr.sin_port));

                // 添加到内核事件
                ev.data.fd = m_clientfd;
                ev.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
                if (epoll_ctl(m_epollfd, EPOLL_CTL_ADD, m_clientfd, &ev) < 0) {
                    LogError("epoll_ctl: %s", strerror(errno));
                }
            } else if (m_events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                // 客户端发送关闭连接，服务端移除内核事件
                epoll_ctl(m_epollfd, EPOLL_CTL_DEL, m_events[i].data.fd, nullptr);
            } else if (m_events[i].events & EPOLLIN) {
                // 处理连接
                int fd = m_events[i].data.fd;     
                test(fd);
            }
        }
    }
}

// 测试
void WebServer::test(int fd) {
    char read_buff[BUFFER_MAX_SIZE]={0};
    char write_buff[BUFFER_MAX_SIZE]={0};
    int n = read(fd, read_buff, BUFFER_MAX_SIZE);
    if (n < 0) {
        LogError("read: ", strerror(errno));
        return ;
    }
    DebugPrint("read: \n%s\n", read_buff);
    

    std::string file = m_root;
    file += "/index.html";
    std::fstream out_file;
    out_file.open(file, std::fstream::in);
    std::string send_str = "HTTP/1.1 200 OK\r\nContent-Type:text/html;utf-8\r\n\r\n";

    while (!out_file.eof()) {
        out_file.getline(write_buff, BUFFER_MAX_SIZE);
        send_str += write_buff;
    }

    write(fd, send_str.c_str(), send_str.size());
    close(fd);
}