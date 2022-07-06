#ifndef __MMACRO_H__
#define __MMACRO_H__

namespace http {
    // 定义相关宏
    namespace macro {
        const int FILE_PATH = 4096;     // 文件路径的长度

    }
}

/* 定义配置文件名和配置文件路径 */
#define CONF_NAME           "conf"
#define CONF_FILE_NAME      "httpserver.conf"

/* 定义阻塞队列的默认大小 */
#define BLOCK_QUEUE_SIZE    1000

/* 定义 buffer 的最大长度 */
#define BUFFER_MAX_SIZE     4096

/* 定义文件的最大行数 */
#define FILE_MAX_COUNT      1000000

/* 定义写入日志类型 */
#define DEBUG_TYPE          0
#define INFO_TYPE           1
#define WARN_TYPE           2
#define ERROR_TYPE          3

/* 定义套接字协议版本 */
#define INET_4              4
#define INET_6              5

/* 定义监听的最大套接字 */
#define LISTEN_MAX          5

/* 内核注册表 epoll */
#define EPOLL_MAX           5
#define EPOLL_EVEMT_MAX     10000    // 最大事件为 10000

/* 定义读写缓冲区大小 */
#define READ_BUFFER_MAX     4096
#define WRITE_BUFFER_MAX    1024

#endif // __MMACRO_H__