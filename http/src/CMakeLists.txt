# 设置所有源文件
set(ALL_SRC main.cpp)

# 导入当前依赖的头文件件所在位置
include_directories(../include)

# 生成可执行文件 httpserver
add_executable(httpserver ${ALL_SRC})