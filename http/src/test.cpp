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

void test_strclr();
void test_getconfigpath();
void test_configclass();
void test_block_queue();
void test_log_file();
void test_recursion_create_dir();

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
    test_log_file();

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
    Log::get()->init("/home/alone/work/project/server/log/test", false, 4);
    Log::get()->write(1, "fdsjkaf");
}
