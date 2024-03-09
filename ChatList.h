#ifndef __CHAT_LIST__
#define __CHAT_LIST__

#include <iostream>
#include <list>
#include <json/json.h>

struct User
{
    std::string name;   //账号
    //bev   //客户端对应事件
};

class ChatInfo
{
private:
    std::list<User> *online_user;
    std::map<std::string, std::list<std::string>> *group_info;
public:
    ChatInfo(/* args */);
    ~ChatInfo();
};





#endif