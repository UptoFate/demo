#ifndef __CONNECTION__
#define __CONNECTION__

#include <functional>
#include "Socket.h"
#include "InetAddress.h"
#include "EventLoop.h"
#include "Channel.h"

class Channel;
class EventLoop;

class Connection
{
private:
    EventLoop *loop_;
    Socket *clientsock_;
    Channel *clientchannel_;
public:
    Connection(EventLoop *loop, Socket *clientsock);
    ~Connection();
};

#endif