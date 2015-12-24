/*

 */

#include "Lock.h"

#include <stdexcept>
#include <errno.h>

CLock::CLock() {
	// TODO Auto-generated constructor stub

	if( 0 != pthread_cond_init(&m_stCond, 0)) {
		throw std::runtime_error("pthread_cond_init error.");
	}

}

CLock::~CLock() {
	// TODO Auto-generated destructor stub
	LockDestroyCond();
}

CMutexLock::CMutexLock()
{
	m_pstAttr = new pthread_mutexattr_t;
	if(!MutexInitAttr()) {
		throw std::runtime_error("MutexInitAttr error.");
	}

	m_pLock = new pthread_mutex_t;
	pthread_mutex_init((pthread_mutex_t*)m_pLock, (pthread_mutexattr_t*)m_pstAttr);
}

CMutexLock::~CMutexLock()
{
	MutexDestroyAttr();
	MUtexDestroyLock();
	if(m_pLock) {
		delete (pthread_mutex_t*)m_pLock;
		m_pLock = NULL;
	}

	if(m_pstAttr) {
		delete (pthread_mutexattr_t*)m_pstAttr;
		m_pstAttr = NULL;
	}
}

bool CMutexLock::MutexInitLock()
{
	return !pthread_mutex_init((pthread_mutex_t*)m_pLock, (pthread_mutexattr_t*)m_pstAttr);
}

void CMutexLock::MutexLock()
{
	if(!m_pLock) {
		fprintf(stderr, "pLock is NULL.\n");
		return;
	}
	pthread_mutex_lock((pthread_mutex_t*)m_pLock);
}

void CMutexLock::MutexUnlock()
{
	if(!m_pLock) {
		fprintf(stderr, "pLock is NULL.\n");
		return;
	}
	pthread_mutex_unlock((pthread_mutex_t*)m_pLock);
}

void CMutexLock::MutexWait()
{
	if(!m_pLock) {
		fprintf(stderr, "pLock is NULL.\n");
		return;
	}
	pthread_cond_wait(&m_stCond, (pthread_mutex_t*)CLock::m_pLock);
}

void CMutexLock::MutexNotify()
{
	pthread_cond_signal(&m_stCond);
}

bool CMutexLock::MutexInitAttr()
{
	if(!m_pstAttr) {
		fprintf(stderr, "m_pstAttr is NULL.\n");
		return false;
	}
	return !pthread_mutexattr_init((pthread_mutexattr_t*)m_pstAttr);
}


bool CMutexLock::MutexDestroyAttr()
{
	if(!m_pstAttr) {
		fprintf(stderr, "m_pstAttr is NULL.\n");
		return false;
	}
	return !pthread_mutexattr_destroy((pthread_mutexattr_t*)m_pstAttr);
}

bool CMutexLock::MutexSetProcessShare()
{
	if(!m_pstAttr) {
		fprintf(stderr, "m_pstAttr is NULL.\n");
		return false;
	}
	return !pthread_mutexattr_setpshared((pthread_mutexattr_t*)m_pstAttr, PTHREAD_PROCESS_SHARED);
}

bool CMutexLock::MUtexDestroyLock()
{
	if(!m_pLock) {
		fprintf(stderr, "pLock is NULL.\n");
		return false;
	}
	return !pthread_mutex_destroy((pthread_mutex_t*)m_pLock);
}

/*
 * 1 busy 0 success   other error.
 */
int CMutexLock::MutexTryLock()
{
	if(!m_pLock) {
		fprintf(stderr, "pLock is NULL.\n");
		return -1;
	}
	int iRes = pthread_mutex_trylock((pthread_mutex_t*)m_pLock);
	if(iRes == EBUSY) {
		return 1;
	}

	return iRes;
}
