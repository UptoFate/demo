#ifndef __ACCEPTOR__
#define __ACCEPTOR__

#include <functional>
#include "Socket.h"
#include "InetAddress.h"
#include "EventLoop.h"
#include "Channel.h"
#include "Connection.h"
#include "User.h"

//封装服务端用于监听的Channel
class Acceptor
{
private:

    EventLoop *loop_;
    Socket *servsock_;
    Channel *acceptchannel_;
    std::function<void(Socket*)> newconnectioncb_;  //处理新客户端连接请求的回调函数
public:
    Acceptor(EventLoop *loop, const std::string &ip, const uint16_t port);
    ~Acceptor();

    void newconnection();                   //处理新客户端连接请求

    void setnewconnectioncb(std::function<void(Socket*)> fn);   //设置新客户端连接请求的回调函数
};

#endif