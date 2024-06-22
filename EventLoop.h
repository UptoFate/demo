#ifndef __EVENT_LOOP__
#define __EVENT_LOOP__

#include "Epoll.h"

class Channel;
class Epoll;

//事件循环类
class EventLoop
{
private:
    Epoll *ep_;             //每个事件循环中只有一个Epoll
public:
    EventLoop();            //创建Epoll
    ~EventLoop();           //销毁Epoll
    void run();             //运行事件循环

    void updateChannel(Channel *ch);          //把chnnel添加/更新到红黑树上，添加事件
    void closefd(int fd);
};


#endif