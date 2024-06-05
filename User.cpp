#include "User.h"

User::User(std::string ip):ip_(ip)
{
}

User::~User()
{
  
}

bool User::login()
{
    return Database().login(username_, password_);
}

bool User::updete()
{
    return Database().update(ip_, CpuID_, BiosID_, username_);
}

void User::getinfo(std::string username, std::string passworld, std::string CpuID, std::string BiosID)
{
    username_ = username;
    password_ = passworld;
    CpuID_ = CpuID;
    BiosID_ = BiosID;
}