#include "EventLoop.h"
EventLoop::EventLoop():ep_(new Epoll)
{

}

EventLoop::~EventLoop()
{
    //delete ep_;
}

void EventLoop::run()
{
    //进入服务器循环
    while(1)
    {
        std::vector<Channel*> channels = ep_->loop();           //存放epoll_wait()返回的事件
        
        //如果Channel为空，表示超时，回调TcpServer：：epolltimeout()
        if(channels.size()==0)
        {
            epolltimeoutcallback_(this);
        }
        else
        {
            for(auto &ch:channels)
            {
                ch->handleevent();
            }
        }

    }
}

void EventLoop::updateChannel(Channel *ch)
{
    ep_->updateChannel(ch);
}

void EventLoop::removeChannel(Channel *ch)
{
    ep_->removeChannel(ch);
}

void EventLoop::closefd(int fd)
{
    ep_->closefd(fd);
}

void EventLoop::setepolltimeoutcallback(std::function<void(EventLoop*)> fn)
{
    epolltimeoutcallback_ = fn;
}