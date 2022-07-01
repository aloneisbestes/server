# 设置所有源文件
set(ALL_SRC common.cpp config.cpp)

option(TEST "use test.cpp?" OFF)

if (TEST)
    set(SRC test.cpp)
else()
    set(SRC main.cpp)
endif()



# 导入当前依赖的头文件件所在位置
include_directories(../include)


# 生成可执行文件 httpserver
add_executable(httpserver ${ALL_SRC} ${SRC})

# 添加依赖库
target_link_libraries(httpserver pthread) # 添加线程依赖库