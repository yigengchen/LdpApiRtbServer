/*
 * TaskMain.cpp
 */

#include "TaskMain.h"
#include "../../CommonTools/UrlEncode/UrlEncode.h"
#include "../../UserQueryWorkThreads/UserQueryWorkThreads.h"

#include "../../CommonTools/Base64Encode/Base64.h"
#include "../../CommonTools/Base64Encode/Base64_2.h"
#include "../../../include/json/json.h"
#include <arpa/inet.h>
#include <stdlib.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/err.h>



extern CLog *gp_log;
const char* CTaskMain::m_pszHttpHeaderEnd = "\r\n\r\n";
const char* CTaskMain::m_pszHttpLineEnd = "\r\n";
const std::string CTaskMain::keyEdcpMd5Sign="edc_543_key_155&";
extern std::map<std::string,BDXPERMISSSION_S> g_mapUserInfo;
extern std::map<std::string,int> g_mapUserQueryLimit;
extern std::map<std::string,QUERYAPIINFO_S> g_vecUrlAPIS;


extern pthread_rwlock_t p_rwlock;
extern pthread_rwlockattr_t p_rwlock_attr;
extern pthread_mutex_t mutex;
extern std::string g_strTokenString ;
extern std::string ssToken;
extern u_int  g_iNeedUpdateToken ;
extern int iAPIQpsLimit;

int InitSSLFlag = 0;

static const string http=" HTTP/1.1";

static const char http200ok[] = "HTTP/1.1 200 OK\r\nServer: Bdx DMP/0.1.0\r\nCache-Control: must-revalidate\r\nExpires: Thu, 01 Jan 1970 00:00:00 GMT\r\nPragma: no-cache\r\nConnection: Keep-Alive\r\nContent-Type: application/json;charset=UTF-8\r\nDate: ";
//static const char http200ok[] = "";
static const char httpReq[]="GET %s HTTP/1.1\r\nHost: %s\r\nAccept-Encoding: identity\r\n\r\n";

#define __EXPIRE__


CTaskMain::CTaskMain(CTcpSocket* pclSock):CUserQueryTask(pclSock)
{
	// TODO Auto-generated constructor stub
	m_piKLVLen = (int*)m_pszKLVBuf;
	m_piKLVContent = m_pszKLVBuf + sizeof(int);
	*m_piKLVLen = 0;
}

CTaskMain::CTaskMain()
{

}

CTaskMain::~CTaskMain() {
	// TODO Auto-generated destructor stub

}

int CTaskMain::BdxRunTask(BDXREQUEST_S& stRequestInfo, BDXRESPONSE_S& stResponseInfo)
{
	string keyReq = "Req_"+BdxTaskMainGetTime();
	string keyEmptyRes = "EmptyRes_"+BdxTaskMainGetTime();
	string strErrorMsg;
	string retKey,retKeyType,retUser;
    HIVELOCALLOG_S stHiveEmptyLog;
	int iRes = 0;
	if(!m_pclSock) {
		LOG(ERROR, "[thread: %d]m_pclSock is NULL.", m_uiThreadId);
		return LINKERROR;
	}

	iRes = 	BdxGetHttpPacket(stRequestInfo,stResponseInfo,retKey,retKeyType,retUser,strErrorMsg);	
	LOG(DEBUG,"BdxGetHttpPacket iRes=%d",iRes);
	//printf("strErrorMsg=%s\n",strErrorMsg.c_str());
	if(iRes == SUCCESS )//&& !stRequestInfo.m_strUserID.empty() /*&& m_bSend*/) 
	{
		return BdxSendRespones( stRequestInfo, stResponseInfo,strErrorMsg);
	}
	else
	{
				
		stHiveEmptyLog.strLogValue=strErrorMsg;
		if(retKey.empty())
		{
			stHiveEmptyLog.strLogKey="Empty";
		}
		else
		{	
			stHiveEmptyLog.strLogKey=retKey;
		}
		if(retKeyType.empty())
		{
			stHiveEmptyLog.strLogKeyType="Empty";
		}
		else
		{	
			stHiveEmptyLog.strLogKeyType=retKeyType;
		}

		stHiveEmptyLog.strCreateTime="Empty";
		stHiveEmptyLog.strLastDataTime="Empty";
		stHiveEmptyLog.strQueryTime=BdxTaskMainGetFullTime();
		stHiveEmptyLog.strDspName=retUser;
		stHiveEmptyLog.strProvider="Empty";
		stHiveEmptyLog.strProvince="Empty";
		stHiveEmptyLog.strDayId=BdxTaskMainGetDate();
		stHiveEmptyLog.strHourId=stHiveEmptyLog.strQueryTime.substr(8,2);	
		CUserQueryWorkThreads::m_vecHiveLog[m_uiThreadId].push(stHiveEmptyLog);

		if ( iRes == ERRORNODATA)
		{

			if(CUserQueryWorkThreads::m_vecReport[m_uiThreadId].m_strUserInfo.find(stResponseInfo.ssUserName)	
				!=	CUserQueryWorkThreads::m_vecReport[m_uiThreadId].m_strUserInfo.end())
		{

			CUserQueryWorkThreads::m_vecReport[m_uiThreadId].m_strUserInfo[stResponseInfo.ssUserName].m_ullEmptyResNum++;
			CUserQueryWorkThreads::m_vecReport[m_uiThreadId].m_strUserInfo[stResponseInfo.ssUserName].m_ullTotalEmptyResNum++;

		}
			
			printf("ssUserCountKeyEmptyRes=%s\n",stResponseInfo.ssUserCountKeyEmptyRes.c_str());
			if( stResponseInfo.queryType==2 )
			{
				m_pGoodsRedis->UserIncr(stResponseInfo.ssUserCountKeyEmptyRes);
			}
			else
			{
				m_pDataRedis->UserIncr(stResponseInfo.ssUserCountKeyEmptyRes);
			}
			
			//return BdxSendEmpyRespones(stResponseInfo);
		}
		else if ( iRes == EXCEEDLIMIT)
		{
			//CUserQueryWorkThreads::m_vecReport[m_uiThreadId].m_strUserInfo[stResponseInfo.ssUserName].m_ullResNum++;
			//CUserQueryWorkThreads::m_vecReport[m_uiThreadId].m_strUserInfo[""].m_ullResNum++;
			CUserQueryWorkThreads::m_vecReport[m_uiThreadId].m_strUserInfo[stResponseInfo.ssUserName].m_ullEmptyResNum++;
			m_pDataRedis->UserIncr(stResponseInfo.ssUserCountKeyEmptyRes);
		}
		else
		{
			CUserQueryWorkThreads::m_vecReport[m_uiThreadId].m_strUserInfo[""].m_ullReqNum++;
			//CUserQueryWorkThreads::m_vecReport[m_uiThreadId].m_strUserInfo[""].m_ullResNum++;
			CUserQueryWorkThreads::m_vecReport[m_uiThreadId].m_strUserInfo[""].m_ullEmptyResNum++;
			CUserQueryWorkThreads::m_vecReport[m_uiThreadId].m_strUserInfo[""].m_ullTotalReqNum++;
			//CUserQueryWorkThreads::m_vecReport[m_uiThreadId].m_strUserInfo[""].m_ullTotalResNum++;
			CUserQueryWorkThreads::m_vecReport[m_uiThreadId].m_strUserInfo[""].m_ullTotalEmptyResNum++;
			m_pDataRedis->UserIncr(keyReq);
			m_pDataRedis->UserIncr(keyEmptyRes);
		}
			
			return BdxSendEmpyRespones(strErrorMsg);

	}
	return iRes;
}


