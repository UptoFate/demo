#include "Database.h"

int main(int argc,char *argv[])
{
    MYSQL conn;
    int res;
    mysql_init(&conn);
    //"root":数据库管理员 "123":root密码 "test":数据库的名字
    if(mysql_real_connect(&conn, "192.168.88.129","root","123456","test",0,NULL,CLIENT_FOUND_ROWS))
    {
        std::cout << "connect success" << std::endl;
        res = mysql_query(&conn, "insert into tb1 values (3,'c')");
        //res = mysql_query(&conn, "select * from tb1;");
    if(res)
    {
        std::cout<<res;
        printf("error\n");
    }
    else
    {
        printf("OK\n");
    }
    mysql_close(&conn);
    }else
    {
        std::cout << "connect failed" << std::endl;
    }
    return 0;
}

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

    if(mysql_query(_mysql, "set names utf-8"))
    {
        std::cout<<"set names utf-8 error"<<std::endl;
    }

    return true;
}

void Database::disconnect()
{
    if(_mysql)mysql_close(_mysql);
}

bool Database::query()
{
    connect();
    char sql[256];
	bool ret = false;
	MYSQL_RES* res;
	MYSQL_ROW row;
	snprintf(sql, 256, "select * from tb1;");
	ret = mysql_query(_mysql, sql);//成功返回0
	if (ret) {
		printf("数据库查询出错，%s错误原因：%s", sql, mysql_error(_mysql));
		disconnect();
		return false;
	}
	res = mysql_store_result(_mysql);
	while ((row = mysql_fetch_row(res))!=NULL)
	{
		for (int i = 0; i < mysql_num_fields(res); i++) {
			printf("%s", row[i]);
		}
		printf("\n");
	}
	if (row == NULL) {
		mysql_free_result(res);//释放结果集
		disconnect();
		return false;
	}
	mysql_free_result(res);
	disconnect();
	return true;

}