#ifndef __MY_EPOLL__
#define __MY_EPOLL__

#include <sys/epoll.h>
#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include <vector>
#include <unistd.h>
#include <string.h>
#include <strings.h>

class Epoll
{
public:
    static const int MAX_EVENT = 1000;       //epoll_wait()返回事件大小
    int epollfd_=-1;                        //epoll句柄
    epoll_event events_[MAX_EVENT];          //存放epoll_wait()返回的事件数组
public:
    Epoll(/* args */);
    ~Epoll();
    void addfd(int fd, uint32_t op);        //把fd和其监视的事件添加到红黑树上
    void closefd(int fd); 
    void writefd(int fd);
    std::vector<epoll_event> loop(int timeout=-1);  //运行epoll_wait()，等待事件发生，已发生的用vector返回
};



#endif