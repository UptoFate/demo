#ifndef __CONNECTION__
#define __CONNECTION__

#include <functional>
#include "Socket.h"
#include "InetAddress.h"
#include "EventLoop.h"
#include "Channel.h"

class Channel;
class EventLoop;

class Connection
{
private:
    EventLoop *loop_;
    Socket *clientsock_;
    Channel *clientchannel_;
    std::function<void(Connection*)> closecallback_;        //关闭连接回调，将回调Tcpserver中的closeconnection
    std::function<void(Connection*)> errorcallback_;        //连接错误回调，将回调Tcpserver中的errorconnection

public:
    Connection(EventLoop *loop, Socket *clientsock);
    ~Connection();
    int fd() const;     //返回fd成员
    std::string ip() const;
    uint16_t port() const;

    void closecallback();       //TCP连接断开的回调函数，供Channel回调
    void errorcallback();       //TCP连接错误的回调函数，供Channel回调

    void setclosecallback(std::function<void(Connection*)> fn);        //设置关闭fd_的回调函数
    void seterrorcallback(std::function<void(Connection*)> fn);        //设置fd_发生错误的回调函数
};

#endif