int CTaskMain::BdxGetHttpPacket(BDXREQUEST_S& stRequestInfo,BDXRESPONSE_S &stResponseInfo,std::string &retKey,std::string &retKeyType,std::string &retUser,std::string &errorMsg)
{

	int iRes = 0;
	int iQueryCategory = 1;
	m_httpType = 0;
	bool bUpdateDatabase = false;
	bool bQueryUser = false;
	bool bQueryGoods = false;
	int iNoDataFlag = 0,isQueryAction = 0;
	std::string	
	ssUser,ssValue,ssKey,ssmoidValue,strUser,filterDate,strToken,strKey,strKeyType,strKeyFilter,tempstrKeyFilter,strShopId,strGoodsId,strProvince,strOperator,strMoId;
	std::string strProvinceReq,strProvinceRes,strProvinceEmptyRes,strProvinceResTag,strOperatorName;
	std::map<std::string,std::string> map_UserValueKey;
	std::map<std::string,std::string>::iterator iter2;
	std::map<std::string,BDXPERMISSSION_S>::iterator iter;
	std::vector<std::string>::iterator itRights;
	int iIncludeNoFields = 0;
	int mapActionFilterSize;
	std::map<std::string,int> mapActionFilter;
	std::string 
	mResUserAttributes,mResUserAttributes2,strTempValue,strCurrentHour;
	Json::Value jValue,jRoot,jResult,jTemp;
	int lenStrTemp;
	Json::FastWriter jFastWriter;
	int isExpire = 0, iIsLocal=0,iRemoteApiFlag = 0;
	string strCreateTime,strCreateTime2,strLastDataTime,strLastDataTime2,mResValueLocal,mResValueRemote;
	struct tm ts;
	time_t timetNow,timetCreateTime,timetCreateTime2;
	//Json::Reader jReader;
	Json::Reader *jReader= new Json::Reader(Json::Features::strictMode()); // turn on strict verify mode

	char chHostName[30];
	char *temp[PACKET]; 
	int  index = 0;
	char bufTemp[PACKET];
	char *buf;
	char *buf2;
	char *outer_ptr = NULL;  
	char *inner_ptr = NULL;  
	char m_httpReq[_8KBLEN];
	memset(chHostName, 0,30);
	memset(m_pszAdxBuf, 0, _8KBLEN);
	memset(m_httpReq, 0, _8KBLEN);
	//memset(buf, 0, _8KBLEN);
	memset(bufTemp, 0, PACKET);

	iRes = m_pclSock->TcpRead(m_pszAdxBuf, _8KBLEN);
  	LOG(DEBUG,"Requrest= %s\n",m_pszAdxBuf);  
	printf("Requrest= %s\n",m_pszAdxBuf);  
	if(iRes <= (int)http.length()) 
	{		
		LOG(DEBUG, "[thread: %d]Read Socket Error [%d].", m_uiThreadId, iRes);
		errorMsg="E0004";
		return LINKERROR;
	}

	std::string ssContent = std::string(m_pszAdxBuf);
	std::string tempssContent;
	unsigned int ipos = ssContent.find(CTRL_N,0);
	unsigned int jpos = ssContent.find(REQ_TYPE,0);
	
	if( std::string::npos !=jpos )
	{
		m_httpType = 1;
	}
	if(m_httpType )
	{
		ssContent = ssContent.substr(jpos+4,ipos-http.length()-(jpos+4));
		int ibegin = ssContent.find(SEC_Q,0);
		int iend =   ssContent.find(BLANK,0);
		if (ibegin!=-1 && iend !=-1)
		{
				if (strcasecmp(ssContent.substr(0,ssContent.find(SEC_Q,0)).c_str(),KEY_UPDATE)== 0)
				{
					bUpdateDatabase = true;
					return OTHERERROR;
				}	
				else if (strcasecmp(ssContent.substr(0,ssContent.find(SEC_Q,0)).c_str(),KEY_QUERY_USER)== 0)
				{						
					bQueryUser = true;
				//	#define __OLD_INTERFACE__
					#ifdef __OLD_INTERFACE__
					string temp = "&filter={\"typeids\":[\"0\",\"1\",\"2\",\"3\",\"4\",\"A\",\"B\",\"C\",\"D\",\"E\",\"idmapping\"]} ";
					iend += temp.length() + 1;
					ssContent=ssContent.substr(0,ssContent.length()-1)+ temp;
					#endif
				}else 
				if(strcasecmp(ssContent.substr(0,ssContent.find(SEC_Q,0)).c_str(),KEY_QUERY_GOODS)== 0)
				{
					bQueryGoods = true;
				}
				else
				{
					errorMsg ="E0000"; //request info is error  
					return OTHERERROR;
				}

				ssContent = ssContent.substr(ibegin+1,iend - ibegin-1);	
				string reqParams=ssContent;
				printf("reqParams=%s\n",reqParams.c_str());
				memcpy(bufTemp,ssContent.c_str(),ssContent.length());
				buf=bufTemp;
				while((temp[index] = strtok_r(buf, STRING_AND, &outer_ptr))!=NULL)   
				{  	
				    buf=temp[index];  
				    while((temp[index]=strtok_r(buf, STRING_EQUAL, &inner_ptr))!=NULL)   
				    {   if(index%2==1)
				        {
				            map_UserValueKey[temp[index-1]]=temp[index];
				            
				        }
				        index++;
				        buf=NULL;  
				    }  
				    buf=NULL;  
				}  			
				std::string strKeyCurrentHour="currentHour";
				if(!m_pDataRedis->UserGet(strKeyCurrentHour,strCurrentHour))
				{
					strCurrentHour=	BdxTaskMainGetTime(0);
				}
				if(strCurrentHour.empty())//fix the bug localtime 20151123
				{
					strCurrentHour=BdxTaskMainGetTime(0);
				}
				
				if( bQueryUser )
				{		
					if(map_UserValueKey.find(KEY_USER)!=map_UserValueKey.end()&&map_UserValueKey.find(KEY_VALUE)!=map_UserValueKey.end()&&map_UserValueKey.find(KEY_KEY)!=map_UserValueKey.end()&&map_UserValueKey.find(KEY_KEY_TYPE)!=map_UserValueKey.end()&&map_UserValueKey.find(KEY_FILTER)!=map_UserValueKey.end())
					{	
						strUser = map_UserValueKey.find(KEY_USER)->second;
						strToken = map_UserValueKey.find(KEY_VALUE)->second;
						strKey = map_UserValueKey.find(KEY_KEY)->second;
						strKeyType = map_UserValueKey.find(KEY_KEY_TYPE)->second;
						strKeyFilter = map_UserValueKey.find(KEY_FILTER)->second;

						//stResponseInfo.ssUserCountKeyReq = "Req_" + BdxTaskMainGetTime(0)+"_"+strUser+"_"+strKeyType;//map_UserValueKey.find(KEY_USER)->second;
						stResponseInfo.ssUserCountKeyReq = "Req_" + strCurrentHour+"_"+strUser+"_"+strKeyType;//map_UserValueKey.find(KEY_USER)->second;

						//LOG(ERROR,"Thread:%d,stResponseInfo.ssUserCountKeyReq=%s",m_uiThreadId,stResponseInfo.ssUserCountKeyReq.c_str());
						//stResponseInfo.ssUserCountKeyEmptyRes = "EmptyRes_" +BdxTaskMainGetTime()+"_"+strUser+"_"+strKeyType;//map_UserValueKey.find(KEY_USER)->second;
						stResponseInfo.ssUserCountKeyLimitReq="Limit_"+BdxTaskMainGetDate()+"_"+strUser;//map_UserValueKey.find(KEY_USER)->second;;
						stResponseInfo.ssUserCountKeyUserLimitReq = "Limit_"+BdxTaskMainGetDate()+"_"+strUser+"_1";//map_UserValueKey.find(KEY_USER)->second;;

						//stResponseInfo.ssUserCountKeyEmptyRes ="EmptyRes_"+BdxTaskMainGetTime()+"_"+ strUser+"_"+strKeyType;
						stResponseInfo.ssUserCountKeyEmptyRes ="EmptyRes_"+strCurrentHour+"_"+ strUser+"_"+strKeyType;
						stResponseInfo.ssUserName = strUser+"_"+strKeyType;//map_UserValueKey.find(KEY_USER)->second ;
						m_pDataRedis->UserIncr(stResponseInfo.ssUserCountKeyUserLimitReq);
						m_pDataRedis->UserIncr(stResponseInfo.ssUserCountKeyReq); ////req++	
						
						CUserQueryWorkThreads::m_vecReport[m_uiThreadId].m_strUserInfo[stResponseInfo.ssUserName].m_ullReqNum++;
						CUserQueryWorkThreads::m_vecReport[m_uiThreadId].m_strUserInfo[stResponseInfo.ssUserName].m_ullTotalReqNum++;

					     retKey = strKey;
					     retKeyType = strKeyType;
					     retUser = strUser;
						if(strKeyFilter.find("date")!=-1)
						{
							isQueryAction = 1;
						}
					    	printf("Line:%d,isQueryAction=%d\n",__LINE__,isQueryAction); 
						printf("strKeyFilter=========++++=%s\n",strKeyFilter.c_str());
						strKeyFilter = url_decode(strKeyFilter.c_str());
						tempstrKeyFilter=strKeyFilter;
						printf("strKeyFilter=========++++=%s\n",strKeyFilter.c_str());
						if(!jReader->parse(strKeyFilter, jValue))
						{ 
							errorMsg = "E0006"; // json data format is wrong
							printf("line %d,s Error: %s\n",__LINE__,errorMsg.c_str());
							stResponseInfo.ssUserCountKeyReqError=stResponseInfo.ssUserCountKeyReq+"_"+errorMsg;
							m_pDataRedis->UserIncr(stResponseInfo.ssUserCountKeyReqError);
							return	ERRORNODATA;
						}
						mResUserAttributes = jValue.toStyledString();

						if(!jValue["date"].isNull())
						{
							string tempString =jValue["date"].toStyledString();
							filterDate=tempString.substr(tempString.find("\"")+1,tempString.rfind("\"")-tempString.find("\"")-1);
							
						}

						printf("strKeyFilter=%s\n",strKeyFilter.c_str());
						unsigned int ipos = strKeyFilter.rfind("]",-1);
						unsigned int jpos = strKeyFilter.rfind("[",-1);
						printf("ipos=%d,jpos=%d\n",ipos,jpos);

						strKeyFilter=strKeyFilter.substr(jpos+1,ipos-jpos-1);
						printf("strKeyFilter=%s\n",strKeyFilter.c_str());
						char *token;
						char *temptoken=NULL;
						//char temptoken[50];
						


						memset(bufTemp, 0, PACKET);
						memcpy(bufTemp,strKeyFilter.c_str(),strKeyFilter.length());
						buf2=bufTemp;
						//buf2= const_cast< char*>(BdxTaskMainReplace_All(std::string(buf2),std::string("\""),std::string("")).c_str());

						std::string ttttt=BdxTaskMainReplace_All(std::string(buf2),std::string("\""),std::string(""));
						buf2=const_cast< char*>(ttttt.c_str());
						
						token = strtok_r( buf2,",",&outer_ptr );
						temptoken=token;
						printf( "token 11111 %s\n", token );
						//temptoken= const_cast< char*>(BdxTaskMainReplace_All(std::string(temptoken),std::string("\""),std::string("")).c_str());
						printf( "temptoken 11111 %s\n", temptoken );
						mapActionFilter.clear();
						mapActionFilter[std::string(temptoken)]=0;
						while( token != NULL )
						{
						     
						      /* Get next token: */
						      token = strtok_r( NULL,",",&outer_ptr );
						      printf( "token 22222 %s\n", token);

						      if(token!=NULL)
						      {	
						       temptoken=token;
						       //temptoken= const_cast< char*>(BdxTaskMainReplace_All(std::string(temptoken),std::string("\""),std::string("")).c_str());
						       printf( "temptoken 22222 %s\n", temptoken );
								mapActionFilter[std::string(temptoken)]=0;
						      }
						      
						}

						for(std::map<string,int>::iterator it=mapActionFilter.begin();it!=mapActionFilter.end();it++)
						{

							if(find(g_mapUserInfo[strUser].mVecFields.begin(),g_mapUserInfo[strUser].mVecFields.end(),it->first)==g_mapUserInfo[strUser].mVecFields.end())
							{
								errorMsg = "E0011"; // filter include  no right field
								
								printf("line %d,s it->first=%s,Error: %s\n",__LINE__,it->first.c_str(),errorMsg.c_str());
								stResponseInfo.ssUserCountKeyReqError=stResponseInfo.ssUserCountKeyReq+"_"+errorMsg;
								m_pDataRedis->UserIncr(stResponseInfo.ssUserCountKeyReqError);
								return	ERRORNODATA;

							}
						
						 }


					     
					     if(map_UserValueKey.find(KEY_KEY_SRC)!=map_UserValueKey.end())
					     {
							strProvince = map_UserValueKey.find(KEY_KEY_SRC)->second;
					     }
					     if(map_UserValueKey.find(KEY_OPERATOR)!=map_UserValueKey.end())
					     {
							strOperator = map_UserValueKey.find(KEY_OPERATOR)->second;
					     }
   
						iter = g_mapUserInfo.find(strUser);
						if(iter!=g_mapUserInfo.end())
						{	
						    if(iter->second.mResToken == strToken)
						    {							    	
								if(	map_UserValueKey.find(KEY_KEY)!= map_UserValueKey.end())
								{
									ssContent = strKey;
								}
								else
								{								
									errorMsg = "E0000";  // request info is error  
									printf("line %d,s Error: %s\n",__LINE__,errorMsg.c_str());
									stResponseInfo.ssUserCountKeyReqError=stResponseInfo.ssUserCountKeyReq+"_"+errorMsg;
									m_pDataRedis->UserIncr(stResponseInfo.ssUserCountKeyReqError);
									//++
									return OTHERERROR;
								}
							}
							else
							{
								errorMsg = "E0003"; //passwd not match
								printf("line %d,s Error: %s\n",__LINE__,errorMsg.c_str());
								stResponseInfo.ssUserCountKeyReqError=stResponseInfo.ssUserCountKeyReq+"_"+errorMsg;
								m_pDataRedis->UserIncr(stResponseInfo.ssUserCountKeyReqError);
								//++
								return OTHERERROR;
							}
							
						}
						else
						{
							errorMsg ="E0002"; //user is not exists
							printf("line %d,s Error: %s\n",__LINE__,errorMsg.c_str());
							stResponseInfo.ssUserCountKeyReqError=stResponseInfo.ssUserCountKeyReq+"_"+errorMsg;
							m_pDataRedis->UserIncr(stResponseInfo.ssUserCountKeyReqError);
							//++
							return OTHERERROR;
						}
					}
					else
					{
							errorMsg ="E0004"; //param missing or error
							printf("line %d,s Error: %s\n",__LINE__,errorMsg.c_str());
							stResponseInfo.ssUserCountKeyReqError=stResponseInfo.ssUserCountKeyReq+"_"+errorMsg;
							m_pDataRedis->UserIncr(stResponseInfo.ssUserCountKeyReqError);
							//++
							return OTHERERROR;
					}
				 }

				if( bQueryGoods )
				{
					if(map_UserValueKey.find(KEY_USER)!=map_UserValueKey.end()&&map_UserValueKey.find(KEY_VALUE)!=map_UserValueKey.end()&&map_UserValueKey.find(KEY_SHOPID)!=map_UserValueKey.end()&&map_UserValueKey.find(KEY_GOODSID)!=map_UserValueKey.end())
					{	
					     strUser = map_UserValueKey.find(KEY_USER)->second;
					     strToken = map_UserValueKey.find(KEY_VALUE)->second;
					     strShopId = map_UserValueKey.find(KEY_SHOPID)->second;
					     strGoodsId = map_UserValueKey.find(KEY_GOODSID)->second;
					     
					     strKey = strShopId + "_" + strGoodsId;
					     strKeyType = "goods_"+strShopId;

					     retKey = strKey;
					     retKeyType = strKeyType;
					     retUser = strUser;
					     
  						//stResponseInfo.ssUserCountKeyReq = "Req_" + BdxTaskMainGetTime()+"_"+strUser+"_"+strKeyType;//map_UserValueKey.find(KEY_USER)->second;
  						stResponseInfo.ssUserCountKeyReq = "Req_" + strCurrentHour+"_"+strUser+"_"+strKeyType;//map_UserValueKey.find(KEY_USER)->second;
  						//stResponseInfo.ssUserCountKeyEmptyRes = "EmptyRes_" +BdxTaskMainGetTime()+"_"+strUser+"_"+strKeyType;//map_UserValueKey.find(KEY_USER)->second;
  						//stResponseInfo.ssUserCountKeyLimitReq="Limit_"+BdxTaskMainGetDate()+"_"+strUser;//map_UserValueKey.find(KEY_USER)->second;;
  						//stResponseInfo.ssUserCountKeyUserLimitReq = "Limit_"+BdxTaskMainGetDate()+"_"+strUser+"_1";//map_UserValueKey.find(KEY_USER)->second;;
  						//stResponseInfo.ssUserCountKeyEmptyRes ="EmptyRes_"+BdxTaskMainGetTime()+"_"+ strUser+"_"+strKeyType;
  						stResponseInfo.ssUserCountKeyEmptyRes ="EmptyRes_"+strCurrentHour+"_"+ strUser+"_"+strKeyType;
  						stResponseInfo.ssUserName = strUser+"_"+strKeyType;//map_UserValueKey.find(KEY_USER)->second ;
  						//m_pDataRedis->UserIncr(stResponseInfo.ssUserCountKeyUserLimitReq);
  						m_pGoodsRedis->UserIncr(stResponseInfo.ssUserCountKeyReq); ////req++	
  						
  						CUserQueryWorkThreads::m_vecReport[m_uiThreadId].m_strUserInfo[stResponseInfo.ssUserName].m_ullReqNum++;
  						CUserQueryWorkThreads::m_vecReport[m_uiThreadId].m_strUserInfo[stResponseInfo.ssUserName].m_ullTotalReqNum++;
						 
					    //stResponseInfo.ssUserCountKeyEmptyRes ="EmptyRes_"+BdxTaskMainGetTime()+"_"+ strUser+"_"+strKeyType;
					    stResponseInfo.ssUserCountKeyEmptyRes ="EmptyRes_"+strCurrentHour+"_"+ strUser+"_"+strKeyType;
					   
						iter = g_mapUserInfo.find(strUser);
						if(iter!=g_mapUserInfo.end())
						{	
						    if(iter->second.mResToken == strToken)
						    {							    	
								if(	map_UserValueKey.find(KEY_SHOPID)!= map_UserValueKey.end())
								{
									ssContent = strKey;
								}
								else
								{								
									errorMsg = "E0000";  // request info is error  
									return OTHERERROR;
								}
							}
							else
							{
								errorMsg = "E0003"; //passwd not match
								return OTHERERROR;
							}
							
						}
						else
						{
							errorMsg ="E0002"; //user is not exists
							return OTHERERROR;
						}
					}
					else
					{
							errorMsg ="E0004"; //param missing or error
							return OTHERERROR;
					}


				}


 
				
				//stResponseInfo.ssUserCountKeyReq = "Req_" + BdxTaskMainGetTime()+"_"+strUser+"_"+strKeyType;//map_UserValueKey.find(KEY_USER)->second;
				//stResponseInfo.ssUserCountKeyEmptyRes = "EmptyRes_" +BdxTaskMainGetTime()+"_"+strUser+"_"+strKeyType;//map_UserValueKey.find(KEY_USER)->second;
				//stResponseInfo.ssUserCountKeyLimitReq="Limit_"+BdxTaskMainGetDate()+"_"+strUser;//map_UserValueKey.find(KEY_USER)->second;;
				//stResponseInfo.ssUserCountKeyUserLimitReq = "Limit_"+BdxTaskMainGetDate()+"_"+strUser+"_1";//map_UserValueKey.find(KEY_USER)->second;;
				stResponseInfo.ssUserCountKeyGoodsLimitReq = "Limit_"+BdxTaskMainGetDate()+"_"+strUser+"_2";//map_UserValueKey.find(KEY_USER)->second;;
				//stResponseInfo.ssUserCountKeyRes = "Res_" + BdxTaskMainGetTime(0)+"_"+strUser+"_"+strKeyType;//map_UserValueKey.find(KEY_USER)->second;
				stResponseInfo.ssUserCountKeyRes = "Res_" +strCurrentHour+"_"+strUser+"_"+strKeyType;//map_UserValueKey.find(KEY_USER)->second;
				//LOG(ERROR,"Thread:%d,stResponseInfo.ssUserCountKeyRes=%s",m_uiThreadId,stResponseInfo.ssUserCountKeyRes.c_str());

				if( bQueryUser )
				{
					//stResponseInfo.ssOperatorNameKeyLimit ="Limit_"+BdxTaskMainGetMonth()+"_"+strProvince+"_"+strOperator;
					stResponseInfo.ssOperatorNameKeyLimit ="Limit_"+BdxTaskMainGetDate()+"_"+strProvince+"_"+strOperator;

					stResponseInfo.queryType=1;
				    if(m_pDataRedis->UserGet(stResponseInfo.ssUserCountKeyUserLimitReq,ssmoidValue)&&(g_mapUserInfo.find(map_UserValueKey.find(KEY_USER)->second)->second.mIntQueryTimes>0))
				    {	
						if(atoi(ssmoidValue.c_str())> g_mapUserInfo.find(strUser)->second.mIntQueryTimes)
						{
							errorMsg = "E0005";//user query times limit
							printf("line %d,s Error: %s\n",__LINE__,errorMsg.c_str());
							stResponseInfo.ssUserCountKeyReqError=stResponseInfo.ssUserCountKeyReq+"_"+errorMsg;
							m_pDataRedis->UserIncr(stResponseInfo.ssUserCountKeyReqError);
							return EXCEEDLIMIT;
						}
					}
					//m_pDataRedis->UserIncr(stResponseInfo.ssUserCountKeyUserLimitReq);
					//m_pDataRedis->UserIncr(stResponseInfo.ssUserCountKeyUserLimitReq);
					//m_pDataRedis->UserIncr(stResponseInfo.ssUserCountKeyReq); ////req++	
				}
				
				ssContent = ssContent.substr(0,ipos);
							
				if( bQueryGoods )
				{
					stResponseInfo.queryType=2;
					if(m_pDataRedis->UserGet(stResponseInfo.ssUserCountKeyGoodsLimitReq,ssmoidValue)&&(g_mapUserInfo.find(map_UserValueKey.find(KEY_USER)->second)->second.mIntGoodsTimes>0))
				    {	
						if(atoi(ssmoidValue.c_str())>= g_mapUserInfo.find(strUser)->second.mIntGoodsTimes)
						{
							errorMsg = "E0005";//user query times limit
							return EXCEEDLIMIT;
						}
					}
						m_pDataRedis->UserIncr(stResponseInfo.ssUserCountKeyGoodsLimitReq);
						if(g_mapUserInfo.find(strUser)->second.mGoodsFields.find(strShopId)==-1)
						{
							//errorMsg = "E1001";//interal use
							errorMsg = "E0012"; 
							return	ERRORNODATA;
						}

						if(m_pGoodsRedis->UserGet(ssContent,ssmoidValue))
						{	
							LOG(DEBUG,"ssmoidValue=%s",ssmoidValue.c_str());
							printf("ssmoidValue=%s\n",ssmoidValue.c_str());
							stResponseInfo.mResValue = ssmoidValue ;
							m_pGoodsRedis->UserIncr(stResponseInfo.ssUserCountKeyRes); 
							//CUserQueryWorkThreads::m_vecReport[m_uiThreadId].m_strUserInfo[stResponseInfo.ssUserName].m_ullResNum++;
							return	SUCCESS;

						}
						else
						{	
							//m_pGoodsRedis->UserIncr(stResponseInfo.ssUserCountKeyEmptyRes); 
							//CUserQueryWorkThreads::m_vecReport[m_uiThreadId].m_strUserInfo[stResponseInfo.ssUserName].m_ullEmptyResNum++;
							ssContent="REAL_"+ssContent;
							printf("ssContent=%s\n",ssContent.c_str());
							m_pGoodsRedis->UserPut(ssContent,"1");
							errorMsg = "E0001";
							return	ERRORNODATA;	
						}
		
				}
				
				//#define __NOLOCAL__
				#ifdef __NOLOCAL__
				if((atol(strProvince.c_str()) == 320000 ||strProvince == "null"||strProvince=="")
					&&(strOperator=="null"||strOperator==""||strOperator=="cuc"))
				#endif //__NOLOCAL__
				{		//	jslt

						tempssContent =  ssContent;
						if( isQueryAction==1 )
						{	
							ssContent=ssContent+"_"+filterDate;
						}
							
						if(m_pDataRedis->UserGet(ssContent,ssmoidValue))
						{	
							LOG(DEBUG,"ssmoidValue=%s",ssmoidValue.c_str());
							printf("line:%d,ssmoidValue=%s\n",__LINE__,ssmoidValue.c_str());
							if(!m_pDataRedis->UserGet(ssmoidValue,stResponseInfo.mResValue))
							{
								iNoDataFlag = 1;

							}
							LOG(DEBUG,"stResponseInfo.mResValue=%s",stResponseInfo.mResValue.c_str());
							printf("stResponseInfo.mResValue=%s\n",stResponseInfo.mResValue.c_str());
							
							if(!jReader->parse(stResponseInfo.mResValue, jValue))
							{ 
								errorMsg = "E0006"; // json data format is wrong
								printf("line %d,s Error: %s\n",__LINE__,errorMsg.c_str());
								stResponseInfo.ssUserCountKeyReqError=stResponseInfo.ssUserCountKeyReq+"_"+errorMsg;
								m_pDataRedis->UserIncr(stResponseInfo.ssUserCountKeyReqError);
								return	ERRORNODATA;
							}

							
							int count=1;
							string temp1;
							for(unsigned int i=0;i<jValue.size();i++)
							{
								if(!jValue[i]["typeid"].isNull())
								{
									if((!jValue[i]["name"].isNull())&&(!jValue[i]["value"].isNull()))
									{
											for(std::map<string,int>::iterator it=mapActionFilter.begin();it!=mapActionFilter.end();it++)
											{
												printf("jValue[i][\"typeid\"]=%s\n",jValue[i]["typeid"].asString().c_str());
												if( (jValue[i]["typeid"] == it->first)||(jValue[i]["typeid"] == "time") )
												{
													if(it->second!=1)
													{
														it->second=1;
														
													}
											
													if(count==1)
													{
														temp1="{\"name\":\""+jValue[i]["name"].asString()+"\",\"value\":\""+jValue[i]["value"].asString()+"\",\"typeid\":\""+jValue[i]["typeid"].asString()+"\"}";
													}
													else
													{
														temp1="{\"name\":\""+jValue[i]["name"].asString()+"\",\"value\":\""+jValue[i]["value"].asString()+"\",\"typeid\":\""+jValue[i]["typeid"].asString()+"\"},"+temp1;
													}
													count++;
													
													printf("temp1=%s\n",temp1.c_str());
													break;
												}

											}

									  }

									}

								}

							if(!temp1.empty())
							{
								stResponseInfo.mResValue = "["+temp1+"]";
							}
							else
							{
								stResponseInfo.mResValue="";
							}
							
							printf("stResponseInfo.mResValue===========%s\n",stResponseInfo.mResValue.c_str());

							for(std::map<string,int>::iterator it=mapActionFilter.begin();it!=mapActionFilter.end();it++)
							{
								if(it->second==0)
								{
									iNoDataFlag = 1;
									break;
								}
							}
							
						}
						else 
						{
							//when no data in local redis,then query remote api
							//errorMsg = "E0001";	// uid is not exists
							iNoDataFlag = 1;
							//return ERRORNODATA;
						}
						mResValueLocal = stResponseInfo.mResValue;
						ssContent = tempssContent;
				}

				//judge the local data if expired

				#ifdef __EXPIRE__				
				if( iNoDataFlag == 0 )
				{			
					LOG(DEBUG,"stResponseInfo.mResValue=%s,isExpire=%d,line %d",stResponseInfo.mResValue.c_str(),isExpire,__LINE__);  
					printf("line %d  stResponseInfo.mResValue=%s\n",__LINE__,stResponseInfo.mResValue.c_str());
					if( stResponseInfo.mResValue.at(0)!='[' )
					{
						LOG(DEBUG,"line: %d,Error:%s ,value %s",__LINE__,errorMsg.c_str(),stResponseInfo.mResValue.c_str());
						printf("line %d,s Error:%s,value %s\n",__LINE__,errorMsg.c_str(),stResponseInfo.mResValue.c_str());
						errorMsg = "E0006"; // json data format is wrong
						printf("line %d,s Error: %s\n",__LINE__,errorMsg.c_str());
						stResponseInfo.ssUserCountKeyReqError=stResponseInfo.ssUserCountKeyReq+"_"+errorMsg;
						m_pDataRedis->UserIncr(stResponseInfo.ssUserCountKeyReqError);
						return	ERRORNODATA;
					}
					if(!jReader->parse(stResponseInfo.mResValue, jValue))
					{ 
						errorMsg = "E0006"; // json data format is wrong
						printf("line %d,s Error: %s\n",__LINE__,errorMsg.c_str());
						stResponseInfo.ssUserCountKeyReqError=stResponseInfo.ssUserCountKeyReq+"_"+errorMsg;
						m_pDataRedis->UserIncr(stResponseInfo.ssUserCountKeyReqError);
						return	ERRORNODATA;
					}
					if( isQueryAction!=1 )
					{
					 for(unsigned int i=0;i<jValue.size();i++)
					 {
						if(jValue[i]["name"].asString()== "create_time")
						{

							timetNow = time(NULL);
							strCreateTime = jValue[i]["value"].asString();
					        ts.tm_year=atoi(strCreateTime.substr(0,4).c_str())-1900;
					        ts.tm_mon=atoi(strCreateTime.substr(4,2).c_str())-1;
					        ts.tm_mday=atoi(strCreateTime.substr(6,2).c_str());
					        ts.tm_hour=atoi(strCreateTime.substr(8,2).c_str());
					        ts.tm_min=atoi(strCreateTime.substr(10,2).c_str());
					        ts.tm_sec=atoi(strCreateTime.substr(12,2).c_str());
					        timetCreateTime = mktime(&ts);
					        LOG(DEBUG,"timetCreateTime=%d",timetCreateTime);
					        LOG(DEBUG,"timetNow=%d",timetNow);
					        printf("timetCreateTime=%d\n",timetCreateTime);
					        printf("timetNow=%d\n",timetNow);
					        //the createtime is more than 2 days,it's to be dealed with expired data
					        if (( timetNow - timetCreateTime)>=86400*2)
					        {
										isExpire = 1;
					        }
						}
						if(jValue[i]["name"].asString()== "last_data_time")
						{
							strLastDataTime = jValue[i]["value"].asString();
						}
						mResValueLocal = stResponseInfo.mResValue;
					}


					}
					
					
				}

				#endif

			printf("iNoDataFlag=%d,isExpire=%d\n",iNoDataFlag,isExpire);
			LOG(DEBUG,"iNoDataFlag=%d,isExpire=%d",iNoDataFlag,isExpire); 
			#if 1
			#define __ZJYD__
			#ifdef __ZJYD__
				char remoteBuffer[_8KBLEN];
				CTcpSocket* remoteSocket;
				std::string remoteIp;
				uint16_t remotePort;
				string key;
				printf("strProvince=%s\n",strProvince.c_str());
				if( (iNoDataFlag == 1 || isExpire == 1)&&(!strProvince.empty())/*&&atoi(strKeyType.c_str())=1*/)
				{
						strKeyFilter= tempstrKeyFilter;
						//iQueryCategory  1 zjyd 2 qhyd 3 jslt
						if(330000 == atol(strProvince.c_str()))
						{
							iQueryCategory = 1;
							strProvinceReq="zhejiang_cmc_req_";
							strProvinceRes="zhejiang_cmc_res_";
							strProvinceResTag="zhejiang_cmc_restag_";
							strProvinceEmptyRes="zhejiang_cmc_emptyres_";
							
							strOperatorName="zhejiang_cmc_";
						}
						if(630000 == atol(strProvince.c_str()))
						{
							iQueryCategory = 2;
							strProvinceReq="qinghai_cmc_req_";
							strProvinceRes="qinghai_cmc_res_";
							strProvinceResTag="qinghai_cmc_restag_";
							strOperatorName="qinghai_cmc_";
							strProvinceEmptyRes="qinghai_cmc_emptyres_";
						}
						if(320000 == atol(strProvince.c_str()))
						{
							iQueryCategory = 3;
							strProvinceReq="jiangsu_cuc_req_";
							strProvinceRes="jisngsu_cuc_res_";
							strProvinceResTag="jiangsu_cuc_restag_";
							strOperatorName="jiangsu_cuc_";
							strProvinceEmptyRes="jiangsu_cuc_emptyres_";
						}
						
						int temp = iQueryCategory;
						printf("line: %d,iQueryCategory=%d\n",__LINE__,iQueryCategory);
						//for(std::map<std::string,QUERYAPIINFO_S>::iterator it=g_vecUrlAPIS.begin();it!=g_vecUrlAPIS.end();)
						std::map<std::string,QUERYAPIINFO_S>::iterator it=g_vecUrlAPIS.begin();	
						{			
							//it=it + iQueryCategory -1;
							iQueryCategory = 3; // urls count
							while( --iQueryCategory > 0 )
							{
								if( it->second.mProvince==atol(strProvince.c_str()))
								{
									break;
								}
								it++;
							}

							if( it->second.mProvince!=atol(strProvince.c_str()))
							{
								printf("Line:%d,strProvince %ld is not exists\n",__LINE__,atol(strProvince.c_str()));
							}
							iQueryCategory = temp;
							printf("line: %d,iQueryCategory=%d\n",__LINE__,iQueryCategory);
							if(it!=g_vecUrlAPIS.end())
							{
							switch(iQueryCategory)
							{
								case 1:

									if((it->second.mProvince == atol(strProvince.c_str())||strProvince == "null"||strProvince=="")
									&&(it->second.mCarrierOperator == strOperator||strOperator == "null"||strOperator == ""))
									//if((it->second.mCarrierOperator == strOperator||strOperator == "null"||strOperator == ""))
									{

									std::string strQpsLimit="zhejiang_qps_limit_"+ BdxTaskMainGetMinute();

									
									stResponseInfo.ssOperatorNameKeyReq = strProvinceReq + strKeyType +"_"+ BdxTaskMainGetTime();
									stResponseInfo.ssOperatorName = strOperatorName + strKeyType;
									stResponseInfo.ssOperatorNameKeyEmptyRes = strProvinceEmptyRes + strKeyType +"_"+ BdxTaskMainGetTime();

									m_pDataRedis->UserIncr(stResponseInfo.ssOperatorNameKeyReq);
									m_pDataRedis->UserIncr(strQpsLimit);
									
									CUserQueryWorkThreads::m_vecReport[m_uiThreadId].m_strUserInfo[stResponseInfo.ssOperatorName].m_ullReqNum++;
									
									if(m_pDataRedis->UserGet(stResponseInfo.ssOperatorNameKeyLimit,ssmoidValue))
									{	
										//运营商和省份都不为空的才限制;没有的时候只查本地,不做限制
										if(atol(ssmoidValue.c_str())>= it->second.mQueryLimits)//&&(!strProvince.empty()&&!strOperator.empty()))
										{
											errorMsg = "EE0005";//interal use
											printf("line %d,s Error: %s\n",__LINE__,errorMsg.c_str());
											stResponseInfo.ssOperatorNameKeyReqError=stResponseInfo.ssOperatorNameKeyReq+"_"+errorMsg;
											m_pDataRedis->UserIncr(stResponseInfo.ssOperatorNameKeyReqError);
											m_pDataRedis->UserIncr(stResponseInfo.ssOperatorNameKeyEmptyRes);
											return EXCEEDLIMIT;
										}
									}
									m_pDataRedis->UserIncr(stResponseInfo.ssOperatorNameKeyLimit);
									if(m_pDataRedis->UserGet(strQpsLimit,ssmoidValue))
									{	//qps limit
										if(atol(ssmoidValue.c_str())> iAPIQpsLimit)
										{
											errorMsg = "E0008";
											printf("line %d,s Error: %s\n",__LINE__,errorMsg.c_str());
											stResponseInfo.ssOperatorNameKeyReqError=stResponseInfo.ssOperatorNameKeyReq+"_"+errorMsg;
											m_pDataRedis->UserIncr(stResponseInfo.ssOperatorNameKeyReqError);
											stResponseInfo.ssUserCountKeyReqError=stResponseInfo.ssUserCountKeyReq+"_"+errorMsg;
											m_pDataRedis->UserIncr(stResponseInfo.ssUserCountKeyReqError);
											m_pDataRedis->UserIncr(stResponseInfo.ssOperatorNameKeyEmptyRes);
											return EXCEEDLIMIT;
										}
									}
										//when no data in local redis,then query remote api
										if(strKeyType =="0")//phone
										{
											//key="dsp_name="+strUser+"&cond=default&imei="+strKey;	
											key="dsp_name="+strUser+"&phone="+strKey+"&filter="+strKeyFilter;	
										}
										if(strKeyType =="1")//imei
										{
											//key="dsp_name="+strUser+"&cond=default&imei="+strKey;	
											key="dsp_name="+strUser+"&imei="+strKey+"&filter="+strKeyFilter;	
										}
										if(strKeyType=="2")//idfa
										{
											//key="dsp_name="+strUser+"&cond=default&idfa="+strKey;
											key="dsp_name="+strUser+"&idfa="+strKey+"&filter="+strKeyFilter;
										}
										if(strKeyType=="3")//aid
										{
											//key="dsp_name="+strUser+"&cond=default&dqc="+strKey;
											key="dsp_name="+strUser+"&dqc="+strKey+"&filter="+strKeyFilter;
										}
										if(strKeyType=="4")//pidfa
										{
											//key="dsp_name="+strUser+"&cond=default&pidfa="+strKey;
											key="dsp_name="+strUser+"&pidfa="+strKey+"&filter="+strKeyFilter;
										}
										if(strKeyType=="5")//sladuid
										{
											//key="dsp_name="+strUser+"&cond=default&pidfa="+strKey;
											key="dsp_name="+strUser+"&sladuid="+strKey+"&filter="+strKeyFilter;
										}
										printf("thread: %d key=%s\n",m_uiThreadId,key.c_str());
										
										std::string strAppSecret = "5543131e56e6a10d";
										std::string strHost = "dacp.sdp.com";
										std::string strUcTime = BdxTaskMainGetUCTime();
										std::string strNonce =  BdxGenNonce(25);
										std::string strPwdDigest =  GenPasswordDigest(strUcTime,strNonce,strAppSecret);
										std::string XWSSEHeader = "UsernameToken Username=\"f2b98698400e4ace9926f83f0d063a57\",PasswordDigest=\""+strPwdDigest + "\", Nonce=\""+strNonce + "\", Created=\""+ strUcTime+ "\"";
										//pthread_rwlock_wrlock(&p_rwlock);
										string strEncrpytKey;
										strEncrpytKey = std::string(it->second.mParam)+"&key="+Encrypt_2(key);	
										sprintf(m_httpReq,"GET %s HTTP/1.1\r\nAuthorization: WSSE realm=\"SDP\",profile=\"UsernameToken\",type=\"AppKey\"\r\nHost: %s\r\nX-WSSE: %s\r\nAccept-Encoding: identity\r\n\r\n",strEncrpytKey.c_str(),strHost.c_str(),XWSSEHeader.c_str());
										printf("%s\n",m_httpReq);
										LOG(DEBUG,"m_httpReq=%s",m_httpReq);
										remoteIp.assign(it->first,0,it->first.find(":",0));
										remotePort = atoi(it->first.substr(it->first.find(":",0)+1).c_str());										
										remoteSocket=new CTcpSocket(remotePort,remoteIp);
										if(remoteSocket->TcpConnect()!=0)
										{						
											errorMsg = "EE0009"; 
											printf("line %d,s Error: %s\n",__LINE__,errorMsg.c_str());
											stResponseInfo.ssOperatorNameKeyReqError=stResponseInfo.ssOperatorNameKeyReq+"_"+errorMsg;
											m_pDataRedis->UserIncr(stResponseInfo.ssOperatorNameKeyReqError);
											CUserQueryWorkThreads::m_vecReport[m_uiThreadId].m_strUserInfo[stResponseInfo.ssOperatorName].m_ullResErrorNum++;

											stResponseInfo.ssOperatorNameKeyReqError=stResponseInfo.ssOperatorNameKeyReq+"_"+errorMsg;
											m_pDataRedis->UserIncr(stResponseInfo.ssOperatorNameKeyReqError);
											
											errorMsg = "E0007";
											printf("line %d,s Error: %s\n",__LINE__,errorMsg.c_str());
											stResponseInfo.ssUserCountKeyReqError=stResponseInfo.ssUserCountKeyReq+"_"+errorMsg;		
											m_pDataRedis->UserIncr(stResponseInfo.ssUserCountKeyReqError);
											m_pDataRedis->UserIncr(stResponseInfo.ssOperatorNameKeyEmptyRes);
											//pthread_rwlock_unlock(&p_rwlock);
											return ERRORNODATA;
										}
										if(remoteSocket->TcpWrite(m_httpReq,strlen(m_httpReq))!=0)
										{
											memset(remoteBuffer,0,_8KBLEN);
											remoteSocket->TcpReadAll(remoteBuffer,_8KBLEN);
											printf("remoteBuffer=%s\n",remoteBuffer);
											LOG(DEBUG,"remoteBuffer=%s",remoteBuffer);
											if( strlen(remoteBuffer) > 0 )
											{
																
												#ifdef __EXPIRE__
												mResValueRemote = std::string(remoteBuffer);	
												#else
												stResponseInfo.mResValue = std::string(remoteBuffer);
												#endif														
												remoteSocket->TcpClose();
												break;
											 }
											 else
											 {
												//iQueryCategory++;
												remoteSocket->TcpClose();
												
												errorMsg = "EE0009";  // data is not exists
												printf("line %d,s Error: %s\n",__LINE__,errorMsg.c_str());
												stResponseInfo.ssOperatorNameKeyReqError=stResponseInfo.ssOperatorNameKeyReq+"_"+errorMsg;
												m_pDataRedis->UserIncr(stResponseInfo.ssOperatorNameKeyReqError);
												CUserQueryWorkThreads::m_vecReport[m_uiThreadId].m_strUserInfo[stResponseInfo.ssOperatorName].m_ullResErrorNum++;

												stResponseInfo.ssOperatorNameKeyReqError=stResponseInfo.ssOperatorNameKeyReq+"_"+errorMsg;
												m_pDataRedis->UserIncr(stResponseInfo.ssOperatorNameKeyReqError);

												errorMsg = "E0007";
												printf("line %d,s Error: %s\n",__LINE__,errorMsg.c_str());
												stResponseInfo.ssUserCountKeyReqError=stResponseInfo.ssUserCountKeyReq+"_"+errorMsg;		
												m_pDataRedis->UserIncr(stResponseInfo.ssUserCountKeyReqError);
												m_pDataRedis->UserIncr(stResponseInfo.ssOperatorNameKeyEmptyRes);
												return ERRORNODATA;
											 }		
										 }
										 else
										{
												
												remoteSocket->TcpClose();
												//errorMsg = "E0001";  //  internal connection socket error
												//return ERRORNODATA;
										}		
											  	
									}
									break;
								case 2:							
									if((it->second.mProvince == atol(strProvince.c_str())||strProvince == "null"||strProvince==""))
									//&&(it->second.mCarrierOperator == strOperator||strOperator == "null"||strOperator == ""))
									//if((it->second.mCarrierOperator == strOperator||strOperator == "null"||strOperator == ""))
									{									
									std::string strQpsLimit="qinghai_qps_limit_"+ BdxTaskMainGetMinute();
									
									if(m_pDataRedis->UserGet(stResponseInfo.ssOperatorNameKeyLimit,ssmoidValue))
									{	
										//运营商和省份都不为空的才限制;没有的时候只查本地,不做限制
										if(atol(ssmoidValue.c_str())>= it->second.mQueryLimits)//&&(!strProvince.empty()&&!strOperator.empty()))
										{
											errorMsg = "EE0005";// interal use
											stResponseInfo.ssOperatorNameKeyReqError=stResponseInfo.ssOperatorNameKeyReq+"_"+errorMsg;
											m_pDataRedis->UserIncr(stResponseInfo.ssOperatorNameKeyReqError);
											m_pDataRedis->UserIncr(stResponseInfo.ssOperatorNameKeyEmptyRes);
											return EXCEEDLIMIT;
										}
									}
									
									m_pDataRedis->UserIncr(stResponseInfo.ssOperatorNameKeyLimit);
									#if 0  //qps limit
									if(m_pDataRedis->UserGet(strQpsLimit,ssmoidValue))
									{	//qps limit
										if(atol(ssmoidValue.c_str())> iAPIQpsLimit)
										{
											errorMsg = "E0008";
											return EXCEEDLIMIT;
										}
									}
									m_pDataRedis->UserIncr(strQpsLimit);
									#endif
									stResponseInfo.ssOperatorNameKeyReq = strProvinceReq + strKeyType +"_"+ BdxTaskMainGetTime();
									m_pDataRedis->UserIncr(stResponseInfo.ssOperatorNameKeyReq);
									stResponseInfo.ssOperatorName = strOperatorName + strKeyType;
									stResponseInfo.ssOperatorNameKeyEmptyRes = strProvinceEmptyRes + strKeyType +"_"+ BdxTaskMainGetTime();
									CUserQueryWorkThreads::m_vecReport[m_uiThreadId].m_strUserInfo[stResponseInfo.ssOperatorName].m_ullReqNum++;
								
										//when no data in local redis,then query remote api
										if(strKeyType =="0")//phone
										{
											//key="dsp_name="+strUser+"&cond=default&imei="+strKey;	
											key="dsp_name="+strUser+"&phone="+strKey+"&filter="+strKeyFilter;	
										}
										if(strKeyType =="1")//imei
										{
											//key="dsp_name="+strUser+"&cond=default&imei="+strKey;	
											key="dsp_name="+strUser+"&imei="+strKey+"&filter="+strKeyFilter;	
										}
										if(strKeyType=="2")//idfa
										{
											//key="dsp_name="+strUser+"&cond=default&idfa="+strKey;
											key="dsp_name="+strUser+"&idfa="+strKey+"&filter="+strKeyFilter;
										}
										if(strKeyType=="3")//aid
										{
											//key="dsp_name="+strUser+"&cond=default&dqc="+strKey;
											key="dsp_name="+strUser+"&dqc="+strKey+"&filter="+strKeyFilter;
										}
										if(strKeyType=="4")//pidfa
										{
											//key="dsp_name="+strUser+"&cond=default&pidfa="+strKey;
											key="dsp_name="+strUser+"&pidfa="+strKey+"&filter="+strKeyFilter;
										}
										if(strKeyType=="5")//sladuid
										{
											//key="dsp_name="+strUser+"&cond=default&pidfa="+strKey;
											key="dsp_name="+strUser+"&sladuid="+strKey+"&filter="+strKeyFilter;
										}
										printf("thread: %d key=%s\n",m_uiThreadId,key.c_str());
										//pthread_rwlock_wrlock(&p_rwlock);
										string strEncrpytKey;

										strEncrpytKey = std::string(it->second.mParam)+"&key="+Encrypt_2(key);	
										
										sprintf(m_httpReq,"GET %s HTTP/1.1\r\nHost: %s\r\nAccept-Encoding: identity\r\n\r\n",strEncrpytKey.c_str(),it->first.c_str());
										printf("%s\n",m_httpReq);
										LOG(DEBUG,"m_httpReq=%s",m_httpReq);
										remoteIp.assign(it->first,0,it->first.find(":",0));
										remotePort = atoi(it->first.substr(it->first.find(":",0)+1).c_str());
										remoteSocket=new CTcpSocket(remotePort,remoteIp);
										if(remoteSocket->TcpConnect()!=0)
										{						
											errorMsg = "EE0009"; 
											printf("line %d,s Error: %s\n",__LINE__,errorMsg.c_str());
											stResponseInfo.ssOperatorNameKeyReqError=stResponseInfo.ssOperatorNameKeyReq+"_"+errorMsg;
											m_pDataRedis->UserIncr(stResponseInfo.ssOperatorNameKeyReqError);
											CUserQueryWorkThreads::m_vecReport[m_uiThreadId].m_strUserInfo[stResponseInfo.ssOperatorName].m_ullResErrorNum++;

											stResponseInfo.ssOperatorNameKeyReqError=stResponseInfo.ssOperatorNameKeyReq+"_"+errorMsg;
											m_pDataRedis->UserIncr(stResponseInfo.ssOperatorNameKeyReqError);
											
											errorMsg = "E0007";
											printf("line %d,s Error: %s\n",__LINE__,errorMsg.c_str());
											stResponseInfo.ssUserCountKeyReqError=stResponseInfo.ssUserCountKeyReq+"_"+errorMsg;		
											m_pDataRedis->UserIncr(stResponseInfo.ssUserCountKeyReqError);
											m_pDataRedis->UserIncr(stResponseInfo.ssOperatorNameKeyEmptyRes);
											//pthread_rwlock_unlock(&p_rwlock);
											return ERRORNODATA;
										}
										if(remoteSocket->TcpWrite(m_httpReq,strlen(m_httpReq))!=0)
										{
											memset(remoteBuffer,0,_8KBLEN);
											remoteSocket->TcpReadAll(remoteBuffer,_8KBLEN);	
											if( strlen(remoteBuffer) > 0 )
											{					
												#ifdef __EXPIRE__
												mResValueRemote = std::string(remoteBuffer);	
												printf("Line:%d,remoteBuffer=%s\n",__LINE__,remoteBuffer);
												LOG(DEBUG,"remoteBuffer=%s",remoteBuffer);
												#else
												stResponseInfo.mResValue = std::string(remoteBuffer);
												printf("Line:%d,stResponseInfo.mResValue====%s\n",__LINE__,stResponseInfo.mResValue.c_str());
												#endif
														
												remoteSocket->TcpClose();
												break;
											 }
											 else
											 {
												//iQueryCategory++;
												remoteSocket->TcpClose();
												
												errorMsg = "EE0009"; 
												printf("line %d,s Error: %s\n",__LINE__,errorMsg.c_str());
												stResponseInfo.ssOperatorNameKeyReqError=stResponseInfo.ssOperatorNameKeyReq+"_"+errorMsg;
												m_pDataRedis->UserIncr(stResponseInfo.ssOperatorNameKeyReqError);
												CUserQueryWorkThreads::m_vecReport[m_uiThreadId].m_strUserInfo[stResponseInfo.ssOperatorName].m_ullResErrorNum++;

												stResponseInfo.ssOperatorNameKeyReqError=stResponseInfo.ssOperatorNameKeyReq+"_"+errorMsg;
												m_pDataRedis->UserIncr(stResponseInfo.ssOperatorNameKeyReqError);
												
												errorMsg = "E0007";
												printf("line %d,s Error: %s\n",__LINE__,errorMsg.c_str());
												stResponseInfo.ssUserCountKeyReqError=stResponseInfo.ssUserCountKeyReq+"_"+errorMsg;		
												m_pDataRedis->UserIncr(stResponseInfo.ssUserCountKeyReqError);
												m_pDataRedis->UserIncr(stResponseInfo.ssOperatorNameKeyEmptyRes);
												return ERRORNODATA;
											 }		
										 }
										 else
										{
												//iQueryCategory++;
												remoteSocket->TcpClose();
												//errorMsg = "E0001";  //  internal connection socket error
												//return ERRORNODATA;
										}		
											  			
									}
										break;
								case 3:
									if((it->second.mProvince == atol(strProvince.c_str())||strProvince == "null"||strProvince==""))
									//&&(it->second.mCarrierOperator == strOperator||strOperator == "null"||strOperator == ""))
									//if((it->second.mCarrierOperator == strOperator||strOperator == "null"||strOperator == ""))
									{
									std::string strQpsLimit="jiangsu_qps_limit_"+ BdxTaskMainGetMinute();
									
									if(m_pDataRedis->UserGet(stResponseInfo.ssOperatorNameKeyLimit,ssmoidValue))
									{	
										//运营商和省份都不为空的才限制;没有的时候只查本地,不做限制
										if(atol(ssmoidValue.c_str())>= it->second.mQueryLimits)//&&(!strProvince.empty()&&!strOperator.empty()))
										{
											errorMsg = "EE0005";//interal use
											printf("line %d,s Error: %s\n",__LINE__,errorMsg.c_str());
											stResponseInfo.ssOperatorNameKeyReqError=stResponseInfo.ssOperatorNameKeyReq+"_"+errorMsg;
											m_pDataRedis->UserIncr(stResponseInfo.ssOperatorNameKeyReqError);
											m_pDataRedis->UserIncr(stResponseInfo.ssOperatorNameKeyEmptyRes);
											return EXCEEDLIMIT;
										}
									}
									
									m_pDataRedis->UserIncr(stResponseInfo.ssOperatorNameKeyLimit);
									#if 0  //qps limit
									if(m_pDataRedis->UserGet(strQpsLimit,ssmoidValue))
									{	//qps limit
										if(atol(ssmoidValue.c_str())> iAPIQpsLimit)
										{
											errorMsg = "E0008";
											return EXCEEDLIMIT;
										}
									}
									m_pDataRedis->UserIncr(strQpsLimit);
									#endif
									stResponseInfo.ssOperatorNameKeyReq = strProvinceReq + strKeyType +"_"+ BdxTaskMainGetTime();
									m_pDataRedis->UserIncr(stResponseInfo.ssOperatorNameKeyReq);
									stResponseInfo.ssOperatorName = strOperatorName + strKeyType;
									stResponseInfo.ssOperatorNameKeyEmptyRes = strProvinceEmptyRes + strKeyType +"_"+ BdxTaskMainGetTime();
									CUserQueryWorkThreads::m_vecReport[m_uiThreadId].m_strUserInfo[stResponseInfo.ssOperatorName].m_ullReqNum++;
									
										//when no data in local redis,then query remote api

										if(strKeyType =="0")//phone
										{
											//key="dsp_name="+strUser+"&cond=default&imei="+strKey;	
											key="dsp_name="+strUser+"&phone="+strKey+"&filter="+strKeyFilter;	
										}
										if(strKeyType =="1")//imei
										{
											//key="dsp_name="+strUser+"&cond=default&imei="+strKey;	
											key="dsp_name="+strUser+"&imei="+strKey+"&filter="+strKeyFilter;	
										}
										if(strKeyType=="2")//idfa
										{
											//key="dsp_name="+strUser+"&cond=default&idfa="+strKey;
											key="dsp_name="+strUser+"&idfa="+strKey+"&filter="+strKeyFilter;
										}
										if(strKeyType=="3")//aid
										{
											//key="dsp_name="+strUser+"&cond=default&dqc="+strKey;
											key="dsp_name="+strUser+"&dqc="+strKey+"&filter="+strKeyFilter;
										}
										if(strKeyType=="4")//pidfa
										{
											//key="dsp_name="+strUser+"&cond=default&pidfa="+strKey;
											key="dsp_name="+strUser+"&pidfa="+strKey+"&filter="+strKeyFilter;
										}
										if(strKeyType=="5")//sladuid
										{
											//key="dsp_name="+strUser+"&cond=default&pidfa="+strKey;
											key="dsp_name="+strUser+"&sladuid="+strKey+"&filter="+strKeyFilter;
										}
										printf("thread: %d key=%s\n",m_uiThreadId,key.c_str());
										//pthread_rwlock_wrlock(&p_rwlock);
										string strEncrpytKey;

										strEncrpytKey = std::string(it->second.mParam)+"&key="+key;	
										
										sprintf(m_httpReq,"GET %s HTTP/1.1\r\nHost: %s\r\nAccept-Encoding: identity\r\n\r\n",strEncrpytKey.c_str(),it->first.c_str());
										printf("%s\n",m_httpReq);
										LOG(DEBUG,"m_httpReq=%s",m_httpReq);
										remoteIp.assign(it->first,0,it->first.find(":",0));
										remotePort = atoi(it->first.substr(it->first.find(":",0)+1).c_str());
										remoteSocket=new CTcpSocket(remotePort,remoteIp);
										if(remoteSocket->TcpConnect()!=0)
										{						
											errorMsg = "EE0009"; 
											printf("line %d,s Error: %s\n",__LINE__,errorMsg.c_str());
											stResponseInfo.ssOperatorNameKeyReqError=stResponseInfo.ssOperatorNameKeyReq+"_"+errorMsg;
											m_pDataRedis->UserIncr(stResponseInfo.ssOperatorNameKeyReqError);
											CUserQueryWorkThreads::m_vecReport[m_uiThreadId].m_strUserInfo[stResponseInfo.ssOperatorName].m_ullResErrorNum++;

											stResponseInfo.ssOperatorNameKeyReqError=stResponseInfo.ssOperatorNameKeyReq+"_"+errorMsg;
											m_pDataRedis->UserIncr(stResponseInfo.ssOperatorNameKeyReqError);
											
											errorMsg = "E0007";
											printf("line %d,s Error: %s\n",__LINE__,errorMsg.c_str());
											stResponseInfo.ssUserCountKeyReqError=stResponseInfo.ssUserCountKeyReq+"_"+errorMsg;		
											m_pDataRedis->UserIncr(stResponseInfo.ssUserCountKeyReqError);
											m_pDataRedis->UserIncr(stResponseInfo.ssOperatorNameKeyEmptyRes);
											//pthread_rwlock_unlock(&p_rwlock);
											return ERRORNODATA;
										}
										if(remoteSocket->TcpWrite(m_httpReq,strlen(m_httpReq))!=0)
										{
											memset(remoteBuffer,0,_8KBLEN);
											remoteSocket->TcpReadAll(remoteBuffer,_8KBLEN);
											printf("remoteBuffer=%s\n",remoteBuffer);
											LOG(DEBUG,"remoteBuffer=%s",remoteBuffer);
											if( strlen(remoteBuffer) > 0 )
											{
																
												#ifdef __EXPIRE__
												mResValueRemote = std::string(remoteBuffer);	
												#else
												stResponseInfo.mResValue = std::string(remoteBuffer);
												#endif
												#if 0
													strMoId = "mix_"+strKey;
													if(m_pDataRedis->UserPut(strKey,strMoId))
													{	
														if(!m_pDataRedis->UserPut(strMoId,stResponseInfo.mResValue))
														{
														 	LOG(ERROR, "[thread: %d]Set HotKey Error.", m_uiThreadId)
;						
														}
													}	
												#endif		
														
												remoteSocket->TcpClose();
												break;
											 }
											 else
											 {
												//iQueryCategory++;
												remoteSocket->TcpClose();
												
												errorMsg = "EE0009"; 
												printf("line %d,s Error: %s\n",__LINE__,errorMsg.c_str());
												stResponseInfo.ssOperatorNameKeyReqError=stResponseInfo.ssOperatorNameKeyReq+"_"+errorMsg;
												m_pDataRedis->UserIncr(stResponseInfo.ssOperatorNameKeyReqError);
												CUserQueryWorkThreads::m_vecReport[m_uiThreadId].m_strUserInfo[stResponseInfo.ssOperatorName].m_ullResErrorNum++;

												stResponseInfo.ssOperatorNameKeyReqError=stResponseInfo.ssOperatorNameKeyReq+"_"+errorMsg;
												m_pDataRedis->UserIncr(stResponseInfo.ssOperatorNameKeyReqError);
												
												errorMsg = "E0007";
												printf("line %d,s Error: %s\n",__LINE__,errorMsg.c_str());
												stResponseInfo.ssUserCountKeyReqError=stResponseInfo.ssUserCountKeyReq+"_"+errorMsg;		
												m_pDataRedis->UserIncr(stResponseInfo.ssUserCountKeyReqError);
												m_pDataRedis->UserIncr(stResponseInfo.ssOperatorNameKeyEmptyRes);
												return ERRORNODATA;
											 }		
										 }
										 else
										{
												//iQueryCategory++;
												remoteSocket->TcpClose();
												//errorMsg = "E0001";  //  internal connection socket error
												//return ERRORNODATA;
										}		
											  	//return SUCCESS; 		
									}
										break;
								default:break;
							}

							}
								
							}
						//printf("clsing....socket\n");
						//remoteSocket->TcpClose();
						//pthread_rwlock_unlock(&p_rwlock);


				}
				#endif //__ZJYD__
				#endif

/**************openssl begin***************/

#if 0
#define __SSL__
#ifdef __SSL__

char sslBuffer[_8KBLEN];
CTcpSocket* sslSocket,* sslSocket2;
std::string sslIp;
uint16_t sslPort;
string key;


				if( iNoDataFlag == 1|| isExpire == 1)
				{
						pthread_rwlock_rdlock(&p_rwlock);
						for(std::map<std::string,QUERYAPIINFO_S>::iterator it=g_vecUrlAPIS.begin();it!=g_vecUrlAPIS.end();it++)
						{		
		
							if( iQueryCategory == 1 )
							{

							if(m_pDataRedis->UserGet(stResponseInfo.ssOperatorNameKeyLimit,ssmoidValue))
							{	
								//运营商和省份都不为空的才限制;没有的时候只查本地,不做限制
								if(atol(ssmoidValue.c_str())>= it->second.mQueryLimits)//&&(!strProvince.empty()&&!strOperator.empty()))
								{
									errorMsg = "E0005";
									return EXCEEDLIMIT;
								}
							}

							m_pDataRedis->UserIncr(stResponseInfo.ssOperatorNameKeyLimit);

							stResponseInfo.ssOperatorNameKeyReq = "zhejiang_cmc_req_" + strKeyType + BdxTaskMainGetTime();
							m_pDataRedis->UserIncr(stResponseInfo.ssOperatorNameKeyReq);

							stResponseInfo.ssOperatorName = "zhejiang_cmc_" + strKeyType;
							CUserQueryWorkThreads::m_vecReport[m_uiThreadId].m_strUserInfo[stResponseInfo.ssOperatorName].m_ullReqNum++;
															

								if((it->second.mProvince == atol(strProvince.c_str())||strProvince == "null"||strProvince=="")
								&&(it->second.mCarrierOperator == strOperator||strOperator == "null"||strOperator == ""))
								//if((it->second.mCarrierOperator == strOperator||strOperator == "null"||strOperator == ""))
								{

									//when no data in local redis,then query remote api
				
									if(strKeyType =="1")//imei
									{
										key="dsp_name="+strUser+"&cond=default&imei="+strKey;	
									}
									if(strKeyType=="2")//idfa
									{
										key="dsp_name="+strUser+"&cond=default&idfa="+strKey;
									}
									if(strKeyType=="3")//aid										if(strKeyType=="2")//idfa
									{
										key="dsp_name="+strUser+"&cond=default&aid="+strKey;
									}
								
								string strEncrpytKey;
								//printf("key=%s\n",key.c_str());
								LOG(DEBUG,"Thread %d strEncrpytKey=%s",m_uiThreadId,key.c_str()); 
								strEncrpytKey = string(it->second.mParam)+"token="+g_strTokenString+"&appkey=f2b98698400e4ace9926f83f0d063a57&key="+Encrypt_2(key);				
								LOG(DEBUG,"Thread %d strEncrpytKey=%s",m_uiThreadId,strEncrpytKey.c_str()); 
								//printf("strEncrpytKey.c_str()=%s\n",strEncrpytKey.c_str()); 
								//strEncrpytKey= it->second+"user=ipinyou&token=e200e86b9ca738aa2f1e5f14c50cf4d3&key=863846021456886";
								sprintf(m_httpReq,"GET %s HTTP/1.1\r\nHost: %s\r\nAccept-Encoding: identity\r\n\r\n",strEncrpytKey.c_str(),it->first.c_str());
								//sprintf(m_httpReq,"GET /query?user=admin&token=2360da5aae0c6bc826e035fc8ff5b5da&key=863846021456886 HTTP/1.1\r\nHost: 10.1.2.213:8089\nAccept-Encoding: identity\r\n\r\n");
								sslIp.assign(it->first,0,it->first.find(":",0));
								printf("Thread %d   m_httpReq=%s\n",m_uiThreadId,m_httpReq);
								LOG(DEBUG,"Thread %d m_httpReq=%s",m_uiThreadId,m_httpReq);  
								sslPort = atoi(it->first.substr(it->first.find(":",0)+1).c_str());	
								sslSocket=new CTcpSocket(sslPort,sslIp);
								printf("TcpGetSockfd =%d\n",sslSocket->TcpGetSockfd());
								if(sslSocket->TcpConnect()!=0)
								{	
									//printf("sslSocket=====%d\n",sslSocket->TcpGetSockfd());
									errorMsg = "E0007"; 
									printf("connect error\n");
									pthread_rwlock_unlock(&p_rwlock);
									return ERRORNODATA; 
								}
								
								//#define __HTTP__
								//__LIBCURL__
								#ifdef __HTTP__
								if(sslSocket->TcpWrite(m_httpReq,strlen(m_httpReq))!=0)
								{
									memset(sslBuffer,0,_8KBLEN);
									sslSocket->TcpReadAll(sslBuffer,_8KBLEN);
									//printf("sslBuffer=%s\n",sslBuffer);
									if(strlen(sslBuffer) > 0 )
									{
										stResponseInfo.mResValue = std::string(sslBuffer);
										sslSocket->TcpClose();  
										pthread_rwlock_unlock(&p_rwlock);
										break;
									}
									else
									{
										sslSocket->TcpClose();  
										iQueryCategory++;
									}
								}		
								else
								{
									sslSocket->TcpClose();  
									iQueryCategory++;
								}

								#else
								#ifndef  __LIBCURL__
								pthread_mutex_lock (&mutex);
								if ( InitSSLFlag == 0 )
								{
									sslSocket->TcpSslInitParams();
									InitSSLFlag = 1;
									
									}
								pthread_mutex_unlock(&mutex);
								if(sslSocket->TcpSslInitEnv()!=0)
								{
									errorMsg = "E0007"; 
									pthread_rwlock_unlock(&p_rwlock);
									return ERRORNODATA;
									
								}
								if(!sslSocket->TcpSslConnect())
								{
									sslSocket->TcpSslDestroy();
									errorMsg = "E0007"; 
									pthread_rwlock_unlock(&p_rwlock);
									return ERRORNODATA;
									
								}	
								//sleep(30);
								//printf("m_uiThreadId %d sending ssl......\n",m_uiThreadId);
								if(sslSocket->TcpSslWriteLen(m_httpReq,strlen(m_httpReq))!=0)
								{
									memset(sslBuffer,0,_8KBLEN);
									sslSocket->TcpSslReadLen(sslBuffer,_8KBLEN);
									printf("sslBuffer=%s\n",sslBuffer); 
									LOG(DEBUG,"Thread: %d sslBuffer=%s",m_uiThreadId,sslBuffer);  
			
									if(strlen(sslBuffer) > 0 )
									{
										mResValueRemote = std::string(sslBuffer);
										LOG(DEBUG,"Thread: %d mResValueRemote=%s",m_uiThreadId,mResValueRemote.c_str());  
										sslSocket->TcpSslDestroy();
										break;
									}
									else
									{
										
										iQueryCategory++;
										sslSocket->TcpSslDestroy();
										
			
									}
								}		
								else 
								{
									iQueryCategory++;		
									sslSocket->TcpSslDestroy();
	
								}
								#else //define __LIBCURL__
								
								
								#endif
								#endif


							  }
							}
							if( iQueryCategory == 2)
							{
								
							}

						}
					
			      pthread_rwlock_unlock(&p_rwlock);
					}

	#endif // __SSL__
#endif
/****************************openssl end******************************/

			}
			else
			{
				 errorMsg = "E0004";	// request param is error
				 return ERRORPARAM;
			}
	}
	else
	{
		errorMsg = "E0000";	// request type  is error ( GET )
		return PROTOERROR;
	}


	
#ifdef __EXPIRE__
HIVELOCALLOG_S stHiveLog;

	//while(!stHiveLog.empty())
	//{
		//stHiveLog.pop();
	//}
	printf("stResponseInfo.mResValue++++++++++++++++=%s\n",stResponseInfo.mResValue.c_str());
	printf("mResValueRemote++++++++=%s\n",mResValueRemote.c_str());
	if( iNoDataFlag == 1 || isExpire == 1 )
	{
		lenStrTemp = mResValueRemote.length();
		if( mResValueRemote.find("\r\n\r\n")!=std::string::npos )
		{
			mResValueRemote = mResValueRemote.substr(mResValueRemote.find("\r\n\r\n")+4,lenStrTemp -(mResValueRemote.find("\r\n\r\n")+4));
		}
			
		if(mResValueRemote.empty())//&&isExpire == 1)
		{
			if(	isExpire==1	)
			{
					mResValueRemote = mResValueLocal;
					stResponseInfo.mResValue=mResValueRemote;
					iIsLocal = 1;		
			}
			else
			{		
					
					errorMsg = "EE0001";  // data is not exists
					printf("line %d,s Error: %s\n",__LINE__,errorMsg.c_str());
					stResponseInfo.ssOperatorNameKeyReqError=stResponseInfo.ssOperatorNameKeyReq+"_"+errorMsg;
					m_pDataRedis->UserIncr(stResponseInfo.ssOperatorNameKeyReqError);
					CUserQueryWorkThreads::m_vecReport[m_uiThreadId].m_strUserInfo[stResponseInfo.ssOperatorName].m_ullResErrorNum++;

					stResponseInfo.ssOperatorNameKeyReqError=stResponseInfo.ssOperatorNameKeyReq+"_"+errorMsg;
					m_pDataRedis->UserIncr(stResponseInfo.ssOperatorNameKeyReqError);
					
					errorMsg = "E0001";
					printf("line %d,s Error: %s\n",__LINE__,errorMsg.c_str());
					stResponseInfo.ssUserCountKeyReqError=stResponseInfo.ssUserCountKeyReq+"_"+errorMsg;		
					m_pDataRedis->UserIncr(stResponseInfo.ssUserCountKeyReqError);
					m_pDataRedis->UserIncr(stResponseInfo.ssOperatorNameKeyEmptyRes);
					LOG(DEBUG,"line: %d,Error:%s ,value %s",__LINE__,errorMsg.c_str(),stResponseInfo.mResValue.c_str());
					printf("line %d,s Error: %s,value %s\n",__LINE__,errorMsg.c_str(),stResponseInfo.mResValue.c_str());
					return ERRORNODATA;
			}

		}
		lenStrTemp =  mResValueRemote.length();
		if(	lenStrTemp != 0 )
		{
				if( (mResValueRemote.at(0)=='<')||(mResValueRemote.at(0)=='{'))
				{
					//if(	isExpire==1)
					//{		
							
							errorMsg="EE0006";  // data format is wrong 
							printf("line %d,s Error: %s\n",__LINE__,errorMsg.c_str());

							CUserQueryWorkThreads::m_vecReport[m_uiThreadId].m_strUserInfo[stResponseInfo.ssOperatorName].m_ullResErrorNum++;

							stResponseInfo.ssOperatorNameKeyReqError=stResponseInfo.ssOperatorNameKeyReq+"_"+errorMsg;
							m_pDataRedis->UserIncr(stResponseInfo.ssOperatorNameKeyReqError);
							CUserQueryWorkThreads::m_vecReport[m_uiThreadId].m_strUserInfo[stResponseInfo.ssOperatorName].m_ullResErrorNum++;
							
							/*
							errorMsg = "E0006";
							printf("line %d,s Error: %s\n",__LINE__,errorMsg.c_str());
							stResponseInfo.ssUserCountKeyReqError=stResponseInfo.ssUserCountKeyReq+"_"+errorMsg;		
							m_pDataRedis->UserIncr(stResponseInfo.ssUserCountKeyReqError);
							
							CUserQueryWorkThreads::m_vecReport[m_uiThreadId].m_strUserInfo[stResponseInfo.ssOperatorName].m_ullResErrorNum++;
							m_pDataRedis->UserIncr(stResponseInfo.ssOperatorNameKeyEmptyRes);
							*/
							stResponseInfo.mResValue = mResValueLocal;
							iIsLocal = 1;				
					//}
					//else
					//{	
					//		errorMsg = "E0001";  // data is not exists
					//		return ERRORNODATA;
					//}
				}	
		}

		if( !iIsLocal )
		{	
			if( iQueryCategory != 3 && iIsLocal!=1) //if data is not jslt,then dectypt;
			{
				printf("Line:%d,before==========%s\n",__LINE__,mResValueRemote.c_str());
				mResValueRemote = Decrypt_2(mResValueRemote);
				printf("Line:%d,after ==========%s\n",__LINE__,mResValueRemote.c_str());
			}
				if(isQueryAction == 1)
				{
					string okString="{\"name\":\"atags_state\",\"typeid\":\"state\",\"value\":\"1\"}";
					string okString2="{\"name\":\"atags_state\",\"typeid\":\"state\",\"value\":\"2\"}";
					string okString3="{\"name\":\"atags_state\",\"typeid\":\"state\",\"value\":\"3\"}";
					//mResValueRemote
					printf("Line:%d,mResValueRemote=%s\n",__LINE__,mResValueRemote.c_str());
					if(mResValueRemote.find(okString)==-1 )//&& iQueryCategory !=1)
					{
						printf("Line:%d,okString2=%s\n",__LINE__,okString2.c_str());
						if(mResValueRemote.find(okString2)!=-1)
						{
							
							errorMsg="EE0001";
						}
						else
						{
							errorMsg="EE0010";
						}
						printf("line %d,s Error: %s\n",__LINE__,errorMsg.c_str());
						stResponseInfo.ssOperatorNameKeyReqError=stResponseInfo.ssOperatorNameKeyReq+"_"+errorMsg;
						m_pDataRedis->UserIncr(stResponseInfo.ssOperatorNameKeyReqError);
						CUserQueryWorkThreads::m_vecReport[m_uiThreadId].m_strUserInfo[stResponseInfo.ssOperatorName].m_ullResErrorNum++;

						stResponseInfo.ssOperatorNameKeyReqError=stResponseInfo.ssOperatorNameKeyReq+"_"+errorMsg;
						m_pDataRedis->UserIncr(stResponseInfo.ssOperatorNameKeyReqError);
					  if(mResValueRemote.find(okString2)!=-1)
                                         {
                                           errorMsg="E0001";
                                          }
					  else
					  {
						errorMsg="E0010";
					   }	
						printf("line %d,s Error: %s\n",__LINE__,errorMsg.c_str());
						stResponseInfo.ssUserCountKeyReqError=stResponseInfo.ssUserCountKeyReq+"_"+errorMsg;		
						m_pDataRedis->UserIncr(stResponseInfo.ssUserCountKeyReqError);
						
						CUserQueryWorkThreads::m_vecReport[m_uiThreadId].m_strUserInfo[stResponseInfo.ssOperatorName].m_ullResErrorNum++;
						m_pDataRedis->UserIncr(stResponseInfo.ssOperatorNameKeyEmptyRes);
						return ERRORNODATA;

					}
					//state = 3;


				}
			

			
			stResponseInfo.mResValue = mResValueRemote;
			printf("Line:%d,Length:%d,mResValueRemote=%s\n",__LINE__,mResValueRemote.length(),mResValueRemote.c_str());
			printf("Line:%d,stResponseInfo.mResValue=%s\n",__LINE__,stResponseInfo.mResValue.c_str());
			//printf("Decrypt_2 stResponseInfo.mResValue =%s\n",stResponseInfo.mResValue.c_str());
			LOG(DEBUG,"Thread : %d Decrypt_2 stResponseInfo.mResValue=%s",m_uiThreadId,stResponseInfo.mResValue.c_str());  
			if(mResValueRemote.length()>0)
			{

				if( mResValueRemote.at(0)!='[' )
				{

					errorMsg="EE0006";  // data format is wrong 
					printf("line %d,s Error: %s\n",__LINE__,errorMsg.c_str());
					stResponseInfo.ssOperatorNameKeyReqError=stResponseInfo.ssOperatorNameKeyReq+"_"+errorMsg;
					m_pDataRedis->UserIncr(stResponseInfo.ssOperatorNameKeyReqError);
					CUserQueryWorkThreads::m_vecReport[m_uiThreadId].m_strUserInfo[stResponseInfo.ssOperatorName].m_ullResErrorNum++;

					stResponseInfo.ssOperatorNameKeyReqError=stResponseInfo.ssOperatorNameKeyReq+"_"+errorMsg;
					m_pDataRedis->UserIncr(stResponseInfo.ssOperatorNameKeyReqError);
					/*
					errorMsg = "E0006";
					printf("line %d,s Error: %s\n",__LINE__,errorMsg.c_str());
					stResponseInfo.ssUserCountKeyReqError=stResponseInfo.ssUserCountKeyReq+"_"+errorMsg;		
					m_pDataRedis->UserIncr(stResponseInfo.ssUserCountKeyReqError);
					
					CUserQueryWorkThreads::m_vecReport[m_uiThreadId].m_strUserInfo[stResponseInfo.ssOperatorName].m_ullResErrorNum++;
					m_pDataRedis->UserIncr(stResponseInfo.ssOperatorNameKeyEmptyRes);
					*/
					stResponseInfo.mResValue = mResValueLocal;
					iIsLocal = 1;
					
				}
				else
				{
					iRemoteApiFlag = 1;
				}


			}

		}

		if(!jReader->parse(stResponseInfo.mResValue, jValue))
		{ 
			
			errorMsg="EE0001";  // data format is wrong 
			printf("line %d,s Error: %s\n",__LINE__,errorMsg.c_str());
			stResponseInfo.ssOperatorNameKeyReqError=stResponseInfo.ssOperatorNameKeyReq+"_"+errorMsg;
			m_pDataRedis->UserIncr(stResponseInfo.ssOperatorNameKeyReqError);
			CUserQueryWorkThreads::m_vecReport[m_uiThreadId].m_strUserInfo[stResponseInfo.ssOperatorName].m_ullResErrorNum++;

			stResponseInfo.ssOperatorNameKeyReqError=stResponseInfo.ssOperatorNameKeyReq+"_"+errorMsg;
			m_pDataRedis->UserIncr(stResponseInfo.ssOperatorNameKeyReqError);
			
			errorMsg = "E0001";
			printf("line %d,s Error: %s\n",__LINE__,errorMsg.c_str());
			stResponseInfo.ssUserCountKeyReqError=stResponseInfo.ssUserCountKeyReq+"_"+errorMsg;		
			m_pDataRedis->UserIncr(stResponseInfo.ssUserCountKeyReqError);
			m_pDataRedis->UserIncr(stResponseInfo.ssOperatorNameKeyEmptyRes);
			LOG(DEBUG,"line: %d,Error:%s ,value %s",__LINE__,errorMsg.c_str(),stResponseInfo.mResValue.c_str());
			printf("line %d,s Error:%s,value %s\n",__LINE__,errorMsg.c_str(),stResponseInfo.mResValue.c_str());
			return	ERRORNODATA;
		}

		for(unsigned int i=0;i<jValue.size();i++)
		{
			if(jValue[i]["name"].asString()== "create_time")
			{

				timetNow = time(NULL);
				strCreateTime2 = jValue[i]["value"].asString();
		        ts.tm_year=atoi(strCreateTime2.substr(0,4).c_str())-1900;
		        ts.tm_mon=atoi(strCreateTime2.substr(4,2).c_str())-1;
		        ts.tm_mday=atoi(strCreateTime2.substr(6,2).c_str());
		        ts.tm_hour=atoi(strCreateTime2.substr(8,2).c_str());
		        ts.tm_min=atoi(strCreateTime2.substr(10,2).c_str());
		        ts.tm_sec=atoi(strCreateTime2.substr(12,2).c_str());
		        timetCreateTime2 = mktime(&ts);
		        printf("iIsLocal=%d\n",iIsLocal);
		        if( !iIsLocal )
		        {
		         	iRemoteApiFlag = 1;
		         	printf("iQueryCategory=%d\n",iQueryCategory);
				 	//if( iQueryCategory == 1 )
				 	{
						stResponseInfo.ssOperatorNameKeyRes = strProvinceRes + strKeyType+"_" + BdxTaskMainGetTime();
						m_pDataRedis->UserIncr(stResponseInfo.ssOperatorNameKeyRes);
						CUserQueryWorkThreads::m_vecReport[m_uiThreadId].m_strUserInfo[stResponseInfo.ssOperatorName].m_ullResNum++;
						
						stResponseInfo.ssOperatorNameKeyResTag = strProvinceResTag + strKeyType +"_"+ BdxTaskMainGetTime();
						m_pDataRedis->UserIncrBy(stResponseInfo.ssOperatorNameKeyResTag,jValue.size());
						CUserQueryWorkThreads::m_vecReport[m_uiThreadId].m_strUserInfo[stResponseInfo.ssOperatorName].m_ullEmptyResNum+=jValue.size();					

				 	}

		         
		        }
					/*
				        if (( timetCreateTime2 < timetCreateTime))//&&(!iIsLocal))
				        {
							stResponseInfo.mResValue = mResValueLocal;
							iRemoteApiFlag = 1;
							iIsLocal = 1;
				        }
		        		*/
			}
			if(jValue[i]["name"].asString()== "last_data_time")
			{
				strLastDataTime2=jValue[i]["name"].asString();	
			}

		}		
		//printf("iQueryCategory=%d\n",iQueryCategory);
		LOG(DEBUG,"iQueryCategory=%d",iQueryCategory);  
		switch(iQueryCategory)
		{
			case 1:
				if(iRemoteApiFlag==1)
				{		
						if( iQueryCategory == 1 )
						{
							stHiveLog.strProvider="cmc";
							stHiveLog.strProvince="zhejiang";	
						}
						if( iQueryCategory == 2 )
						{
							stHiveLog.strProvider="cmc";
							stHiveLog.strProvince="qinghai";	
						}
						if( iQueryCategory == 3 )
						{
							stHiveLog.strProvider="cuc";
							stHiveLog.strProvince="jiangsu";	
						}

				}
				else
				{
						stHiveLog.strProvider="cuc";
						stHiveLog.strProvince="local";	
				}
				stHiveLog.strLogValue=stResponseInfo.mResValue;
				stHiveLog.strLogKey=strKey;
				stHiveLog.strLogKeyType=strKeyType;
				stHiveLog.strDspName=strUser;
				stHiveLog.strQueryTime=BdxTaskMainGetFullTime();
			
				if(iIsLocal != 1)
				{
					stHiveLog.strCreateTime=strCreateTime2;
					stHiveLog.strLastDataTime=strLastDataTime2;
					//stHiveLog.strDayId=strCreateTime2.substr(0,8);
					stHiveLog.strDayId=BdxTaskMainGetDate();
					stHiveLog.strHourId=stHiveLog.strQueryTime.substr(8,2);
				}
				else
				{
					stHiveLog.strCreateTime=strCreateTime;
					stHiveLog.strLastDataTime=strLastDataTime;
					//stHiveLog.strDayId=strCreateTime.substr(0,8);
					stHiveLog.strDayId=BdxTaskMainGetDate();
					stHiveLog.strHourId=stHiveLog.strQueryTime.substr(8,2);
				}
				CUserQueryWorkThreads::m_vecHiveLog[m_uiThreadId].push(stHiveLog);
				break;
			
			case 2:
				break;
			default :
			break;
			
		 }
	}
	else
	{
		lenStrTemp = mResValueLocal.length();
		if( mResValueLocal.find("\r\n\r\n")!=std::string::npos )
		{
			mResValueLocal = mResValueLocal.substr(mResValueLocal.find("\r\n\r\n")+4,lenStrTemp -(mResValueRemote.find("\r\n\r\n")+4));
		}

		if(mResValueLocal.empty())
		{

			errorMsg = "E0001";
			printf("line %d,s Error: %s\n",__LINE__,errorMsg.c_str());
			stResponseInfo.ssUserCountKeyReqError=stResponseInfo.ssUserCountKeyReq+"_"+errorMsg;		
			m_pDataRedis->UserIncr(stResponseInfo.ssUserCountKeyReqError);

			
			LOG(DEBUG,"line: %d,Error:%s ,value %s",__LINE__,errorMsg.c_str(),stResponseInfo.mResValue.c_str());
			printf("line %d,s Error:%s,value %s\n",__LINE__,errorMsg.c_str(),stResponseInfo.mResValue.c_str());
			return ERRORNODATA;
		}
		printf("mResValueLocal=%s\n",mResValueLocal.c_str());
		if( mResValueLocal.at(0)!='[' )
		{

			errorMsg = "E0006";
			printf("line %d,s Error: %s\n",__LINE__,errorMsg.c_str());
			stResponseInfo.ssUserCountKeyReqError=stResponseInfo.ssUserCountKeyReq+"_"+errorMsg;		
			m_pDataRedis->UserIncr(stResponseInfo.ssUserCountKeyReqError);
			
			printf("Line:%d,errMsg %s\n",__LINE__,errorMsg.c_str());
			return	ERRORNODATA;
		}
		stResponseInfo.mResValue = mResValueLocal;	
		stHiveLog.strProvider="cuc";
		stHiveLog.strProvince="local";
		stHiveLog.strLogValue=stResponseInfo.mResValue;
		stHiveLog.strLogKey=strKey;
		stHiveLog.strLogKeyType=strKeyType;
		stHiveLog.strQueryTime=BdxTaskMainGetFullTime();
		stHiveLog.strDspName=strUser;
		stHiveLog.strCreateTime=strCreateTime;
		stHiveLog.strLastDataTime=strLastDataTime;
		stHiveLog.strDayId=BdxTaskMainGetDate();
		stHiveLog.strHourId=stHiveLog.strQueryTime.substr(8,2);
	
		CUserQueryWorkThreads::m_vecHiveLog[m_uiThreadId].push(stHiveLog);
			
		
	}

	printf("iIsLocal=%d,isExpire=%d,iRemoteApiFlag=%d\n",iIsLocal,isExpire,iRemoteApiFlag);
  	LOG(DEBUG,"iIsLocal=%d,isExpire=%d,iRemoteApiFlag=%d",iIsLocal,isExpire,iRemoteApiFlag);
	#define __LOCAL_STORE__
	#ifdef __LOCAL_STORE__
	if( (iIsLocal != 1 || isExpire == 1 )&&(iRemoteApiFlag == 1))
	{
			//if ( iQueryCategory == 1 )
			{
				strMoId = strOperatorName+strKeyType+"_"+strKey;
			}

			if( isQueryAction==1 )
			{
				strKey=strKey+"_"+filterDate;
				strMoId = strMoId + "_"+ filterDate;
			}

				if(m_pDataRedis->UserPut(strKey,strMoId))
				{	
					if(!m_pDataRedis->UserPut(strMoId,stResponseInfo.mResValue))
					{
					 	LOG(ERROR, "[thread: %d]Set HotKey Error.", m_uiThreadId);						
					}
				}



	}
	#endif
	//printf("m_uiThreadId=%d\n,stResponseInfo.mResValue=%s\n",m_uiThreadId,stResponseInfo.mResValue.c_str());
	if(!jReader->parse(stResponseInfo.mResValue, jValue))
	{ 

		errorMsg = "E0006";
		printf("line %d,s Error: %s\n",__LINE__,errorMsg.c_str());
		stResponseInfo.ssUserCountKeyReqError=stResponseInfo.ssUserCountKeyReq+"_"+errorMsg;		
		m_pDataRedis->UserIncr(stResponseInfo.ssUserCountKeyReqError);
		
		return	ERRORNODATA;
	}
	
	#if 0
	if( iNoDataFlag == 1  )
	{ 
			if( jValue["code"].toStyledString()== "\"1\"\n" )
			//if( jFastWriter.write(jValue["code"]) == "\"1\"\n"  )
			{
					//strTempValue = jValue["data"].toStyledString().c_str();
					//strTempValue=strTempValue.substr(1,strTempValue.length()-3);
					//strTempValue = mConf.replace_all(strTempValue,"\\",NULLSTRING);
				//if(!jReader->parse(strTempValue,jValue))
				if(!jReader->parse(jValue["data"].toStyledString(), jValue))	
				{				
					//printf("format is worong \n");
					errorMsg = "E0001"; // json data format is wrong
					return	ERRORNODATA;
				}
				/*
				else
				{
						if(!jReader->parse(stResponseInfo.mResValue, jValue))
						{
							errorMsg = "E0001"; // json data format is wrong
							return	ERRORNODATA;
						}
				}
				*/
			}
			else
			{
					//printf("return data is null\n");
					errorMsg = "E0001"; // json data format is wrong
					return	ERRORNODATA;
			}
	}
	
	#endif

// get the fields according to rights	

	#ifdef __P_LOCK__
		pthread_rwlock_wrlock(&p_rwlock);
	#endif //__P_LOCK__
	std::string tagsNumber,oPeratorTagsNumber;
	for(itRights = g_mapUserInfo.find(strUser)->second.mVecFields.begin();itRights!= g_mapUserInfo.find(strUser)->second.mVecFields.end();itRights++)
	{
		
		for(unsigned int i=0;i<jValue.size();i++)
		{
			if(!jValue[i]["typeid"].isNull())
			{
				if((!jValue[i]["name"].isNull())&&(!jValue[i]["value"].isNull()))
				{
					jTemp.clear();
					jTemp[jValue[i]["name"].asString()] = jValue[i]["value"];
					

					CUserQueryWorkThreads::m_vecReport[m_uiThreadId].m_strUserInfo[stResponseInfo.ssOperatorName].m_ullResTagNum++;
					

					// deal with json string to pinyou format
					strTempValue = jFastWriter.write(jTemp); 
					strTempValue = mConf.replace_all(strTempValue,LEFTBIGBRACE,NULLSTRING);
					strTempValue = mConf.replace_all(strTempValue,RIGHTBIGBRACE,NULLSTRING);
					
					//cout<<"strTempValue="<<strTempValue<<::endl;
					//cout<<"strTempValue.length()"<<strTempValue.length()<<::endl;
					//cout<<"strTempValue[strTempValue.length()-2]"<<strTempValue[strTempValue.length()-2]<<::endl;
					//cout<<"strTempValue[0]"<<strTempValue[0]<<::endl;
					
					//if( (stResponseInfo.mResValue.find("\"")!=std::string::npos) && (stResponseInfo.mResValue.rfind("\"")!=std::string::npos)))
					
					if((strTempValue[0]=='"')&&(strTempValue[strTempValue.length()-2]=='"') )
					{	
						lenStrTemp = strTempValue.length();
						strTempValue = strTempValue.substr(1,lenStrTemp - 3);
						//cout<<"strTempValue="<<strTempValue<<::endl;
					}
					else
					{

							errorMsg = "E0006";
							printf("line %d,s Error: %s\n",__LINE__,errorMsg.c_str());
							stResponseInfo.ssUserCountKeyReqError=stResponseInfo.ssUserCountKeyReq+"_"+errorMsg;		
							m_pDataRedis->UserIncr(stResponseInfo.ssUserCountKeyReqError);
							
							#ifdef __P_LOCK__
							pthread_rwlock_unlock(&p_rwlock);
							#endif //__P_LOCK__
							return	ERRORNODATA;

					}
					strTempValue = mConf.replace_all(strTempValue,"\n",NULLSTRING);
					
				}
				
				if( *itRights == jValue[i]["typeid"].asString() )
				{
					jRoot[*itRights].append(strTempValue);

					//tagsNumber="Restag_" + BdxTaskMainGetTime(0)+"_"+strUser+"_"+strKeyType+"_"+jValue[i]["typeid"].asString();
					tagsNumber="Restag_" + strCurrentHour+"_"+strUser+"_"+strKeyType+"_"+jValue[i]["typeid"].asString();
					//LOG(ERROR,"Thread:%d,stResponseInfo.tagsNumber=%s",m_uiThreadId,tagsNumber.c_str());
					//m_pDataRedis->UserIncrBy(tagsNumber,jValue[i].size());

					m_pDataRedis->UserIncr(tagsNumber);
					if( iIsLocal!=1 && iRemoteApiFlag ==1 )
					{
						oPeratorTagsNumber= strProvinceResTag + BdxTaskMainGetTime()+"_" + strUser + "_" + strKeyType + "_" + jValue[i]["typeid"].asString();
						m_pDataRedis->UserIncr(oPeratorTagsNumber);

					}
					//cout<<"jRoot"<<jFastWriter.write(jRoot);
					//cout<<"+++++++++++++++++++++++++++++++"<<std::endl;
					
				}

				#if 0
				switch jValue[*itRights]
				{
			
					case "1":
							if((!jValue["name"].isNull())&&(!jValue["value"].isNull()))
							{
								jTemp[jValue["name"]] = jValue["value"]];
							}
							jRoot[*itRights].append(jTemp);


				}
				#endif
					
			}	


		}

		
	}
	
	//mResUserAttributes = jFastWriter.write(jRoot);
