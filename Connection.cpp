#include "Connection.h"

enum LOGIN{
    SUCCESS,
    PASERROR,
    NULLUSER,
    SQLERROR
};

Connection:: Connection(EventLoop *loop, Socket *clientsock):loop_(loop), clientsock_(clientsock)
{
    //设置该句柄为边缘触发（数据没处理完后续不会再触发事件，水平触发是不管数据有没有触发都返回事件），
    clientchannel_ = new Channel(loop_, clientsock_->fd());
    clientchannel_->setreadcallback(std::bind(&Connection::onmessage, this));
    clientchannel_->setclosecallback(std::bind(&Connection::closecallback, this));
    clientchannel_->seterrorcallback(std::bind(&Connection::errorcallback, this));
    //测试GET
    //clientchannel->setreadcallback(std::bind(&Channel::read_client_request, clientchannel));

    clientchannel_->useet();
    clientchannel_->enablereading();
}
Connection:: ~Connection()
{
    delete clientsock_;

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
    closecallback_(this);
}

void Connection::errorcallback()
{
    // printf("client(eventfd=%d) error.\n", fd());
    // close(fd());
    errorcallback_(this);
}

void Connection::setclosecallback(std::function<void(Connection*)> fn)
{
    closecallback_ = fn;
}

void Connection::seterrorcallback(std::function<void(Connection*)> fn)
{
    errorcallback_ = fn;
}

void Connection::onmessage()
{
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
        ssize_t nread = read (fd() , buf , sizeof (buf));   

        //成功读取到了数据。
        if (nread>0){
            //把接收到的报文内容原封不动的发回去。
            //printf ("recv(eventfd=%d):%s\n",fd_, buf);
            //SSL_write(ssl, buf, strlen(buf)+1);
            //send (fd(), buf, strlen(buf),0);
            //std::cout<<buf<<std::endl;
            inputbuffer_.append(buf, nread);

            EVP_PKEY* publicKey = loadPublicKey("public_key.pem");
            EVP_PKEY* privateKey = loadPrivateKey("private_key.pem");
            std::vector<unsigned char> str(buf, buf + nread); 
            //std::cout<<rsaDecrypt(privateKey,str)<<std::endl;
            Json::Reader reader;
            Json::FastWriter writer;
            Json::Value root;
            Json::Value data;
            std::string hashcode;
            //bool parsingSuccess = reader.parse(buf, root);
            // for(char i:str){
            //     printf("%02x", i);
            // }
            //std::cout<<std::endl;
            std::string decrypted = rsaDecrypt(privateKey,str);
            EVP_PKEY_free(publicKey);
            EVP_PKEY_free(privateKey);
            //std::cout<<decrypted<<std::endl;
            bool parsingSuccess = reader.parse(decrypted, root);
            if (!parsingSuccess) {
                std::cerr << "Failed to parse JSON string" << std::endl;
                errorcallback();
                if(Channel::userlist[fd()] != nullptr)free(Channel::userlist[fd()]);    //这个后续再改
            }
            std::cout<<root.toStyledString()<<std::endl;
            data = root["Data"];
            hashcode = root["HashCode"].asString();
            std::string toCalculate = writer.write(data);
            // std::cout<<"hashcode:"<< hashcode <<"\n toCalculate:" << toCalculate<<"[s]" << std::endl;
            // std::cout<<"TOCALCULATE:"<<sha256(toCalculate)<<std::endl;
            if(sha256(toCalculate) == hashcode)
            {
                std::cout<<"哈希值验证成功"<<std::endl;
            }
            else
            {
                std::cout<<"哈希值验证失败"<<std::endl;
                break;
            }
            std::string cmd = data["CMD"].asString();
            if (cmd == "LOGIN")
            {
                Channel::userlist[fd()]->getinfo(data["username"].asString(), data["password"].asString(), data["CpuID"].asString(), data["BiosID"].asString());
                //std::cout<<"username:"<<data["username"].toStyledString()<<" \npassword:"<<data["password"].toStyledString()<<std::endl;
                int validation =  Channel::userlist[fd()]->login() ;
                if(validation == SUCCESS)
                {                    
                    std::cout <<"登入成功"<<std::endl;
                    data["Validation"] = "SUCCESS";

                    if(Channel::userlist[fd()]->updete()){
                        std::cout <<"修改数据成功"<<std::endl;
                    }
                    else{
                        std::cout <<"修改数据失败"<<std::endl;
                        data["Validation"] = "MODFAIL";
                    }
                }
                else if(validation == PASERROR)
                {
                    std::cout<<"密码错误"<<std::endl;
                    data["Validation"] = "PASERROR";
                } 
                else if(validation == NULLUSER)
                {
                    std::cout<<"用户不存在"<<std::endl;
                    data["Validation"] = "NULLUSER";
                }
                else if(validation == SQLERROR)
                {
                    std::cout<<"SQLERROR"<<std::endl;
                    data["Validation"] = "SQLERROR";
                }
            }
            else
            {
                std::cout<<"unknow command"<<std::endl;
            }
            std::string style = data.toStyledString();
            //SSL_write(ssl, style.c_str(), strlen(style.c_str())+1);
            send (fd(), style.c_str(), strlen(style.c_str()),0);
            //std::cout << style << std::endl;
        }
        
        //读取数据的时候被信号中断，继续读取。
        else if (nread ==-1 && errno == EINTR )continue;
        //全部的数据已读取完毕。 
        else if (nread ==-1 &&(( errno == EAGAIN )||( errno == EWOULDBLOCK )))
        {   
            printf ("recv(eventfd=%d):%s\n",fd(), inputbuffer_.data());
            //经过运算...
            outputbuffer_ = inputbuffer_;
            inputbuffer_.clear();
            //send (fd(), outputbuffer_.data(), outputbuffer_.size(),0);      //暂时不要这么做
            break;
        }
        else if (nread ==0)//客户端连接已断开。
        {
            closecallback();
            printf (" client(eventfd=%d)disconnected.\n ", fd());
            //_close(fd_ );//关闭客户端的fd
            //if(Channel::userlist[fd()] != nullptr)free(Channel::userlist[fd()]);    //这个后续再改
            break ;
        }
    }
    //SSL_shutdown(ssl);
    //SSL_free(ssl);
}
