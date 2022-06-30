#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include "mexception.h"
#include "common.h"
#include "mmacro.h"

using namespace http::macro;

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

const char *getconfigpath(char *ret_path) {
    // 获取配置文件路径
    static char conf_path[FILE_PATH];

    char *tmp;
    tmp = getcwd(conf_path, FILE_PATH);
    if (tmp == nullptr) {
        throw HttpException("配置文件路径获取失败。");
        return nullptr;
    }

    for (int i = 0; i < 2; i++) {
        tmp = strrchr(conf_path, '/');
        if (tmp == nullptr) {   // 当前所在的路径是 bin 下面的 http
        throw HttpException("配置文件路径获取失败。");
        }
        *tmp = '\0';
    }

    printf("conf_path: %s\n", conf_path);

    // 获取配置文件路径
    DIR *dir;
    dirent *dp;
    dir = opendir(conf_path);
    while ((dp = readdir(dir)) != nullptr) {
        if (strcmp(dp->d_name, CONF_NAME) == 0) {
            strcat(conf_path, "/");
            strcat(conf_path, dp->d_name);
            if (ret_path != nullptr) 
                strcpy(ret_path, conf_path);
        }
    }
    closedir(dir);

    return nullptr;
}