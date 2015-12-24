/*
 * UserQueryUpdate.cpp
 */

#include "UserQueryUpdate.h"

#include "../UserQueryWorkThreads/UserQueryWorkThreads.h"
#include "../UserQueryServer/UserQueryServer.h"


extern CLog *gp_log;
extern std::string strServerName;
extern std::string strConfigFileName;
pthread_rwlock_t p_rwlock;
pthread_rwlockattr_t p_rwlock_attr;
pthread_mutex_t mutex;
std::string g_strTokenString = "";
u_int  g_iNeedUpdateToken = 0;
extern u_int  InitSSLFlag ;

extern	IPPORT_S m_stMonitorApi;
extern	IPPORT_S m_stEmailServer;
extern	string m_strUserName;
extern	string m_strPassWord;
extern	string m_strMailFrom;
extern	string m_strRcptTo;
extern	string m_strSubject;
extern	string m_strErrorMsg;
	


std::map<std::string,BDXPERMISSSION_S> g_mapUserInfo;



CUserQueryUpdate::CUserQueryUpdate(const IPPORT_S &stTokenServer,const MYSQL_SERVERINFO_S &mySqlInfo ) {
	// TODO Auto-generated constructor stub


	char chCount[10];
	char chPort [10];
	char chIdx  [10];
    //m_pTokenRedis = new CDataAdapter;
    cTaskMonitorHuaWei = new CTaskMain;
    m_stMysqlServerInfo = new CMYSQL;
	sprintf(chCount,"%d",stTokenServer.m_count);
	sprintf(chPort,"%d",stTokenServer.m_uiPort);
	sprintf(chIdx,"%d",stTokenServer.m_idx);
	string strServerList = string(chCount)+";"+string(stTokenServer.m_pchIp)+":"+string(chPort)+","+string(chIdx)+";";
	
  	printf("Redis Server=%s\n",strServerList.c_str());
	//m_pTokenRedis->Init(strServerList.c_str());
  
	printf("==============mysql info==============\n");
	printf("%s\n",mySqlInfo.m_stMysqlLinkInfo.m_pchIp);
	printf("%d\n",mySqlInfo.m_stMysqlLinkInfo.m_uiPort);
	printf("%s\n",mySqlInfo.pchUserName);
	printf("%s\n",mySqlInfo.pchPassWord);
	printf("%s\n",mySqlInfo.pchDbName);
	printf("======================================\n");
	m_stMysqlServerInfo->InitMysql(mySqlInfo.m_stMysqlLinkInfo.m_pchIp,mySqlInfo.m_stMysqlLinkInfo.m_uiPort,mySqlInfo.pchUserName,mySqlInfo.pchPassWord,mySqlInfo.pchDbName);

}

CUserQueryUpdate::~CUserQueryUpdate() {
	// TODO Auto-generated destructor stub
	//delete m_pTokenRedis ;
    delete m_stMysqlServerInfo ;
}

void CUserQueryUpdate::GetMysqlFieldsUserInfo(std::string strUserInfo,BDXPERMISSSION_S &mVecFieldsUser,std::string &mUserName) 
{
	char *buf;
	int  index = 0;
	char bufTemp[PACKET];
	char *outer_ptr = NULL;  
	char *inner_ptr = NULL;  
	char *temp[PACKET]; 
	
	memset(temp,0,PACKET);
	memset(bufTemp,0,PACKET);
	mVecFieldsUser.mVecFields.clear();
					printf("strUserInfo=%s\n",strUserInfo.c_str());
					buf = const_cast<char*>(strUserInfo.c_str());
					while((temp[index] = strtok_r(buf,";", &outer_ptr))!=NULL)   
					{  	
					    buf=temp[index];
					    while((temp[index]=strtok_r(buf,"|", &inner_ptr))!=NULL)   
					    {   			    
					    	if(index >= 5)
					        {
					            //g_vecUrlAPIS[temp[index-1]]=temp[index];
					            mVecFieldsUser.mVecFields.push_back(temp[index]);
					        }					     	       					        
					        index++;
					        buf=NULL;  
					    }  
					    buf=NULL;  
					}  
					mUserName =	std::string(temp[0]);
					mVecFieldsUser.mResToken =	temp[1];
					mVecFieldsUser.mIntQueryTimes =	atoi(temp[2]);
					mVecFieldsUser.mIntGoodsTimes =	atoi(temp[3]);
					mVecFieldsUser.mGoodsFields =	std::string(temp[4]);
					std::vector<std::string>::iterator itr;
					#if 0
					for(itr=mVecFieldsUser.mVecFields.begin();itr!=mVecFieldsUser.mVecFields.end();itr++)
					{
						printf("=====%s\n",(*itr).c_str());
					}
					#endif


}

