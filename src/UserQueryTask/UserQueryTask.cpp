/*
 * UserQueryTask.cpp
 */

#include "UserQueryTask.h"
#include "../UserQueryWorkThreads/UserQueryWorkThreads.h"
#include "../CommonTools/UrlEncode/UrlEncode.h"
//#include "../CommonTools/Socket/UdpSocket.h"
 

extern CLog *gp_log;
extern struct sockaddr_in m_stSockaddr_local;
extern pthread_rwlock_t p_rwlock;

static const char *g_week[] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
static const char *g_month[] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

CUserQueryTask::CUserQueryTask(CTcpSocket* pclSock): m_pclSock(pclSock)
{
	// TODO Auto-generated constructor stub

}

CUserQueryTask::CUserQueryTask()
{
	// TODO Auto-generated constructor stub

}
CUserQueryTask::~CUserQueryTask() {
	// TODO Auto-generated destructor stub
	if(m_pclSock) {
		delete m_pclSock;
		m_pclSock = NULL;
	}
}


std::string CUserQueryTask::BdxGetHttpDate()
{
	time_t t_time;
	time(&t_time);
	tm *ptm = localtime(&t_time);

	char pszTmpBuf[_128BYTES] = "";

	sprintf(pszTmpBuf,"%s, %02d %s %04d %02d:%02d:%02d GMT\r\n",
			g_week[ptm->tm_wday],ptm->tm_mday,g_month[ptm->tm_mon],ptm->tm_year+1900,
			ptm->tm_hour,ptm->tm_min,ptm->tm_sec);
	return std::string(pszTmpBuf);
}

void CUserQueryTask::BdxGetUrlDomain(const char* pchUrl, const u_int uiUrlLen, std::string& strMediaDomain)
{
    std::vector<std::string> vecDomain;
    std::vector<std::string> vecSuffix;
    char* pszUrl = (char *)pchUrl;
    if(!strncmp(pszUrl, "http://", 7)) {
            pszUrl += 7;
    }
    if(!strncmp(pszUrl, "https://", 8)) {
            pszUrl += 8;
    }
    int iUrlLen = strlen(pszUrl);
    int iBegin = 0;
    std::string strDomain;
    int iIndex = 0;
    for(; iIndex < iUrlLen; ++iIndex) {

            if(pszUrl[iIndex] == '.') {
                    strDomain = std::string(pszUrl + iBegin, 0, iIndex - iBegin);
                    if(m_stSuffix != strDomain)
                            vecDomain.push_back(strDomain);
                    else if(strDomain != "www")
                    	vecSuffix.push_back(strDomain);
                    iBegin = iIndex + 1;

            } else if(pszUrl[iIndex] == '/') {
                    break;
            }
    }

    if(iIndex > iBegin) {
    	vecSuffix.push_back(std::string(pszUrl + iBegin, 0, iIndex - iBegin));
    }

    for(u_int i = /*2 < vecDomain.size() ? (vecDomain.size() - 2) : */0; i < vecDomain.size(); ++i) {
    	strMediaDomain += vecDomain[i] + ".";
    }

    for(u_int i = 0; i < vecSuffix.size(); ++i) {
    	if(i != vecSuffix.size() - 1) {
    		strMediaDomain += vecSuffix[i] + ".";
    	} else {
    		strMediaDomain += vecSuffix[i];
    	}
    }
}

ULONGLONG CUserQueryTask::BdxHtonll(const ULONGLONG ullNum)
{

	ULONGLONG ullTmp = (ullNum>>32), ullAll = 0xffffffffULL & ullNum;
	ullTmp=htonl(ullTmp);
	ullAll=htonl(ullAll);
	return ((ULONGLONG)ullAll<<32)| ullTmp;
}


