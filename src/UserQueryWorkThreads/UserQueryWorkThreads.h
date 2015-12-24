/*
 * UserQueryWorkThreads.h
 *
 */

#ifndef __WORKTHREADS_H__
#define __WORKTHREADS_H__

#include "../UserQueryStruct/UserQueryStruct.h"
#include "../UserQueryTask/UserQueryTask.h"

class CUserQueryWorkThreads {
public:
	CUserQueryWorkThreads(const IPPORT_S& stRedisServer,const IPPORT_S& stRedisServer2);
	virtual ~CUserQueryWorkThreads();

	static std::vector<TASKQUE_S* > m_vecTaskBuf;
	static std::vector<std::queue<TASKSTATUS_S> > m_vecTaskStauts;
	static std::vector<UESRQUERYRPORT_S > m_vecReport;
    static std::vector<std::queue<HIVELOCALLOG_S> > m_vecHiveLog;

	static int AdWorkAdd(){
		m_PassCtrLock.MutexLock();
		int iTmp = ++m_uiPassCtr;
		m_PassCtrLock.MutexUnlock();
		return iTmp;
	}
	static void AdWorkSet(){
		m_PassCtrLock.MutexLock();
		m_uiPassCtr = 0;
		m_PassCtrLock.MutexUnlock();
	}

	void Core();
protected:
	bool AdWorkInit(const IPPORT_S& stRedisServer,const IPPORT_S& stRedisServer2);
	
private:

	static u_int m_uiCount;
	u_int m_uiThreadId;
	CUdpSocket* m_pUdpSocket;
	static u_int m_uiPassCtr;
	static CMutexLock m_PassCtrLock;
	CDataAdapter *m_pDataRedis;
	CDataAdapter *m_pGoodsRedis;
	//std::vector m_vecUrlApis;
};

#endif /* __WORKTHREADS_H__ */
