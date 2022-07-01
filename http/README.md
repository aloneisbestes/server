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

