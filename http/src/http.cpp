#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "http.h"
#include "mdebug.h"
#include "log.h"
#include "common.h"


// 返回内容的类型
const static char *content_type = "text/html";
// 内部服务器错误 500
const std::string error500_title = "Internal Error";
// 服务请求文件时出现了一个不寻常的问题
const std::string error500_from = "There was an unusual problem serving the request file.\n";

// 找不到请求的内容 404
const std::string error404_title = "Not Found";
// 在此服务器上找不到所请求的文件
const std::string error404_from = "The requested file was not found on this server.\n";

// 被禁止的 403
const std::string error403_title = "Forbidden";
// 您没有权限从此服务器获取文件
const std::string error403_from = "You do not have permission to get file form this server.\n";

// 错误的请求 400
const std::string error400_title = "Bad Request";
// 你的请求有错误的语法或者本质上不可能稳定
const std::string error400_from = "Your request has bad syntax or is inherently impossible to staisfy.\n";

// 请求成功 200
const std::string ok200_title = "Ok";


void HttpConn::init(int sockfd, int isclose, const SockAddrIn &addr, const std::string &web_root, \
                    int mode, int epollfd) {
    m_sockfd = sockfd;
    m_addrs = addr;
    m_root = web_root;
    m_isclose = isclose;
    m_burst_mode = mode;
    m_epollfd = epollfd;

    init();
}

void HttpConn::init() {
    m_read_idx = 0;
    m_write_idx = 0;
    m_check_state = STATE_LINE;     // 初始化时 m_check_state 为解析请求行
    m_check_idx = 0;                // 从状态机当前的解析位置
    m_start_idx = 0;
    m_contetn_len = 0;
    m_isimage = false;

    memset(m_read_buf, 0, READ_BUFFER_MAX);
    memset(m_write_buf, 0, WRITE_BUFFER_MAX);
}

/* 修改epoll内核事件 */
/* 修改 epoll 内核事件 */
void modfd(int epollfd, int sockfd, int ev, int burst_mode) {
    struct epoll_event event;
    event.data.fd = sockfd;

    if (burst_mode == 1) {
        event.events = ev | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
    } else {
        event.events = ev | EPOLLONESHOT | EPOLLRDHUP;
    }
    
    epoll_ctl(epollfd, EPOLL_CTL_MOD, sockfd, &event);
}

void addfd(int epollfd, int sockfd, bool one_shot, int burst_mode) {
    epoll_event event;
    event.data.fd = sockfd;

    if (burst_mode == 1) {
        event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    } else {
        event.events = EPOLLIN | EPOLLRDHUP;
    }

    if (one_shot) 
        event.events |= EPOLLONESHOT;
    
    epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &event);
    setnonblocking(sockfd);    
}

void delfd(int epollfd, int sockfd) {
    epoll_ctl(epollfd, EPOLL_CTL_DEL, sockfd, nullptr);
    close(sockfd);
}

// 读写总进程
void HttpConn::process() {
    // 读取数据
    if (!readData()) {
        delfd(m_epollfd, m_sockfd);
        return ;
    }

    // 读取成功后解析数据
    HttpCode read_ret = processRead();

    bool write_ret = processWrite(read_ret);
    if (!write_ret){
        closefd();
    }

    // 写数据
    writeData(read_ret);
}

// 读数据
bool HttpConn::readData() {
    if (m_read_idx >= READ_BUFFER_MAX) {
        return false;
    }

    int bytes_read = 0;

    if (m_burst_mode == 0) {  // EPOLLIN 模式
        bytes_read = recv(m_sockfd, m_read_buf, READ_BUFFER_MAX, 0);
        m_read_idx += bytes_read;

        if (bytes_read <= 0) {
            return false;
        }

        return true;
    } else {    // EPOLLET 模式
        while (true) {
            bytes_read = recv(m_sockfd, m_read_buf+m_read_idx, READ_BUFFER_MAX-m_read_idx, 0);
            if (bytes_read == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {  // 这两种情况是正常的，不是错误
                    break;
                }
                return false;
            } else if (bytes_read == 0){
                return false;
            }

            m_read_idx = bytes_read;
        }

        DebugPrint("m_read_idx: \n%s\n", m_read_buf);
        return true;
    }
}

