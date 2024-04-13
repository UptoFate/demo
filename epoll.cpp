//epoll模型实现网络通信服务端
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>
// #include <fcntl.h>
// #include <cassert>
// #include <sys/types.h>
// #include <sys/stat.h>
// #include <iostream>
// #include <netinet/tcp.h>
// #include <error.h>
// #include "InetAddress.h"
// #include "Socket.h"
// #include <json/json.h>
// #include "Channel.h"
// #include "EventLoop.h"
#include "TcpServer.h"

const int MAX_EVENT_NUMBER = 10000; //最大事件数

int main(int argc, char* argv[]){

    if(argc!=3){
        printf("tcpepoll: ip port\n");
        return -1; 
    }

    TcpServer tcpserver(argv[1],atoi(argv[2]));

    //进入服务器循环
    tcpserver.start();
}

