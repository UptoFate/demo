#ifndef __TCP_SERVER__
#define __TCP_SERVER__

#include <map>
#include "Socket.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Acceptor.h"
#include "Connection.h"

//网络服务类——和业务最密切相关
class TcpServer
{
private:
    EventLoop loop_;                //一个TCPServer可以有多个事件循环，现在是单线程，暂时只用一个
    Acceptor *acceptor_;            //一个TCPserver 只有一个 acceptor对象
    std::map<int, Connection*> conns_;
public:
    TcpServer(const std::string &ip, const uint16_t port);
    ~TcpServer();
    void start();                   //进入服务器循环
    void newconnection(Socket* clientsock);         //处理新客户端连接请求
    void closeconnection(Connection *conn);         //关闭客户端连接，在Connection中调用    
    void errorconnection(Connection *conn);         //客户端连接错误，在Connection中调用  
};


#endif