#if 0
bool CUserQueryUpdate::VectorIsEqual(std::vector<std::string> srcVector,std::vector<std::string> destVector) 
{
	std::vector<std::string>::iterator itrSrcVec;
	std::vector<std::string>::iterator itrDestVec;
	std::vector<std::string>::iterator itr2;

	if( srcVector.size() != srcVector.size())
	{
		return false;
	}
	for(itrSrcVec=srcVector.begin();itrSrcVec!=srcVector.end();itrSrcVec++)
	{
		itrDestVec = destVector.find(*itrSrcVec);
		if(itrDestVec !=itrDestVec.end())
		{
			continue;
		}
		else
		{
			return false;
		}

	}
	
return true;
}

#endif

bool CUserQueryUpdate::MapIsEqual(std::map<std::string,BDXPERMISSSION_S> &srcMap,std::map<std::string,BDXPERMISSSION_S> &destMap) 
{
	std::map<std::string,BDXPERMISSSION_S>::iterator itrSrcMap;
	std::map<std::string,BDXPERMISSSION_S>::iterator itrDestMap;
	std::vector<std::string>::iterator itr2;

	if( srcMap.size() != destMap.size())
	{
		return false;
	}
	
	for(itrSrcMap=srcMap.begin();itrSrcMap!=srcMap.end();itrSrcMap++)
	{
		itrDestMap = destMap.find(itrSrcMap->first);
		if(itrDestMap !=destMap.end())
		{
				if(itrSrcMap->second.mResToken!=itrDestMap->second.mResToken||
				itrSrcMap->second.mIntQueryTimes!=itrDestMap->second.mIntQueryTimes||
				itrSrcMap->second.mVecFields!=itrDestMap->second.mVecFields||
				itrSrcMap->second.mGoodsFields!=itrDestMap->second.mGoodsFields||
				itrSrcMap->second.mIntGoodsTimes!=itrDestMap->second.mIntGoodsTimes)
				//VectorIsEqual(itrSrcMap->second.mVecFields,)
				{
					return false;

				}
				

		}
		else
		{
			return false;
		}

	}

return true;
}

void CUserQueryUpdate::SwapMap(std::map<std::string,BDXPERMISSSION_S> &srcMap,std::map<std::string,BDXPERMISSSION_S> &destMap) 
{
	std::map<std::string,BDXPERMISSSION_S>::iterator itrSrcMap;
	BDXPERMISSSION_S mUserInfoVecFields;
	std::vector<std::string>::iterator itrSrcVector;

	destMap.clear();
	for(itrSrcMap=srcMap.begin();itrSrcMap!=srcMap.end();itrSrcMap++)
	{
		mUserInfoVecFields.mVecFields.clear();
		mUserInfoVecFields.mVecFields 		= itrSrcMap->second.mVecFields;
		mUserInfoVecFields.mResToken  		= itrSrcMap->second.mResToken;
		mUserInfoVecFields.mIntQueryTimes   = itrSrcMap->second.mIntQueryTimes;
		mUserInfoVecFields.mIntGoodsTimes   = itrSrcMap->second.mIntGoodsTimes;
		mUserInfoVecFields.mGoodsFields   = itrSrcMap->second.mGoodsFields;
		destMap.insert(std::pair<std::string,BDXPERMISSSION_S>(itrSrcMap->first,mUserInfoVecFields));

	}
	
}

