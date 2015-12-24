//============================================================================
// Name        : mysqlClient.cpp
// Author      : chenyg
// Date		     : 20150708
// Version     : v 0.0.0
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include "mysqlClient.h"
#include <stddef.h>
#include <stdio.h>

CMYSQL::CMYSQL()
{

}
CMYSQL::~CMYSQL()
{

}
bool CMYSQL::InitMysql(const char *host, unsigned short port, const char *userName,const char *passwd,const char *dbName)
{
	
	pmysqlConn = mysql_init(NULL);
	if(!pmysqlConn)
	{
		//LOG(ERROR,"%s:%d;mysql_init failed,errno:%d,%s",__FILE__,__LINE__,mysql_errno(pmysqlConn),mysql_error(pmysqlConn));
		printf("ERROR,%s:%d;mysql_init failed,errno:%d,%s\n",__FILE__,__LINE__,mysql_errno(pmysqlConn),mysql_error(pmysqlConn));
		return false;
	}
	
	pmysqlConn=mysql_real_connect(pmysqlConn,host,userName,passwd,dbName,port,NULL,0);

	if(!pmysqlConn)
	{
		//LOG(ERROR,"%s:%d;mysql_real_connect failed,errno:%d,%s",__FILE__,__LINE__,mysql_errno(pmysqlConn),mysql_error(pmysqlConn));
		printf("ERROR,%s:%d;mysql_real_connect failed,errno:%d,%s\n",__FILE__,__LINE__,mysql_errno(pmysqlConn),mysql_error(pmysqlConn));
		return false;
	}

	bInitFlag = true;
	return true;
}

/*
bool CMYSQL::ReInitMysql(const char *host, unsigned short port, const char *userName,const char *passwd,const char *dbName)
{
	
	pmysqlConn = mysql_init(NULL);
	if(!pmysqlConn)
	{
		//LOG(ERROR,"%s:%d;mysql_init failed,errno:%d,%s",__FILE__,__LINE__,mysql_errno(pmysqlConn),mysql_error(pmysqlConn));
		printf("ERROR,%s:%d;mysql_init failed,errno:%d,%s\n",__FILE__,__LINE__,mysql_errno(pmysqlConn),mysql_error(pmysqlConn));
		return false;
	}
	
	pmysqlConn=mysql_real_connect(pmysqlConn,host,userName,passwd,dbName,port,NULL,0);

	if(!pmysqlConn)
	{
		//LOG(ERROR,"%s:%d;mysql_real_connect failed,errno:%d,%s",__FILE__,__LINE__,mysql_errno(pmysqlConn),mysql_error(pmysqlConn));
		printf("ERROR,%s:%d;mysql_real_connect failed,errno:%d,%s\n",__FILE__,__LINE__,mysql_errno(pmysqlConn),mysql_error(pmysqlConn));
		return false;
	}

	
	return true;
}

*/


bool CMYSQL::ExecuteMySql(const char *str)
{
	int res = 0;
	res = mysql_query(pmysqlConn,str);

	//totalRecords = mysql_affected_rows(pmysqlConn);
	
	if(res!=0)
	{
		//LOG(ERROR,"%s:%d;mysql_query failed,errno:%d,%s,sql=%s",__FILE__,__LINE__,mysql_errno(pmysqlConn),mysql_error(pmysqlConn),str);
		printf("ERROR,%s:%d;mysql_query failed,errno:%d,%s,sql=%s\n",__FILE__,__LINE__,mysql_errno(pmysqlConn),mysql_error(pmysqlConn),str);
		return false;
	}

	return true;
}

bool CMYSQL::DisplayHeader()
{

    printf("Column details:\n");
    while((pmysqlFields = mysql_fetch_field(pmysqlRes))!=NULL)
    {
        printf("Name: %s \t\t",pmysqlFields->name);
        printf("Type: \t\t");
        if(IS_NUM(pmysqlFields->type))
        {
            printf("Numeric filed");
        }
        else
        {
            switch(pmysqlFields->type)
            {
            case FIELD_TYPE_VAR_STRING:
                printf("VARCHAR");
                break;
            case FIELD_TYPE_LONG:
                printf("LONG");
                break;
            case FIELD_TYPE_TIMESTAMP:
                printf("MYSQL_TYPE_TIMESTAMP");
                break;
            default:
                printf("Type is %d,check in mysql_com.h",pmysqlFields->type);
            }
        }
        printf("MAX width %ld\t\t\n",pmysqlFields->length);
        if(pmysqlFields->flags&AUTO_INCREMENT_FLAG)
            printf("Auto increments\t\t\n");
        printf("\n");
    }
    return true;
}

