/*
 * TaskMain.h

 */

#ifndef __TASK_MAIN__
#define __TASK_MAIN__

#include "../UserQueryTask.h"
#include "../../CommonTools/Conf/Conf.h"

class CTaskMain : public CUserQueryTask {

public:
	CTaskMain();
	CTaskMain(CTcpSocket* pclSock);
	virtual ~CTaskMain();
	
	UESRQUERYRPORT_S m_st;

	virtual int BdxRunTask(BDXREQUEST_S& stRequestInfo, BDXRESPONSE_S& stResponseInfo);
//protected:

	int BdxGetHttpPacket(BDXREQUEST_S& stRequestInfo,BDXRESPONSE_S &stResponseInfo,std::string &retKey,std::string &retKeyType,std::string &retUser,std::string &errorMsg,std::string&retParams);
	int BdxParseHttpPacket(char*& pszBody, u_int& uiBodyLen, const u_int uiParseLen);
	int BdxParseBody(char *pszBody, u_int uiBodyLen, BDXREQUEST_S& stRequestInfo);
	int BdxSendEmpyRespones(std::string errorMsg);
	int BdxSendRespones(BDXREQUEST_S& stRequestInfo, BDXRESPONSE_S& stAdxRes,std::string errorMsg);
	std::string BdxTaskMainGetDate(const time_t ttime = 0);
	std::string BdxTaskMainGetNextDate(const time_t ttime = 0);
	std::string BdxTaskMainGetLastDate(const time_t ttime = 0);
	std::string BdxTaskMainGetMonth(const time_t ttime = 0);
	string   BdxTaskMainReplace_All(string    str,   string   old_value,   string   new_value);

	std::string BdxTaskMainGetTime(const time_t ttime = 0);
	std::string BdxTaskMainGetMinute(const time_t ttime = 0);
	std::string BdxTaskMainGetFullTime(const time_t ttime=0);
	std::string BdxTaskMainGetUCTime(const time_t ttime = 0);
	std::string BdxGenNonce(int length);
	std::string GenPasswordDigest(std::string utcTime, std::string nonce, std::string appSecret);
	std::string BdxGetParamSign(const std::string& strParam, const std::string& strSign);
private:
	char m_pszAdxBuf[_8KBLEN];
	static const char* m_pszHttpHeaderEnd;
	static const char* m_pszHttpLineEnd;
	static const std::string keyEdcpMd5Sign;
	std::string ssCountKeyReq;
	std::string ssCountKeyRes;
	std::string ssCountKeyEmptyRes;
	CMd5 mdSHA1;
	int m_httpType ;
	CConf  mConf;	
	std::map<std::string,std::string> m_mapUserValue;
	CTime m_cTime;
	


	
	
};

#endif /* __TASK_MAIN__ */
