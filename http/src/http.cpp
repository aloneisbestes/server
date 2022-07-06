#include <string.h>

#include "http.h"


void HttpConn::init(int sockfd, int isclose, const SockAddrIn &addr, const std::string &web_root, \
                    int mode) {
    m_sockfd = sockfd;
    m_addrs = addr;
    m_root = web_root;
    m_isclose = isclose;
    m_burst_mode = mode;

    init();
}

void HttpConn::init() {
    m_read_idx = 0;
    m_write_idx = 0;

    memset(m_read_buf, 0, READ_BUFFER_MAX);
    memset(m_write_buf, 0, WRITE_BUFFER_MAX);
}