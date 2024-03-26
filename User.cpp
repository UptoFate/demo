#include "User.h"

User:: User(std::string username, std::string passworld):username_(username), password_(passworld)
{
}

User::~User()
{
    
}

bool User::login()
{
    return Database().login(username_, password_);
}
