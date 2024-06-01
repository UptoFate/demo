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
    std::string mask_;
    std::string geteway_;
    std::string ip_;    
    bool iflog_;
    //bev   //客户端对应事件

public:
    User(std::string username, std::string password, std::string CpuID_, std::string BiosID_, std::string geteway_,std::string mask_);
    ~User();
    bool login();
    bool updete();
    void setip(std::string ip);
    //bool regist();
    //bool logout();
};


#endif