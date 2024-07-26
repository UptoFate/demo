#include "EventLoop.h"
EventLoop::EventLoop()
    :ep_(new Epoll),wakeupfd_(eventfd(0,EFD_NONBLOCK)),wakechannel_(new Channel(this,wakeupfd_))
{
    wakechannel_->setreadcallback(std::bind(&EventLoop::handlewakeup,this));
    wakechannel_->enablereading();
}

EventLoop::~EventLoop()
{
    //delete ep_;
}

void EventLoop::run()
{
    //进入服务器循环
    threadid_ = syscall(SYS_gettid);

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

bool EventLoop::isinloopthread()
{
    return threadid_ == syscall(SYS_gettid);
}

void EventLoop::queueinloop(std::function<void()> fn)
{
    //声明一个锁的作用域
    {
        std::lock_guard<std::mutex> gd(mutex_);     //给任务队列加锁
        taskqueue_.push(fn);
    }
    //唤醒事件循环
    wakeup();
}

void EventLoop::wakeup()
{
    uint64_t val=1;
    write(wakeupfd_, &val, sizeof(val));
}

void EventLoop::handlewakeup()
{
    uint64_t val;
    read(wakeupfd_, &val, sizeof(val));

    std::function<void()> fn;
    std::lock_guard<std::mutex> gd(mutex_);

    while (taskqueue_.size()>0)
    {
        fn = std::move(taskqueue_.front());     //出队元素（为什么要用移动语义？）
        taskqueue_.pop();
        fn();                                   //执行
    }
    
}