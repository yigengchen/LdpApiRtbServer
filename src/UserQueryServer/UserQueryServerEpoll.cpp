/*
 * UserQueryServerEpoll.cpp
 
 */

#include "UserQueryServerEpoll.h"
#include <unistd.h>
#include <errno.h>
#include <stdexcept>

CUserQueryServerEpoll::CUserQueryServerEpoll() {
	// TODO Auto-generated constructor stub
	m_iEpoll = -1;
	
	if(!BdxEpollInit()) {
		throw std::runtime_error("Init Epoll error.");
	}
	
}

CUserQueryServerEpoll::~CUserQueryServerEpoll() {
	// TODO Auto-generated destructor stub
}

bool CUserQueryServerEpoll::BdxEpollCtr(const int iVerb, const EPOLL_EVENTS enEvent, const int iSockfd)
{
	struct epoll_event stSocket;
	stSocket.events = enEvent;
	stSocket.data.fd = iSockfd;
	if(-1 == epoll_ctl(m_iEpoll, iVerb, stSocket.data.fd, &stSocket)) {
		fprintf(stderr, "epoll_ctl [error: %s]\n", strerror(errno));
		return false;
	}
	return true;
}

int CUserQueryServerEpoll::BdxEpollWait(struct epoll_event* pstEvent, int iTimeOut)
{
	return epoll_wait(m_iEpoll, pstEvent, EVENT, iTimeOut);
}

bool CUserQueryServerEpoll::BdxEpollClose()
{
	if(m_iEpoll != -1) {
		::close(m_iEpoll);
		m_iEpoll = -1;
	}
	return true;
}

bool CUserQueryServerEpoll::BdxEpollInit()
{
	//struct epoll_event stEpoll;

	m_iEpoll = epoll_create(EPOLL);
	if (m_iEpoll < 0) {
		fprintf(stderr, "%s\n", strerror(errno));
		return false;
	}
	return true;
}
