#ifndef __EVENT_LOOP__
#define __EVENT_LOOP__

#include <functional>
#include <sys/syscall.h>
#include <unistd.h>
#include <queue>
#include <mutex>
#include <sys/eventfd.h>
#include <sys/timerfd.h>
#include <map>
#include "Epoll.h"
#include "Connection.h"



// 如何通知事件循环?
// 通知线程的方法:条件变量、信号量、socket、管道、eventfd。
// 事件循环阻塞在epoll_wait()函数，条件变量、信号量有自己的等待函数，不适合用于通知事件循环。
//socket、管道、eventfd都是fd，可加入epoll，如果要通知事件循环，往socket、管道、eventfd中写入数据即可。


class Channel;
class Epoll;
class Connection;
using spConnection=std::shared_ptr<Connection>;

//事件循环类
class EventLoop
{
private:
    int timetvl_;                                    //闹钟时间间隔（s）
    int timeout_;                                    //Connection 超时时间
    std::unique_ptr<Epoll> ep_;             //每个事件循环中只有一个Epoll //一个网络程序中最多只有十几个事件循环 //头文件互相包含用栈内存会报错
    std::function<void(EventLoop*)> epolltimeoutcallback_;
    pid_t threadid_;                        //事件 循环所在线程的id
    std::queue<std::function<void()>> taskqueue_;   //事件循环被eventfd唤醒后执行的任务队列
    std::mutex mutex_;                              //任务队列同步的互斥锁
    int wakeupfd_;                                  //用于唤醒事件循环线程的eventfd
    std::unique_ptr<Channel> wakechannel_;
    int timerfd_;                                    //定时器fd
    std::unique_ptr<Channel> timerchannel_;         //定时器Channel
    bool mainloop_;                                 //true 为主事件循环，false为从事件循环
    std::map<int ,spConnection> conns_;
    std::mutex mmutex_;                             //保护conns_的互斥锁
    std::function<void(int)> timercallback_;        //用于删除TcpServer中的Connection对象

public:
    EventLoop(bool mainloop, int timetvl=30, int timeout=80);            //创建Epoll
    ~EventLoop();           //销毁Epoll
    void run();             //运行事件循环

    void updateChannel(Channel *ch);          //把chnnel添加/更新到红黑树上，添加事件
    void removeChannel(Channel *ch);           //从红黑树上删除channnel
    void closefd(int fd);
    void setepolltimeoutcallback(std::function<void(EventLoop*)> fn);

    bool isinloopthread();                      //判断当前线程是否为事件循环线程
    void queueinloop(std::function<void()> fn);  //把任务添加入队列
    void wakeup();                              //唤醒事件循环
    void handlewakeup();                        //事件循环被唤醒后执行的函数

    void handletimer();                         //定时执行任务

    void newconnection(spConnection conn);

    void settimercallback(std::function<void(int)> fn); 
};


#endif