#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "http.h"
#include "config.h"
#include "log.h"
#include "mdebug.h"


HttpSocket::HttpSocket(int version, int buff_size, bool isclose) {
    memset(&m_http_addr, 0, sizeof(m_http_addr));

    // 确定协议版本
    if (version == INET_4) 
        m_http_addr.sin_family = AF_INET;
    else 
        m_http_addr.sin_family = AF_INET6;

    // 初始化服务器端口号，从配置文件读取
    auto conf = Config::getInstance()->getConfig();
    if (conf.find("http-port") == conf.end()) {
        // 表示配置文件中没有配置，使用默认端口
        m_http_addr.sin_port = 8080;
    } else {
        // 使用配置文件的配置
        m_http_addr.sin_port = htons(atoi(conf["http-port"].c_str()));
    }

    // 初始化服务器监听所有ip
    m_http_addr.sin_addr.s_addr = inet_addr("0.0.0.0");

    // 初始化读写缓存
    m_buff_size = buff_size;
    m_read_buff = new char[m_buff_size];
    m_write_buff = new char[m_buff_size];

    m_isclose = isclose;
}

HttpSocket::~HttpSocket() {
    if (m_read_buff) delete [] m_read_buff;
    if (m_write_buff) delete [] m_write_buff;
}

int HttpSocket::startListen(int max_listen) {
    // 创建 socket 套接字
    int ret;
    DebugPrint("create socket: ");
    if ((ret = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
        return ret;
    m_http_fd = ret;
    DebugPrint("%d\n", m_http_fd);
    
    // 绑定
    DebugPrint("bind: ");
    if ((ret = bind(m_http_fd, (SockAddr *)&m_http_addr, sizeof(SockAddrIn))) < 0) 
        return ret;
    DebugPrint("%d\n", ret);

    // 监听套接字
    DebugPrint("listen: ");
    if ((ret = listen(m_http_fd, max_listen)) < 0)
        return ret;
    DebugPrint("%d\n", m_http_fd);

    return m_http_fd;
}


// 写数据
void HttpSocket::writeData() {
    // return write(m_client_fd, write_str.c_str(), write_str.size());
    // 获取http根目录
    auto conf = Config::getInstance()->getConfig();
    std::string index = conf["http-root"] + "/" + "index.html";
    DebugPrint("index.html: %s\n", index.c_str());
    std::fstream fp;
    fp.open(index, std::fstream::in);
    if (!fp.is_open()) {
        LogError("www/index.html open error.");
        return ;
    }

    std::string send_str;
    send_str = "HTTP/1.1 200 OK\r\nContent-Type:text/html;utf-8\r\n\r\n";
    while (!fp.eof()) {
        memset(m_write_buff, 0, m_buff_size);
        fp.getline(m_write_buff, m_buff_size);
        send_str += m_write_buff;
    }
    DebugPrint("send_str: %s\n", send_str.c_str());
    send(m_client_fd, send_str.c_str(), send_str.size(), 0);
    fp.close();
}


// 读数据
void HttpSocket::readData() {
    int ret = read(m_client_fd, m_read_buff, m_buff_size);
}

// 连接队列中取出已连接套接字
int HttpSocket::startAccept(SockAddrIn *addr, socklen_t *len) {
    return accept(m_http_fd, nullptr, nullptr);
}

// 开始
void HttpSocket::start() {
    if (startListen(10) < 0) {
        LogError("func: %s : %s", "startListen", strerror(errno));
        usleep(500);
        exit(1);
    }

    while (1) {
        m_client_fd = startAccept();
        if (m_client_fd < 0) 
            continue;
        
        readData();
        DebugPrint("read data: \n%s\n", m_read_buff);

        writeData();
        close(m_client_fd);
    }
}