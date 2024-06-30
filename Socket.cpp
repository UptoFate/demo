#include "Socket.h"

int createnonblocking(){
    int listenfd = socket(AF_INET, SOCK_STREAM|SOCK_NONBLOCK, IPPROTO_TCP);
    if(listenfd<0)
    {
        printf("%s:%s:%d listen socket create error:%d\n", __FILE__, __FUNCTION__, __LINE__, errno);
        exit(-1);
    }
    return listenfd;
}

Socket::Socket(int fd):m_fd(fd)
{

}

Socket::~Socket()
{
    ::close(m_fd);
}

int Socket:: fd() const
{
    return m_fd;
}   

std::string Socket::ip() const
{
    return ip_;
}   

uint16_t Socket::port() const
{
    return port_;
}   

void Socket:: setreuseaddr(bool on)
{
    int optval = on?1 : 0;
    ::setsockopt(m_fd,SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
}

void Socket:: setreuseport(bool on)
{
    int optval = on?1 : 0;
    ::setsockopt(m_fd,SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
}     

void Socket:: settcpnodelay(bool on)
{
    int optval = on?1 : 0;
    ::setsockopt(m_fd, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
}      

void Socket:: setkeepalive(bool on)
{
    int optval = on?1 : 0;
    ::setsockopt(m_fd,SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
}       

void Socket:: bind(const InetAddress serveraddr)
{
    int ret = ::bind(m_fd, serveraddr.addr(), sizeof(sockaddr));
    if(ret<0)
    {
        printf("fail to bind!,errno :%d",errno);
        exit(-1);
    }
    setipport(serveraddr.ip(), serveraddr.port());
    }

void Socket:: setipport(const std::string& ip, uint16_t port)
{
    ip_=ip;
    port_=port;
}

void Socket:: listen(int n)
{
    int ret = ::listen(m_fd, n);
    if(ret<0)
    {
        printf("fail to listen!,errno :%d",errno);
        exit(-1);
    }
}

int Socket:: accept(InetAddress& m_clientaddr )
{
    sockaddr_in peeraddr;
    socklen_t client_addrlength = sizeof(peeraddr);
    int clientfd = ::accept4(m_fd, (sockaddr*)&peeraddr, &client_addrlength,SOCK_NONBLOCK);
    m_clientaddr.setaddr(peeraddr);

    return clientfd;
}