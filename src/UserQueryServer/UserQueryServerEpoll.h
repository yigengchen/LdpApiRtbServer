/*
 * UserQueryServerEpoll.h

 */

#ifndef __USERQUERYSERVEREPOLL_H__
#define __USERQUERYSERVEREPOLL_H__

#include <sys/epoll.h>
#include <stdio.h>
#include <string.h>

#define	EVENT			256
#define	EPOLL			10000

class CUserQueryServerEpoll {
public:
	CUserQueryServerEpoll();
	virtual ~CUserQueryServerEpoll();

	bool BdxEpollCtr(const int iVerb, const EPOLL_EVENTS enEvent, const int iSockfd);
	int  BdxEpollWait(struct epoll_event* pstEvent, int iTimeOut = -1);
	bool BdxEpollClose();
protected:
	bool BdxEpollInit();
private:
	int m_iEpoll;
};

#endif /* __USERQUERYSERVEREPOLL_H__ */
