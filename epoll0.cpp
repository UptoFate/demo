//epoll模型实现网络通信服务端（初稿）
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <cassert>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <iostream>
#include <netinet/tcp.h>
#include <error.h>
#include "InetAddress.h"
#include "Socket.h"

const int MAX_EVENT_NUMBER = 10000; //最大事件数

int main(int argc, char* argv[]){

    if(argc!=3){
        printf("tcpepoll: ip port\n");
        return -1; 
    }

    // 创建监听套接字 // 直接为非阻塞
    int m_listenfd = socket(AF_INET, SOCK_STREAM|SOCK_NONBLOCK, 0);
    if(m_listenfd<0)
    {
        perror("socket():");return -1;
    }
    
    // 设置ip可重用 //设置listenfd属性
    int opt = 1;
    setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    setsockopt(m_listenfd, SOL_SOCKET, TCP_NODELAY, &opt, sizeof(opt));

    InetAddress serveraddress(argv[1],atoi(argv[2]));

    // 绑定端口号
    int ret = bind(m_listenfd, serveraddress.addr(), sizeof(sockaddr));
    if(ret<0)
    {
        printf("fail to bind!,errno :%d",errno);
        return ret;
    }

    // 监听连接fd
    ret = listen(m_listenfd, 200);
    if(ret<0)
    {
        printf("fail to listen!,errno :%d",errno);
        return ret;
    }

    // 初始化红黑树和事件链表结构rdlist结构
    epoll_event events[MAX_EVENT_NUMBER];
    // 创建epoll实例
    int m_epollfd = epoll_create(5);
    if(m_epollfd==-1)
    {
        printf("fail to epoll create!");
        return m_epollfd;
    }

    // 创建节点结构体将监听连接句柄
    epoll_event event;
    event.data.fd = m_listenfd;
    //设置该句柄为边缘触发（数据没处理完后续不会再触发事件，水平触发是不管数据有没有触发都返回事件），
    event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    // 添加监听连接句柄作为初始节点进入红黑树结构中，该节点后续处理连接的句柄
    epoll_ctl(m_epollfd, EPOLL_CTL_ADD, m_listenfd, &event);

    //进入服务器循环
    while(1)
    {
        int infds = epoll_wait(m_epollfd, events, MAX_EVENT_NUMBER, -1); // -1表示不会超时
        if (infds < 0 && errno != EINTR)
        {
            printf( "epoll failure");break;
        }
        if (infds == 0 )
        {
            printf( "epoll_wait() timeout.\n");break;
        }
        for (int i = 0; i < infds; i++)
        {
            int sockfd = events[i].data.fd;
            //连接中断
            if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))
            {
                //服务器端关闭连接，
                epoll_ctl(m_epollfd, EPOLL_CTL_DEL, sockfd, 0);
                close(sockfd);
            }
            //缓冲区有数据可读 
            else if (events[i].events & EPOLLIN|EPOLLPRI)
            {
                // 属于处理新到的客户连接 // listen事件——有新客户端连接
                if (sockfd == m_listenfd) 
                {
                    sockaddr_in peeraddr;
                    socklen_t client_addrlength = sizeof(peeraddr);
                    int connfd = accept4(m_listenfd, (struct sockaddr *)&peeraddr, &client_addrlength,SOCK_NONBLOCK);
                    InetAddress clientaddr(peeraddr);
                    
                    if (connfd < 0)
                    {
                        printf("errno is:%d accept error", errno);
                        return false;
                    }
                    epoll_event event;
                    event.data.fd = connfd;

                    //设置该句柄为边缘触发（数据没处理完后续不会再触发事件，水平触发是不管数据有没有触发都返回事件），
                    event.events = EPOLLIN | EPOLLET;

                    // 添加监听连接句柄作为初始节点进入红黑树结构中，该节点后续处理连接的句柄
                    epoll_ctl(m_epollfd, EPOLL_CTL_ADD, connfd, &event);

                    printf("accept client(fd=%d,ip=%s,port=%d)ok\n", connfd, clientaddr.ip(), clientaddr.port());
                } 

                // 处理客户连接上接收到的数据
                else{
                    char buf[1024]={0};// 使用非阻塞I/O，每次读取buffer大小数据直到读完
                    
                    while ( true ){
                        bzero (&buf, sizeof(buf));
                        ssize_t nread = read (events[i].data.fd , buf , sizeof (buf));
                        
                        //成功的读取到了数据。
                        if (nread>0){
                            //把接收到的报文内容原封不动的发回去。
                            printf ("recv(eventfd=%d):%s\n", events[i].data.fd, buf);
                            send (events[i].data.fd, buf, strlen(buf),0);
                        }
                        //读取数据的时候被信号中断，继续读取。
                        else if (nread ==-1 && errno == EINTR )continue;
                        //全部的数据已读取完毕。 
                        else if (nread ==-1 &&(( errno == EAGAIN )||( errno == EWOULDBLOCK )))break;

                        else if (nread ==0)//客户端连接已断开。
                        {
                            printf (" client(eventfd=%d)disconnected.\n ", events[i].data.fd);
                            close ( events[i].data.fd );//关闭客户端的fd
                            break ;
                        }

                    }
                    // 将事件设置为写事件返回数据给客户端?
                    //events[i].data.fd = sockfd;
                    //events[i].events = EPOLLOUT | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
                    //epoll_ctl(m_epollfd, EPOLL_CTL_MOD, sockfd, &events[i]);
                }
            }
            //写事件
            else if (events[i].events & EPOLLOUT)
            {
                std::string response = "server response \n";
                write(sockfd,response.c_str(),response.length());

                // 将事件设置为读事件，继续监听客户端
                events[i].data.fd = sockfd;
                events[i].events = EPOLLIN | EPOLLRDHUP;
                epoll_ctl(m_epollfd, EPOLL_CTL_MOD, sockfd, &events[i]);
            }
            //else if 可以加管道，unix套接字等等数据
        }
    }


}


