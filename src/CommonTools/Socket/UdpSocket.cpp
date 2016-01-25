/*
 * UdpSocket.cpp
 
 */
#include <stdexcept>

#include "UdpSocket.h"
#include <unistd.h>

#include <sys/socket.h>


CUdpSocket::CUdpSocket(const u_int uiTimeout) : m_uiTimeout(uiTimeout)
{
	m_iSockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(m_iSockfd < 0) {
		throw std::runtime_error("get socket fd error.");
	}
	m_socklen = sizeof (struct sockaddr);
}

CUdpSocket::~CUdpSocket() {
	if(m_iSockfd > 0) {
		::close(m_iSockfd);
	}
}

/*
 *
 */
void CUdpSocket::UdpSetTimeout(const u_int uiTimeout)
{
	m_uiTimeout = uiTimeout;
}



void CUdpSocket::UdpSetAddr(void* pAddr)
{
	//m_stSockaddr.sin_family = AF_INET;
	memcpy(&m_stSockaddr, pAddr, sizeof(struct sockaddr_in));
	//m_stSockaddr.sin_addr.s_addr=((struct sockaddr_in*)& pAddr)->sin_addr.s_addr;
	//m_stSockaddr.sin_port=((struct sockaddr_in*)& pAddr)->sin_port;
	//printf("m_stSockaddr sin_addr==%s\n",inet_ntoa(m_stSockaddr.sin_addr));
	//printf("m_stSockaddr sin_port==%d\n",ntohs(m_stSockaddr.sin_port));

}


void CUdpSocket::UdpPrintAddr()
{

	//m_stSockaddr.sin_addr.s_addr=((struct sockaddr_in*)& pAddr)->sin_addr.s_addr;
	//m_stSockaddr.sin_port=((struct sockaddr_in*)& pAddr)->sin_port;
	printf("m_stSockaddr sin_addr==%s\n",inet_ntoa(m_stSockaddr.sin_addr));
	printf("m_stSockaddr sin_port==%d\n",ntohs(m_stSockaddr.sin_port));

}



/*
 *
 */ 
bool CUdpSocket::UdpInitClient(const std::string& strIp, const in_port_t in_portPort)
{
	if(strIp.empty()) {
		return false;
	}
    bzero((char*)&m_stSockaddr, sizeof(m_stSockaddr));
    m_stSockaddr.sin_family = AF_INET;
    m_stSockaddr.sin_addr.s_addr = inet_addr(strIp.c_str());
    m_stSockaddr.sin_port = htons(in_portPort);
	return true;
}

/*
 *
 */