#ifdef __P_LOCK__
	pthread_rwlock_unlock(&p_rwlock);
#endif //__P_LOCK__
	mResUserAttributes = jRoot.toStyledString();
	mResUserAttributes = mConf.replace_all(mResUserAttributes,LEFTMIDBRACE,LEFTBIGBRACE);
	mResUserAttributes = mConf.replace_all(mResUserAttributes,RIGHTMIDBRACE,RIGHTBIGBRACE);
	mResUserAttributes = mConf.replace_all(mResUserAttributes,"\\",NULLSTRING);
	if( mResUserAttributes == "null\n" )
	{

		errorMsg = "E0006";
		printf("line %d,s Error: %s\n",__LINE__,errorMsg.c_str());
		stResponseInfo.ssUserCountKeyReqError=stResponseInfo.ssUserCountKeyReq+"_"+errorMsg;		
		m_pDataRedis->UserIncr(stResponseInfo.ssUserCountKeyReqError);
		
		return	ERRORNODATA;
	}
	//std::cout<< "mResUserAttributes="<<mResUserAttributes<<::endl;
	stResponseInfo.mResValue = mResUserAttributes;

	return SUCCESS;

//================================================================================
//================================================================================
//================================================================================
#else

	lenStrTemp = stResponseInfo.mResValue.length();
	if( stResponseInfo.mResValue.find("\r\n\r\n")!=std::string::npos )
	{
		stResponseInfo.mResValue = stResponseInfo.mResValue.substr(stResponseInfo.mResValue.find("\r\n\r\n")+4,lenStrTemp -(stResponseInfo.mResValue.find("\r\n\r\n")+4));
	}


	

	if( iNoDataFlag == 1 || isExpire == 1 )
	{
		if(stResponseInfo.mResValue.empty())
		{
			printf("2222222222222222222222222\n");
			errorMsg = "E0001";  // data is not exists
			return ERRORNODATA;
		}
		stResponseInfo.mResValue = Decrypt_2(stResponseInfo.mResValue);	
		
	}
	
	if( stResponseInfo.mResValue.at(0)!='[' )
	{
		//printf("3333333333333333333\n");
		errorMsg = "E0006"; // json data format is wrong
		return	ERRORNODATA;
	}

	if ( iNoDataFlag == 1 )
	{

		#define __LOCAL_STORE__
		#ifdef __LOCAL_STORE__
			strMoId = "mix_"+strKey;
			if(m_pDataRedis->UserPut(strKey,strMoId))
			{	
				if(!m_pDataRedis->UserPut(strMoId,stResponseInfo.mResValue))
				{
				 	LOG(ERROR, "[thread: %d]Set HotKey Error.", m_uiThreadId);						
				}
			}	
		#endif
	}
	
	//printf("m_uiThreadId=%d\nstrkey=%s\n,stResponseInfo.mResValue=%s\n",m_uiThreadId,strkey.c_str(),stResponseInfo.mResValue.c_str());
	if(!jReader->parse(stResponseInfo.mResValue, jValue))
	{ 
		
		errorMsg = "E0006"; // json data format is wrong
		return	ERRORNODATA;
	}
	
	#if 0
	if( iNoDataFlag == 1  )
	{ 
			if( jValue["code"].toStyledString()== "\"1\"\n" )
			//if( jFastWriter.write(jValue["code"]) == "\"1\"\n"  )
			{
					//strTempValue = jValue["data"].toStyledString().c_str();
					//strTempValue=strTempValue.substr(1,strTempValue.length()-3);
					//strTempValue = mConf.replace_all(strTempValue,"\\",NULLSTRING);
				//if(!jReader->parse(strTempValue,jValue))
				if(!jReader->parse(jValue["data"].toStyledString(), jValue))	
				{				
					//printf("format is worong \n");
					errorMsg = "E0001"; // json data format is wrong
					return	ERRORNODATA;
				}
				/*
				else
				{
						if(!jReader->parse(stResponseInfo.mResValue, jValue))
						{
							errorMsg = "E0001"; // json data format is wrong
							return	ERRORNODATA;
						}
				}
				*/
			}
			else
			{
					//printf("return data is null\n");
					errorMsg = "E0001"; // json data format is wrong
					return	ERRORNODATA;
			}
	}
	
	#endif

