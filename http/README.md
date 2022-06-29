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

