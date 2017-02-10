#pragma once
#include"preinclude.h"

typedef enum
{
	TEXT,
	INTEGER,
	_DOUBLE,
	DATETIME
 }DataType;

typedef enum
{
	_NULL,
	NOT_NULL
}NullFlag;

typedef struct
{
	std::string col_name;
	DataType data_type;
	NullFlag null_flag;
	std::string default;
}Field;

typedef std::vector<Field> Fields;
typedef std::string TableName;
typedef std::unordered_map<TableName, Fields> TableList;

class Database
{
protected:
	virtual std::string Connect(const char* address = NULL, const char* user_id = NULL, const char* password = NULL) = 0;// if not return "ok", then the returned string means the exact error message
	virtual bool IsConnected() = 0;
	virtual bool IsTableExist(const char* table_name) = 0;
	virtual std::string CreateTable(const char* table_name) = 0;// if not return "ok", then the returned string means the exact error message
	virtual std::string Insert(const char* table_name) = 0;
	virtual std::string ExecuteSelect(const char* full_sql, std::vector<std::vector<std::string>>* result_set, int column_count) = 0;
	virtual std::string Delete(const char* sql) = 0;
	virtual std::string Update(const char* sql) = 0;
	//TODO:delete log file 30 days ago
	//TODO:����������ѡ�������ĵط�����������index��QueryInstrument��index(��update open interest of contract������)
	//TODO:��OnRtnOrder������ǿ��ֻر�������ռ�ñ�֤��margin������ʾ��datagrid position��
};

