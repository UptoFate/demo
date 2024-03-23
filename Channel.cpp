#include "Channel.h"

Channel::Channel(Epoll*ep, int fd):ep_(ep),fd_(fd)
{

}

Channel::~Channel()
{
    //ep_与fd_不属于Channel类，只是使用而已不能在这delete
}

int Channel::fd()
{
    return fd_;
}      

void Channel::useet()
{
    events_ = events_|EPOLLET;
}                  

void Channel::enablereading()
{
    events_ = events_|EPOLLIN;
    ep_->updateChannel(this);
}   

void Channel::setinepoll()
{
    inepoll_ = true;
}       

void Channel::setrevent(uint32_t event)
{
    revents_ = event;
}       

bool Channel::inepoll()
{
    return inepoll_;
}

uint32_t Channel::events()
{
    return events_;
}

uint32_t Channel::revents()
{
    return revents_;
}