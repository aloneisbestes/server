#ifndef __M_DEBUF_H__
#define __M_DEBUF_H__

/**
 * @brief 
 * 主要用来调试用的 debug 函数
 * user: aloneisbestes
 * email: 910319432@qq.com
 */

#ifdef MDEBUG
#include <stdio.h>
#endif 

#ifdef MDEBUG
// MDEBUG 宏被定义，则使用 Debug 测试宏
#define DebugPrint(format, arg...) { \
    printf("\033[36m");\
    printf(format, ##arg); \
    printf("\033[0m");\
}
#else
#define DebugPrint(format, arg...)
#endif 


#endif // __M_DEBUF_H__