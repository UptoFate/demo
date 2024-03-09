#include "InetAddress.h"

    InetAddress::InetAddress()
    {
        
    }
    
    InetAddress:: InetAddress(const std::string &ip, uint16_t port){
        m_addr.sin_family = AF_INET;                        //IPv4网络协议套接字
        m_addr.sin_addr.s_addr = inet_addr(ip.c_str());     //监听ip ，可用INADDR_ANY监听任意
        m_addr.sin_port = htons(port);                      
    }

    InetAddress:: InetAddress(const sockaddr_in m_addr):m_addr(m_addr)
    {

    }

    InetAddress:: ~InetAddress()
    {

    }

    const char* InetAddress:: ip() const
    {
        return inet_ntoa(m_addr.sin_addr);
    }

    uint16_t InetAddress:: port() const
    {
        return ntohs(m_addr.sin_port);
    }    

    const sockaddr* InetAddress:: addr() const
    {
        return (sockaddr*)&m_addr;
    }

    void InetAddress::setaddr(sockaddr_in clientaddr)
    {
        m_addr=clientaddr;
    }