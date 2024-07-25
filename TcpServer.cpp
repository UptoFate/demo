#include "TcpServer.h"

enum LOGIN{
    SUCCESS,
    PASERROR,
    NULLUSER,
    SQLERROR,
    HASHERROR,
    UNKNOWCMD
};

TcpServer::TcpServer(const std::string &ip, const uint16_t port, int threadnum)
    :mainloop_(new EventLoop()),acceptor_(mainloop_, ip, port),threadnum_(threadnum),threadpool_(threadnum_,"IO")
{
    acceptor_.setnewconnectioncb(std::bind(&TcpServer::newconnection, this, std::placeholders::_1));
    mainloop_->setepolltimeoutcallback(std::bind(&TcpServer::epolltimeout, this, std::placeholders::_1));

    //创建从事件循环
    for(int i=0; i<threadnum_; i++){
        subloops_.emplace_back(new EventLoop);     //创建从事件循环放入容器
        subloops_[i]->setepolltimeoutcallback(std::bind(&TcpServer::epolltimeout, this, std::placeholders::_1));
        threadpool_.addtask(std::bind(&EventLoop::run, subloops_[i].get()));     //bind函数记得用普通指针
    }
}

TcpServer::~TcpServer()
{
    //delete acceptor_;
    //delete mainloop_;

    // for(auto &i:subloops_){
    //     delete i;
    // }
    //delete threadpool_;
}

void TcpServer::start()
{
    mainloop_->run();
}
void TcpServer::newconnection(std::unique_ptr<Socket> clientsock)
{
    //Connection* conn = new Connection(mainloop_, clientsock);  //还未释放
    spConnection conn (new Connection(subloops_[clientsock->fd()%threadnum_], std::move(clientsock)));
    conn->setclosecallback(std::bind(&TcpServer::closeconnection,this,std::placeholders::_1));
    conn->seterrorcallback(std::bind(&TcpServer::errorconnection,this,std::placeholders::_1));
    conn->setonmessagercallback(std::bind(&TcpServer::onmessage, this, std::placeholders::_1, std::placeholders::_2));
    conn->sendcompletecallback(std::bind(&TcpServer::sendcomplete,this,std::placeholders::_1));

    conns_ [conn->fd()] = conn;

    if(newconnectioncb_)newconnectioncb_(conn);
}

void TcpServer::closeconnection(spConnection conn)
{
    if(closecohnectioncb_)closecohnectioncb_(conn);

    conns_.erase(conn->fd());   //conn里会关fd

}

void TcpServer::errorconnection(spConnection conn)
{
    if(errorconnectioncb_)errorconnectioncb_(conn);

    conns_.erase(conn->fd());   //conn里会关fd

}

void TcpServer::onmessage(spConnection conn, std::string& message)
{
    if(onmessagecb_)onmessagecb_(conn,message);
}

 void TcpServer::sendcomplete(spConnection conn)
 {
    if(sendcompletecb_)sendcompletecb_(conn);
 }

 void TcpServer::epolltimeout(EventLoop*loop)
 {
    if(timeoutcb_)timeoutcb_(loop);
 }

void TcpServer::setnewconnectioncb(std::function<void(spConnection)> fn)
{
    newconnectioncb_ = fn;
}

void TcpServer::setclosecohnectioncb(std::function<void(spConnection)> fn)
{
    closecohnectioncb_ = fn;
}

void TcpServer::seterrorconnectioncb(std::function<void(spConnection)> fn)
{
    errorconnectioncb_ = fn;
}

void TcpServer::setonmessagecb(std::function<void(spConnection,std::string &message)> fn)
{
    onmessagecb_ = fn;
}

void TcpServer::setsendcompletecb(std::function<void(spConnection)> fn) 
{
    sendcompletecb_ = fn;
}

void TcpServer::settimeoutcb(std::function<void(EventLoop*)> fn)
{
    timeoutcb_ = fn;
}   