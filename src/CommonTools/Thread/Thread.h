/*

 */

#ifndef __THREAD_H__
#define __THREAD_H__

#include <pthread.h>
#include <vector>
#include <string.h>
#include <sys/types.h>
#include <stdio.h>

template <class T>
void* StartRoutine(void *pvPrms)
{
    ((T *)(pvPrms))->Core();
    return pvPrms;
}

class CThreads{
public:
	CThreads();
	virtual ~CThreads();

	void SetRoutine(void* (fpFun)(void*));
	//void SetParameter(void* pvPara);
	pthread_t CreateThead(void* pvPara);
	void ThreadJionALL();
	void ThreadJion();
	int ThreadAttrSetDetach();
	int ThreadAttrInit();
	void ThreadAttrSet(void* pAttr);
	int ThreadDestroy();
	int ThreadDestroyALL();

protected:

private:
	pthread_t m_iID;
	pthread_attr_t m_stAttr;
	std::vector<pthread_t> m_vecID;
	void* (*m_fpFun)(void *);
};




#endif /* __THREAD_H__ */
