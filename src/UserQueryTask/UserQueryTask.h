/*
 * UserQueryTask.h

 */

#ifndef __USERQUERYTASK__
#define __USERQUERYTASK__

#include "../CommonTools/Socket/TcpSocket.h"
#include "../CommonTools/Socket/UdpSocket.h"
#include "../UserQueryStruct/UserQueryStruct.h"
#include "../CommonTools/Time/Time.h" 
#include "../CommonTools/Md5/Md5.h"
#include "../redisCommon/data_adapter.h"


class CUserQueryTask {
public:
	CUserQueryTask(CTcpSocket* pclSock);
	CUserQueryTask();
	virtual ~CUserQueryTask();

	virtual int BdxRunTask(BDXREQUEST_S& stRequestInfo, BDXRESPONSE_S& stResponseInfo) = 0;
	virtual int BdxGetTaskFd() {
		return m_pclSock->TcpGetSockfd();
	};
	virtual void BdxSetUdpSocket(const u_int uiThreadId, CUdpSocket* pUdpSocket) {
		m_uiThreadId = uiThreadId;
		m_pUdpSocket = pUdpSocket;

	};
	virtual void BdxSetDataRedis(const u_int uiThreadId, CDataAdapter* pDataRedis,CDataAdapter* pDataRedis2) {
		m_uiThreadId = uiThreadId;
		m_pDataRedis = pDataRedis;
		m_pGoodsRedis= pDataRedis2;

	};



protected:
	virtual std::string BdxGetHttpDate();
	virtual void BdxGetUrlDomain(const char* pchUrl, const u_int uiUrlLen, std::string& strMediaDomain);
	virtual ULONGLONG BdxHtonll(const ULONGLONG ullNum);


protected:
	CTcpSocket* m_pclSock;
	CUdpSocket* m_pUdpSocket;
	CDataAdapter* m_pDataRedis;
	CDataAdapter* m_pGoodsRedis;
	u_int m_uiThreadId;
	CMd5 m_clMd5;
	CTime m_clEmTime;
	bool m_bSend;
	char m_pszKLVBuf[_64KBLEN];	
	int* m_piKLVLen;	
	char* m_piKLVContent;
	BDXURLSUFFIX_S m_stSuffix;

	char m_pszYunRequset[BUF_SIZE_64KB];
	char m_pszAdxResponse[BUF_SIZE_64KB];
	//char m_htmlBody[BUF_SIZE_64KB];
};

typedef struct {
	std::queue<CUserQueryTask*> m_queUserQueryTask;
	CMutexLock m_clLock;
}TASKQUE_S;

#endif /* __USERQUERYTASK__ */
