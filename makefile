#指定编译的目标文件
all:epoll
#依赖文件&编译（若依赖文件改变需要重新编译）
epoll:epoll.cpp InetAddress.cpp Socket.cpp Epoll.cpp Channel.cpp User.cpp Database.cpp EventLoop.cpp TcpServer.cpp Acceptor.cpp Connection.cpp myEncrypt.cpp Buffer.cpp
	g++ -g -o epoll epoll.cpp InetAddress.cpp Socket.cpp Epoll.cpp Channel.cpp User.cpp Database.cpp EventLoop.cpp TcpServer.cpp Acceptor.cpp Connection.cpp myEncrypt.cpp Buffer.cpp /usr/local/lib/libjsoncpp.a -L/usr/lib/x86_64-linux-gnu -lmysqlclient -lssl -lcrypto 
#清理编译目标文件，仅在make clean才执行
clean:
	rm -f epoll