void CUserQueryUpdate::MonitorRemoteApiHuaWei()
{
	int monitorFlag,lenStrTemp ;
	string strEncrpytKey,mResValueRemote,errorMsg,tempRcptTo,monitorValue="[{\"name\":\"0\",\"value\":\"0\",\"typeid\":\"0\"}]";
	//string strIpPort="210.14.129.121:8443";
	//string strIpPort=m_stMonitorApi.m_pchIp + 
	//string errorMsg="There is some problem in API Service,please check it";
	char m_httpReq[_8KBLEN],sslBuffer[_8KBLEN];
	memset(m_httpReq, 0, _8KBLEN);
	memset(sslBuffer, 0, _8KBLEN);
		
		monitorFlag = 0;
		strEncrpytKey="/dg/rtb/tag?&key=JSQkdnFjfm4pfXF5fTJgeGF1cnFwKXB6e3cydWB1cGx9eSlxeXV6S2RncBQ=";
		printf("monitor strEncrpytKey=%s\n",strEncrpytKey.c_str());
		
		std::string strAppSecret = "5543131e56e6a10d";
		std::string strHost = "dacp.sdp.com";
		//std::string strHost = "dacp.test.sdp.com";
		std::string strUcTime = cTaskMonitorHuaWei->BdxTaskMainGetUCTime();						
		std::string strNonce =  cTaskMonitorHuaWei->BdxGenNonce(25);
		printf("strUcTime=%s\n",strUcTime.c_str());
		printf("strNonce=%s\n",strNonce.c_str());	

		std::string strPwdDigest =  cTaskMonitorHuaWei->GenPasswordDigest(strUcTime,strNonce,strAppSecret);
		printf("strPwdDigest=%s\n",strPwdDigest.c_str());
		
		std::string XWSSEHeader = "UsernameToken Username=\"f2b98698400e4ace9926f83f0d063a57\",PasswordDigest=\""+strPwdDigest + "\", Nonce=\""+strNonce + "\", Created=\""+ strUcTime+ "\"";
		sprintf(m_httpReq,"GET %s HTTP/1.1\r\nAuthorization: WSSE realm=\"SDP\",profile=\"UsernameToken\",type=\"AppKey\"\r\nHost: %s\r\nX-WSSE: %s\r\nAccept-Encoding: identity\r\n\r\n",strEncrpytKey.c_str(),strHost.c_str(),XWSSEHeader.c_str());
		printf("%s\n",m_httpReq);
		LOG(ERROR,"m_httpReq=%s",m_httpReq);
		string tempMonitor="JSQkdnFjfm4pfXF5fTJgeGF1cnFwKXB6e3cydWB1cGx9eSlxeXV6S2RncBQ=";
		printf("tempMonitor+++++++++++++++++++++=%s\n",tempMonitor.c_str());
		tempMonitor=Decrypt_2(tempMonitor);
		printf("tempMonitor======================%s\n",tempMonitor.c_str());
	  //sslMonitorSocket=new CTcpSocket(8000,"218.205.68.67");
	  //sslMonitorSocket=new CTcpSocket(8443,"210.14.129.121");
	  printf("m_stMonitorApi.m_uiPort=%d\n",m_stMonitorApi.m_uiPort);
	  printf("m_stMonitorApi.m_pchIp=%s\n",string(m_stMonitorApi.m_pchIp).c_str());
	  sslMonitorSocket=new CTcpSocket(m_stMonitorApi.m_uiPort,string(m_stMonitorApi.m_pchIp));
	  errorMsg = m_strErrorMsg;
	  if(sslMonitorSocket->TcpConnect()!=0)
	  {	
			printf("sslMonitorSocket connect error\n");
			monitorFlag = 1;
	  }

	  printf("m_strRcptTo.c_str()=%s\n",m_strRcptTo.c_str());

		#if 0
		pthread_mutex_lock (&mutex);
		if ( InitSSLFlag == 0 )
		{
			sslMonitorSocket->TcpSslInitParams();
			InitSSLFlag = 1;
			}
		pthread_mutex_unlock(&mutex);
		if(sslMonitorSocket->TcpSslInitEnv()!=0)
		{
			printf("TcpSslInitEnv error\n");
			monitorFlag = 1;							
		}
		if(!sslMonitorSocket->TcpSslConnect())
		{
			printf("TcpSslConnect error\n");
			sslMonitorSocket->TcpSslDestroy();
			monitorFlag = 1;
		}
		#endif
		if(monitorFlag == 0)
		{
			printf("m_httpReq=%s\n",m_httpReq);
			if(sslMonitorSocket->TcpWrite(m_httpReq,strlen(m_httpReq))!=0)
			{
				memset(sslBuffer,0,_8KBLEN);
				sslMonitorSocket->TcpReadAll(sslBuffer,_8KBLEN);
				printf("Monitor sslBuffer=%s\n",sslBuffer); 
				LOG(ERROR,"Monitor sslBuffer=%s",sslBuffer);  

				if(strlen(sslBuffer) > 0 )
				{
					mResValueRemote = std::string(sslBuffer);
					LOG(ERROR,"Monitor mResValueRemote=%s",mResValueRemote.c_str());  
					sslMonitorSocket->TcpClose();
				}
				else
				{
					sslMonitorSocket->TcpClose();
					monitorFlag =  2;				

				}
			}		
			else 
			{	
				sslMonitorSocket->TcpClose();
				monitorFlag =  1;
			}

			lenStrTemp = mResValueRemote.length();

			if( mResValueRemote.find("\r\n\r\n")!=std::string::npos )
			{
				mResValueRemote = mResValueRemote.substr(mResValueRemote.find("\r\n\r\n")+4,lenStrTemp -(mResValueRemote.find("\r\n\r\n")+4));
			}
			printf("before decrypt mResValueRemote=%s\n",mResValueRemote.c_str());
			LOG(ERROR,"before decrypt mResValueRemote=%s",mResValueRemote.c_str()); 
			if(!mResValueRemote.empty())
			{
			   printf("mResValueRemote=%s\n",mResValueRemote.c_str());
			   if((mResValueRemote.at(0)!='{')&&(mResValueRemote.at(0)!='<')&&(mResValueRemote.find("description")==std::string::npos))
			   {
					mResValueRemote = Decrypt_2(mResValueRemote);
			   }
			   
			}
			LOG(ERROR,"after decrypt mResValueRemote=%s",mResValueRemote.c_str()); 
			printf("after decrypt mResValueRemote=%s\n",mResValueRemote.c_str());
			printf("monitorValue=%s\n",monitorValue.c_str());
			
			if( monitorFlag==2 )
			{
				errorMsg+="  \nError Massage: Read data empty\n";

			}
			else
			{
				errorMsg+="  \nError Massage: \n"+mResValueRemote;
			}
			if( mResValueRemote.compare(monitorValue)!= 0 )
			{
				#if 1
				//sendEmailSocket = new  CTcpSocket(25,"114.255.20.23");
				sendEmailSocket = new  CTcpSocket(m_stEmailServer.m_uiPort,string(m_stEmailServer.m_pchIp));
				if( sendEmailSocket->TcpConnect()==0 )
				{
					sendEmailSocket->TcpSendEmail(m_stEmailServer.m_pchIp,m_stEmailServer.m_uiPort,const_cast<char*>(m_strUserName.c_str()),const_cast<char*>(m_strPassWord.c_str()),const_cast<char*>(m_strMailFrom.c_str()),const_cast<char*>(m_strRcptTo.c_str()),const_cast<char*>(m_strSubject.c_str()),const_cast<char*>(errorMsg.c_str()));
				}
				sendEmailSocket->TcpClose();
				#endif
			}
		}
		else
		{
			#if 1
			mResValueRemote = "The Client can't connect to Api server!";
		    errorMsg+="  \nError Massage: \n"+mResValueRemote;
			sendEmailSocket = new  CTcpSocket(m_stEmailServer.m_uiPort,string(m_stEmailServer.m_pchIp));
			if( sendEmailSocket->TcpConnect()==0 )
			{
				sendEmailSocket->TcpSendEmail(m_stEmailServer.m_pchIp,m_stEmailServer.m_uiPort,const_cast<char*>(m_strUserName.c_str()),const_cast<char*>(m_strPassWord.c_str()),const_cast<char*>(m_strMailFrom.c_str()),const_cast<char*>(m_strRcptTo.c_str()),const_cast<char*>(m_strSubject.c_str()),const_cast<char*>(errorMsg.c_str()));
			}
			sendEmailSocket->TcpClose();
			#endif
		}
		



}
void CUserQueryUpdate::MonitorRemoteApiWangGuan()
{
	int monitorFlag,lenStrTemp ;
	string strEncrpytKey,mResValueRemote,errorMsg,monitorValue="[{\"name\":\"0\",\"value\":\"0\",\"typeid\":\"0\"}]";
	//string strIpPort="210.14.129.121:8443";
	//string strIpPort=m_stMonitorApi.m_pchIp + 
	//string errorMsg="There is some problem in API Service,please check it";
	char m_httpReq[_8KBLEN],sslBuffer[_8KBLEN];
	memset(m_httpReq, 0, _8KBLEN);
	memset(sslBuffer, 0, _8KBLEN);

	  monitorFlag = 0;
	  strEncrpytKey="/dg/api/query/rtbtag2?token="+g_strTokenString+"&appkey=f2b98698400e4ace9926f83f0d063a57&key=JSQkdnFjfm4pfXF5fTJgeGF1cnFwKXB6e3cydWB1cGx9eSlxeXV6S2RncBQ=";
	  printf("monitor strEncrpytKey=%s\n",strEncrpytKey.c_str());
	  sprintf(m_httpReq,"GET %s HTTP/1.1\r\nHost: %s:%d\r\nAccept-Encoding: identity\r\n\r\n",strEncrpytKey.c_str(),m_stMonitorApi.m_pchIp,m_stMonitorApi.m_uiPort);

	  //sslMonitorSocket=new CTcpSocket(8000,"218.205.68.67");
	  //sslMonitorSocket=new CTcpSocket(8443,"210.14.129.121");
	  sslMonitorSocket=new CTcpSocket(m_stMonitorApi.m_uiPort,string(m_stMonitorApi.m_pchIp));
	  errorMsg = m_strErrorMsg;
	  if(sslMonitorSocket->TcpConnect()!=0)
	  {	
			printf("sslMonitorSocket connect error\n");
			monitorFlag = 1;
	  }

		pthread_mutex_lock (&mutex);
		if ( InitSSLFlag == 0 )
		{
			sslMonitorSocket->TcpSslInitParams();
			InitSSLFlag = 1;
			}
		pthread_mutex_unlock(&mutex);
		if(sslMonitorSocket->TcpSslInitEnv()!=0)
		{
			printf("TcpSslInitEnv error\n");
			monitorFlag = 1;							
		}
		if(!sslMonitorSocket->TcpSslConnect())
		{
			printf("TcpSslConnect error\n");
			sslMonitorSocket->TcpSslDestroy();
			monitorFlag = 1;
		}
		if(monitorFlag == 0)
		{
			printf("m_httpReq=%s\n",m_httpReq);
			if(sslMonitorSocket->TcpSslWriteLen(m_httpReq,strlen(m_httpReq))!=0)
			{
				memset(sslBuffer,0,_8KBLEN);
				sslMonitorSocket->TcpSslReadLen(sslBuffer,_8KBLEN);
				printf("Monitor sslBuffer=%s\n",sslBuffer); 
				LOG(DEBUG,"Monitor sslBuffer=%s",sslBuffer);  

				if(strlen(sslBuffer) > 0 )
				{
					mResValueRemote = std::string(sslBuffer);
					LOG(DEBUG,"Monitor mResValueRemote=%s",mResValueRemote.c_str());  
					sslMonitorSocket->TcpSslDestroy();
				}
				else
				{
					sslMonitorSocket->TcpSslDestroy();
					monitorFlag =  1;				

				}
			}		
			else 
			{	
				sslMonitorSocket->TcpSslDestroy();
				monitorFlag =  1;
			}

			lenStrTemp = mResValueRemote.length();

			if( mResValueRemote.find("\r\n\r\n")!=std::string::npos )
			{
				mResValueRemote = mResValueRemote.substr(mResValueRemote.find("\r\n\r\n")+4,lenStrTemp -(mResValueRemote.find("\r\n\r\n")+4));
			}
			printf("before decrypt mResValueRemote=%s\n",mResValueRemote.c_str());
			if(!mResValueRemote.empty())
			{
			   mResValueRemote = Decrypt_2(mResValueRemote);
			}
			printf("after decrypt mResValueRemote=%s\n",mResValueRemote.c_str());
			printf("monitorValue=%s\n",monitorValue.c_str());
			errorMsg+="  \nError Massage: \n"+mResValueRemote;
			if( mResValueRemote.compare(monitorValue)!= 0 )
			{
				#if 1
				//sendEmailSocket = new  CTcpSocket(25,"114.255.20.23");
				sendEmailSocket = new  CTcpSocket(m_stEmailServer.m_uiPort,string(m_stEmailServer.m_pchIp));
				if( sendEmailSocket->TcpConnect()==0 )
				{
					//sendEmailSocket->TcpSendEmail("114.255.20.23",25,"dmp-message","MIX@asiainfo","dmp-message@asiainfo.com","BDX-DMS-Data@asiainfo.com","Monitor Api",const_cast<char*>(errorMsg.c_str()));
					sendEmailSocket->TcpSendEmail(m_stEmailServer.m_pchIp,m_stEmailServer.m_uiPort,const_cast<char*>(m_strUserName.c_str()),const_cast<char*>(m_strPassWord.c_str()),const_cast<char*>(m_strMailFrom.c_str()),const_cast<char*>(m_strRcptTo.c_str()),const_cast<char*>(m_strSubject.c_str()),const_cast<char*>(errorMsg.c_str()));
				}
				sendEmailSocket->TcpClose();
				#endif
			}
		}
		else
		{
			#if 1
			mResValueRemote = "The Client can't connect to Api server!";
		    //sendEmailSocket = new  CTcpSocket(25,"114.255.20.23");
		    errorMsg+="  \nError Massage: \n"+mResValueRemote;
			sendEmailSocket = new  CTcpSocket(m_stEmailServer.m_uiPort,string(m_stEmailServer.m_pchIp));
			if( sendEmailSocket->TcpConnect()==0 )
			{
				//sendEmailSocket->TcpSendEmail("114.255.20.23",25,"dmp-message","MIX@asiainfo","dmp-message@asiainfo.com","dmp-message@asiainfo.com","Monitor Api",const_cast<char*>(errorMsg.c_str()));
				sendEmailSocket->TcpSendEmail(m_stEmailServer.m_pchIp,m_stEmailServer.m_uiPort,const_cast<char*>(m_strUserName.c_str()),const_cast<char*>(m_strPassWord.c_str()),const_cast<char*>(m_strMailFrom.c_str()),const_cast<char*>(m_strRcptTo.c_str()),const_cast<char*>(m_strSubject.c_str()),const_cast<char*>(errorMsg.c_str()));
			}
			sendEmailSocket->TcpClose();
			#endif
		}
		



}
void CUserQueryUpdate::Core()
{

	std::string strToken,strTokenValue;
	Json::Value jValue,jRoot,jResult;
	Json::Reader jReader;
	Json::FastWriter jFastWriter;
	std::string sts="yd_zhejiang_mobile_token";
	std::string strMysqlRecord;
	const char *pchSqlPermissions = "select name,password,query_count,goods_count,goods_perm,permissions from dmp_user_permissions";
	BDXPERMISSSION_S mUserInfoVecFields;
	std::string strUserName;
	int times = 0;
	std::map<std::string,BDXPERMISSSION_S> temp_mapUserInfo;
	
	while(true)
	{
		times=1;	
		int first_row = 1;
		#if 0
        if(m_pTokenRedis->UserGet(sts,strToken))
        {
	        //if(jReader.parse(strToken, jValue))
	        //{
	        //      strTokenValue = jValue[TOKEN].asString();
	            if(strToken.compare(g_strTokenString)!=0)
	            {
	                    pthread_rwlock_wrlock(&p_rwlock);
	                    g_strTokenString = strToken;
	                    g_iNeedUpdateToken = 1;
	                    pthread_rwlock_unlock(&p_rwlock);
	            }
	        //}
        }
        //st_emrtb_ip_port_weight_flag = 0;
	  	printf("g_strTokenString = %s\n",g_strTokenString.c_str());
		#endif

	 	#define __MONITOR_API__
	  	#ifdef __MONITOR_API__

		//MonitorRemoteApiWangGuan();
		MonitorRemoteApiHuaWei();
									
		#endif //__MONITOR_API__
		
		while(times--)
		{
				temp_mapUserInfo.clear();
				if(m_stMysqlServerInfo->GetMysqlInitState())
				{
					if(m_stMysqlServerInfo->ExecuteMySql(pchSqlPermissions))
					{

						if(m_stMysqlServerInfo->MysqlUseResult())
						{	
							//m_stMysqlServerInfo->DisplayHeader();
							while(m_stMysqlServerInfo->MysqlFetchRow())
							{			
								//if(!first_row)
								{	
									strMysqlRecord = m_stMysqlServerInfo->GetColumnValue();
									//printf("strMysqlRecord = %s\n",strMysqlRecord.c_str());
									GetMysqlFieldsUserInfo(strMysqlRecord,mUserInfoVecFields,strUserName);
									temp_mapUserInfo.insert(std::pair<std::string,BDXPERMISSSION_S>(strUserName,mUserInfoVecFields));
									
								}
								first_row = 0;
							}

							std::map<std::string,BDXPERMISSSION_S>::iterator itr;
							std::vector<std::string>::iterator itr2;
							#if 1
							printf("===================temp_mapUserInfo========================\n");
							for(itr=temp_mapUserInfo.begin();itr!=temp_mapUserInfo.end();itr++)
							{	
								printf("%s ",itr->first.c_str());
								printf("%s ",itr->second.mResToken.c_str());
								printf("%d ",itr->second.mIntQueryTimes);
								printf("%d ",itr->second.mIntGoodsTimes);
								printf("%s ",itr->second.mGoodsFields.c_str());
								for(itr2=itr->second.mVecFields.begin();itr2!=itr->second.mVecFields.end();itr2++)
								{
									printf("%s ",(*itr2).c_str());
									
								}
								printf("\n");
							}
							#endif
							#if 1
							printf("===================g_mapUserInfo========================\n");
							for(itr=g_mapUserInfo.begin();itr!=g_mapUserInfo.end();itr++)
							{	
								printf("%s ",itr->first.c_str());
								printf("%s ",itr->second.mResToken.c_str());
								printf("%d ",itr->second.mIntQueryTimes);
								printf("%d ",itr->second.mIntGoodsTimes);
								printf("%s ",itr->second.mGoodsFields.c_str());
								for(itr2=itr->second.mVecFields.begin();itr2!=itr->second.mVecFields.end();itr2++)
								{
									printf("%s ",(*itr2).c_str());
								}
								printf("\n");
							}
							#endif
							if( !MapIsEqual(temp_mapUserInfo,g_mapUserInfo) )
							{
							//	g_mapUserInfo = temp_mapUserInfo;
								
								SwapMap(temp_mapUserInfo,g_mapUserInfo);
								printf("\nswap map g_mapUserInfo\n\n");				
							}

						}
						
					}

				}
				
		sleep(60);
		}	
	}

}

