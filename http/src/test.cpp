#include <iostream>
#include <string.h>
#include "common.h"
#include "mexception.h"
#include "mdebug.h"
#include "config.h"


int main() {
    const char *str = "{\"hello\":\"yes\",\n\"test\":\"temp\"}";

    char str_tmp[1000];
    strcpy(str_tmp, str); 

    strclrs(str_tmp, " \",{}");
    std::cout << str_tmp << std::endl;

    std::cout << strlen("/") << std::endl;

    try {
        const char *tmp = getconfigpath(str_tmp);
    } catch (HttpException e) {
        std::cout << e.what() << std::endl;
    }
    std::cout << str_tmp << std::endl;

    Config::getInstance()->init(str_tmp);

    auto conf = Config::getInstance()->getConfig();
    for (auto it : conf) {
        std::cout << it.first << ":" << it.second << std::endl;
    }


    return 0;
}