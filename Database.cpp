#include "Database.h"


Database::Database(/* args */)
{
    _mysql = mysql_init(NULL);
}

Database::~Database()
{
    disconnect();
}

bool Database::connect()
{

    if(!mysql_real_connect(_mysql, "192.168.88.129","root","123456","test",0,NULL,CLIENT_FOUND_ROWS))
    {
        return false;
    }

    if(mysql_query(_mysql, "set names utf8"))
    {
        std::cout<<"set names utf-8 error"<<std::endl;
    }

    return true;
}

void Database::disconnect()
{
	if (_mysql != NULL) {
        mysql_close(_mysql);
        _mysql = NULL;
    }
}

// bool Database::query()
// {
//     connect();
//     char sql[256];
// 	bool ret = false;
// 	MYSQL_RES* res;
// 	MYSQL_ROW row;
// 	snprintf(sql, 256, "SELECT * from dept WHERE json_value->'$.deptLeaderId'='5'");
// 	ret = mysql_query(_mysql, sql);//成功返回0
// 	if (ret) {
// 		printf("数据库查询出错，%s错误原因：%s", sql, mysql_error(_mysql));
// 		disconnect();
// 		return false;
// 	}
// 	res = mysql_store_result(_mysql);
// 	while ((row = mysql_fetch_row(res))!=NULL)
// 	{
// 		for (int i = 0; i < mysql_num_fields(res); i++) {
// 			printf("%s", row[i]);
// 		}
// 		printf("\n");
// 	}
// 	if (row == NULL) {
// 		mysql_free_result(res);//释放结果集
// 		disconnect();
// 		return false;
// 	}
// 	mysql_free_result(res);
// 	disconnect();
// 	return true;
// }

bool Database::login(std::string username, std::string password)
{
    connect();
    char sql[256];
    MYSQL_RES* res;
    MYSQL_ROW row;
    bool ret = false;
    std::string query = "SELECT username FROM usercredentials WHERE username = '" + username + "' AND password = '" + password + "';";
    ret = mysql_query(_mysql, query.c_str()); // 成功返回0
    if (ret != 0) {
        printf("数据库查询出错：%s", mysql_error(_mysql));
        disconnect();
        return false;
    }
    res = mysql_store_result(_mysql);
    if (!res) {
        disconnect();
        return false;
    }
    row = mysql_fetch_row(res);
	disconnect();
	mysql_free_result(res);
	
    if (row) 
	{
        return true;
    } 
	else 
	{
        return false;
    }
}

bool Database::update(std::string ip, std::string mask, std::string geteway, std::string CpuID, std::string BiosID, std::string username)
{	
    connect();
    MYSQL_RES* res;
    bool ret = false;
    std::string query = "UPDATE usercredentials SET ip_address = '"
    + ip
    + "', mask = '"
    + mask
    + "', gateway = '"
    + geteway
    + "', CpuID = '"
    + CpuID
    + "',BiosID = '"
    + BiosID
    + "' WHERE username = '"
    + username 
    + "';";
	if (ret = mysql_query(_mysql, query.c_str())) {
        printf("数据库修改出错：%s", mysql_error(_mysql));
        disconnect();
		return false;
	}
    disconnect();
	return true;
}