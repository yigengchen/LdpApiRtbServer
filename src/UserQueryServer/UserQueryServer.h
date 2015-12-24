/*
 * UserQueryServer.h
 */

#ifndef __USERQUERYSERVER_H__
#define __USERQUERYSERVER_H__

#include "../UserQueryStruct/UserQueryStruct.h"
#include "../CommonTools/Socket/TcpSocket.h"
#include "UserQueryServerEpoll.h"
#include "../UserQueryTask/TaskMain/TaskMain.h"

#define NOFREE_THREAD        -5

class CUserQueryServer {
public:
	CUserQueryServer(const IPPORT_S& stServer, const u_int uiWorkThreadNum);
	virtual ~CUserQueryServer();

	void Core();
protected:
	bool BdxInitServer();
	int  BdxServerPutTask(CUserQueryTask* pclTask);
	bool BdxServerReadEvent(int iSockfd);
	bool BdxServerGetTask(CTcpSocket* pclSock);
	bool BdxServerGetFreeTask();
private:
	CTcpSocket* m_pclSock;
	u_int m_uiWorkThreadNum;
	std::vector<std::string> m_vecDevice;
	CUserQueryServerEpoll m_clEpoll;

	std::map<int, CUserQueryTask*> m_mpSocketTask;
};

#endif /* __USERQUERYSERVER_H__ */
