#include <json/json.h>
#include <string>
#include <iostream>
#include "mysql/Database.h"
using namespace std;

int main()
{
    Database* database = new Database();
    database->query();
	Json::Value array;
	Json::StyledWriter writer;	// 输出带格式的json
	//array["array"] = Json::Value(root);
	string data = writer.write(array);
	cout << data<< endl;

	return 0;
}