// get the fields according to rights	

	#ifdef __P_LOCK__
		pthread_rwlock_wrlock(&p_rwlock);
	#endif //__P_LOCK__
	for(itRights = g_mapUserInfo.find(strUser)->second.mVecFields.begin();itRights!= g_mapUserInfo.find(strUser)->second.mVecFields.end();itRights++)
	{
		
		for(unsigned int i=0;i<jValue.size();i++)
		{
		//printf("m_uiThreadId=%d\n jValue[%d][\"typeid\"]=%s\n",m_uiThreadId,i,jValue[i].asString().c_str());
		//if( jValue[i]["typeid"].asString().length()>0 )
		if(!jValue[i]["typeid"].isNull())
		{
			if((!jValue[i]["name"].isNull())&&(!jValue[i]["value"].isNull()))
			{
				jTemp.clear();
				jTemp[jValue[i]["name"].asString()] = jValue[i]["value"];

				// deal with json string to pinyou format
				
				strTempValue = jFastWriter.write(jTemp); 
				strTempValue = mConf.replace_all(strTempValue,LEFTBIGBRACE,NULLSTRING);
				strTempValue = mConf.replace_all(strTempValue,RIGHTBIGBRACE,NULLSTRING);
				
				//cout<<"strTempValue="<<strTempValue<<::endl;
				//cout<<"strTempValue.length()"<<strTempValue.length()<<::endl;
				//cout<<"strTempValue[strTempValue.length()-2]"<<strTempValue[strTempValue.length()-2]<<::endl;
				//cout<<"strTempValue[0]"<<strTempValue[0]<<::endl;
				
				//if( (stResponseInfo.mResValue.find("\"")!=std::string::npos) && (stResponseInfo.mResValue.rfind("\"")!=std::string::npos)))
				
				if((strTempValue[0]=='"')&&(strTempValue[strTempValue.length()-2]=='"') )
				{	
					lenStrTemp = strTempValue.length();
					strTempValue = strTempValue.substr(1,lenStrTemp - 3);
					//cout<<"strTempValue="<<strTempValue<<::endl;
				}
				else
				{
						errorMsg = "E0006"; // json data format is wrong
						#ifdef __P_LOCK__
						pthread_rwlock_unlock(&p_rwlock);
						#endif //__P_LOCK__
						return	ERRORNODATA;

				}
				strTempValue = mConf.replace_all(strTempValue,"\n",NULLSTRING);
				
			}
			
			if( *itRights == jValue[i]["typeid"].asString() )
			{
				jRoot[*itRights].append(strTempValue);
				//cout<<"jRoot"<<jFastWriter.write(jRoot);
				//cout<<"+++++++++++++++++++++++++++++++"<<std::endl;
				
			}

			#if 0
			switch jValue[*itRights]
			{
		
				case "1":
						if((!jValue["name"].isNull())&&(!jValue["value"].isNull()))
						{
							jTemp[jValue["name"]] = jValue["value"]];
						}
						jRoot[*itRights].append(jTemp);


			}
			#endif
				
		}	


		}

		
	}
	
	//mResUserAttributes = jFastWriter.write(jRoot);
