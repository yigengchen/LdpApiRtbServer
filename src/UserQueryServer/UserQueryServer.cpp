/*
 * CUserQueryServer.cpp
 */

#include "UserQueryServer.h"
#include "../UserQueryWorkThreads/UserQueryWorkThreads.h"

extern CLog *gp_log;
extern std::string strServerName;

CUserQueryServer::CUserQueryServer(const IPPORT_S& stServer, const u_int uiWorkThreadNum): m_uiWorkThreadNum(uiWorkThreadNum)
{
	// TODO Auto-generated constructor stub
	m_pclSock = new CTcpSocket(stServer.m_uiPort, stServer.m_pchIp);
	if(!m_pclSock) {
		throw std::runtime_error("new socket error.");
	}

	if(!BdxInitServer()) {
		throw std::runtime_error("AdInitServer error.");
	}
}

CUserQueryServer::~CUserQueryServer() {
	// TODO Auto-generated destructor stub
	if(m_pclSock) delete m_pclSock;
}

void CUserQueryServer::Core()
{
	CTcpSocket* pclClient = NULL;
	struct epoll_event stEvent[EVENT];
	int iNumber = 0;
	int iCount = 0;

	while(true) {
		BdxServerGetFreeTask();
		memset(&stEvent, '\0', sizeof(stEvent));
		iNumber = m_clEpoll.BdxEpollWait(stEvent, 5);//5ms time out.
		for (iCount = 0; iCount < iNumber; iCount ++) {
			if (stEvent[iCount].data.fd == m_pclSock->TcpGetSockfd()) {//accept

				pclClient = m_pclSock->TcpAccept();
				if(!pclClient) {
					LOG(ERROR, "Accept client error, continue now.");
					continue;
				}

				if(!BdxServerGetTask(pclClient)) {
					LOG(ERROR, "BdxServerGetTask error.");
					delete pclClient;
					continue;
				}

			} else {//read

				if ((stEvent[iCount].events & EPOLLIN)) {

					if(!BdxServerReadEvent(stEvent[iCount].data.fd)) {
						LOG(ERROR, "BdxServerReadEvent error.");
						continue;
					}

				} else {
					//not listen other event?
				}
			}
		}
	}
}

bool CUserQueryServer::BdxInitServer()
{
	if(!m_pclSock->TcpBind()) {
		LOG(ERROR, "bind server error.");
		return false;
	}

	if(!m_pclSock->TcpListen()) {
		LOG(ERROR, "lister server error.");
		return false;
	}

	if(!m_pclSock->TcpSetNoblock() ){
		LOG(ERROR, "TcpSetNoblock error.");
		return false;
	}

	if(!m_clEpoll.BdxEpollCtr(EPOLL_CTL_ADD, EPOLLIN, m_pclSock->TcpGetSockfd())) {
		LOG(ERROR, "BdxEpollCtr error.");
		return false;
	}
	return true;
}

//0: success -1: error  -5:no free thread
int CUserQueryServer::BdxServerPutTask(CUserQueryTask* pclTask)
{

	int iRes = 0;
	for(u_int i = 0; i < m_uiWorkThreadNum; ++i) {
		iRes = CUserQueryWorkThreads::m_vecTaskBuf[i]->m_clLock.MutexTryLock();
		if(iRes == 1) continue;

		if(iRes == 0) {
			CUserQueryWorkThreads::m_vecTaskBuf[i]->m_queUserQueryTask.push(pclTask);
			CUserQueryWorkThreads::m_vecTaskBuf[i]->m_clLock.MutexNotify();
			CUserQueryWorkThreads::m_vecTaskBuf[i]->m_clLock.MutexUnlock();
			return SUCCESS;
		}

		return OTHERERROR;
	}

	return NOFREE_THREAD;
}

bool CUserQueryServer::BdxServerReadEvent(int iSockfd)
{
	int iRes = 0;
	std::map<int, CUserQueryTask*>::iterator mit;
	mit = m_mpSocketTask.find(iSockfd);
	if(mit == m_mpSocketTask.end()) {
		LOG(REQUIRED, "can not find [fd:%d] Task.", iSockfd);
		close(iSockfd);
		return false;
	}

	m_clEpoll.BdxEpollCtr(EPOLL_CTL_DEL, EPOLLIN, iSockfd);
	iRes = BdxServerPutTask(mit->second);
	if(iRes != SUCCESS) {
		if(iRes == NOFREE_THREAD)
			LOG(REQUIRED, "no free work threads now ...");
		else
			LOG(ERROR, "put task error.");
		if(mit->second)  delete mit->second;
		m_mpSocketTask.erase(mit->first);
		return false;
	}
	return true;
}

bool CUserQueryServer::BdxServerGetTask(CTcpSocket* pclSock)
{
	pclSock->TcpSetNoblock();
	if(!m_clEpoll.BdxEpollCtr(EPOLL_CTL_ADD, EPOLLIN, pclSock->TcpGetSockfd())) {
		LOG(ERROR, "AdEpollCtr error.");
		return false;
	}

	CUserQueryTask* pclTask = NULL;
	if(strServerName == "query") {
		pclTask = new CTaskMain(pclSock);
	}
    else {
		LOG(ERROR, "Unknown server type %s.", strServerName.c_str());
		return false;
	}

	if(!pclTask) {
		LOG(ERROR, "New  Task error.");
		m_clEpoll.BdxEpollCtr(EPOLL_CTL_DEL, EPOLLIN, pclSock->TcpGetSockfd());
		return false;
	}

	m_mpSocketTask[pclSock->TcpGetSockfd()] = pclTask;
	return true;
}

bool CUserQueryServer::BdxServerGetFreeTask()
{
	TASKSTATUS_S stTaskStatus;
	std::map<int, CUserQueryTask*>::iterator mit;
	for(u_int i = 0; i < m_uiWorkThreadNum; ++i) {

		while(!CUserQueryWorkThreads::m_vecTaskStauts[i].empty()) {

			stTaskStatus = CUserQueryWorkThreads::m_vecTaskStauts[i].front();
			CUserQueryWorkThreads::m_vecTaskStauts[i].pop();
			mit = m_mpSocketTask.find(stTaskStatus.m_iSockFd);
			//printf("stTaskStatus.m_iSockFd=%d\n",stTaskStatus.m_iSockFd);
			if(mit == m_mpSocketTask.end()) {
				LOG(REQUIRED, "can not find task  memory can not free !!!!!.");
				continue;
			}

			if(stTaskStatus.m_bStatus) {

				if(!m_clEpoll.BdxEpollCtr(EPOLL_CTL_ADD, EPOLLIN, stTaskStatus.m_iSockFd)) {
					LOG(ERROR, "BdxEpollCtr error.");
					if(mit->second)  delete mit->second;
					m_mpSocketTask.erase(mit->first);
					continue;
				}

			} else {
				if(mit->second)  delete mit->second;
				m_mpSocketTask.erase(mit->first);
				continue;
			}
		}
	}
	return true;
}
