#ifndef __CHANNEL__
#define __CHANNEL__
#include "Epoll.h"

class Epoll;

//如果一个服务器同时提供不同的服务，如HTTP、FTP等，那么就算文件描述符上发生的事件都是可读事件，
//不同的连接类型也将决定不同的处理逻辑，仅仅通过一个文件描述符来区分显然会很麻烦，我们更加希望拿到关于这个文件描述符更多的信息。
class Channel
{
private:
    int fd_ = -1;
    Epoll*ep_ = nullptr;         //Channel对应的红黑树，Channel与Epoll是多对一
    bool inepoll_ = false;       //Channel是否已添加到epoll树上，若未则调用epoll_ctl()时使用EPOLL_CTL_ADD,否则用.._MOD
    uint32_t events_ = 0;         //fd_需要监视的事件：listenfd和clientfd需要监视EPOLLIN，clientfd还可能要EPOLLOUT
    uint32_t revents_ = 0;      //fd_已发生事件
public:
    Channel(Epoll*ep, int fd);
    ~Channel();
    int fd();                                //返回fd_
    void useet();                           //采用边缘触发
    void enablereading();                   //让epoll_wait()监视fd_的读事件
    void setinepoll();                      //设置inepoll_
    void setrevent(uint32_t event);                      //设置inepoll_
    bool inepoll();
    uint32_t events();
    uint32_t revents();
};

#endif