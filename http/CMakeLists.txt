# 编译 http 服务器

# 定义项目名
project(server)

message("object: " ${PROJECT_SOURCE_DIR})

# 调用 http 中的 cmake 文件
add_subdirectory(src http)