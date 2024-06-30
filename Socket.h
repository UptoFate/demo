#ifndef __MY_SOCKET__
#define __MY_SOCKET__

#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <string.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include "InetAddress.h"

class Socket
{
private:
    const int m_fd;
    std::string ip_;
    uint16_t port_;
public:
    Socket(int fd);
    ~Socket();
    int fd() const;     //返回fd成员
    std::string ip() const;
    uint16_t port() const;
    void setreuseaddr(bool on);         //设置SO_REUSEADDR
    void setreuseport(bool on);         //设置SO_REUSEPORT
    void settcpnodelay(bool on);        //设置TCP_NODELAY
    void setkeepalive(bool on);         //设置KEEPALIVE
    void bind(const InetAddress);
    void listen(int n=200);
    int accept(InetAddress& m_clientaddr); 

};

int createnonblocking();

#endif