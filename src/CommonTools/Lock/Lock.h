/*
 * Lock.h
 *
 *  Created on: 2012-10-23
 *      Author: root
 */

#ifndef __LOCK_H__
#define __LOCK_H__

#include <pthread.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

class CLock
{
public:
	CLock();
	virtual ~CLock();

	virtual void* LockGetLock() { return m_pLock; }
	virtual bool LockDestroyCond(){return !pthread_cond_destroy(&m_stCond);}

protected:
	void* m_pLock;
	void* m_pstAttr;
	pthread_cond_t m_stCond;
private:
};

class CMutexLock : public CLock
{
public:
	CMutexLock();
	virtual ~CMutexLock();

	bool MutexInitLock();
	void MutexLock();
	void MutexUnlock();
	void MutexWait();
	void MutexNotify();
	bool MutexInitAttr();
	bool MutexDestroyAttr();
	bool MutexSetProcessShare();
	bool MUtexDestroyLock();
	int MutexTryLock();
protected:
private:


};

class CRWLock
{
public:
	CRWLock();
	virtual ~CRWLock();

protected:
private:
};


class MutexLock {
public:
	explicit MutexLock(pthread_mutex_t *lock) :lock_(lock){
		pthread_mutex_lock(lock_);
	}
	~MutexLock() {
		pthread_mutex_unlock(lock_);
	}
private:
	MutexLock(const MutexLock&);
	MutexLock& operator=(const MutexLock&);
private:
	pthread_mutex_t 	*lock_;
};

#endif /* __LOCK_H__ */
