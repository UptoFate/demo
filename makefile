#指定编译的目标文件
all:epoll
#依赖文件&编译（若依赖文件改变需要重新编译）
epoll:epoll.cpp InetAddress.cpp Socket.cpp Epoll.cpp Channel.cpp
	g++ -g -o epoll epoll.cpp InetAddress.cpp Socket.cpp Epoll.cpp Channel.cpp
#清理编译目标文件，仅在make clean才执行
clean:
	rm -f epoll

