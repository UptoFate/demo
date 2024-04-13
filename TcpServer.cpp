#include "TcpServer.h"


TcpServer::TcpServer(const std::string &ip, const uint16_t port):loop_()
{
    Socket *serversock = new Socket(createnonblocking());
    InetAddress serveraddress(ip,port);
    serversock->setreuseaddr(true);
    serversock->setreuseport(true);
    serversock->setkeepalive(true);
    serversock->settcpnodelay(true);
    serversock->bind(serveraddress);
    serversock->listen(200);

    //ep.addfd(serversock.fd(), EPOLLIN | EPOLLET | EPOLLRDHUP);      //epoll 监视listenfd 的读事件，水平触发
    Channel *servchannel = new Channel(loop_.ep(), serversock->fd());
    servchannel->setreadcallback(std::bind(&Channel::newconnection, servchannel, serversock));
    servchannel->enablereading();                                     //epoll 监视listenfd 的读事件，水平触发 

}

TcpServer::~TcpServer()
{
}

void TcpServer::start()
{
    loop_.run();
}