#ifdef __P_LOCK__
	pthread_rwlock_unlock(&p_rwlock);
#endif //__P_LOCK__
	mResUserAttributes = jRoot.toStyledString();
	mResUserAttributes = mConf.replace_all(mResUserAttributes,LEFTMIDBRACE,LEFTBIGBRACE);
	mResUserAttributes = mConf.replace_all(mResUserAttributes,RIGHTMIDBRACE,RIGHTBIGBRACE);
	mResUserAttributes = mConf.replace_all(mResUserAttributes,"\\",NULLSTRING);
	if( mResUserAttributes == "null\n" )
	{
		errorMsg = "E0006"; // json data format is wrong
		return	ERRORNODATA;
	}
	//std::cout<< "mResUserAttributes="<<mResUserAttributes<<::endl;
	stResponseInfo.mResValue = mResUserAttributes;

	return SUCCESS;

	#endif

	
	}

int CTaskMain::BdxParseHttpPacket(char*& pszBody, u_int& uiBodyLen, const u_int uiParseLen)
{
	u_int uiHeadLen = 0;

	char* pszTmp = NULL;
	char* pszPacket = m_pszAdxBuf;
	if(strncmp(m_pszAdxBuf, "GET", strlen("GET"))) {
//		LOG(ERROR, "[thread: %d]It is not POST request.", m_uiThreadId);
		return PROTOERROR;
	}

	//find body
	pszTmp = strstr(pszPacket, m_pszHttpHeaderEnd);
	if(pszTmp == NULL) {
		LOG(ERROR, "[thread: %d]can not find Header End.", m_uiThreadId);
		return PROTOERROR;
	}
	pszBody = pszTmp + strlen(m_pszHttpHeaderEnd);
	uiHeadLen = pszBody - m_pszAdxBuf;

	return SUCCESS;
	//return OTHERERROR;
}

