
#include "EchoServer.h"

enum LOGIN{
    SUCCESS,
    PASERROR,
    NULLUSER,
    SQLERROR,
    HASHERROR,
    UNKNOWCMD
};

EchoServer::EchoServer(const std::string &ip, const uint16_t port,  int subthreadnum, int workthreadnum)
    :tcpserver_(ip,port,subthreadnum),threadpool_(workthreadnum,"WORKS")
{
    tcpserver_.setnewconnectioncb(std::bind(&EchoServer::HandleNewConnection, this, std::placeholders::_1));
    tcpserver_.setclosecohnectioncb(std::bind(&EchoServer::HandleClose, this, std::placeholders::_1));
    tcpserver_.seterrorconnectioncb(std::bind(&EchoServer::HandleError, this, std::placeholders::_1));
    tcpserver_.setonmessagecb(std::bind(&EchoServer::HandleMessage, this, std::placeholders::_1, std::placeholders::_2));
    tcpserver_.setsendcompletecb(std::bind(&EchoServer::HandleSendComplete, this, std::placeholders::_1));
    tcpserver_.settimeoutcb(std::bind(&EchoServer::HandleTimeOut, this, std::placeholders::_1));
}

EchoServer::~EchoServer()
{

}

void EchoServer::start()
{
    tcpserver_.start();
}

void EchoServer::HandleNewConnection(spConnection conn)
{
    std::cout<<"New Connection "<<std::endl;
    printf("(fd=%d,ip=%s,port=%d)ok\n", conn->fd(), conn->ip(), conn->port());
}

void EchoServer::HandleClose(spConnection conn)
{
    printf (" client(eventfd=%d)disconnected.\n ", conn->fd());
} 

void EchoServer::HandleError(spConnection conn)
{
    
}

void EchoServer::Login(spConnection conn, std::string& message)
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
    //std::cout<<root.toStyledString()<<std::endl;
    data = root["Data"];
    hashcode = root["HashCode"].asString();
    std::string toCalculate = writer.write(data);
    //std::cout<<"hashcode:"<< hashcode <<"\n toCalculate:" << toCalculate<<"[s]" << std::endl;
    //std::cout<<"TOCALCULATE:"<<sha256(toCalculate)<<std::endl;
    if(sha256(toCalculate) == hashcode)
    {
        std::cout<<"哈希值验证成功"<<std::endl;
    }
    else
    {
        std::cout<<"哈希值验证失败"<<std::endl;
        conn->errorcallback();              //发生段错误
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
    Json::Value toSend(Json::objectValue);
    toSend["Data"] = data;
    toSend["HashCode"] = Json::String(sha256(writer.write(data)));
    //std::cout<<writer.write(data)<<std::endl;
    std::string style = toSend.toStyledString();
    std::cout<<"style:"<<style<<std::endl;
                // std::string aesKeyStr = root["AESKEY"].asString();
                // std::string ivStr = root["IV"].asString();
                // std::cout << "AESKEY: " << aesKeyStr << " (length: " << aesKeyStr.size() << ")\n";
                // std::cout << "IV: " << ivStr << " (length: " << ivStr.size() << ")\n";
                // if (aesKeyStr.size() != 32 || ivStr.size() != 16) {
                //     throw std::runtime_error("AES key must be 32 bytes and IV must be 16 bytes long");
                // }

                // const unsigned char* key = reinterpret_cast<const unsigned char*>(aesKeyStr.data());
                // const unsigned char* iv = reinterpret_cast<const unsigned char*>(ivStr.data());

                // std::vector<unsigned char> ciphertext = aes_encrypt(key, iv, style);

                // // 打印加密后的密文
                // for (unsigned char c : ciphertext) {
                //     std::cout << std::hex << static_cast<int>(c);
                // }
                // std::cout << std::endl;
    


                //const char* tosend = reinterpret_cast<const char*>(ciphertext.data());
    //SSL_write(ssl, style.c_str(), strlen(style.c_str())+1);

    send (conn->fd(), style.c_str(), style.size(),0);
    //std::cout << "\n\n" << std::endl;

    //conn->send(style.c_str(), strlen(style.c_str()));
    //std::cout << style << std::endl;
}

void EchoServer::HandleMessage(spConnection conn, std::string& message)
{
    if(threadpool_.size()==0)
    {
        //如果没有工作线程，直接在IO线程中计算
    }
    else
    {
        threadpool_.addtask(std::bind(&EchoServer::Login, this, conn, message));
    }
}

void EchoServer::HandleSendComplete(spConnection conn)
{

}

void EchoServer::HandleTimeOut(EventLoop*loop)
{

}
