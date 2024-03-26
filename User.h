#ifndef __USER__
#define __USER__

#include <string>
#include "Database.h"

class User
{
private:
    std::string username_;  
    std::string password_;
    bool iflog_;
    //bev   //客户端对应事件

public:
    User(std::string username, std::string password);
    ~User();
    bool login();
    //bool regist();
    //bool logout();
};


#endif