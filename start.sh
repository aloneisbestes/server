# 自动化编译运行该项目

# 获取当前路径
now_path=`pwd`
echo "now path is $now_path"

# 得到编译路径
cmake_build="$now_path/build"
cmake_bin="$now_path/bin"

# echo "cmake build: $cmake_build"

# 判断编译目录是否存在
if [ ! -d "$cmake_build" ]; then
#   mkdir /myfolder
    echo "create dir: $cmake_build"
    mkdir $cmake_build
fi

# 切换到构建目录
cd $cmake_build

# 编译项目
cmake ..

# 切换到运行目录
cd $cmake_bin

# 切换到 htpp 目录下运行 httpserver
cd ./http 
./httpserver