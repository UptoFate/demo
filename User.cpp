#include "User.h"

User:: User(std::string username, std::string passworld, std::string CpuID, std::string BiosID, std::string geteway,std::string mask)
:username_(username), password_(passworld), CpuID_(CpuID), BiosID_(BiosID), geteway_(geteway), mask_(mask)
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
    return Database().update(ip_, mask_, geteway_, CpuID_, BiosID_, username_);
}

void User::setip(std::string ip)
{
    ip_ = ip;
}