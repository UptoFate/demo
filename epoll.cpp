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
#include <signal.h>
#include "EchoServer.h"

EchoServer *echoserver;

//信号处理函数
void Stop(int sig)
{
    printf("sig=%d\n",sig);
    echoserver->stop();
    delete echoserver;
    exit(0);
}
const int MAX_EVENT_NUMBER = 10000; //最大事件数

int main(int argc, char* argv[]){

    if(argc!=3){
        printf("tcpepoll: ip port\n");
        return -1; 
    }

    signal(SIGTERM,Stop);       //信号15 --kill / killall
    signal(SIGINT,Stop);        //信号2 --Ctrl+c
    
    echoserver= new EchoServer(argv[1],atoi(argv[2]));

    //进入服务器循环
    echoserver->start();
}

