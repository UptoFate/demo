#ifndef __CONNECTION__
#define __CONNECTION__  

#include <functional>
#include <atomic>
#include <syscall.h>
#include "Socket.h"
#include "InetAddress.h"
#include "EventLoop.h"
#include "Channel.h"
#include "Buffer.h"
#include "memory"

class Channel;
class EventLoop;
class Connection;
using spConnection=std::shared_ptr<Connection>;



class Connection:public std::enable_shared_from_this<Connection>
{
private:
    EventLoop *loop_;
    std::unique_ptr<Socket> clientsock_;
    std::unique_ptr<Channel> clientchannel_;
    Buffer inputbuffer_;         //接收缓冲区
    Buffer outputbuffer_;        //发送缓冲区
    std::atomic_bool disconnect_;           //客户端是否已经断开 是则true 

    std::function<void(spConnection)> closecallback_;        //关闭连接回调，将回调Tcpserver中的closeconnection
    std::function<void(spConnection)> errorcallback_;        //连接错误回调，将回调Tcpserver中的errorconnection
    std::function<void(spConnection, std::string&)> onmessagecallback_;       //处理报文回调函数 
    std::function<void(spConnection)> sendcompletecallback_;       //数据发送完成回调函数 

public:
    Connection(EventLoop *loop, std::unique_ptr<Socket> clientsock);
    ~Connection();
    int fd() const;     //返回fd成员
    std::string ip() const;
    uint16_t port() const;

    void onmessage();
    void closecallback();       //TCP连接断开的回调函数，供Channel回调
    void errorcallback();       //TCP连接错误的回调函数，供Channel回调
    void writecallback();       //处理写事件的回调函数

    void setclosecallback(std::function<void(spConnection)> fn);        //设置关闭fd_的回调函数
    void seterrorcallback(std::function<void(spConnection)> fn);        //设置fd_发生错误的回调函数
    void setonmessagercallback(std::function<void(spConnection, std::string&)> fn);       //设置处理报文回调函数
    void sendcompletecallback(std::function<void(spConnection)> fn);       //设置数据发送完成回调函数

    void send(const char*data, size_t size);            //在任意线程中发送数据
    void sendinloop(std::shared_ptr<std::string> data);      //在IO线程中发送数据（如果当前是工作线程将传给IO线程）
};

#endif