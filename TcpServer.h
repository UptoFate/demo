#ifndef __TCP_SERVER__
#define __TCP_SERVER__

#include "Socket.h"
#include "Channel.h"
#include "EventLoop.h"

class TcpServer
{
private:
    EventLoop loop_;                //一个TCPServer可以有多个事件循环，现在是单线程，暂时只用一个
public:
    TcpServer(const std::string &ip, const uint16_t port);
    ~TcpServer();
    void start();                   //进入服务器循环
};


#endif