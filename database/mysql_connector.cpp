#include <iostream>
#include "mysql_connector.hpp"
#include "elegance/memory/mempool.hpp"

namespace face2wind
{
	bool MysqlConnector::Connect(const std::string &host, short port, const std::string &user, const std::string &passwd, const std::string &db_name)
	{
		this->Disconnect();

		mysql_init(&connection_);
		if (!mysql_real_connect(&connection_, host.c_str(), user.c_str(), passwd.c_str(), db_name.c_str(), port, nullptr, CLIENT_MULTI_STATEMENTS))
		{
			return false;
		}

		is_connected_ = true;
		return true;
	}

	void MysqlConnector::Disconnect()
	{
		if (is_connected_)
		{
			mysql_close(&connection_);
			is_connected_ = false;
		}
	}

	int MysqlConnector::Query(const std::string &command)
	{
		int effect_row = -1;
		if (!is_connected_)
			return effect_row;

		if (0 != mysql_query(&connection_, command.c_str()))
			return effect_row;

		effect_row = mysql_affected_rows(&connection_);
		return effect_row;
	}

	int MysqlConnector::Query(const MysqlStmt &stmt)
	{
		int effect_row = -1;
		if (!is_connected_)
			return effect_row;

		return effect_row;
	}

	bool MysqlConnector::FetchQueryResult(MysqlResult &query_result, const MysqlTableDescribe &describe)
	{
		MYSQL_RES *mysql_result = mysql_store_result(&connection_);//保存查询到的数据到result
		if (nullptr == mysql_result)
			return false;

		std::cout << "the result number is " << mysql_num_rows(mysql_result) << std::endl;

		//		MYSQL_FIELD *fd;
		//		for (int i = 0; fd = mysql_fetch_field(mysql_result); i++)//获取列名
		//		{
		//			printf("%s\t\t", fd->name);
		//		}
		//		printf("\n");

		int field_num = int(mysql_num_fields(mysql_result));
		if (int(describe.node_list.size()) < field_num)
			return false;

		MYSQL_ROW sql_row = mysql_fetch_row(mysql_result);
		unsigned long *length_list = mysql_fetch_lengths(mysql_result);
		while (nullptr != sql_row)
		{
			std::vector<MysqlNodeData> node_list;
			for (int field_index = 0; field_index < field_num; field_index++)
			{
				MysqlNodeData node_data;
				this->MallocAndSetNodeData(node_data, describe.node_list[field_index], sql_row[field_index], length_list[field_index]);
				printf("%s\t\t", sql_row[field_index]);
				node_list.push_back(node_data);
			}
			printf("\n");
			query_result.row_list.push_back(node_list);

			sql_row = mysql_fetch_row(mysql_result);
		}

		mysql_free_result(mysql_result);

		return true;
	}

	void MysqlConnector::MallocAndSetNodeData(MysqlNodeData & node_data, const MysqlNodeDescribe &node_describe, char * data, int data_length)
	{
		switch (node_describe.type)
		{
		case MysqlType::INT8:
			if (data_length > 0)
				node_data.int8 = *data;
			break;

		case MysqlType::INT16:
			if (data_length > 0)
				node_data.int16 = atoi(data);
				//node_data.int16 = *(short*)data;
			break;

		case MysqlType::INT32:
			if (data_length > 0)
				node_data.int32 = atoi(data);
			//node_data.int32 = *(int*)data;
			break;

		case MysqlType::INT64:
			if (data_length > 0)
				node_data.int64 = atoll(data);
				//node_data.int64 = *(long long*)data;
			break;

		case MysqlType::STRING:
		case MysqlType::BINARY_DATA:
			{
				int copy_len = data_length;
				if (copy_len >= node_describe.data_length)
					copy_len = node_describe.data_length;

				if (copy_len > 0)
				{
					node_data.ChangeDataLength(copy_len);
					memcpy(node_data.data, data, copy_len);
				}
			}
			break;

		default:
			break;
		}
	}
}
