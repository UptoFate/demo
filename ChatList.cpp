#include "ChatList.h"

ChatInfo::ChatInfo(/* args */)
{
    online_user = new std::list<User>;
    
    group_info = new std::map<std::string, std::list<std::string>>;
}

ChatInfo::~ChatInfo()
{
    if (online_user)delete online_user;

    if (group_info)delete group_info;
}