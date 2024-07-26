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
    //对不属于自己但会使用的资源其实采用裸指针更合适
    EventLoop *loop_;               //Acceptor对事件循环没有所有权，不能使用移动语义，只能采用重引用
    Socket servsock_;
    Channel acceptchannel_;         //使用栈内存，而在Connection中使用unique指针（堆内存）
    std::function<void(std::unique_ptr<Socket>)> newconnectioncb_;  //处理新客户端连接请求的回调函数
public:
    Acceptor(EventLoop *loop, const std::string &ip, const uint16_t port);
    ~Acceptor();

    void newconnection();                   //处理新客户端连接请求

    void setnewconnectioncb(std::function<void(std::unique_ptr<Socket>)> fn);   //设置新客户端连接请求的回调函数
};

#endif