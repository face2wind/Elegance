#pragma once

#include "mysql.h"
#include <string>
#include <vector>
#include "elegance/memory/mempool.hpp"

namespace face2wind
{
	/* MYSQL的类型定义，拿来参考一下
	MYSQL_TYPE_DECIMAL, MYSQL_TYPE_TINY,
		MYSQL_TYPE_SHORT, MYSQL_TYPE_LONG,
		MYSQL_TYPE_FLOAT, MYSQL_TYPE_DOUBLE,
		MYSQL_TYPE_NULL, MYSQL_TYPE_TIMESTAMP,
		MYSQL_TYPE_LONGLONG, MYSQL_TYPE_INT24,
		MYSQL_TYPE_DATE, MYSQL_TYPE_TIME,
		MYSQL_TYPE_DATETIME, MYSQL_TYPE_YEAR,
		MYSQL_TYPE_NEWDATE, MYSQL_TYPE_VARCHAR,
		MYSQL_TYPE_BIT,
		MYSQL_TYPE_TIMESTAMP2,
		MYSQL_TYPE_DATETIME2,
		MYSQL_TYPE_TIME2,
		MYSQL_TYPE_JSON = 245,
		MYSQL_TYPE_NEWDECIMAL = 246,
		MYSQL_TYPE_ENUM = 247,
		MYSQL_TYPE_SET = 248,
		MYSQL_TYPE_TINY_BLOB = 249,
		MYSQL_TYPE_MEDIUM_BLOB = 250,
		MYSQL_TYPE_LONG_BLOB = 251,
		MYSQL_TYPE_BLOB = 252,
		MYSQL_TYPE_VAR_STRING = 253,
		MYSQL_TYPE_STRING = 254,
		MYSQL_TYPE_GEOMETRY = 255
	*/
	enum class MysqlType
	{
		INVALID = 0,

		INT8,
		INT16,
		INT32,
		INT64,
		STRING,
		BINARY_DATA
	};

	struct MysqlNodeDescribe
	{
		MysqlNodeDescribe() : type(MysqlType::INVALID), data_length(0) {}

		std::string name;

		MysqlType type;
		int data_length;
	};

	struct MysqlTableDescribe
	{
		std::vector<MysqlNodeDescribe> node_list;
	};

	struct MysqlNodeData
	{
		MysqlNodeData() { this->Reset(); }

		void Reset()
		{
			data_length_ = 0;
			int64 = 0;
		}

		void ChangeDataLength(int length)
		{
			if (data_length_ > 0 && nullptr != data)
				MemoryPoolManager::GetInstance().Free(data);

			data_length_ = length;
			if (data_length_ <= 0)
				data = nullptr;
			else
				data = (char*)MemoryPoolManager::GetInstance().Alloc(length);
		}

		unsigned int DataLength() { return data_length_; }

		union
		{
			char int8;
			short int16;
			int int32;
			long long int64;
			char *data;
		};

	protected:
		unsigned int data_length_;
	};

	struct MysqlResult
	{
		std::vector<std::vector<MysqlNodeData> > row_list;
	};

	class MysqlStmt
	{
	};

	class MysqlConnector
	{
	public:
		MysqlConnector(): is_connected_(false) {}

		~MysqlConnector() {}

		bool Connect(const std::string &host, short port, const std::string &user, const std::string &passwd, const std::string &db_name);
		void Disconnect();
		bool IsConnected() const { return is_connected_; }

		int Query(const std::string &command);
		int Query(const MysqlStmt &stmt);
		bool FetchQueryResult(MysqlResult &query_result, const MysqlTableDescribe &describe);

	protected:
		void MallocAndSetNodeData(MysqlNodeData & node_data, const MysqlNodeDescribe &mysql_node_describe, char * data, int data_length);

	private:
		MYSQL connection_;
		bool is_connected_;
	};

}
