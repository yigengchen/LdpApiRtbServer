#ifndef __MYSQLCLIENT_H__
#define __MYSQLCLIENT_H__

#include "mysql.h"
#include <map>
#include <string>
//#include "../../UserQueryStruct/UserQueryStruct.h"

class CMYSQL
{
	public:
		CMYSQL();
		~CMYSQL();
		bool ExecuteMySql(const char *pch);
		bool DisplayHeader();
	  bool DisplayRows();
		bool DeleteRecord(const char *pch);
		bool UpdateRecord(const char *pch);
		bool InitMysql(const char *host, unsigned short port, const char *userName,const char *passwd,const char *dbName);
		bool CloseMysqlConn();
	  MYSQL * GetMysqlConnState();
	  bool GetRecordFields(const char *str);
		//bool GetColumnValue(MYSQL_ROW sqlRow,std::map<std::string,BDXPERMISSSION_S> &mapUserInfo);
		std::string GetColumnValue();
		std::string GetColumnValue2(MYSQL_ROW sqlRow);
	  unsigned int GetMysqlTotalColumns();
	  unsigned long GetMysqlTotalRecords();
	  bool GetMysqlInitState();
	  bool DestroyResultEnv();
	  MYSQL_ROW MysqlFetchRow();
	  bool MysqlUseResult();
	  
	private:
		MYSQL *pmysqlConn;
		MYSQL_RES *pmysqlRes;
		MYSQL_ROW mysqlRow; //support max rows 300
		bool bInitFlag;
		unsigned int totalColumns;
		unsigned long totalRecords;
		MYSQL_FIELD *pmysqlFields;

};

#endif //__MYSQLCLIENT_H__