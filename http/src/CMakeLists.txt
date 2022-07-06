# 设置所有源文件
set(ALL_SRC common.cpp config.cpp log.cpp http.cpp webserver.cpp)

option(TEST "use test.cpp?" OFF)

if (TEST)
    set(SRC test.cpp)
else()
    set(SRC main.cpp)
endif()

# 添加Debug宏
add_definitions(-DMDEBUG)

# 导入当前依赖的头文件件所在位置
include_directories(../include)

# 添加 GDB 调试
SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_C_FLAGS_DEBUG "$ENV{CFLAGS} -O0 -Wall -g -ggdb")
SET(CMAKE_C_FLAGS_RELEASE "$ENV{CFLAGS} -O3 -Wall")

# 生成可执行文件 httpserver
add_executable(httpserver ${ALL_SRC} ${SRC})

# 添加依赖库
target_link_libraries(httpserver pthread) # 添加线程依赖库