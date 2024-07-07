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
public:
    EchoServer(const std::string &ip, const uint16_t port,  int threadnum=3);
    ~EchoServer();

    void start();

    void HandleNewConnection(Connection *conn);         //处理新客户端连接请求
    void HandleClose(Connection *conn);         //关闭客户端连接，在Connection中调用    
    void HandleError(Connection *conn);         //客户端连接错误，在Connection中调用  
    void HandleMessage(Connection *conn, std::string& message);      //处理客户端的请求报文在Connection中回调
    void HandleSendComplete(Connection *conn);             //数据发送完成后回调
    void HandleTimeOut(EventLoop*loop);          //epoll_wait()超时
};



#endif