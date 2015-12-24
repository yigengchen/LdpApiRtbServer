/*
 * UdpSocket.h
 
 */

#ifndef __UDPSOCKET_H__
#define __UDPSOCKET_H__

#include <string>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include <errno.h>
#include <strings.h>
#include <stdio.h>
#include <string.h>

class CUdpSocket {
public:
	CUdpSocket(const u_int uiTimeout = 30);
	virtual ~CUdpSocket();

	void UdpSetTimeout(const u_int uiTimeout);
	bool UdpInitClient(const std::string& strIp, const in_port_t in_portPort);
	bool UdpInitServer(const in_port_t in_portPort, const std::string& strIp = "");
	void UdpClose();
	bool UdpNewSocket();
	int  UdpGetSockfd();
	u_int UdpGetTimeout();
	int UdpRead(void *const pvRecvBuf, size_t sizeRecvLen);
	int UdpReadAll(void *const pvRecvBuf, size_t sizeRecvLen);
	bool UdpWrite(const void *pvSendBuf,size_t sizeSendLen);
	bool UdpBindNewSocket();
	void UdpSetAddr(void* pAddr);
	void UdpPrintAddr();
	void UdpInitWeight();
	void UdpInitIndexDevice();
	void UdpAddWeight();
	void UdpAddIndexDevice();
	int  UdpGetWeight();
	int  UdpGetIndexDevice();
	int  UdpPrintf();
	void  UdpSetTimeOut(u_int ui_Timeout);
	struct sockaddr_in m_stSockaddr_local;
	
private:
	int m_iSockfd;
	u_int m_uiTimeout;
	int m_iWeight;
	int m_iIndexDevice;
	struct sockaddr_in m_stSockaddr;
	//pthread_mutex_t m_mutex;
	pthread_rwlock_t a;
	socklen_t m_socklen;
};

#endif /* __UDPSOCKET_H__ */
