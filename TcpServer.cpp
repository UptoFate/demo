#include "TcpServer.h"


TcpServer::TcpServer(const std::string &ip, const uint16_t port):acceptor_(new Acceptor(&loop_, ip, port))
{
    acceptor_->setnewconnectioncb(std::bind(&TcpServer::newconnection, this, std::placeholders::_1));
}

TcpServer::~TcpServer()
{
    delete acceptor_;
    for(auto i:conns_){
        delete i.second;
    }
}

void TcpServer::start()
{
    loop_.run();
}
void TcpServer::newconnection(Socket* clientsock)
{
    Connection* conn = new Connection(&loop_, clientsock);  //还未释放
    conn->setclosecallback(std::bind(&TcpServer::closeconnection,this,std::placeholders::_1));
    conn->seterrorcallback(std::bind(&TcpServer::errorconnection,this,std::placeholders::_1));
    printf("newconnection(fd=%d,ip=%s,port=%d)ok\n", conn->fd(), conn->ip().c_str(), conn->port());
    conns_ [conn->fd()] = conn;
}

void TcpServer::closeconnection(Connection *conn)
{
    printf("client(eventfd=%d) disconnected.\n", conn->fd());
    conns_.erase(conn->fd());   //conn里会关fd
    delete conn;
}

void TcpServer::errorconnection(Connection *conn)
{
    printf("client(eventfd=%d) error.\n", conn->fd());
    conns_.erase(conn->fd());   //conn里会关fd
    delete conn;
}