bool CUdpSocket::UdpInitServer(const in_port_t in_portPort, const std::string& strIp)
{
    bzero((char*)&m_stSockaddr, sizeof(m_stSockaddr));
    m_stSockaddr.sin_family = AF_INET;
    if(!strIp.empty()) {
    	m_stSockaddr.sin_addr.s_addr = inet_addr(strIp.c_str());
    } else {
    	m_stSockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    m_stSockaddr.sin_port = htons(in_portPort);
    if(bind(m_iSockfd, (struct sockaddr *)&m_stSockaddr, sizeof(m_stSockaddr)) == -1) {//bind error
    	return false;
    }
	return true;
}

/*
 *
 */
void CUdpSocket::UdpClose()
{
	if(m_iSockfd > 0) {
		::close(m_iSockfd);
		m_iSockfd = -1;
	}
}

/*
 *
 */
bool CUdpSocket::UdpNewSocket()
{
	if(m_iSockfd > 0) {
		::close(m_iSockfd);
		m_iSockfd = -1;
	}
	m_iSockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(m_iSockfd < 0) {
		return false;
	}
	return true;
}

/*
 *
 */
int CUdpSocket::UdpGetSockfd()
{
	return m_iSockfd;
}

/*
 *
 */
u_int CUdpSocket::UdpGetTimeout()
{
	return m_uiTimeout;
}

/*
 * error -1 timeout -2
 */
int CUdpSocket::UdpRead(void *const pvRecvBuf, size_t sizeRecvLen)
{
	if(pvRecvBuf == NULL || sizeRecvLen <= 0 || m_iSockfd == -1) {//参数有误
		return -1;
	}
	struct timeval stTimeout;
	stTimeout.tv_sec = 0;
	stTimeout.tv_usec = m_uiTimeout * 1000;

	if(::setsockopt(m_iSockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&stTimeout, sizeof(stTimeout)) == -1) {//设置读取超时 失败
		return -1;
	}
	struct sockaddr_in stSockaddr;
	
	int iRes = ::recvfrom(m_iSockfd, pvRecvBuf, sizeRecvLen, 0, (struct sockaddr *) &stSockaddr,   (socklen_t *)&m_socklen);
	switch(iRes)
	{
	case -1:
		if(errno == EAGAIN) {//超时
			errno = 0;
			return -2;
		}//else出错
		return -1;
		break;
//	case 0://socket 读取对象不存在
//		return 0;
//		break;
	default:
		if(iRes < 0)
			return -1;
		break;
	}
	return iRes;
}

/*
 *
 */
int CUdpSocket::UdpReadAll(void *const pvRecvBuf, size_t sizeRecvLen)
{
	if(pvRecvBuf == NULL || sizeRecvLen == 0) {
		return -1;
	}
	int ilen = 0;
	int ilength = 0;
	while((ilength = UdpRead(((char *)pvRecvBuf + ilen), sizeRecvLen - ilen)) > 0)
	{//读取所有的socket 数据
		ilen += ilength;//已读取的长度
		if((u_int)ilen >= sizeRecvLen) {//读取recvLen完毕
			break;
		}
	}
	if(ilength < 0) {
		return ilength;
	}
	return ilen;
}

/*
 *
 */
bool CUdpSocket::UdpWrite(const void *pvSendBuf,size_t sizeSendLen)
{
	if(!pvSendBuf || sizeSendLen <= 0) {//参数有误
		#if 0 //cyg add for test
		printf("pvSendBuf is NULL or sizeSendLen <=0, error [%s].",strerror(errno));
		if(sizeSendLen <=0)
		{
			printf("=======sizeSendLen <=0=========\n");	
		}
		if(!pvSendBuf)
		{
			printf("=======!pvSendBuf  is NULL======\n");
		}
		#endif
		return false;
	}

	#if 0 //cyg add for test
	struct sockaddr_in m_stSockaddr_local;
	int i=sizeof(struct sockaddr_in);
	connect(m_iSockfd,(struct sockaddr *)&m_stSockaddr,(socklen_t)(i));
	getsockname(m_iSockfd, (struct sockaddr *)&m_stSockaddr_local, (socklen_t*)(&i));
	//getpeername(connfd, (struct sockaddr *)&guest, &guest_len);
	printf("m_stSockaddr_local1111=%s\n",inet_ntoa(m_stSockaddr_local.sin_addr));
	printf("m_stSockaddr_local2222=%d\n",ntohs(m_stSockaddr_local.sin_port));
	printf("m_stSockaddr1111=%s\n",inet_ntoa(m_stSockaddr.sin_addr));
	printf("m_stSockaddr2222=%d\n",ntohs(m_stSockaddr.sin_port));

	#endif

	#if 0
	int iRet;
	iRet=connect(m_iSockfd,(struct sockaddr *)&m_stSockaddr,m_socklen);
	if(-1 == iRet)
	{
		printf("===connecting time out===%s\n",strerror(errno));
	}
	else
	{
		printf("m_stSockaddr_local1111=%s\n",inet_ntoa(m_stSockaddr.sin_addr));
	    printf("m_stSockaddr_local2222=%d\n",ntohs(m_stSockaddr.sin_port));
	}
	#endif
	//20130903 cyg add to get ip and port before send udp data 

	/*
	if(::connect(m_iSockfd,(struct sockaddr *)&m_stSockaddr,m_socklen)==-1)
	{
		return false;
	}
	
	::getsockname(m_iSockfd, (struct sockaddr *)&m_stSockaddr_local, (socklen_t*)(&m_socklen));

	*/
	 
	if(::sendto(m_iSockfd, pvSendBuf, sizeSendLen, 0, (struct sockaddr *) &m_stSockaddr,  m_socklen) == -1) {//发送失败
	
		return false;
	}	 

	#if 0 //cyg add for test
	struct sockaddr_in m_stSockaddr_local;	
	int j=sizeof(struct sockaddr_in);
	getsockname(m_iSockfd, (struct sockaddr *)&m_stSockaddr_local, (socklen_t*)(&j));
	//getpeername(connfd, (struct sockaddr *)&guest, &guest_len);
	printf("m_stSockaddr3333=%s\n",inet_ntoa(m_stSockaddr_local.sin_addr));
	printf("m_stSockaddr4444=%d\n",ntohs(m_stSockaddr_local.sin_port));
	#endif
	
	return true;
}

bool CUdpSocket::UdpBindNewSocket()
{
	if(!UdpNewSocket()) {
		fprintf(stderr, "%s\n", strerror(errno));
		return false;
	}

    return true;
}
void CUdpSocket::UdpInitWeight()
{
		 m_iWeight = 1;

}

void CUdpSocket::UdpInitIndexDevice()
{

		 m_iIndexDevice = 0;
}



void CUdpSocket::UdpAddWeight()
{
		 m_iWeight++;
		 
}


void CUdpSocket::UdpAddIndexDevice()
{
		
		 m_iIndexDevice++;
}

int CUdpSocket::UdpGetWeight()
{
		 return m_iWeight;
		 
}

int CUdpSocket::UdpGetIndexDevice()
{
		
		 return m_iIndexDevice;
}

int  CUdpSocket::UdpPrintf()
{
		
		 printf("m_iWeight=%d\n",m_iWeight);
		 printf("m_iIndexDevice=%d\n",m_iIndexDevice);
		 return true;
}



void CUdpSocket::UdpSetTimeOut(u_int ui_Timeout)
{
		
		 m_uiTimeout = ui_Timeout;

}



