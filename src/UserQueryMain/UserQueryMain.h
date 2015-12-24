/*

 */

#ifndef __RTBADAPTERMAIN_H__
#define __RTBADAPTERMAIN_H__

#include "../UserQueryStruct/UserQueryStruct.h"
#include "../CommonTools/Conf/Conf.h"
#include "../CommonTools/Thread/Thread.h"
#include <stdlib.h>



class CUserQueryMain {
public:
	CUserQueryMain(char* pchConfFile = const_cast<char*>(std::string("./config/userQuery.conf").c_str()));
	//CUserQueryMain(char* pchConfFile = "./config/userQuery.conf");
	virtual ~CUserQueryMain();

	void UserQueryMainCore();
protected:
	bool BdxInitConfig(const char *pszCfgFile);
	bool BdxGetLogPrms(CConf *pCConf, char *pszSection);
	bool BdxGetStatisticPrms(CConf *pCConf, char *pszSection);
	bool BdxGetHiveLogPrms(CConf *pCConf, char *pszSection);
	bool BdxGetServerPrms(CConf *pCConf, char *pszSection, IPPORT_S& stIpPort);
	bool BdxGetMysqlServerPrms(CConf *pCConf, char *pszSection, MYSQL_SERVERINFO_S& stMysqlServer);
	bool BdxGetServerUrlAPI(CConf *pCConf, char *pszSection);
	bool BdxGetMonitorApiPrms(CConf *pCConf, char *pszSection);
	bool BdxGetThreadsPrm(CConf *pCConf, char *pszSection);
	void BdxPrintConfig();

private:
	LOGPRMS_S m_stLogPrms;
	SERVERINFO_S m_stServerInfo;
	MYSQL_SERVERINFO_S m_stMysqlInfo;
	std::vector<std::string> m_vecDevices;
	u_int m_uiThreadsNum;
	CThreads m_cThreads;
	STATISTICSPRM_S m_stStatisticsPrm;
	STATISTICSPRM_S m_stHiveLogPrm;
	MORNITORAPI_INFO_S m_stMornitorApi;
};

#endif /* __RTBADAPTERMAIN_H__ */

