#include "EventLoop.h"
EventLoop::EventLoop():ep_(new Epoll)
{

}

EventLoop::~EventLoop()
{
    delete ep_;
}

void EventLoop::run()
{
    //进入服务器循环
    while(1)
    {
        std::vector<Channel*> channels = ep_->loop();           //存放epoll_wait()返回的事件
        for(auto &ch:channels)
        {
            ch->handleevent();
        }
    }
}

Epoll* EventLoop::ep()
{
    return ep_;
}