int CTaskMain::BdxParseBody(char *pszBody, u_int uiBodyLen, BDXREQUEST_S& stRequestInfo)
{

    LOG(DEBUG,"SUCCESS");
	return SUCCESS;
}



int CTaskMain::BdxSendEmpyRespones(std::string errorMsg)
{
	m_clEmTime.TimeOff();
	std::string strOutput=errorMsg;	
	char pszDataBuf[_8KBLEN];
	memset(pszDataBuf, 0, _8KBLEN);
	sprintf((char *)pszDataBuf, "%s%sContent-Length: %d\r\n\r\n", http200ok,BdxGetHttpDate().c_str(),(int)strOutput.length());
	int iHeadLen = strlen(pszDataBuf);
	
	memcpy(pszDataBuf + iHeadLen, strOutput.c_str(), strOutput.length());
	LOG(DEBUG,"Thread : %d ,AdAdxSendEmpyRespones=%s\n",m_uiThreadId,pszDataBuf);
	if(!m_pclSock->TcpWrite(pszDataBuf, iHeadLen + strOutput.length())) {
		LOG(ERROR, "[tread: %d]write empty response data error.", m_uiThreadId);
		return LINKERROR;
	}

	return SUCCESS;
}

int CTaskMain::BdxSendRespones(BDXREQUEST_S& stRequestInfo, BDXRESPONSE_S& stAdxRes,std::string errorMsg)
{
	memset(m_pszAdxResponse, 0, _64KBLEN);
	if( stAdxRes.mResValue.empty())
	{		
		std::string strOutput=errorMsg;
	}
	if(m_httpType)
	{
		sprintf((char *)m_pszAdxResponse, "%s%sContent-Length: %d\r\n\r\n", http200ok,BdxGetHttpDate().c_str(),(int)stAdxRes.mResValue.length());
		int iHeadLen = strlen(m_pszAdxResponse);
		memcpy(m_pszAdxResponse + iHeadLen, stAdxRes.mResValue.c_str(),stAdxRes.mResValue.length());
	}
	else
	{
		sprintf((char *)m_pszAdxResponse,"%s",stAdxRes.mResValue.c_str());
	}
	
	int iBodyLength = strlen(m_pszAdxResponse);
	iBodyLength=strlen(m_pszAdxResponse);



	if(!m_pclSock->TcpWrite(m_pszAdxResponse, iBodyLength)) 
	{
		CUserQueryWorkThreads::m_vecReport[m_uiThreadId].m_strUserInfo[stAdxRes.ssUserName].m_ullEmptyResNum++;
		CUserQueryWorkThreads::m_vecReport[m_uiThreadId].m_strUserInfo[stAdxRes.ssUserName].m_ullTotalEmptyResNum++;
		if(stAdxRes.queryType==1)// 1 query user index ,2 query goods 
		{
			m_pDataRedis->UserIncr(stAdxRes.ssUserCountKeyRes);

		}
		LOG(ERROR, "[thread: %d]write  response error.", m_uiThreadId);
		return LINKERROR;
	}

	CUserQueryWorkThreads::m_vecReport[m_uiThreadId].m_strUserInfo[stAdxRes.ssUserName].m_ullResNum++;
	CUserQueryWorkThreads::m_vecReport[m_uiThreadId].m_strUserInfo[stAdxRes.ssUserName].m_ullTotalResNum++;
	
	if(stAdxRes.queryType==1)// 1 query user index ,2 query goods 
	{
		m_pDataRedis->UserIncr(stAdxRes.ssUserCountKeyRes);

	}
	
	LOG(DEBUG, "[thread: %d]write response iBodyLength=%d.",m_uiThreadId,iBodyLength);
	
    return SUCCESS;
}


