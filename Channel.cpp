#include "Channel.h"

Channel::Channel(Epoll*ep, int fd, bool islisten):ep_(ep),fd_(fd), islisten_(islisten)
{

}

Channel::~Channel()
{
    //ep_与fd_不属于Channel类，只是使用而已不能在这delete
}

int Channel::fd()
{
    return fd_;
}      

void Channel::useet()
{
    events_ = events_|EPOLLET;
}                  

void Channel::enablereading()
{
    events_ = events_|EPOLLIN;
    ep_->updateChannel(this);
}   

void Channel::setinepoll()
{
    inepoll_ = true;
}       

void Channel::setrevent(uint32_t event)
{
    revents_ = event;
}       

bool Channel::inepoll()
{
    return inepoll_;
}

uint32_t Channel::events()
{
    return events_;
}

uint32_t Channel::revents()
{
    return revents_;
}

void Channel::handleevent(Socket* serversock)
{
    //连接中断
    if (revents_ & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))
    {
        printf("client(eventfd=%d) disconnected.\n", fd_);
        close(fd_);
    }
    //缓冲区有数据可读 
    else if (revents_ & EPOLLIN|EPOLLPRI)
    {
        // 属于处理新到的客户连接 // listen事件——有新客户端连接
        if (islisten_) //ch==servchannel
        {
            InetAddress clientaddr;
            //客户端socket只能new出来，否则会被析构函数关闭fd
            //还未释放
            Socket *clientsock=new Socket(serversock->accept(clientaddr));

            //设置该句柄为边缘触发（数据没处理完后续不会再触发事件，水平触发是不管数据有没有触发都返回事件），
            Channel *clientchannel = new Channel(ep_, clientsock->fd(), false);
            clientchannel->useet();
            clientchannel->enablereading();
            printf("accept client(fd=%d,ip=%s,port=%d)ok\n", clientsock->fd(), clientaddr.ip(), clientaddr.port());
        } 

        // 处理客户连接上接收到的数据
        else{
            char buf[1024]={0};// 使用非阻塞I/O，每次读取buffer大小数据直到读完
            
            while (true){
                bzero (&buf, sizeof(buf));
                ssize_t nread = read (fd_ , buf , sizeof (buf));
                
                //成功的读取到了数据。
                if (nread>0){
                    //把接收到的报文内容原封不动的发回去。
                    //printf ("recv(eventfd=%d):%s\n",fd_, buf);
                    send (fd_, buf, strlen(buf),0);
                    Json::Reader reader;
                    Json::Value root;
                    bool parsingSuccess = reader.parse(buf, root);
                        if (!parsingSuccess) {
                        std::cerr << "Failed to parse JSON string" << std::endl;
                        break;
                    }
                    std::string cmd = root["CMD"].asString();
                    if (cmd == "LOGIN")
                    {
                        User usr(root["username"].asString(), root["password"].asString());
                        std::cout<<"username:"<<root["username"].toStyledString()<<" \npassword:"<<root["password"].toStyledString()<<std::endl;
                        if(usr.login())
                        {
                            std::cout <<"登入成功"<<std::endl;
                        }
                        else
                        {
                            std::cout<<"登入失败"<<std::endl;
                            break;
                        } 
                    }
                    else
                    {
                        std::cout<<"unknow command"<<std::endl;
                        break;
                    }

                    std::cout<<root.toStyledString()<<std::endl;
                }
                
                //读取数据的时候被信号中断，继续读取。
                else if (nread ==-1 && errno == EINTR )continue;
                //全部的数据已读取完毕。 
                else if (nread ==-1 &&(( errno == EAGAIN )||( errno == EWOULDBLOCK )))break;

                else if (nread ==0)//客户端连接已断开。
                {
                    printf (" client(eventfd=%d)disconnected.\n ", fd_);
                    close (fd_ );//关闭客户端的fd
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