/*

 */

#ifndef __DATA_STRUCT_H__
#define __DATA_STRUCT_H__

#include <string>
#include <vector>
#include <map>
#include <queue>
#include <netinet/in.h>

#include "../CommonTools/Log/Log.h"
#include "../CommonTools/Lock/Lock.h"


#define BUF_SIZE_8BYTE 8
#define BUF_SIZE_64KB 64*1024

#define	PACKET			1024
#define _128BYTES			128
#define _8KBLEN			8192
#define _64KBLEN			64*1024

#define SUCCESS 			0
#define OTHERERROR		-1
#define LINKERROR		-2
#define PROTOERROR		-3
#define TIMEOUT          -4
#define ERRORPARAM	-5
#define ERRORNODATA -6
#define EXCEEDLIMIT -7






#define	UINT				unsigned int
#define	UCHAR				unsigned char
#define	ULONG				unsigned long
#define	USHORT				unsigned short
#define	ULONGLONG			unsigned long long
#define uint64_t			unsigned long int

#define	CTRL_T			'\t'
#define	CTRL_R			'\r'
#define	CTRL_N			'\n'


#define	SEC_Q			'?'
#define	SEC_M			','
#define	SEC_A			'&'
#define	SEC_L			'['
#define	SEC_R			']'

#define	BLANK			' '
#define	EQUAL			'='
#define	REMARK			'#'


//#define PACKET           1024

#define	STRING_EQUAL	"="
#define	STRING_AND		"&"
#define KEY_QUERY_USER	"/query"
#define KEY_QUERY_GOODS	"/goods"
#define KEY_UPDATE 		"/update"
#define REQ_TYPE 		"GET"

#define KEY_USER		"user"
#define KEY_RET_KEY		"rkey"
#define KEY_VALUE 		"auth"
#define KEY_TABLENAME	"tablename"
#define KEY_SHOPID		"shopid"
#define KEY_GOODSID		"goodsid"
#define KEY_KEY 		"key"
#define KEY_KEY_TYPE	"keytype"
#define KEY_FILTER	"filter"
#define KEY_KEY_SRC		"province"
#define KEY_OPERATOR	"ctype"
#define LEFTBIGBRACE     string("{")
#define RIGHTBIGBRACE    string("}")
#define NULLSTRING    	 string("")
#define LEFTMIDBRACE     string("[")
#define RIGHTMIDBRACE    string("]")
//#define NULLSTRING    	 string("")


typedef unsigned int		uint32_t;

//log sev msg types
#define LOG_TYPE_REQ 		1
#define LOG_TYPE_RESP 		2


typedef struct {
	LOG_RANK m_Rank;
	std::string m_strPath;
	std::string m_strFileName;
	u_int m_uiMaxLine;
}LOGPRMS_S;

typedef struct M_HIVELOG
{
std::string strLogKey;
std::string strLogKeyType;
std::string strLogValue;
std::string strCreateTime;
std::string strLastDataTime;
std::string strQueryTime;
std::string strDspName;
std::string strReqParams;
std::string strProvider;
std::string strProvince;
std::string strDayId;
std::string strHourId;

void operator =(const M_HIVELOG &hive) 
{
	strLogKey		=	hive.strLogKey;
	strLogKeyType	=	hive.strLogKeyType;
	strLogValue		=	hive.strLogValue;
	strCreateTime	=	hive.strCreateTime;
	strLastDataTime	=	hive.strLastDataTime;
	strQueryTime	=	hive.strQueryTime;
	strDspName		=	hive.strDspName;  
	strReqParams	=	hive.strReqParams;  
	strProvider		=	hive.strProvider; 
	strProvince		=	hive.strProvince;
	strDayId		=	hive.strDayId;
	strHourId		=	hive.strHourId;

}


}HIVELOCALLOG_S;

typedef struct {
	std::string m_strStatisticsPath;
	//std::string m_strSslKeyPath;
	std::string m_strStatisticsFileName;
	u_int m_uiStatisticsTime;
}STATISTICSPRM_S;

typedef struct UserInfo{
	
	ULONGLONG m_ullReqNum;
	ULONGLONG m_ullTotalReqNum;
	ULONGLONG m_ullResNum;
	ULONGLONG m_ullResTagNum;
	ULONGLONG m_ullTotalResNum;
	ULONGLONG m_ullEmptyResNum;
	
	ULONGLONG m_ullResErrorNum;
	ULONGLONG m_ullTotalEmptyResNum;
	
/*
	bool operator <(const UserInfo& other) const
    {
        if (nType < other.nType)        //类型按升序排序
        {
            return true;
        }
        else if (nType == other.nType)  //如果类型相同，按比例尺升序排序
        {
            return nScale < other.nScale;
        }
        
        return false;
    }
    
*/


	
}USERINFO_S;


