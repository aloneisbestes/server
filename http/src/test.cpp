#include <iostream>
#include <string.h>
#include <string>
#include <time.h>
#include <unistd.h>
#include "common.h"
#include "mexception.h"
#include "mdebug.h"
#include "config.h"
#include "mblock.h"
#include "log.h"
#include "http.h"

void test_strclr();
void test_getconfigpath();
void test_configclass();
void test_block_queue();
void test_log_file();
void test_recursion_create_dir();
void test_http_server();
void test_http_parse();

using namespace std;

int main() {
    // 随机数种子
    srand((unsigned)time(nullptr));

    // 测试strclr函数
    test_strclr();

    // 测试getconfigpath函数
    test_getconfigpath();

    // 测试config类
    test_configclass();

    // 测试block阻塞队列
    // test_block_queue();

    // 测试递归创建目录
    // test_recursion_create_dir();

    // 测试log日志文件
    // test_log_file();

    // 测试webserver.h
    // test_http_server();

    // 测试http.h，解析测试
    test_http_parse();

    return 0;
}

// 测试 strclr 函数
void test_strclr() {
    const char *str = "{\"hello\":\"yes\",\n\"test\":\"temp\"}";
    char str_tmp[1000];
    strcpy(str_tmp, str); 
    strclrs(str_tmp, " \",{}");
    std::cout << str_tmp << std::endl;
}

// 测试 getconfigpath 函数
void test_getconfigpath() {
    char str_tmp[1000];
    try {
        const char *tmp = getconfigpath(str_tmp);
    } catch (HttpException e) {
        std::cout << e.what() << std::endl;
    }
    std::cout << str_tmp << std::endl;
}

// 测试 config 配置文件类
void test_configclass() {
    Config::getInstance()->init("/home/alone/work/project/server/conf");

    auto conf = Config::getInstance()->getConfig();
    for (auto it : conf) {
        std::cout << it.first << ":" << it.second << std::endl;
    }
}

// 测试 BlockQueue 阻塞队列
void *block_run(void *arg) {
    pthread_detach(pthread_self());
    BlockQueue<int> *block = (BlockQueue<int> *)arg;
    int value;
    while (block->pop(value)) {
        cout << "tid: " << pthread_self() << ", value = " << value << endl;
    }

    return (void *)0;
}

void test_block_queue() {
    BlockQueue<int> block;

    // 创建两个线程用于处理
    pthread_t tid[2];
    for (int i = 0; i < 2; i++) {
        pthread_create(&tid[i], nullptr, block_run, (void *)&block);
    }

    int i = 0;
    while (1) {
        int r = rand()%100;        
        printf("r = %d\n", r);
        block.push(r);
        if ((i % 1000) == 999) {
            sleep(1);
        }
        i++;
    }
}

// 测试 递归创建目录
void test_recursion_create_dir() {
    const char *path = "/home/alone/work/project/server/log/http";
    createdir(path);
}

// 测试log日志文件
void test_log_file() {
    int m_isclose;
    Log::get()->init("/home/alone/work/project/server/log/test", false, 100, BUFFER_MAX_SIZE);

    sleep(1);
    for (int i = 0; i < 1000; i++) {
        Log::get()->write(DEBUG_TYPE, "write: %d", i+1);
        usleep(100);
    }

    LogInfo("write: %d", 1);

    while (1);
}

// 测试http解析
void test_http_parse() {
    HttpConn http_parse;
    struct sockaddr_in t;
    http_parse.init(1, false, t, "http", 1);

    // http_parse.setReadBuf("GET / HTTP/1.1\r\n");
    // http_parse.parseLine();
    
    char request[1000]={0};
    // strcpy(request, "GET https://aloneisbestes.cn/ HTTP/1.1");
    // http_parse.parseRequestLine(request);
    strcpy(request, "Host: aloneisbestes.cn:8080\
                     Connection: keep-alive\
                     User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/103.0.0.0 Safari/537.36\
                     Accept: image/avif,image/webp,image/apng,image/svg+xml,image/*,*/*;q=0.8 \
                     Referer: http://aloneisbestes.cn:8080/ \
                     Accept-Encoding: gzip, deflate\
                     Accept-Language: zh-CN,zh;q=0.9");
    http_parse.setReadBuf(request);

    // http_parse.processRead();

}