#ifndef __EVENT_LOOP__
#define __EVENT_LOOP__

#include <functional>
#include "Epoll.h"


class Channel;
class Epoll;

//事件循环类
class EventLoop
{
private:
    std::unique_ptr<Epoll> ep_;             //每个事件循环中只有一个Epoll //一个网络程序中最多只有十几个事件循环 //头文件互相包含用栈内存会报错
    std::function<void(EventLoop*)> epolltimeoutcallback_;
public:
    EventLoop();            //创建Epoll
    ~EventLoop();           //销毁Epoll
    void run();             //运行事件循环

    void updateChannel(Channel *ch);          //把chnnel添加/更新到红黑树上，添加事件
    void removeChannel(Channel *ch);           //从红黑树上删除channnel
    void closefd(int fd);
    void setepolltimeoutcallback(std::function<void(EventLoop*)> fn);
};


#endif