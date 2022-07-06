# Http 服务器
## 1. 定义异常类
自定义自己的异常类，在某些地方发生错误时，可以直接使用异常处理，保证程序的健壮性。

自定义的异常类如下：
```c++
// 用于自己程序的异常测试类，详情见 mexception.h
class HttpException : public std::exception {
    virtual const char *what() const noexcept;  // 返回异常错误提示

    HttpException &operator=(const HttpException &exce) noexcept;   // 重载等号运算符
}
```

## 2. 配置文件
配置文件头文件 config.h 主要作用是用来读取配置文件中的内容，程序刚运行时就会从配置文件路径中去读取 httpserver.conf 配置文件。

主要的配置有 mysql 所在的服务器，mysql 的用户名和密码以及 mysql 端口号，使用的 databases；httpserver log 日志文件所在的路径；httpsever 服务器的端口号，默认端口是 10010

## 3. 信号量、条件变量、互斥锁
信号量、条件变量和互斥锁主要用来保证数据的安全和线程安全及同步。

在多线程编程过程中，数据的安全是很重要的，在线程中，如果操作公共数据，就需要对该数据进行上锁，否则该数据的值最后是否正确是不确定的。例如：当使用链表的时，如果不对链表进行加锁，就有可能出现数据丢失或链表丢失。

一共封装3个类：
```c++
class Locker;   // 互斥锁
class Cond;     // 条件变量
class Sem;      // 信号量
```

## 4. 阻塞队列
阻塞队列的实现是用异步线程，在实现log日志文件的时候会有两种方式，一种是同步方式，一种是异步方式；同步方式是在程序出现异常或错误情况下会把错误信息写入日志文件，但是同步线程会占用主线程的时间，导致当前服务器被阻塞，异步线程是创建一个或多个线程去处理，则就需要保证创建的线程同步，所以使用阻塞队列去让线程同步。

## 5. webserver
webserver是http服务器的实现，采用的是 epoll 机制，用于处理高并发。

类 webserver
```c++
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
};
```
1. 使用 typedef 重定义：struct sockaddr_in, struct sockaddr, struct epoll_event 三个结构体。
2. 三个主要函数:<br>
    * init_server: 初始化服务器，从配置文件中读取一些服务器的配置
    * init_socket: 实现基本的网络编程，创建socket套接字listen监听套接字、epoll 内核机制的初始化。
    * epoll_loop: 服务器运行的逻辑代码，现在是在主进程中实现处理，后期会使用线程池去处理请求，主线程只接收请求，然后把请求放入线程中，由子线程处理。