// 解析一行，从状态机
HttpConn::LineStatus HttpConn::parseLine() {
    char tmp;
    // GET / HTTP/1.1\r\n
    for (;m_check_idx < m_read_idx; ++m_check_idx) {
        tmp = m_read_buf[m_check_idx];
        if (tmp == '\r') {  // 如果tmp=='\r'，表示请求行已找到
            if ( (m_check_idx+1) >= m_read_idx) {
                // 如果 m_check_idx + 1 >= m_read_idx，表示该请求的数据没有读取完
                return LineStatus::LINE_OPEN;
            } else if (m_read_buf[m_check_idx+1] == '\n') {
                // 表示一行读取玩
                m_read_buf[m_check_idx++] = '\0';
                m_read_buf[m_check_idx++] = '\0';
                // DebugPrint("parseLine: %s\n", m_read_buf+m_start_idx);
                return LineStatus::LINE_OK;
            }
            
            // 否则的话是一个错误的请求
            return LineStatus::LINE_BAD;
        } else if (tmp == '\n') {
            if (m_check_idx > 1 && m_read_buf[m_check_idx-1] == '\r') {
                // 则需要将 \r \n 都赋值为 \0
                m_read_buf[m_check_idx-1] = '\0';
                m_read_buf[m_check_idx++] = '\0';
                // DebugPrint("parseLine: %s\n", m_read_buf+m_start_idx);
                return LineStatus::LINE_OK;
            }

            // 否则的话是一个错误的请求，错误的请求是当请求头中出现单独的 \r 或 \n 时，就是一个错误的请求
            return LineStatus::LINE_BAD;
        }
    }

    return LineStatus::LINE_OPEN;   // 不完整的请求
} 

// 处理数据
HttpConn::HttpCode HttpConn::processData() {
    std::string source_path;
    if (m_method == Method::GET) {
        if (!getRequest())  {
            return HttpCode::BAD_REQUEST;
        }

    }

    return HttpCode::FILE_REQUEST;
}

// 
HttpConn::HttpCode HttpConn::processRead() {
    // 定义解析当前行的状态和解析函数的返回值
    LineStatus line_status = LINE_OK;   // 初始化为当前行解析完成
    HttpCode ret = NO_REQUEST;          // 当前的解析状态为未完成
    char *text = nullptr;

    // 循环解析请求内容
    while ((m_check_state == CheckState::STATE_CONTENT && line_status == LineStatus::LINE_OK) || \
           (line_status = parseLine()) == LineStatus::LINE_OK)
    {
        text = m_read_buf + m_start_idx;
        m_start_idx = m_check_idx;          // 更新解析的开始位置
        LogInfo("%s", text);
        switch (m_check_state) {
            case CheckState::STATE_LINE:    // 解析请求行
                ret = parseRequestLine(text);
                if (ret == HttpCode::BAD_REQUEST)
                    return HttpCode::BAD_REQUEST;
                break;
            case CheckState::STATE_HEADER:  // 解析请求头
                ret = parseRequestHeader(text);
                if (ret == HttpCode::BAD_REQUEST)
                    return ret;
                else if (ret == HttpCode::GET_REQUEST) 
                    return processData();   // 返回请求解析完成
                break;
            case CheckState::STATE_CONTENT: // 解析请求体
                ret = parseRequestContent(text);
                if (ret == HttpCode::GET_REQUEST) {
                    return processData();
                }
                line_status = LineStatus::LINE_OPEN;
                break;
            default:
                return HttpCode::INTERNAL_ERROR;    // 服务器内部错误
        }
    }

    return NO_REQUEST;  // 不完整的请求
}