#if 0
std::string CTaskMain::BdxTaskMainGetTime(const time_t ttime)
{
	
	time_t tmpTime1=0;
	if(ttime == 0)
		{
			tmpTime1 = time(0);
			//LOG(ERROR, "[thread: %d]ttime=%ld,tmpTime=%ld",m_uiThreadId,ttime,tmpTime1);
		}
	else
		tmpTime1 = ttime;
	struct tm* timeinfo1 = localtime(&tmpTime1);
	char dt1[10];
	memset(dt1, 0, 10);
	//LOG(ERROR, "[thread: %d]dt1=%s",m_uiThreadId,dt1);
	//LOG(ERROR, "[thread: %d]timeinfo1=%4d%02d%02d%02",m_uiThreadId,timeinfo1->tm_year + 1900,timeinfo1->tm_mon+1,timeinfo1->tm_mday,timeinfo1->tm_hour);
	sprintf(dt1, "%4d%02d%02d%02d", timeinfo1->tm_year + 1900,timeinfo1->tm_mon+1,timeinfo1->tm_mday,timeinfo1->tm_hour);
	//LOG(ERROR, "[thread: %d]dt1=%s",m_uiThreadId,dt1);
	//sprintf(dt, "%4d%02d%02d%02d%02d%02d", timeinfo->tm_year + 1900, timeinfo->tm_mon+1,timeinfo->tm_mday,timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
	//return (timeinfo->tm_year + 1900) * 10000 + (timeinfo->tm_mon + 1) * 100 + timeinfo->tm_mday;
	return std::string(dt1);
}
#endif

