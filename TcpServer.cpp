#include "TcpServer.h"

enum LOGIN{
    SUCCESS,
    PASERROR,
    NULLUSER,
    SQLERROR,
    HASHERROR,
    UNKNOWCMD
};

TcpServer::TcpServer(const std::string &ip, const uint16_t port):acceptor_(new Acceptor(&loop_, ip, port))
{
    acceptor_->setnewconnectioncb(std::bind(&TcpServer::newconnection, this, std::placeholders::_1));
    loop_.setepolltimeoutcallback(std::bind(&TcpServer::epolltimeout, this, std::placeholders::_1));
}

TcpServer::~TcpServer()
{
    delete acceptor_;
    for(auto i:conns_){
        delete i.second;
    }
}

void TcpServer::start()
{
    loop_.run();
}
void TcpServer::newconnection(Socket* clientsock)
{
    Connection* conn = new Connection(&loop_, clientsock);  //还未释放
    conn->setclosecallback(std::bind(&TcpServer::closeconnection,this,std::placeholders::_1));
    conn->seterrorcallback(std::bind(&TcpServer::errorconnection,this,std::placeholders::_1));
    conn->setonmessagercallback(std::bind(&TcpServer::onmessage, this, std::placeholders::_1, std::placeholders::_2));
    conn->sendcompletecallback(std::bind(&TcpServer::sendcmplete,this,std::placeholders::_1));
    printf("newconnection(fd=%d,ip=%s,port=%d)ok\n", conn->fd(), conn->ip().c_str(), conn->port());
    conns_ [conn->fd()] = conn;
}

void TcpServer::closeconnection(Connection *conn)
{
    //printf("Client(eventfd=%d) disconnected.\n", conn->fd());
    conns_.erase(conn->fd());   //conn里会关fd
    delete conn;
}

void TcpServer::errorconnection(Connection *conn)
{
    printf("client(eventfd=%d) error.\n", conn->fd());
    conns_.erase(conn->fd());   //conn里会关fd
    delete conn;
}

void TcpServer::onmessage(Connection *conn, std::string message)
{
    EVP_PKEY* publicKey = loadPublicKey("public_key.pem");
    EVP_PKEY* privateKey = loadPrivateKey("private_key.pem");
    std::vector<unsigned char> str(message.begin(), message.end()); 
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
        conn->errorcallback();
        return;
        //if(Channel::userlist[conn->fd()] != nullptr)free(Channel::userlist[conn->fd()]);    //这个后续再改
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
        conn->errorcallback();
        return;
    }
    std::string cmd = data["CMD"].asString();
    if (cmd == "LOGIN")
    {
        Channel::userlist[conn->fd()]->getinfo(data["username"].asString(), data["password"].asString(), data["CpuID"].asString(), data["BiosID"].asString());
        //std::cout<<"username:"<<data["username"].toStyledString()<<" \npassword:"<<data["password"].toStyledString()<<std::endl;
        int validation =  Channel::userlist[conn->fd()]->login() ;
        if(validation == SUCCESS)
        {                    
            std::cout <<"登入成功"<<std::endl;
            data["Validation"] = "SUCCESS";

            if(Channel::userlist[conn->fd()]->updete()){
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
        data["Validation"] = "UNKNOWCMD";
    }
    std::string style = data.toStyledString();
    //SSL_write(ssl, style.c_str(), strlen(style.c_str())+1);
    send (conn->fd(), style.c_str(), strlen(style.c_str()),0);
    //conn->send(style.c_str(), strlen(style.c_str()));
    //std::cout << style << std::endl;
}

 void TcpServer::sendcmplete(Connection *conn)
 {
    printf("send complete.\n");
 }

 void TcpServer::epolltimeout(EventLoop*loop){
    printf("epoll_wait() timeout\n");
 }
 