#include <iostream>
#include <string.h>
#include "common.h"


int main() {
    const char *str = "{\"hello\":\"yes\",\n\"test\":\"temp\"}";

    char str_tmp[1000];
    strcpy(str_tmp, str); 

    strclrs(str_tmp, " \",{}");
    std::cout << str_tmp << std::endl;
    return 0;
}