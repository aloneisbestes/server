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

    char *tmp = getcwd(conf_path, FILE_PATH);
    if (tmp == nullptr) {
        throw HttpException("配置文件路径获取失败。");
        return nullptr;
    }
    printf("now_path: %s\n", tmp);

    DIR *dir;
    dirent *dp;
    while (*conf_path != '\0') {    
        // 打开目录
        DIR *dir = opendir(conf_path);  // 打开目录
        while ((dp = readdir(dir)) != nullptr) {
            // 遍历该目录
            if (strcmp(dp->d_name, CONF_NAME) == 0) {
                strncat(conf_path, dp->d_name, strlen(dp->d_name));
                if (ret_path != nullptr) 
                    strcpy(ret_path, conf_path);

                return conf_path;
            }
        }
        closedir(dir); // 关闭目录

        // tmp 不为空，并且conf_path配置文件路径的长度不为一，也就是根路径
        tmp = strrchr(conf_path, '/');
        if (tmp == nullptr) {
            throw HttpException("配置文件路径获取失败。");
            break;
        }
        *tmp = '\0';
        printf("now_path: %s\n", conf_path);
    }

    return nullptr;
}