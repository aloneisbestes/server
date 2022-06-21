#include "common.h"


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