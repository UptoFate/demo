#include "Acceptor.h"

Acceptor::Acceptor(EventLoop *loop, const std::string &ip, const uint16_t port):loop_(loop)
{
    servsock_ = new Socket(createnonblocking());
    InetAddress serveraddress(ip,port);
    servsock_->setreuseaddr(true);
    servsock_->setreuseport(true);
    servsock_->setkeepalive(true);
    servsock_->settcpnodelay(true);
    servsock_->bind(serveraddress);
    servsock_->listen(200);

    acceptchannel_ = new Channel(loop_, servsock_->fd());
    acceptchannel_->setreadcallback(std::bind(&Acceptor::newconnection, this));
    acceptchannel_->enablereading();  //epoll 监视listenfd 的读事件，水平触发 

}

Acceptor::~Acceptor()
{
    delete servsock_;
    delete acceptchannel_;
}

void Acceptor::newconnection()
{
    InetAddress clientaddr;
    //客户端socket只能new出来，否则会被析构函数关闭fd
    Socket *clientsock=new Socket(servsock_->accept(clientaddr));

    //设置该句柄为边缘触发（数据没处理完后续不会再触发事件，水平触发是不管数据有没有触发都返回事件），
    // Channel *clientchannel = new Channel(loop_, clientsock->fd());
    // clientchannel->setreadcallback(std::bind(&Channel::onmessage, clientchannel));
    // //测试GET
    // //clientchannel->setreadcallback(std::bind(&Channel::read_client_request, clientchannel));
    // clientchannel->useet();
    // clientchannel->enablereading();

    // SSL *ssl = init_ssl("cert.pem", "key.pem", SSL_MODE_SERVER, clientsock->fd());   
    // SSL_shutdown(ssl);
    // SSL_free(ssl);

    //Connection* conn = new Connection(loop_, clientsock);
    clientsock->setipport(clientaddr.ip(), clientaddr.port());
    newconnectioncb_(clientsock);

    //userlist[fd_] = std::make_unique<User>(clientaddr.ip());
    if (clientsock->fd() >= Channel::userlist.size()) {
        Channel::userlist.resize(clientsock->fd() + 1);
    }

    Channel::userlist[clientsock->fd()] = new User(clientaddr.ip());
    printf("accept client(fd=%d,ip=%s,port=%d)ok\n", clientsock->fd(), clientaddr.ip(), clientaddr.port());
}  

void Acceptor::setnewconnectioncb(std::function<void(Socket*)> fn)
{
    newconnectioncb_ = fn;
}