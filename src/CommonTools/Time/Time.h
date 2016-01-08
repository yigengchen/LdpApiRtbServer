/********************************************************/

/********************************************************/

#ifndef __TIME_H__
#define	__TIME_H__

//#include "../../UserQueryStruct/UserQueryStruct.h"

#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
 
class CTime
{
public:
	CTime();
	virtual ~CTime();

	void TimeOn();
	void TimeOff();
	void TimeDisplay();
	u_int TimeGetSeconds(){return m_uiSeconds;};
	u_int TimeGetMicSeconds(){return m_uiMicSeconds;};
	struct timeval TimeGetTimeval(){return m_stOn;};

private:
	struct timeval m_stOn;
	struct timeval m_stOff;
	u_int m_uiSeconds;
	u_int m_uiMicSeconds;
};

#endif /* __TIME_H__ */
