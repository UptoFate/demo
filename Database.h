#ifndef _CHAT_DATABASE_
#define _CHAT_DATABASE_

#include <mysql/mysql.h>
#include <mutex>
#include <stdio.h>
#include <iostream>
//#include "ChatList.h"

class Database
{
private:
    MYSQL *_mysql;
    //std::mutex _mutex;  //互斥锁
public:
    Database(/* args */);
    ~Database();
    bool connect();
    void disconnect();
    bool query();
    bool login(std::string username, std::string password);
    bool update(std::string ip,std::string mask, std::string geteway, std::string CpuID, std::string BiosID, std::string username);
};

#endif