bool  CMYSQL::DisplayRows()
{
		unsigned int field_count;
    field_count = 0;
    totalColumns = mysql_field_count(pmysqlConn);
    printf("mysql_field_count(pmysqlConn)=%d\n",mysql_field_count(pmysqlConn));
    while(field_count < totalColumns)
    {
        if(mysqlRow[field_count]) 
        {
        	printf("%s ",mysqlRow[field_count]);
        }
        else
        {
        	printf("NULL");	
        }
        field_count++;
    }
    printf("\n");
    return  true;
}
bool CMYSQL::GetRecordFields(const char *str)
{
 	printf("Connection success:\n");
 	int res,first_row = 1,lineNumber = 0;
	res = mysql_query(pmysqlConn,str);
	
	if(res!=0)
	{
		//LOG(ERROR,"%s:%d;mysql_query failed,errno:%d,%s,sql=%s",__FILE__,__LINE__,mysql_errno(pmysqlConn),mysql_error(pmysqlConn),str);
		printf("ERROR,%s:%d;mysql_query failed,errno:%d,%s,sql=%s\n",__FILE__,__LINE__,mysql_errno(pmysqlConn),mysql_error(pmysqlConn),str);
		return false;
	}
	else
	{
		pmysqlRes = mysql_use_result(pmysqlConn);
		if( pmysqlRes )
		{
			while(mysqlRow = mysql_fetch_row(pmysqlRes))
			{
				if(first_row)
				{
					DisplayHeader();
					first_row = 0;	
				}
				
				else
				{
					 DisplayRows();		
				}
				lineNumber++;
			}
			
		}
		
	}

	return true;
}


bool CMYSQL::MysqlUseResult()
{
		pmysqlRes = mysql_use_result(pmysqlConn);	
		if(!pmysqlRes)
		{
			return false;
		}
	return true;
}

MYSQL_ROW CMYSQL::MysqlFetchRow()
{

	mysqlRow = mysql_fetch_row(pmysqlRes);
	return mysqlRow;
}


/*
bool CMYSQL::GetColumnValue(MYSQL_ROW sqlRow,std::map<std::string,BDXPERMISSSION_S> &mapUserInfo)
{
		unsigned int field_count;
    field_count = 0;
    while(field_count < GetMysqlTotalColumns())
    {
        if(sqlRow[field_count]) 
        {
        	printf("%s ",sqlRow[field_count]);
        }
        else
        {
        	printf("NULL");	
        }
        field_count++;
    }
    
    mapUserInfo.mResToken=sqlRow[1];
    mapUserInfo.mIntQueryTimes=sqlRow[2];
    mapUserInfo.mVecFields=sqlRow[3];
    mapUserInfo.insert(std::pair<std::string,BDXPERMISSSION_S>((sqlRow[0]),mVecFields));

    printf("\n");
    return  true;
 	
}

*/

std::string CMYSQL::GetColumnValue()
{
		unsigned int field_count;
    field_count = 0;
    std::string strParam="";
    
    while(field_count < GetMysqlTotalColumns())
    {
        if(mysqlRow[field_count]) 
        {
        	//printf("%s ",mysqlRow[field_count]);
        	strParam+=mysqlRow[field_count];
        }
        else
        {
        	//printf("NULL");	
        	strParam+="NULL";
        }
        strParam+=";";
        field_count++;
    }

    return  strParam;
 	
}

std::string CMYSQL::GetColumnValue2(MYSQL_ROW sqlRow)
{
		unsigned int field_count;
    field_count = 0;
    std::string strParam="";
    
    while(field_count < GetMysqlTotalColumns())
    {
        if(sqlRow[field_count]) 
        {
        	printf("%s ",sqlRow[field_count]);
        	strParam+=sqlRow[field_count];
        }
        else
        {
        	printf("NULL");	
        	strParam+="NULL";
        }
        strParam+=";";
        field_count++;
    }

    return  strParam;
 	
}

bool CMYSQL::CloseMysqlConn()
{

	if(pmysqlConn!=NULL)
	{
		mysql_close(pmysqlConn);
	}
	return true;
	
}

MYSQL * CMYSQL::GetMysqlConnState()
{

	return pmysqlConn;
	
}

unsigned int CMYSQL::GetMysqlTotalColumns()
{

	totalColumns = mysql_field_count(pmysqlConn);
	return totalColumns;
	
}

unsigned long CMYSQL::GetMysqlTotalRecords()
{

	//totalRecords = mysql_num_rows(pmysqlRes);
	//totalRecords = mysql_affected_rows(pmysqlConn);
	//printf("+++++++++ %ld\n",totalRecords);
	return totalRecords;
	
}

bool CMYSQL::GetMysqlInitState()
{

	return bInitFlag;
	
}

bool CMYSQL::DestroyResultEnv()
{
	mysql_free_result(pmysqlRes);
	return true;
} 