# server
Implement your own HTTP server and associated TCP server

**总述:**

1. http 目录主要实现关于 http 网页请求相关的处理，例如从网页发起一个请求，都在 http 这个文件下处理。
2. tcp 目录主要实现关于 tcp 连接的相关请求，可能用于后期的桌面界面开发使用，例如可能设计一个自己的聊天软件等等。
3. 服务端主要使用 c/c++ 实现，网页主要使用原生的 html+css+js 实现

## 1. CMakeLists.txt
该 CMakeLists.txt 是该项目的总编译文件，可以选择编译 http 还是 tcp

## 2. start.sh 
该 start.sh 脚本是用于自动化编译并运行程序

## 3. 项目编译运行步骤
如何手动编译并运行该项目。
