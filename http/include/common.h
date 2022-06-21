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


#endif // __COMMON_H__