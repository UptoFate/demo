#include "Connection.h"

Connection:: Connection(EventLoop *loop, Socket *clientsock):loop_(loop), clientsock_(clientsock)
{
    //设置该句柄为边缘触发（数据没处理完后续不会再触发事件，水平触发是不管数据有没有触发都返回事件），
    clientchannel_ = new Channel(loop_, clientsock_->fd());
    clientchannel_->setreadcallback(std::bind(&Channel::onmessage, clientchannel_));
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