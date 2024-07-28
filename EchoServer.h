#ifndef __ECHO_SERVER__
#define __ECHO_SERVER__

#include "TcpServer.h"
#include "EventLoop.h"
#include "Connection.h"

//业务类
class EchoServer
{
private:
    TcpServer tcpserver_;
    ThreadPool threadpool_;
public:
    EchoServer(const std::string &ip, const uint16_t port,  int subthreadnum=3, int workthreadnum=5);
    ~EchoServer();

    void start();
    void stop();

    void HandleNewConnection(spConnection conn);         //处理新客户端连接请求
    void HandleClose(spConnection conn);         //关闭客户端连接，在Connection中调用    
    void HandleError(spConnection conn);         //客户端连接错误，在Connection中调用  
    void HandleMessage(spConnection conn, std::string& message);      //处理客户端的请求报文在Connection中回调
    void HandleSendComplete(spConnection conn);             //数据发送完成后回调
    void HandleTimeOut(EventLoop*loop);          //epoll_wait()超时

    void Login(spConnection conn, std::string& message);  //业务处理
};



#endif