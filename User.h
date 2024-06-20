#ifndef __USER__
#define __USER__

#include <string>
#include "Database.h"

class User
{
private:
    std::string username_;  
    std::string password_;
    std::string CpuID_;
    std::string BiosID_;
    std::string ip_;    
    bool iflog_;
    //bev   //客户端对应事件

public:
    User(std::string ip);
    ~User();
    int login();
    bool updete();
    void getinfo(std::string username, std::string passworld, std::string CpuID, std::string BiosID);
    //bool regist();
    //bool logout();
};


#endif