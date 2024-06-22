# CMake的最低版本要求
cmake_minimum_required(VERSION 3.10)

# 项目信息
project(epoll_project)

# 设置编译选项
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "-g")

# 包含目录
include_directories(/usr/include/mysql)

# 添加子目录（如果有其他子目录的话）
# add_subdirectory(src)

# 查找库文件
find_library(JSONCPP_LIB jsoncpp /usr/local/lib)
find_library(MYSQLCLIENT_LIB mysqlclient /usr/lib/x86_64-linux-gnu)

# 添加可执行文件
add_executable(epoll epoll.cpp InetAddress.cpp Socket.cpp Epoll.cpp Channel.cpp User.cpp Database.cpp EventLoop.cpp TcpServer.cpp)

# 链接库文件
target_link_libraries(epoll ${JSONCPP_LIB} ${MYSQLCLIENT_LIB})

