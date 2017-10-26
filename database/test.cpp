#include <iostream>
#include "mysql.h"
#include "elegance/platform/common/timer.hpp"
#include "mysql_connector.hpp"
#include <bitset>

int main()
{
	face2wind::MysqlConnector connector;
	if (!connector.Connect("127.0.0.1", 3306, "root", "12345", "ttaxx_cn"))
	{
		std::cout << "Connect Error" << std::endl;
		return -1;
	}

	face2wind::MysqlTableDescribe describe;
	face2wind::MysqlNodeDescribe node;

	node.name = "role_id";
	node.type = face2wind::MysqlType::INT32;
	describe.node_list.push_back(node);

	node.name = "role_name";
	node.type = face2wind::MysqlType::STRING;
	node.data_length = 32;
	describe.node_list.push_back(node);

	node.name = "level";
	node.type = face2wind::MysqlType::INT32;
	describe.node_list.push_back(node);

	node.name = "create_time";
	node.type = face2wind::MysqlType::INT64;
	describe.node_list.push_back(node);

	face2wind::MysqlResult result;
	int effect_row = connector.Query("select role_id,role_name,level,create_time from role where role_inc_id = 1301");
	if (-1 == effect_row)
	{
		会进来这里，有空再继续弄
		std::cout << "Query Error" << std::endl;
		return -2;
	}
	std::cout << "effect_row = " << effect_row << std::endl;

	if (!connector.FetchQueryResult(result, describe))
	{
		std::cout << "FetchQueryResult Error" << std::endl;
		return -3;
	}

	std::cout << "\ntest mysql ... END" << std::endl;

	face2wind::Timer::Sleep(500);

	return 0;
}
