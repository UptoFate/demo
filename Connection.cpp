#include "Connection.h"

Connection:: Connection(EventLoop *loop, Socket *clientsock):loop_(loop), clientsock_(clientsock)
{
    //设置该句柄为边缘触发（数据没处理完后续不会再触发事件，水平触发是不管数据有没有触发都返回事件），
    clientchannel_ = new Channel(loop_, clientsock_->fd());
    clientchannel_->setreadcallback(std::bind(&Channel::onmessage, clientchannel_));
    clientchannel_->setclosecallback(std::bind(&Connection::closecallback, this));
    clientchannel_->seterrorcallback(std::bind(&Connection::errorcallback, this));
    //测试GET
    //clientchannel->setreadcallback(std::bind(&Channel::read_client_request, clientchannel));

    clientchannel_->useet();
    clientchannel_->enablereading();
}
Connection:: ~Connection()
{
    delete clientsock_;
    delete clientchannel_;
}

int Connection::fd() const
{
    return clientsock_->fd();
}
std::string Connection::ip() const
{
    return clientsock_->ip();
}
uint16_t Connection::port() const
{
    return clientsock_->port();
}

void Connection::closecallback()
{
    // printf("client(eventfd=%d) disconnected.\n", fd());
    // close(fd());
    closecallback_(this);
}

void Connection::errorcallback()
{
    // printf("client(eventfd=%d) error.\n", fd());
    // close(fd());
    errorcallback_(this);
}

void Connection::setclosecallback(std::function<void(Connection*)> fn)
{
    closecallback_ = fn;
}

void Connection::seterrorcallback(std::function<void(Connection*)> fn)
{
    errorcallback_ = fn;
}