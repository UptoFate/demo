#ifndef __EVENT_LOOP__
#define __EVENT_LOOP__

#include "Epoll.h"

class EventLoop
{
private:
    Epoll *ep_;             //每个事件循环中只有一个Epoll
public:
    EventLoop();            //创建Epoll
    ~EventLoop();           //销毁Epoll
    void run();             //运行事件循环
    Epoll* ep();
};


#endif