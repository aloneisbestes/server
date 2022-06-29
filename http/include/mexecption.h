#ifndef __M_EXCEPTION_H__
#define __M_EXCEPTION_H__

#include <exception>
#include <string>

namespace http {
    class HttpException;
}

class HttpException : public std::exception {
public:
    // 构造
    HttpException(const char *e) {
        m_exce = e;
    }
    HttpException(const HttpException &exce) {
        m_exce = exce.m_exce;
    }

    // 析构
    virtual ~HttpException() { }

    // 返回异常错误提示
    virtual const char *what() const noexcept {
        return m_exce.c_str();
    }

    // 等号运算符重载
    HttpException &operator=(const HttpException &exce) noexcept {
        m_exce = exce.m_exce;
    }

private:
    std::string m_exce;
};

#endif // __M_EXCEPTION_H__