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

#endif // __MMACRO_H__