typedef struct UserQueryReport{

        std::map<std::string,USERINFO_S> m_strUserInfo;
        void operator +=(  UserQueryReport & stReport) {
                        std::map<std::string,USERINFO_S>::iterator itr;
                        for( itr = stReport.m_strUserInfo.begin();itr!= stReport.m_strUserInfo.end();itr++)
                        {
                             
                                m_strUserInfo[itr->first].m_ullReqNum += stReport.m_strUserInfo[itr->first].m_ullReqNum ;
								//m_strUserInfo[itr->first].m_ullTotalReqNum += stReport.m_strUserInfo[itr->first].m_ullTotalReqNum ;
                                m_strUserInfo[itr->first].m_ullResNum+= stReport.m_strUserInfo[itr->first].m_ullResNum ;
                                m_strUserInfo[itr->first].m_ullEmptyResNum+= stReport.m_strUserInfo[itr->first].m_ullEmptyResNum ;
                                m_strUserInfo[itr->first].m_ullResTagNum+= stReport.m_strUserInfo[itr->first].m_ullResTagNum ;
								m_strUserInfo[itr->first].m_ullResErrorNum+= stReport.m_strUserInfo[itr->first].m_ullResErrorNum ;
								

                        }

         }
	


	void operator =(const int iValue) {
			//printf("operator = clear\n");
			m_strUserInfo.clear();

	 }
}UESRQUERYRPORT_S;


		



typedef struct {
	char m_pchIp[_128BYTES];
	in_port_t m_uiPort;
	int m_idx;
	int m_count;
}IPPORT_S;

typedef struct {
	IPPORT_S m_stRedisServer;
	IPPORT_S m_stLocalServer;
	IPPORT_S m_stTokenServer;
	IPPORT_S m_stGoodsServer;
	
}SERVERINFO_S;

typedef struct {
	IPPORT_S m_stMysqlLinkInfo;
	char  pchUserName[_128BYTES];
	char  pchPassWord[_128BYTES];
	char  pchDbName[_128BYTES];
	
}MYSQL_SERVERINFO_S;

typedef struct {
	char  mParam[_128BYTES];
	char  mCarrierOperator[BUF_SIZE_8BYTE];
	//long int mProvince;
	std::string mProvince;
	long int mQueryLimits;
}QUERYAPIINFO_S;

	
typedef struct {
	IPPORT_S m_stMonitorApi;
	IPPORT_S m_stEmailServer;
	std::string m_strUserName;
	std::string m_strPassWord;
	std::string m_strMailFrom;
	std::string m_strRcptTo;
	std::string m_strSubject;
	std::string m_strErrorMsg;
	
}MORNITORAPI_INFO_S;
typedef struct BdxReuest
{
	struct timeval m_stTimeStamp;  
	std::string m_strRequestID;

	//int m_iHttpType;
}BDXREQUEST_S;


typedef struct ResponseInfo
{
	struct timeval timeStamp;
	std::string mResValue;
	std::string ssUserCountKeyReq;
	std::string ssUserCountKeyReqError;
	std::string ssUserCountKeyRes;
	std::string ssUserCountKeyEmptyRes;
	std::string ssUserCountKeyLimitReq;
	std::string ssUserCountKeyUserLimitReq;
	std::string ssUserCountKeyGoodsRes;
	std::string ssUserCountKeyGoodsLimitReq;
	std::string ssUserName;
	std::string ssOperatorName;
	std::string ssOperatorNameKeyReq;
	std::string ssOperatorNameKeyReqError;
	std::string ssOperatorNameKeyEmptyRes;
	std::string ssOperatorNameKeyRes;
	std::string ssOperatorNameKeyResTag;
	std::string ssOperatorNameKeyLimit;
	int queryType;
	
}BDXRESPONSE_S;

typedef struct ReqPermission
{

	std::string mResToken;
	int mIntQueryTimes;
	int mIntGoodsTimes;
	std::vector<std::string> mVecFields;
	std::string mGoodsFields;
	
}BDXPERMISSSION_S;

typedef struct {
	int m_iSockFd;
	bool m_bStatus;
}TASKSTATUS_S;

typedef struct BdxSuffix {      
	std::string str1;        
	std::string str2;        
	std::string str3;        
	std::string str4;        
	std::string str5;        
	std::string str6;        
	std::string str7;        
	std::string str8;        
	std::string str9;        
	std::string str10;        
	std::string str11;        
	std::string str12;        
	std::string str13;        
	std::string str14;        
	std::string str15;        
	std::string str16;        
	std::string str17;        
	std::string str18;       
	std::string str19;        
	BdxSuffix() { 
		str1 = "asia"; 
		str2 = "biz"; 
		str3 = "cc";
		str4 = "cn";
		str5 = "co";
		str6 = "com";
		str7 = "edu";
		str8 = "gov";
		str9 = "hk";
		str10 = "info";
		str11 = "me"; 
		str12 = "mobi"; 
		str13 = "name";
		str14 = "net";
		str15 = "org";
		str16 = "so";
		str17 = "tel"; 
		str18 = "tv"; 
		str19 = "www";        
	};     
	bool operator !=(const std::string strDomain) 
		{   
		  return (strDomain != str1 && strDomain != str2 && strDomain != str3 && strDomain != str4 && strDomain != str5 && strDomain != str6 && strDomain != str7 && strDomain != str8&& strDomain != str9 && strDomain != str10 && strDomain != str11 && strDomain != str12 && strDomain != str13 && strDomain != str14 && strDomain != str15 && strDomain != str16 && strDomain != str17 && strDomain != str18 && strDomain != str19);
		};
	}BDXURLSUFFIX_S;


#endif /* __DATA_STRUCT_H__ */
