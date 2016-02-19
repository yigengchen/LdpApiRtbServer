/*

 */

#include "UserQueryWorkThreads.h"
#include <stdlib.h>
#include <stdio.h>

extern CLog *gp_log;

std::vector<TASKQUE_S* > CUserQueryWorkThreads::m_vecTaskBuf;
std::vector<std::queue<TASKSTATUS_S> > CUserQueryWorkThreads::m_vecTaskStauts;
std::vector<UESRQUERYRPORT_S> CUserQueryWorkThreads::m_vecReport;
std::vector< std::queue< HIVELOCALLOG_S > > CUserQueryWorkThreads::m_vecHiveLog;


u_int CUserQueryWorkThreads::m_uiCount = 0;
extern pthread_rwlock_t p_rwlock;
extern  pthread_rwlockattr_t p_rwlock_attr;


u_int CUserQueryWorkThreads::m_uiPassCtr;
CMutexLock CUserQueryWorkThreads::m_PassCtrLock;


CUserQueryWorkThreads::CUserQueryWorkThreads(const IPPORT_S& stRedisServer,const IPPORT_S& stRedisServer2) {
	// TODO Auto-generated constructor stub

	if(!AdWorkInit(stRedisServer,stRedisServer2)) {
		throw std::runtime_error("AdWorkInit error.");
	}

/*	m_pclTairClient = new tair::tair_client_api;
	if(!m_pclTairClient->startup(stTairServer.m_strTairMasterServer.c_str(), stTairServer.m_strTairBakServer.c_str(), stTairServer.m_strTairGroupName.c_str())){
		//throw std::runtime_error("init tair server error.");
	}
*/
//	m_uiPassCtr = 0;
}

CUserQueryWorkThreads::~CUserQueryWorkThreads() {
	// TODO Auto-generated destructor stub
	pthread_rwlock_destroy(&p_rwlock);
	pthread_rwlockattr_destroy(&p_rwlock_attr);
	if(m_vecTaskBuf[m_uiThreadId]) {
		delete m_vecTaskBuf[m_uiThreadId];
	}
}

void CUserQueryWorkThreads::Core()
{
	int iRes = 0;
	CUserQueryTask* pclTask = NULL;

	TASKSTATUS_S stTaskStatus;
	while(true) {
		m_vecTaskBuf[m_uiThreadId]->m_clLock.MutexLock();
		while (m_vecTaskBuf[m_uiThreadId]->m_queUserQueryTask.empty()) {
			m_vecTaskBuf[m_uiThreadId]->m_clLock.MutexWait();
		}
		pclTask = m_vecTaskBuf[m_uiThreadId]->m_queUserQueryTask.front();
		m_vecTaskBuf[m_uiThreadId]->m_queUserQueryTask.pop();

		pclTask->BdxSetDataRedis(m_uiThreadId,m_pDataRedis,m_pGoodsRedis);

		BDXREQUEST_S stRequestInfo;
		BDXRESPONSE_S stResponseInfo;
		iRes = pclTask->BdxRunTask(stRequestInfo, stResponseInfo);

		stTaskStatus.m_iSockFd = pclTask->BdxGetTaskFd();
		if(iRes != SUCCESS) {
			LOG(DEBUG, "[thread: %d]BdxRunTask error [return : %d].", m_uiThreadId, iRes);
			stTaskStatus.m_bStatus = false;
			pclTask = NULL;
		} else {
			stTaskStatus.m_bStatus = true;
		}

		if(iRes == LINKERROR || iRes == PROTOERROR) {//socket link break off
			LOG(WARNING, "[thread: %d]Socket link break off[return: %d].", m_uiThreadId, iRes);
		}
		m_vecTaskStauts[m_uiThreadId].push(stTaskStatus);
		m_vecTaskBuf[m_uiThreadId]->m_clLock.MutexUnlock();
		m_vecTaskBuf[m_uiThreadId]->m_clLock.MutexNotify();
	}
}

bool CUserQueryWorkThreads::AdWorkInit(const IPPORT_S& stRedisServer,const IPPORT_S& stRedisServer2)
{
	m_uiThreadId = m_uiCount++;
	m_pDataRedis = new CDataAdapter;

	//m_pGoodsRedis = new CDataAdapter;
	/*
	m_pUdpSocket = new CUdpSocket;
		
	if(m_pUdpSocket == NULL) {
		LOG(ERROR, "[thread: %d]new CEmUdpSocket error.", m_uiThreadId);
		return false;
	}
	if(!m_pUdpSocket->UdpInitClient(stRedisServer.m_pchIp, stRedisServer.m_uiPort)) {
		LOG(ERROR, "[thread: %d]UdpInitClient error.", m_uiThreadId);
		return false;
	}
	*/
	char chCount[10];
	char chPort [10];
	char chIdx  [10];
    
	sprintf(chCount,"%d",stRedisServer.m_count);
	sprintf(chPort,"%d",stRedisServer.m_uiPort);
	sprintf(chIdx,"%d",stRedisServer.m_idx);

	string strServerList = string(chCount)+";"+string(stRedisServer.m_pchIp)+":"+string(chPort)+","+string(chIdx)+";";
	m_pDataRedis->Init(strServerList.c_str());
  	//printf("Redis Server=%s\n",strServerList2.c_str());

	sprintf(chCount,"%d",stRedisServer2.m_count);
	sprintf(chPort,"%d",stRedisServer2.m_uiPort);
	sprintf(chIdx,"%d",stRedisServer2.m_idx);

	string strServerList2 = string(chCount)+";"+string(stRedisServer2.m_pchIp)+":"+string(chPort)+","+string(chIdx)+";";
	printf("==============redis info==============\n");


	//m_pGoodsRedis->Init(strServerList2.c_str());

	TASKQUE_S* pstTaskQue = new TASKQUE_S;
	if(!pstTaskQue) {
		LOG(ERROR, "[thread: %d]new TASKQUE_S error.", m_uiThreadId);
		return false;
	}
	m_vecTaskBuf.push_back(pstTaskQue);

	UESRQUERYRPORT_S stReport;
	//memset(&stReport, 0, sizeof(UESRQUERYRPORT_S));
	m_vecReport.push_back(stReport);

	std::queue<HIVELOCALLOG_S> stHiveLog;
	m_vecHiveLog.push_back(stHiveLog);
	

	std::queue<TASKSTATUS_S> queTaskStatus;
	m_vecTaskStauts.push_back(queTaskStatus);
	return true;
}




