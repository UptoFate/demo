#ifndef __MY_INETADDRESS__
#define __MY_INETADDRESS__

#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>

class InetAddress
{
private:
    sockaddr_in m_addr;// 地址协议结构体

public:
    InetAddress();
    InetAddress(const std::string &ip, uint16_t port); // 监听fd构造
    InetAddress(const sockaddr_in m_addr); //客户端fd构造
    ~InetAddress();

    const char* ip()const;      //返回字符串表示地址
    uint16_t port()const;       //返回整数表示端口
    const sockaddr *addr()const;    //返回m_addr地址用于bind
    void setaddr(sockaddr_in clientaddr);   //设置m_addr成员的值
};

#endif