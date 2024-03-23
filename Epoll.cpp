#include "Epoll.h"
Epoll::Epoll(/* args */)
{
    // 创建epoll实例
    if((epollfd_ = epoll_create(5))==-1)        //md 左边“=” 括号没加 找了好久饭都没吃www
    {
        printf("fail to epoll create!");
        exit(-1);
    }
}

Epoll::~Epoll()
{
    close(epollfd_);
}

// void Epoll::addfd(int fd, uint32_t op)
// {
//     if (fd < 0)
//     {
//         printf("errno is: accept() error");
//         exit(-1);
//     }
//     // 创建节点结构体将监听连接句柄
//     epoll_event ev;
//     ev.data.fd = fd;
//     //设置该句柄为边缘触发（数据没处理完后续不会再触发事件，水平触发是不管数据有没有触发都返回事件），
//     ev.events = op;
//     // 添加监听连接句柄作为初始节点进入红黑树结构中，该节点后续处理连接的句柄
//     if(epoll_ctl(epollfd_, EPOLL_CTL_ADD, fd, &ev)==-1)
//     {
//         perror("epoll_ctl()add failed ");
//         exit(-1);
//     }
// }

void Epoll::updateChannel(Channel *ch)
{
    epoll_event ev;
    ev.data.ptr = ch;
    ev.events = ch->events();
    if(ch->inepoll())       //channel 已经在树上了 
    {
        if(epoll_ctl(epollfd_, EPOLL_CTL_MOD, ch->fd(), &ev)==-1)
        {
            perror("epoll_ctl()failed.\n");
            exit(-1);
        }
    }
    else
    {
        if(epoll_ctl(epollfd_, EPOLL_CTL_ADD, ch->fd(), &ev)==-1)
        {
            perror("epoll_ctl()failed.\n");
            exit(-1);
        }
        ch->setinepoll();
    }
}

void Epoll::closefd(int fd)
{
    //服务器端关闭连接，
    if(epoll_ctl(epollfd_, EPOLL_CTL_DEL, fd, 0))
    {
        printf("epoll_ctl()close failed\n");
        exit(-1);
    }
    close(fd);
}

void Epoll::writefd(int fd)
{

}

std::vector<Channel*> Epoll:: loop(int timeout)
{
    std::vector<Channel*> channels;
    bzero(events_, sizeof(events_));
    int infds = epoll_wait(epollfd_, events_, MAX_EVENT, timeout); 
    if (infds < 0 )
    {
        printf( "epoll_wait() failure");
        exit(-1);
    }
    if (infds == 0 )
    {
        printf( "epoll_wait() timeout.\n");
        return channels;
    }
    for (int i = 0; i < infds; i++)
    {
        //evs.push_back(events_[i]);
        Channel *ch = (Channel*)events_[i].data.ptr;
        ch->setrevent(events_[i].events);
        channels.push_back(ch);
    }
    return channels;
}