// 解析请求行
HttpConn::HttpCode HttpConn::parseRequestLine(char *request) {
    // move 用于一定
    char *move = strpbrk(request, " \t");
    if (!move) // 如果 move 是空，则是一个错误的请求
        return BAD_REQUEST;

    // 获取请求方式
    *move++ = '\0';
    // DebugPrint("move: %s\n", move);
    char *method = request;
    // DebugPrint("mothod: %s\n", request);
    if (strcasecmp(method, "GET") == 0) {
        m_method = Method::GET;
    } else if (strcasecmp(method, "POST") == 0) {
        m_method = Method::POST;
    } else if (strcasecmp(method, "PUT") == 0) {
        m_method = Method::PUT;
    } else if (strcasecmp(method, "DELETE") == 0) {
        m_method = Method::DELETE;
    } else {
        return BAD_REQUEST;
    }

    move += strspn(move, " \t");  // 检索字符串 str1 中第一个不在字符串 str2 中出现的字符下标
    char *tmp_url = move;
    move = strpbrk(move, " \t");    // 找到版本号所在位置
    *move++ = '\0';

    // 获取 http 版本号
    m_version = move;
    // DebugPrint("m_version: %s\n", m_version.c_str());
    if (strcasecmp(m_version.c_str(), "HTTP/1.1") != 0) {   // 版本号不对，则是错误的请求
        return BAD_REQUEST;
    }

    // 获取 url 把 url 中的 http 或 https 去除
    if (strncasecmp(tmp_url, "http://", 7) == 0) {
        tmp_url += 7;
        tmp_url = strchr(tmp_url, '/');     // 保证url的第一个字符是 /
    } else if (strncasecmp(tmp_url, "https://", 8) == 0) {
        tmp_url += 8;
        tmp_url = strchr(tmp_url, '/');     // 保证url的第一个字符是 /
    }
    m_url = tmp_url;
    // DebugPrint("m_url: %s\n", m_url.c_str());

    // 判断 url 是否正确
    if (!m_url.c_str() || m_url[0] != '/') {
        return HttpCode::BAD_REQUEST;   // 坏的请求
    }

    // 获取开始页面
    if (m_url.size() == 1) {
        m_url += "index.html";
    }
    // DebugPrint("m_url: %s\n", m_url.c_str());

    // 更新解析状态
    m_check_state = CheckState::STATE_HEADER;   // 更新为解析请求体
    return HttpCode::NO_REQUEST;
}

