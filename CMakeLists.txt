# server 的总 CMakeLists.txt 编译文件

# 定义 cmake 的版本号
cmake_minimum_required(VERSION 3.0)

# 定义项目名
project(server)

# 调用 http 中的 cmake 文件
add_subdirectory(http http)