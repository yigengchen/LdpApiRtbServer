/*
 * TcpSocket.h
 */

#ifndef __TCP_SOCKET_H__
#define __TCP_SOCKET_H__

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/ioctl.h>
//#include <asm-generic/errno-base.h>
#include <errno.h>
#include <string>
#include <strings.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/err.h>

class CTcpSocket
{
public:
	CTcpSocket(const in_port_t in_portPort = 0, const std::string& strIp = "", const u_int uiTimeout = 300000);
	virtual ~CTcpSocket() throw();

	std::string TcpGetSip();
	int TcpGetSockfd();
	in_addr_t TcpGetIntIp();
	std::string TcpGetServerIp();
	in_port_t TcpGetPort();
	bool TcpBind(const in_port_t in_portPort = 0, const std::string& strIp = "");
	bool TcpListen(const int iReqNum = 1/*SOMAXCONN*/);
	CTcpSocket* TcpAccept();
	int TcpTimeoutAccept(CTcpSocket*& pEmSocket);
	int TcpConnect(const in_port_t in_portPort = 0, const std::string& strIp = "");
	int TcpReconnect();
	int TcpRead(void *const pvRecvBuf, size_t sizeRecvLen);
	int TcpReadAll(void *const pvRecvBuf, size_t sizeRecvLen);
	bool TcpWrite(const void *pvSendBuf,size_t sizeSendLen);
	bool TcpClose();
	int  EncodingBase64(char * pInput, char * pOutput);
	int  TcpSendEmail(char * smtpServer,int port,char* username,char * password,char* from,char * to,char * subject,char * body);
	bool TcpCloseFinal();
	bool TcpNewSocket();
	void TcpSetIp(const std::string& strIp);
	void TcpSetPort(const in_port_t in_portPort);
	void TcpSetTimeout(const u_int uiTimeout);
	u_int TcpGetTimeOut();
	bool TcpGetConnectStatus();
	bool TcpIsConnect();
	bool TcpSetNoblock();
	void TcpSslInitParams();
	int TcpSslInitEnv();
	int TcpSslReadLen(void* buf, int nBytes);
	int TcpSslWriteLen(void* buf, int nBytes);
	void  TcpSslDestroy();
	bool TcpSslConnect();
	int TcpGetSocket();
protected:
	CTcpSocket(const int iSockfd, const u_int uiTimeout);
private:
	int m_iSockfd;
	in_port_t m_in_portPort;
	std::string m_strIp;
	u_int m_uiTimeout;
	bool m_bConnect;
	SSL *ssl;
	SSL_CTX *ssl_ctx;
	SSL_METHOD  *ssl_method;
	X509 *server_cert;

};

#endif /* __TCP_SOCKET_H__ */