std::string CTaskMain::BdxTaskMainGetTime(const time_t ttime)
{

        time_t tmpTime1=0;
        if(ttime == 0)
                {tmpTime1 = time(0);
                //LOG(ERROR, "[thread: %d]ttime=%ld,tmpTime=%ld",m_uiThreadId,ttime,tmpTime1);
                }
        else
                tmpTime1 = ttime;
        struct tm* timeinfo1 = localtime(&tmpTime1);
        char dt1[10],dt2[20];
        memset(dt1, 0, 10);
        memset(dt2, 0, 10);
        string tempDate;
        long int tmp=0;
        strftime(dt2,20,"%Y%m%d%H",timeinfo1);
        //LOG(ERROR, "[thread: %d]dt2###########################=%s",m_uiThreadId,dt2);
        tmp=(timeinfo1->tm_year + 1900)*1000000+(timeinfo1->tm_mon+1)*10000+(timeinfo1->tm_mday)*100+(timeinfo1->tm_hour);
        //LOG(ERROR, "[thread: %d]dt1###########################=%ld",m_uiThreadId,tmp);
        //sprintf(dt1, "%4d%02d%02d%02d", timeinfo1->tm_year + 1900,timeinfo1->tm_mon+1,timeinfo1->tm_mday,timeinfo1->tm_hour);
        //sprintf(dt, "%4d%02d%02d%02d%02d%02d", timeinfo->tm_year + 1900, timeinfo->tm_mon+1,timeinfo->tm_mday,timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
        sprintf(dt1,"%ld",tmp);
        //LOG(ERROR, "[thread: %d]dt1###########################=%s",m_uiThreadId,dt1);
        return std::string(dt2);
        
}

std::string CTaskMain::BdxTaskMainGetMinute(const time_t ttime)
{

	time_t tmpTime;
	if(ttime == 0)
		tmpTime = time(0);
	else
		tmpTime = ttime;
	struct tm* timeinfo = localtime(&tmpTime);
	char dt2[20];
	memset(dt2, 0, 20);
	sprintf(dt2, "%4d%02d%02d%02d%02d", timeinfo->tm_year + 1900,timeinfo->tm_mon+1,timeinfo->tm_mday,timeinfo->tm_hour,timeinfo->tm_min);
	//sprintf(dt, "%4d%02d%02d%02d%02d%02d", timeinfo->tm_year + 1900, timeinfo->tm_mon+1,timeinfo->tm_mday,timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
	//return (timeinfo->tm_year + 1900) * 10000 + (timeinfo->tm_mon + 1) * 100 + timeinfo->tm_mday;
	return std::string(dt2);
}

std::string CTaskMain::BdxTaskMainGetFullTime(const time_t ttime)
{

	time_t tmpTime;
	if(ttime == 0)
		tmpTime = time(0);
	else
		tmpTime = ttime;
	struct tm* timeinfo = localtime(&tmpTime);
	char dt3[20];
	memset(dt3, 0, 20);
	sprintf(dt3, "%4d%02d%02d%02d%02d%02d", timeinfo->tm_year + 1900,timeinfo->tm_mon+1,timeinfo->tm_mday,timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
	//sprintf(dt, "%4d%02d%02d%02d%02d%02d", timeinfo->tm_year + 1900, timeinfo->tm_mon+1,timeinfo->tm_mday,timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
	//return (timeinfo->tm_year + 1900) * 10000 + (timeinfo->tm_mon + 1) * 100 + timeinfo->tm_mday;
	return std::string(dt3);
}
std::string CTaskMain::BdxTaskMainGetUCTime(const time_t ttime)
{

	time_t tmpTime;
	if(ttime == 0)
	{
		tmpTime = time(0);
	}
	else
	{
		tmpTime = ttime;
	}
	tmpTime -= 8*3600;
	struct tm* timeinfo = localtime(&tmpTime);
	char dt4[20];
	memset(dt4, 0, 20);

	sprintf(dt4, "%4d-%02d-%02dT%02d:%02d:%02dZ", timeinfo->tm_year + 1900,timeinfo->tm_mon+1,timeinfo->tm_mday,timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
	//sprintf(dt, "%4d%02d%02d%02d%02d%02d", timeinfo->tm_year + 1900, timeinfo->tm_mon+1,timeinfo->tm_mday,timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
	//return (timeinfo->tm_year + 1900) * 10000 + (timeinfo->tm_mon + 1) * 100 + timeinfo->tm_mday;
	return std::string(dt4);
}

std::string CTaskMain::BdxTaskMainGetDate(const time_t ttime)
{

	time_t tmpTime;
	if(ttime == 0)
		tmpTime = time(0);
	else
		tmpTime = ttime;
	struct tm* timeinfo = localtime(&tmpTime);
	char dt5[20];
	memset(dt5, 0, 20);
	sprintf(dt5, "%4d%02d%02d", timeinfo->tm_year + 1900, timeinfo->tm_mon+1,timeinfo->tm_mday);
	//return (timeinfo->tm_year + 1900) * 10000 + (timeinfo->tm_mon + 1) * 100 + timeinfo->tm_mday;
	return std::string(dt5);
}

std::string CTaskMain::BdxTaskMainGetMonth(const time_t ttime)
{

	time_t tmpTime;
	if(ttime == 0)
		tmpTime = time(0);
	else
		tmpTime = ttime;
	struct tm* timeinfo = localtime(&tmpTime);
	char dt6[20];
	memset(dt6, 0, 20);
	sprintf(dt6, "%4d%02d", timeinfo->tm_year + 1900, timeinfo->tm_mon+1);
	//return (timeinfo->tm_year + 1900) * 10000 + (timeinfo->tm_mon + 1) * 100 + timeinfo->tm_mday;
	return std::string(dt6);
}

std::string CTaskMain::BdxGenNonce(int length) 
{
        char CHAR_ARRAY[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b','c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x','y', 'z', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H','I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T','U', 'V', 'W', 'X', 'Y', 'Z'};
        srand((int)time(0));
         
        std::string strBuffer ;
        //int nextPos = strlen(CHAR_ARRAY);
        int nextPos = sizeof(CHAR_ARRAY);
        //printf("nextPos=%d\n",nextPos);
        int tmp = 0;
        for (int i = 0; i < length; ++i) 
        { 
            tmp = rand()%nextPos;
            
            strBuffer.append(std::string(1,CHAR_ARRAY[tmp]));
        }
        return strBuffer;
}

std::string CTaskMain::GenPasswordDigest(std::string utcTime, std::string nonce, std::string appSecret)
{
		std::string strDigest;

		std::string strValue = nonce + utcTime + appSecret;

        unsigned char *dmg = mdSHA1.SHA1_Encode(strValue.c_str());
        const  char *pchTemp = (const  char *)(char*)dmg;
        //std::string strDmg = base64_encode((const unsigned char*)pchTemp,strlen(pchTemp));
        std::string strDmg = base64_encode((const unsigned char*)pchTemp,SHA_DIGEST_LENGTH);
		//std::string strDmg = base64_encode(reinterpret_cast<const char *>(static_cast<void*>(dmg)),strlen(dmg));
        return strDmg;
}

string   CTaskMain::BdxTaskMainReplace_All(string    str,   string   old_value,   string   new_value)   
{   
    while(true)   {   
        string::size_type   pos(0);   
        if(   (pos=str.find(old_value))!=string::npos )  
        	{	
        		printf("Line:%d,str=%s\n",__LINE__,str.c_str());
        		str.replace(pos,old_value.length(),new_value);   
            }
        else   break;   
    }   
    return   str;   
}   


