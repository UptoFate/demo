//epoll模型实现网络通信服务端
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <cassert>
#include <sys/types.h>
#include <sys/stat.h>
#include <iostream>
#include <netinet/tcp.h>
#include <error.h>
#include "InetAddress.h"
#include "Socket.h"
#include <json/json.h>
#include "Epoll.h"

const int MAX_EVENT_NUMBER = 10000; //最大事件数

int main(int argc, char* argv[]){

    if(argc!=3){
        printf("tcpepoll: ip port\n");
        return -1; 
    }

    Socket serversock(createnonblocking());
    InetAddress serveraddress(argv[1],atoi(argv[2]));
    serversock.setreuseaddr(true);
    serversock.setreuseport(true);
    serversock.setkeepalive(true);
    serversock.settcpnodelay(true);
    serversock.bind(serveraddress);
    serversock.listen(200);

    Epoll ep;
    //ep.addfd(serversock.fd(), EPOLLIN | EPOLLET | EPOLLRDHUP);      //epoll 监视listenfd 的读事件，水平触发
    Channel *servchannel = new Channel(&ep, serversock.fd());
    servchannel->enablereading();

    //进入服务器循环
    while(1)
    {
        std::vector<Channel*> channels = ep.loop();           //存放epoll_wait()返回的事件
        for(auto &ch:channels)
        {
            //连接中断
            if (ch->revents() & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))
            {
                printf("client(eventfd=%d) disconnected.\n",ch->fd());
                close(ch->fd());
            }
            //缓冲区有数据可读 
            else if (ch->revents() & EPOLLIN|EPOLLPRI)
            {
                // 属于处理新到的客户连接 // listen事件——有新客户端连接
                if (ch->fd() == serversock.fd()) //ch==servchannel
                {
                    InetAddress clientaddr;
                    //客户端socket只能new出来，否则会被析构函数关闭fd
                    //还未释放
                    Socket *clientsock=new Socket(serversock.accept(clientaddr));

                    //设置该句柄为边缘触发（数据没处理完后续不会再触发事件，水平触发是不管数据有没有触发都返回事件），
                    Channel *clientchannel = new Channel(&ep, clientsock->fd());
                    clientchannel->useet();
                    clientchannel->enablereading();
                    printf("accept client(fd=%d,ip=%s,port=%d)ok\n", clientsock->fd(), clientaddr.ip(), clientaddr.port());
                } 

                // 处理客户连接上接收到的数据
                else{
                    char buf[1024]={0};// 使用非阻塞I/O，每次读取buffer大小数据直到读完
                    
                    while (true){
                        bzero (&buf, sizeof(buf));
                        ssize_t nread = read (ch->fd() , buf , sizeof (buf));
                        
                        //成功的读取到了数据。
                        if (nread>0){
                            //把接收到的报文内容原封不动的发回去。
                            printf ("recv(eventfd=%d):%s\n", ch->fd(), buf);
                            send (ch->fd(), buf, strlen(buf),0);
                        }
                        //读取数据的时候被信号中断，继续读取。
                        else if (nread ==-1 && errno == EINTR )continue;
                        //全部的数据已读取完毕。 
                        else if (nread ==-1 &&(( errno == EAGAIN )||( errno == EWOULDBLOCK )))break;

                        else if (nread ==0)//客户端连接已断开。
                        {
                            printf (" client(eventfd=%d)disconnected.\n ", ch->fd());
                            close ( ch->fd() );//关闭客户端的fd
                            break ;
                        }
                    }
                }
            }
            //写事件
            /*
            else if (ch->fd() & EPOLLOUT)
            {
                std::string response = "server response \n";
                write(sockfd,response.c_str(),response.length());

                // 将事件设置为读事件，继续监听客户端
                ch->fd() = sockfd;
                ev.events = EPOLLIN | EPOLLRDHUP;
                epoll_ctl(m_epollfd, EPOLL_CTL_MOD, sockfd, &events[i]);
            }
            */
            //else if 可以加管道，unix套接字等等数据
        }
    }


}


