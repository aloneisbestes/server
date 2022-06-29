#include "common.h"
#include <string.h>

const char *strclr(char *src, char clr) {
    if (src == nullptr) 
        return nullptr;
    char *tmp = src, *ret = src;
    for (;*tmp != '\0'; ++tmp) {
        if (*tmp != clr) {
            *src++ = *tmp; 
        }
    }
    *src = '\0';
    return ret;
}

const char *strclrs(char *src, const char *clrs) {
    if (src == nullptr)
        return nullptr;
    
    bool clr_flags=false;
    char *tmp = src, *ret = src;
    size_t len = strlen(clrs);
    for (;*tmp != '\0'; ++tmp) {
        for (int i = 0; i < len; i++) {
            if (*tmp == clrs[i]) {
                clr_flags = true;   // 如果当前字符和需要清除的字符相同，则为true
                break;
            }
        }

        if (!clr_flags) {   // 如果不相同才更新
            *ret++ = *tmp;
        }
        clr_flags = false;
    }

    *ret = '\0';
    return ret;
}