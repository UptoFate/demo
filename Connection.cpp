#include "Connection.h"

enum LOGIN{
    SUCCESS,
    PASERROR,
    NULLUSER,
    SQLERROR
};

Connection:: Connection(EventLoop *loop, std::unique_ptr<Socket> clientsock)
    :loop_(loop), clientsock_(std::move(clientsock)),disconnect_(false),clientchannel_(new Channel(loop_, clientsock_->fd()))
{
    //设置该句柄为边缘触发（数据没处理完后续不会再触发事件，水平触发是不管数据有没有触发都返回事件），
    //clientchannel_ = new Channel(loop_, clientsock_->fd());
    clientchannel_->setreadcallback(std::bind(&Connection::onmessage, this));
    clientchannel_->setclosecallback(std::bind(&Connection::closecallback, this));
    clientchannel_->seterrorcallback(std::bind(&Connection::errorcallback, this));
    clientchannel_->setwritecallback(std::bind(&Connection::writecallback, this));
    //测试GET
    //clientchannel->setreadcallback(std::bind(&Channel::read_client_request, clientchannel));

    clientchannel_->useet();
    clientchannel_->enablereading();
}
Connection:: ~Connection()
{
    //delete clientsock_;

    //delete clientchannel_;        //会发生段错误
}

int Connection::fd() const
{
    return clientsock_->fd();
}
std::string Connection::ip() const
{
    return clientsock_->ip();
}
uint16_t Connection::port() const
{
    return clientsock_->port();
}

void Connection::closecallback()
{
    // printf("client(eventfd=%d) disconnected.\n", fd());
    // close(fd());
    clientchannel_->remove();
    disconnect_=true;
    closecallback_(shared_from_this());
}

void Connection::errorcallback()
{
    // printf("client(eventfd=%d) error.\n", fd());
    // close(fd());
    clientchannel_->remove();
    disconnect_=true;
    errorcallback_(shared_from_this());
}

void Connection::setclosecallback(std::function<void(spConnection)> fn)
{
    closecallback_ = fn;
}

void Connection::seterrorcallback(std::function<void(spConnection)> fn)
{
    errorcallback_ = fn;
}

void Connection::setonmessagercallback(std::function<void(spConnection, std::string&)> fn)
{
    onmessagecallback_ = fn;
}

void Connection::sendcompletecallback(std::function<void(spConnection)> fn)
{
    sendcompletecallback_= fn;
}

void Connection::onmessage()
{
    //把内核中的数据先拷贝到临时变量中再拷贝到缓冲区inputbuffer_中
    char buf[1024]={0};// 使用非阻塞I/O，每次读取buffer大小数据直到读完
    //SSL *ssl = init_ssl("./myssl/cert.pem", "./myssl/key.pem", SSL_MODE_SERVER, fd_);   
    while (true){

        bzero (&buf, sizeof(buf));
        //ssize_t nread = SSL_read (ssl , buf , sizeof (buf));
        //*********************************************/     
        //*1.为解决粘包和分包，可设置固定大小包在第三个参数
        //*2.也可先读取指定大小头部获取报文大小
        //*3.像http协议一样报文间使用/r/n/r/n分隔符
        //**********************************************/
        ssize_t nread = read(fd() , buf , sizeof (buf));   

        //成功读取到了数据。
        if (nread>0){
            //把接收到的报文内容原封不动的发回去。
            //printf ("recv(eventfd=%d):%s\n",fd_, buf);
            //SSL_write(ssl, buf, strlen(buf)+1);
            //send (fd(), buf, strlen(buf),0);
            //std::cout<<buf<<std::endl;
            inputbuffer_.append(buf, nread);
        }
        //读取数据的时候被信号中断，继续读取。
        else if (nread ==-1 && errno == EINTR )continue;
        //全部的数据已读取完毕。 
        else if (nread ==-1 &&(( errno == EAGAIN )||( errno == EWOULDBLOCK )))
        {   
            //if(inputbuffer_.size())printf ("recv(eventfd=%d):%s\n",fd(), inputbuffer_.data());
            //std::string message(inputbuffer_.data(),inputbuffer_.size());
            std::string message;
            while (inputbuffer_.pickmessage(message))
            {
                lastatime_=Timestamp::now();         //更新时间戳
                onmessagecallback_(shared_from_this(),message);
            }

            //if(message.size()>0)onmessagecallback_(shared_from_this(),message);
            //outputbuffer_ = inputbuffer_;
            //inputbuffer_.clear();
            
            //lastatime_ = Timestamp::now();      //更新时间戳

            //onmessagecallback_(shared_from_this(),message);
            break;
        }
        else if (nread ==0)//客户端连接已断开。
        {   
            //printf (" client(eventfd=%d)disconnected.\n ", fd());
            closecallback();
            //_close(fd_ );//关闭客户端的fd
            break ;
        }
    }
    //SSL_shutdown(ssl);
    //SSL_free(ssl);
}

void Connection::send(const char*data, size_t size)
{
    if(disconnect_==true)return;

    //因为数据发送要给其他线程处理，将它包装成智能指针；
    std::shared_ptr<std::string> message(new std::string(data));
    if(loop_->isinloopthread())     //判断当前线程是否为IO线程
    {
        //直接发送
        sendinloop(message,size);
    }
    else
    {
        //将sendinloop放入任务队列，用eventfd唤醒IO线程
        loop_->queueinloop(std::bind(&Connection::sendinloop,this,message,size));
        //由于添加完后立即返回，data会被释放，得用智能指针
    }

}

void Connection::sendinloop(std::shared_ptr<std::string> data, size_t size)
{
    //outputbuffer_.append(data->data(), data->size());
    outputbuffer_.appendwithsep(data->data(),size);
    //注册写事件
    //std::string s(data->data(), data->size());
    //std::cout <<"send:" <<s <<"size:"<<data->size()<<std::endl;
    clientchannel_->enablewriting();
}


void Connection::writecallback()
{
    int writen=::send(fd(), outputbuffer_.data(), outputbuffer_.size(), 0);
    if(writen>0)outputbuffer_.eraser(0,writen);
    //发送缓冲区没有数据不再关注写事件
    if(outputbuffer_.size()==0)
    {
        clientchannel_->disablewriting();
        sendcompletecallback_(shared_from_this());
    }
}

bool Connection::timeout(time_t now,int val)
{
    return now-lastatime_.toint()>val;
}