// 解析请求头
HttpConn::HttpCode HttpConn::parseRequestHeader(char *request) {
    /*
    std::string m_host;             // 请求的主机
    std::string m_connection;       // 连接方式
    std::string m_user_agent;       // 客户端使用的操作系统和浏览器的名称和版本
    std::string m_accept;           // 浏览器端可以接受的媒体类型
    std::string m_referer;          // 从哪个页面连接过来的
    std::string m_accept_encoding;  // 浏览器申明自己接收的编码方法
    std::string m_accept_language;  // 浏览器申明自己接收的语言
    std::string m_content;          // 请求体内容
    int m_contetn_len;              // 请求体长度
    */
    if (request[0] == '\0') {   // 表示头部解析解析
        if (m_contetn_len != 0) {
            m_check_state = STATE_CONTENT;  // 更新为解析请求内容
            return HttpCode::NO_REQUEST;
        }
        return GET_REQUEST;
    } else if (strncasecmp(request, "Connection:", 11) == 0) {
        request += 11;
        request += strspn(request, " \t");
        m_connection = request;
        if (strncasecmp(m_connection.c_str(), "keep-alive", 10) == 0) {
            m_connect_type = 1;
        }
        DebugPrint("m_connection:%s\n", m_connection.c_str());
    } else if (strncasecmp(request, "Host:", 5) == 0) {
        request += 5;
        request += strspn(request, " \t");
        m_host = request;
        DebugPrint("m_host: %s\n", m_host.c_str());
    }else if (strncasecmp(request, "User-Agent:", 11) == 0) {
        request += 11;
        request += strspn(request, " \t");
        m_user_agent = request;
        DebugPrint("m_user_agent: %s\n", m_user_agent.c_str());
    } else if (strncasecmp(request, "Accept:", 7) == 0) {
        request += 7;
        request += strspn(request, " \t");
        m_accept = request;
        DebugPrint("m_accept: %s\n", m_accept.c_str());
    } else if (strncasecmp(request, "Referer:", 8) == 0) {
        request += 8;
        request += strspn(request, " \t");
        m_referer = request;
        DebugPrint("m_referer: %s\n", m_referer.c_str());
    } else if (strncasecmp(request, "Accept-Encoding:", 16) == 0) {
        request += 16;
        request += strspn(request, " \t");
        m_accept_encoding = request;
        DebugPrint("m_accept_encoding: %s\n", m_accept_encoding.c_str());
    } else if (strncasecmp(request, "Accept-Language:", 16) == 0) {
        request += 16;
        request += strspn(request, " \t");
        m_accept_language = request;
        DebugPrint("m_accept_language: %s\n", m_accept_language.c_str());
    } else {
        LogInfo("oop!unknow header: %s", request);
    }

    return HttpCode::NO_REQUEST;
}

// 解析请求体
HttpConn::HttpCode HttpConn::parseRequestContent(char *request) {
    if (m_read_idx >= (m_contetn_len+m_check_idx)) {  // 表示有请求内容
        // request[m_contetn_len] = '\0';
        m_content = request;
        DebugPrint("m_content: %s\n", m_content.c_str());
        return GET_REQUEST;
    }

    return NO_REQUEST;
}

// 添加响应行
bool HttpConn::addResponse(const char *format, ...) {
    if (m_write_idx >= WRITE_BUFFER_MAX) {
        return false;
    }


    va_list vlist;
    va_start(vlist, format);
    // 格式化
    int len = vsnprintf(m_write_buf+m_write_idx, WRITE_BUFFER_MAX-1-m_write_idx, format, vlist);
    if (len >= (WRITE_BUFFER_MAX-1-m_write_idx))  {
        // 判断写入的数据是否正确
        va_end(vlist); return false;
    }

    m_write_idx += len;
    va_end(vlist);

    LogInfo("request: %s", m_write_buf);
    return true;
}

// 添加连接类型
bool HttpConn::addConnectionType() {
    return addResponse("Connection: %s\r\n", m_connect_type == 1 ? m_connection.c_str() : "close");
}       

// 添加 \r\n
bool HttpConn::addBlockLine() {
    return addResponse("\r\n");
}

// 添加响应体的类型
bool HttpConn::addContentType() {
    return addResponse("Content-Type: %s\r\n", content_type);
}

// 添加相应长度
bool HttpConn::addContentLength(int content_len) {
    return addResponse("Content-Length: %d\r\n", content_len);
}

// 添加状态
bool HttpConn::addStatusLine(int status, const char *title) {
    return addResponse("%s %d %s\r\n", m_version.c_str(), status, title);
}

// 添加响应头
bool HttpConn::addHeaders(int content_len) {
    return addConnectionType() && addContentType() && addContentLength(content_len) && addBlockLine();
}

// 添加返回体内容
bool HttpConn::addContent(const char *content) {
    return addResponse("%s", content);
}

