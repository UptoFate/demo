#ifndef __TCP_SERVER__
#define __TCP_SERVER__

#include <map>
#include "Socket.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Acceptor.h"
#include "Connection.h"

//网络服务类
class TcpServer
{
private:
    EventLoop loop_;                //一个TCPServer可以有多个事件循环，现在是单线程，暂时只用一个
    Acceptor *acceptor_;            //一个TCPserver 只有一个 acceptor对象
    std::map<int, Connection*> conns_;

    std::function<void(Connection*)> newconnectioncb_;                  //回调EchoServer::HandleNewConnection()。    
    std::function<void(Connection*)> closecohnectioncb_;                //回调EchoServer::HandleClose()。
    std::function<void(Connection*)> errorconnectioncb_;                //回调EchoServer::HandleError().
    std::function<void(Connection*,std::string &message)> onmessagecb_; //回调EchoServer::HandleMessage(0)
    std::function<void(Connection*)> sendcompletecb_ ;                  //回调EchoServer::HandleSendComplete0.
    std::function<void(EventLoop*)>timeoutcb_;                          //回调EchoServer::HandleTimeOut

public:
    TcpServer(const std::string &ip, const uint16_t port);
    ~TcpServer();
    void start();                   //进入服务器循环
    void newconnection(Socket* clientsock);         //处理新客户端连接请求
    void closeconnection(Connection *conn);         //关闭客户端连接，在Connection中调用    
    void errorconnection(Connection *conn);         //客户端连接错误，在Connection中调用  
    void onmessage(Connection *conn, std::string& message);      //处理客户端的请求报文在Connection中回调
    void sendcomplete(Connection *conn);             //数据发送完成后回调
    void epolltimeout(EventLoop*loop);          //epoll_wait()超时

    void setnewconnectioncb(std::function<void(Connection*)> fn) ;                  //回调EchoServer::HandleNewConnection()。    
    void setclosecohnectioncb(std::function<void(Connection*)> fn);                //回调EchoServer::HandleClose()。
    void seterrorconnectioncb(std::function<void(Connection*)> fn);                //回调EchoServer::HandleError().
    void setonmessagecb(std::function<void(Connection*,std::string &message)> fn); //回调EchoServer::HandleMessage(0)
    void setsendcompletecb(std::function<void(Connection*)> fn) ;                  //回调EchoServer::HandleSendComplete0.
    void settimeoutcb(std::function<void(EventLoop*)> fn);                          //回调EchoServer::HandleTimeOut
};


#endif