//epoll模型实现网络通信服务端
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <cassert>
#include <sys/types.h>
#include <sys/stat.h>
#include <iostream>
#include <netinet/tcp.h>
#include <error.h>
#include "InetAddress.h"
#include "Socket.h"
#include <json/json.h>
#include "Channel.h"

const int MAX_EVENT_NUMBER = 10000; //最大事件数

int main(int argc, char* argv[]){

    if(argc!=3){
        printf("tcpepoll: ip port\n");
        return -1; 
    }

    Socket serversock(createnonblocking());
    InetAddress serveraddress(argv[1],atoi(argv[2]));
    serversock.setreuseaddr(true);
    serversock.setreuseport(true);
    serversock.setkeepalive(true);
    serversock.settcpnodelay(true);
    serversock.bind(serveraddress);
    serversock.listen(200);

    Epoll ep;
    //ep.addfd(serversock.fd(), EPOLLIN | EPOLLET | EPOLLRDHUP);      //epoll 监视listenfd 的读事件，水平触发
    Channel *servchannel = new Channel(&ep, serversock.fd());
    servchannel->setreadcallback(std::bind(&Channel::newconnection, servchannel, &serversock));
    servchannel->enablereading();                                     //epoll 监视listenfd 的读事件，水平触发 

    //进入服务器循环
    while(1)
    {
        std::vector<Channel*> channels = ep.loop();           //存放epoll_wait()返回的事件
        for(auto &ch:channels)
        {
            ch->handleevent();
        }
    }
}


