#ifndef __CHANNEL__
#define __CHANNEL__
#include <json/json.h>
#include <iostream>
#include <functional>
#include <sys/stat.h>
#include <fcntl.h>
#include <vector>
#include "Epoll.h"
#include "InetAddress.h"
#include "Socket.h"
#include "User.h"


class Epoll;

//如果一个服务器同时提供不同的服务，如HTTP、FTP等，那么就算文件描述符上发生的事件都是可读事件，
//不同的连接类型也将决定不同的处理逻辑，仅仅通过一个文件描述符来区分显然会很麻烦，我们更加希望拿到关于这个文件描述符更多的信息。
class Channel
{
private:
    int fd_ = -1;
    Epoll*ep_ = nullptr;         //Channel对应的红黑树，Channel与Epoll是多对一
    bool inepoll_ = false;       //Channel是否已添加到epoll树上，若未则调用epoll_ctl()时使用EPOLL_CTL_ADD,否则用.._MOD
    uint32_t events_ = 0;        //fd_需要监视的事件：listenfd和clientfd需要监视EPOLLIN，clientfd还可能要EPOLLOUT
    uint32_t revents_ = 0;       //fd_已发生事件
    std::function<void()> readcallback_;    //fd_读事件的回调函数
    //std::vector<std::unique_ptr<User>> userlist;
    void _close(int fd);
public:
    static std::vector<User*> userlist;
    Channel(Epoll*ep, int fd);
    ~Channel();
    int fd();                               //返回fd_
    void useet();                           //采用边缘触发
    void enablereading();                   //让epoll_wait()监视fd_的读事件
    void setinepoll();                      //设置inepoll_
    void setrevent(uint32_t events);         //设置inepoll_
    bool inepoll();
    uint32_t events();
    uint32_t revents();

    void handleevent();                     //处理循环事件
    void newconnection(Socket* servsock);                   //处理新客户端连接请求
    void onmessage();                                       //处理对端报文
    void setreadcallback(std::function<void()> fn);         //设置fd_读事件的回调函数

    //测试websocket用
    //bool isHTTPRequest();
    void read_client_request(); 
    void send_header(int code, char* info, char* filetype, int length);  
    void send_file(char* path); 
};

bool readline(int fd, char buf[], size_t buf_size);

#endif