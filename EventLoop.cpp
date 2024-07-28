#include "EventLoop.h"

int createtimefd(int sec=30)
{
    // 把定时器加入epoll。
    int tfd=timerfd_create(CLOCK_MONOTONIC,TFD_CLOEXEC|TFD_NONBLOCK);   // 创建timerfd。
    struct itimerspec timeout;                                // 定时时间的数据结构。
    memset(&timeout,0,sizeof(struct itimerspec));
    timeout.it_value.tv_sec = sec;                             // 定时时间为sec。
    timeout.it_value.tv_nsec = 0;
    timerfd_settime(tfd,0,&timeout,0);                  // 开始计时。alarm(5)
    return tfd;
}

EventLoop::EventLoop(bool mainloop, int timetvl, int timeout)
    :ep_(new Epoll),mainloop_(mainloop),wakeupfd_(eventfd(0,EFD_NONBLOCK)),wakechannel_(new Channel(this,wakeupfd_)),
    timerfd_(createtimefd(timeout_)),timerchannel_(new Channel(this,timerfd_)),timetvl_(timetvl),timeout_(timeout),stop_(false)
{
    //设置回调函数以及注册读事件
    wakechannel_->setreadcallback(std::bind(&EventLoop::handlewakeup,this));
    wakechannel_->enablereading();

    timerchannel_->setreadcallback(std::bind(&EventLoop::handletimer,this));
    timerchannel_->enablereading();
}

EventLoop::~EventLoop()
{
    //delete ep_;
}

void EventLoop::run()
{
    //进入服务器循环
    threadid_ = syscall(SYS_gettid);

    while(stop_==false)
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

void EventLoop::stop()
{
    stop_ = true;
    wakeup();           //唤醒事件循环，否则事件循环将在下次闹钟响或epollwait超时停止
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

void EventLoop::handletimer()
{
    //重新计时
    struct itimerspec timeout;                                // 定时时间的数据结构。
    memset(&timeout,0,sizeof(struct itimerspec));
    timeout.it_value.tv_sec = timetvl_;                      // 定时时间为timetvl_。
    timeout.it_value.tv_nsec = 0;
    timerfd_settime(timerfd_,0,&timeout,0);                  // 开始计时。alarm(5)

    if(mainloop_)
    {
        printf("主事件循环.\n");
    }
    else
    {
        //printf("从事件循环.\n");
        printf("thread:%ld. fd:",syscall(SYS_gettid));
        time_t now = time(0);

        for(auto it=conns_.begin();it!=conns_.end();)
        {
            printf("%d ",it->first);
            if(it->second->timeout(now,timeout_))
            {
                timercallback_(it->first);        //调用回调函数，从TcpServer删除超时conn
                std::lock_guard<std::mutex> gd(mmutex_);
                it = conns_.erase(it);
            }else it++;
        }
        printf("\n");
    }
}

void EventLoop::newconnection(spConnection conn)
{
    std::lock_guard<std::mutex> gd(mmutex_);
    conns_[conn->fd()] = conn;
}

void EventLoop::settimercallback(std::function<void(int)> fn)
{
    timercallback_ = fn;
}