// 写
bool HttpConn::processWrite(HttpConn::HttpCode code) {
    switch (code) {
        case HttpCode::INTERNAL_ERROR:  // 500
            addStatusLine(500, error500_title.c_str());
            DebugPrint("134\n");
            addHeaders(error500_from.size());
            if (!addContent(error500_from.c_str())) {
                return false;
            }
            break;
        case HttpCode::BAD_REQUEST:     // 404
            addStatusLine(404, error404_title.c_str());
            addHeaders(error404_from.size());
            if (!addContent(error404_from.c_str())) {
                return false;
            }
            break;
        case HttpCode::FORBIDDEN_REQUEST:   // 403
            addStatusLine(403, error403_title.c_str());
            addHeaders(error403_from.size());
            if (!addContent(error403_from.c_str())) {
                return false;
            }
            break;
        case HttpCode::FILE_REQUEST:    // 200
            addStatusLine(200, ok200_title.c_str());
            break;
        default:
            return false;
    }

    return true;
}

// 关闭连接的套接字
void HttpConn::closefd(bool real_close) {
    if (real_close && (m_sockfd != -1)) {
        DebugPrint("close fd: %d\n", m_sockfd);
        delfd(m_epollfd, m_sockfd);
        m_sockfd = -1;
    }
}

// 写数据
void HttpConn::writeData(HttpConn::HttpCode code) {
    LogInfo("send to: %s", inet_ntoa(m_addrs.sin_addr));

    if (code != HttpCode::FILE_REQUEST) {
        send(m_sockfd, m_write_buf, m_write_idx, 0);
        closefd();
        return ;
    }

    int n = 0;
    char tmpbuf[1024] = {0};
    char *imgbuf;
    bool flag;
    int imglen=0;
    if (strncasecmp(m_url.c_str(), "/images", 7) == 0) {
        // m_out_file.read(tmpbuf, 1024);
        m_out_file.seekg(0, m_out_file.end);
        int len = m_out_file.tellg();
        m_out_file.seekg(0, m_out_file.beg);
        imgbuf = new char[len];
        m_out_file.read(imgbuf, len);
        imglen = len;
        flag = true;
        addHeaders(len);
    } else {
        while (!m_out_file.eof()) {
            m_out_file.read(tmpbuf, 1024);
            m_resp_content += tmpbuf;
        }
        DebugPrint("m_resp_count: %s\n", m_resp_content.c_str());
        addHeaders(m_resp_content.size());
    } 

    n = send(m_sockfd, m_write_buf, m_write_idx, 0);
    if (n < 0) {
        closefd();
    }

    if (flag == true) {
        n = send(m_sockfd, imgbuf, imglen, 0);
        delete [] imgbuf;
    } else {
        // 数据过大，需要循环发送
        const char *tmp_send = m_resp_content.c_str();
        n = 0;
        int count_size = 0;
        do {
            n = send(m_sockfd, tmp_send+count_size, 4096, 0);
            count_size += n;
        }while (n > 0 && count_size != m_resp_content.size());
            

    }

    closefd();
}

// get请求
bool HttpConn::getRequest() {
    // GET /login.html?username=admin&password=admin HTTP/1.1   带参数
    // GET /test/index.css HTTP/1.1                             无参数

    const char *tmp_cut = strchr(m_url.c_str(), '?');
    if (tmp_cut == nullptr) {   // 表示没有携带参数
        std::string src_path = m_root + m_url;
        // 判断是否为图片读取
        if (strncasecmp(m_accept.c_str(), "image", 5) == 0) {
            LogInfo("request image");
            m_isimage = true;
            // 以二进制文件读取图片
            m_out_file.open(src_path, std::fstream::in | std::fstream::binary);
        } else {    // 读取正常文件
            LogInfo("request file");
            m_isimage = false;
            m_out_file.open(src_path, std::fstream::in);
        }

        // 判断文件是否打开
        if (!m_out_file.is_open()) {
            // 表示文件不成在
            return HttpCode::BAD_REQUEST;
        }

        return HttpCode::FILE_REQUEST;
    } else {    // 否则就是携带参数
        char tmp_arg[1024]={0};
        strcpy(tmp_arg, tmp_cut+1);
        
    }
}