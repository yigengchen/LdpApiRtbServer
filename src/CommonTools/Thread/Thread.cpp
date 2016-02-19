/*

 */
#include "Thread.h"
#include <stdexcept>
#include <errno.h>

CThreads::CThreads()
{
	if (0 != pthread_attr_init(&m_stAttr)) {
		throw std::runtime_error("pthread_attr_init error.");
	}
	//m_pvPara = NULL;
}

CThreads::~CThreads()
{

}

void CThreads::SetRoutine(void* (fpFun)(void*))
{
	m_fpFun = fpFun;
}

//void CEmThreads::SetParameter(void* pvPara)
//{
//	m_pvPara = pvPara;
//}


pthread_t CThreads::CreateThead(void* pvPara)
{
	if (0 != pthread_create(&m_iID, &m_stAttr, m_fpFun, (void*)pvPara)) {
		fprintf(stderr, "Create new thread error [%s].\n", strerror(errno));
		return 0;
	}
	m_vecID.push_back(m_iID);
	m_fpFun = NULL;
	ThreadAttrInit();
	return m_iID;
}

void CThreads::ThreadJionALL()
{
	for(u_int i = 0; i < m_vecID.size(); ++i) {
		pthread_join(m_vecID[i], NULL);
	}
}

void CThreads::ThreadJion()
{
	pthread_join(m_iID, NULL);
}

int CThreads::ThreadAttrSetDetach()
{
	if (0 != pthread_attr_setdetachstate(&m_stAttr, PTHREAD_CREATE_DETACHED)) {
		fprintf(stderr, "%s\n", strerror(errno));
		return -1;
	}
	return 0;
}

int CThreads::ThreadAttrInit()
{
	if (0 != pthread_attr_init(&m_stAttr)) {
		fprintf(stderr, "%s\n", strerror(errno));
		return -1;
	}

	return 0;
}

void CThreads::ThreadAttrSet(void* pAttr)
{
	memcpy((void*)&m_stAttr, pAttr, sizeof(pthread_attr_t));

	return;
}

int CThreads::ThreadDestroy()
{
	if (0 != pthread_cancel(m_iID)) {
		fprintf(stderr, "%s\n", strerror(errno));
		return -1;
	}

	return 0;
}

int CThreads::ThreadDestroyALL()
{
	for(u_int i = 0; i < m_vecID.size(); ++i) {

		if (0 != pthread_cancel(m_vecID[i])) {
			fprintf(stderr, "%s\n", strerror(errno));
			return -1;
		}
	}
	return 0;
}



