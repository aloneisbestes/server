#ifndef __M_EXCEPTION_H__
#define __M_EXCEPTION_H__

#include <exception>
#include <string>

namespace http {
    class HttpException;
}

class HttpException : public std::exception {
public:
    HttpException(const char *e) {
        m_exce = e;
    }


private:
    std::string m_exce;
};

#endif // __M_EXCEPTION_H__