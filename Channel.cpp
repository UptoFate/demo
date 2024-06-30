#include "Channel.h"

#define SUCCESS 0
#define PASERROR 1
#define NULLUSER 2
 #define SQLERROR 3

std::vector<User*> Channel::userlist;

bool readline(int fd, char buf[], size_t buf_size) {
    char ch;
    ssize_t bytes_read;
    size_t i = 0;

    // 循环读取每个字符，直到读取到换行符或者缓冲区已满
    while ((bytes_read = read(fd, &ch, 1)) > 0) {
        if (ch == '\n') {
            buf[i] = '\0'; // 添加字符串结束符
            return true;
        }
        buf[i++] = ch; // 将字符存储到缓冲区中
        if (i >= buf_size - 1) {
            // 缓冲区已满
            buf[i] = '\0'; // 添加字符串结束符
            return false;
        }
    }

    // 如果读取失败或者文件已经结束
    if (bytes_read == 0 && i == 0) {
        // 文件已经结束
        return false;
    }

    // 读取失败
    return false;
}

Channel::Channel(EventLoop*loop, int fd):loop_(loop),fd_(fd)
{

}

Channel::~Channel()
{
    //ep_与fd_不属于Channel类，只是使用而已不能在这delete
}

void Channel::_close(int fd)
{
    loop_->closefd(fd_);
    if(Channel::userlist[fd] != nullptr)free(Channel::userlist[fd]);
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
    loop_->updateChannel(this);
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

void Channel::read_client_request()
{
    char buf[1024] = "";
    char tmp[1024] = "";
    //web实现：先读取一行，再把其他行读取扔掉
    int n = readline(fd_, buf, sizeof(buf));
    if(n <= 0)
    {
        printf ("close or err client(eventfd=%d)disconnected.\n ", fd_);
        _close(fd_);
        return;
    }
    printf("[%s]\n", buf);
    int ret = 0;
    while ((ret = readline(fd_, tmp, sizeof(tmp))));

    //解析请求行 GET /a.txt HTTP/1.1\R\name
    char method[256] = "";
    char content[256] = "";
    char protocol[256] = "";
    sscanf(buf,"%[^ ] %[^ ] %[^ \r\n]",method, content, protocol);

    //判断是否为get请求
    if(strcasecmp(method, "get") == 0)
    {
        char *strfile = content + 1;

        //切换目录
        char pwd_path[256] = "";
        char* path = getenv("PWD");
        strcpy(pwd_path, path);
        strcat(pwd_path, "/web-http");
        chdir(pwd_path);
        //判断文件是否存在
        if(*strfile == 0)strfile = (char*)"./";
        struct stat s;
        char* type = (char*)"text/html; charset=utf-8\r\n";
        if(stat(strfile, &s) < 0)
        {
            printf("file not fount\n");
            //先发送报头（状态行 消息头 空行）再发送文件 error.html
            //要写一个get_mime_type()函数判断文件类型
            send_header(404, (char*)"NOT FOUND", type, 0);
            send_file((char*)"error.html");
        }
        else
        {
            //请求为普通文件
            if(S_ISREG(s.st_mode))
            {   
                std::cout << "file" << std::endl;
                //先发送报头（状态行 消息头 空行）再发送文件
                send_header(200, (char*)"OK", type, s.st_size);
                send_file(strfile);
            }
            //请求为目录
            else if(S_ISDIR(s.st_mode))std::cout << "dir" << std::endl;
        }

    }
}

void Channel::handleevent()
{
    //连接中断
    if (revents_ & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))
    {
        closecallback_();
        if(Channel::userlist[fd_] != nullptr)free(Channel::userlist[fd_]);    //这个后续再改
    }
    //缓冲区有数据可读 
    else if (revents_ & EPOLLIN|EPOLLPRI)
    {
       readcallback_();     //采用回调函数实现以上操作
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
    else
    {
        errorcallback_();
        if(Channel::userlist[fd_] != nullptr)free(Channel::userlist[fd_]);    //这个后续再改
    }
} 

void Channel::onmessage()
{
    char buf[1024]={0};// 使用非阻塞I/O，每次读取buffer大小数据直到读完
    //SSL *ssl = init_ssl("./myssl/cert.pem", "./myssl/key.pem", SSL_MODE_SERVER, fd_);   
    while (true){

        bzero (&buf, sizeof(buf));
        //ssize_t nread = SSL_read (ssl , buf , sizeof (buf));
        ssize_t nread = read (fd_ , buf , sizeof (buf));
        
        //成功读取到了数据。
        if (nread>0){
            //把接收到的报文内容原封不动的发回去。
            //printf ("recv(eventfd=%d):%s\n",fd_, buf);
            //SSL_write(ssl, buf, strlen(buf)+1);
            send (fd_, buf, strlen(buf),0);
            //std::cout<<buf<<std::endl;
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
                errorcallback_();
                if(Channel::userlist[fd_] != nullptr)free(Channel::userlist[fd_]);    //这个后续再改
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
                Channel::userlist[fd_]->getinfo(data["username"].asString(), data["password"].asString(), data["CpuID"].asString(), data["BiosID"].asString());
                //std::cout<<"username:"<<data["username"].toStyledString()<<" \npassword:"<<data["password"].toStyledString()<<std::endl;
                int validation =  userlist[fd_]->login() ;
                if(validation == SUCCESS)
                {                    
                    std::cout <<"登入成功"<<std::endl;
                    data["Validation"] = "SUCCESS";

                    if(userlist[fd_]->updete()){
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
            send (fd_, style.c_str(), strlen(style.c_str()),0);
            std::cout << style << std::endl;
        }
        
        //读取数据的时候被信号中断，继续读取。
        else if (nread ==-1 && errno == EINTR )continue;
        //全部的数据已读取完毕。 
        else if (nread ==-1 &&(( errno == EAGAIN )||( errno == EWOULDBLOCK )))break;

        else if (nread ==0)//客户端连接已断开。
        {
            printf (" client(eventfd=%d)disconnected.\n ", fd_);
            //_close(fd_ );//关闭客户端的fd
            closecallback_();
            if(Channel::userlist[fd_] != nullptr)free(Channel::userlist[fd_]);    //这个后续再改
            break ;
        }
    }
    //SSL_shutdown(ssl);
    //SSL_free(ssl);
}

void  Channel::setreadcallback(std::function<void()> fn)
{
    readcallback_ = fn;
}

// bool Channel::isHTTPRequest() {
//     const int max_header_size = 1024; // 假设 HTTP 请求头的最大大小为 1024 字节
//     char header[max_header_size];
//     ssize_t bytes_read;

//     // 从套接字中读取一部分数据
//     bytes_read = read(fd_, header, max_header_size);
//     if (bytes_read <= 0) {
//         // 读取失败或者连接已关闭
//         return false;
//     }

//     // 检查读取的数据中是否包含 HTTP 请求的关键词
//     if (bytes_read >= 4 && (strncmp(header, "GET ", 4) == 0 ||
//                             strncmp(header, "POST ", 5) == 0 ||
//                             strncmp(header, "PUT ", 4) == 0 ||
//                             strncmp(header, "DELETE ", 7) == 0)) {
//         // 包含 HTTP 请求关键词，可以认为是 HTTP 请求
//         return true;
//     }

//     // 不包含 HTTP 请求关键词，可能是其他类型的请求
//     return false;
// }

void Channel::send_header(int code, char* info, char* filetype, int length)
{
    //状态行 
    char buf[1024] = "";
    int len = 0;
    len =  sprintf(buf,"HTTP/1.1 %d %s\r\n",code, info);
    send(fd_, buf, len, 0);
    //消息头 
    len = sprintf(buf,"Content-Type:%s\r\n", filetype);
    send(fd_, buf, len, 0);
    if(length > 0)
    {
        len = sprintf(buf, "Content-Length:%d\r\n", length);
        send(fd_, buf,len, 0);
    }
    //空行
    send(fd_, "\r\n", 2, 0);
} 
void Channel::send_file(char* path)
{
    int fd = open(path, O_RDONLY);
    if(fd < 0){
        perror("");
        return;
    }
    char buf[1024] = "";
    int len = 0;
    while (1)
    {
        len = read(fd, buf, sizeof(buf));
        if(len < 0)
        {
            perror("");
            break;
        }
        else if(len == 0) break;
        else
        {
            send(fd_, buf, len, 0);
        }
    }
    _close(fd);

}

void Channel::setclosecallback(std::function<void()> fn)
{
    closecallback_ = fn;
}

void Channel::seterrorcallback(std::function<void()> fn)
{
    errorcallback_ = fn;
}