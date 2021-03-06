#ifndef __COMMON_H__
#define __COMMON_H__

/**
 * @brief 
 * 通用函数声明
 */

/**
 * @brief 
 * 清除指定的字符 clr 
 * @param src 
 * @param clr 
 * @return const char* 
 */
const char *strclr(char *src, char clr);

/**
 * @brief 
 * 清除指定的字符集 clrs
 * @param src 
 * @param clrs 
 * @return const char* 
 */
const char *strclrs(char *src, const char *clrs);

/**
 * @brief 
 * 获取配置文件路径
 * @param retpath=nullptr
 * @return const char* 
 */
const char *getconfigpath(char *ret_path=nullptr);

/**
 * @brief 
 * 用于递归创建目录
 * @param dir 
 */
void createdir(const char *dir);

/**
 * @brief 
 * 设置非阻塞, 返回旧的文件描述符标志, 如果失败返回-1
 * @param sock 
 * @return int 
 */
int setnonblocking(int sock);

#endif // __COMMON_H__