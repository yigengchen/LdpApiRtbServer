/*
 * TcpSocket.cpp
 *

 */
#include <stdexcept>

#include "TcpSocket.h"
#include <errno.h>


const char Base64ValTab[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
#define AVal(x) Base64ValTab[x]

extern std::string strSslKeyPath;
extern std::string strSslKeyPassWord;
CTcpSocket::CTcpSocket(const int iSockfd, const u_int uiTimeout) : m_iSockfd(iSockfd), m_uiTimeout(uiTimeout)
{
	m_in_portPort = 0;
	m_strIp.clear();
	m_bConnect = true;
}

/*
 * 构造
 */
CTcpSocket::CTcpSocket(const in_port_t in_portPort, const std::string& strIp, const u_int uiTimeout)
 : m_in_portPort(in_portPort), m_strIp(strIp), m_uiTimeout(uiTimeout)
{	
	m_iSockfd = socket(AF_INET, SOCK_STREAM, 0);
	printf("m_iSockfd=%d is created....\n",m_iSockfd);
	if (m_iSockfd == -1) {
		throw std::runtime_error("get socketfd error.");
	}
	m_bConnect = false;
}

/*
 * 析构
 */
CTcpSocket::~CTcpSocket() throw()
{
	struct linger so_linger;
	so_linger.l_onoff = true;
	so_linger.l_linger = 0;
	setsockopt(m_iSockfd,SOL_SOCKET,SO_LINGER,&so_linger,sizeof(so_linger));
	if(m_iSockfd != -1){
		::close(m_iSockfd);
		m_iSockfd = -1;
	}
	
}

/*
 * 获取socket ip
 * 返回值：点格式ip 错误空
 */
std::string CTcpSocket::TcpGetSip()
{
	std::string strIp;
	u_int uiSinSize = sizeof(struct sockaddr_in);
	struct sockaddr_in stSockaddr;
	if (m_iSockfd > 0 && getpeername(m_iSockfd, (struct sockaddr *)&stSockaddr, &uiSinSize) == 0) {//获取socket 参数结构
		strIp = inet_ntoa(stSockaddr.sin_addr);
	}
	return strIp;
}

/*
 * 获取socket描述符
 * 返回值：socket 描述符
 */
int CTcpSocket::TcpGetSockfd()
{
	return m_iSockfd;
}


u_int CTcpSocket::TcpGetTimeOut()
{
	return m_uiTimeout;
}

bool CTcpSocket::TcpGetConnectStatus()
{
	return m_bConnect;
}

/*
 *获取整型ip
 *返回值：整型ip  错误0
 */
in_addr_t CTcpSocket::TcpGetIntIp()
{
	in_addr_t in_addrIp = 0;
	u_int uiSinSize = sizeof(struct sockaddr_in);
	struct sockaddr_in stSockaddr;
	if (m_iSockfd > 0 && getpeername(m_iSockfd, (struct sockaddr *)&stSockaddr, &uiSinSize) == 0) {
		in_addrIp = stSockaddr.sin_addr.s_addr;
		
	}
	return in_addrIp;
}

std::string CTcpSocket::TcpGetServerIp()
{
	//in_addr_t in_addrServerIp = 0;
	std::string strIp;
	u_int uiSinSize = sizeof(struct sockaddr_in);
	struct sockaddr_in stSockaddr;
	if (m_iSockfd > 0 && getsockname(m_iSockfd, (struct sockaddr *)&stSockaddr, &uiSinSize) == 0) {
		//in_addrServerIp = stSockaddr.sin_addr.s_addr;
		strIp = inet_ntoa(stSockaddr.sin_addr);
	}
	return strIp;
}


/*
 * get socket port
 * Return-Value：success local Sequence port else return 0
 */
in_port_t CTcpSocket::TcpGetPort()
{
	in_port_t in_portPort = 0;
	u_int uiSinSize = sizeof(struct sockaddr_in);
	struct sockaddr_in stSockaddr;
	if (m_iSockfd > 0 && getpeername(m_iSockfd, (struct sockaddr *)&stSockaddr, &uiSinSize) == 0) {
		in_portPort = ntohs(stSockaddr.sin_port);
	}
	return in_portPort;
}


/*
 * socket 绑定ip 端口
 * 参数：
 * port: 连接端口
 * ip:绑定ip  默认为空
 * 返回值：成功true  失败false
 */
bool CTcpSocket::TcpBind(const in_port_t in_portPort, const std::string& strIp)
{
	std::string strTmpIp;
	in_port_t in_portTmpPort = 0;
	if (strIp.empty()) {//服务器ip为空
		strTmpIp = m_strIp;
	} else {
		strTmpIp = strIp;
	}
	if (in_portPort == 0) {
		in_portTmpPort = m_in_portPort;
	} else {
		in_portTmpPort = in_portPort;
	}
	struct sockaddr_in stServerAddr;
	stServerAddr.sin_family = AF_INET;
	stServerAddr.sin_port = htons(in_portTmpPort);
	if (strTmpIp.empty()) {//ip为空
		stServerAddr.sin_addr.s_addr = INADDR_ANY;//任意地址监听
	} else {//ip非空
		inet_aton(strTmpIp.c_str(),&stServerAddr.sin_addr);//指定ip段监听
	}
	if (::bind(m_iSockfd,(struct sockaddr *)&stServerAddr,sizeof(struct sockaddr)) == -1) {//绑定失败
		return false;
	}
	return true;
}

/*
 * socket 监听
 * 参数：
 * reqNum: 监听队列最大长度， 默认由系统控制
 * 返回值:成功true 失败false
 */
bool CTcpSocket::TcpListen(const int iReqNum)
{
	if (::listen(m_iSockfd, iReqNum) < 0) {
		return false;
	}
	return true;
}

/*
 * server 接收client 连接
 * 参数：
 * 返回值：成功：客户端socket 指针, 失败：空
 */
CTcpSocket* CTcpSocket::TcpAccept()
{
	if (m_iSockfd < 0) {//描述符错误
		return false;
	}
	struct sockaddr_in stClientAddr;
	u_int uiSinSize = sizeof(struct sockaddr_in);
	int iclientfd = -1;
	if((iclientfd = ::accept(m_iSockfd, (struct sockaddr *)&stClientAddr, &uiSinSize)) < 0) {//accept失败
		return false;
	}
	return new CTcpSocket(iclientfd, m_uiTimeout); 
}

/*
 * 带有超时的监听
 * 参数：psocket 保存客户端的socket对象指针
 * 返回值：－1 出错  0 超时  1成功
 */
int CTcpSocket::TcpTimeoutAccept(CTcpSocket*& pEmSocket)
{
	struct sockaddr_in stClientAddr;
	u_int uiSinSize = sizeof(struct sockaddr_in);
	struct timeval stTimeval;
	fd_set fdReadfds;
	stTimeval.tv_sec = m_uiTimeout;
	stTimeval.tv_usec = 0;
	FD_ZERO(&fdReadfds);
	FD_SET(m_iSockfd, &fdReadfds);
	while(true) {
		switch(select(m_iSockfd + 1, &fdReadfds, 0, NULL, &stTimeval))
		{
		case -1://select出错
			return -1;
			break;
		case 0://监听超时
			return 0;
			break;
		default:
			if (FD_ISSET(m_iSockfd, &fdReadfds)) {
				int iClientfd = -1;
				if ((iClientfd = ::accept(m_iSockfd, (struct sockaddr *) &stClientAddr, &uiSinSize)) == -1) {
					return -1;
				}
				pEmSocket = new CTcpSocket(iClientfd, m_uiTimeout);//创建客户端对象
				return 1;
			}
			//LOG(ERROR, "which readfd is reading?!");
			return -1;
			break;
		}
	}
	return -1;
}

/*
 * socket连接服务器
 * 参数： 端口  ip
 * 返回值：-1 出错  -2 超时  -3 server未开 0 成功
 */
int CTcpSocket::TcpConnect(const in_port_t in_portPort, const std::string& strIp)
{
 
	if(m_iSockfd == -1) {//描述符错误
		return -1;
	}
	struct sockaddr_in stAddress;
	stAddress.sin_family = AF_INET;
	inet_aton(m_strIp.c_str(), &stAddress.sin_addr);
	stAddress.sin_port = htons(m_in_portPort);
	struct timeval stTimeval;
	//stTimeval.tv_sec = m_uiTimeout;
	//stTimeval.tv_usec = 0;
	
	stTimeval.tv_sec = 1;
	stTimeval.tv_usec = m_uiTimeout;
	if (-1 == setsockopt(m_iSockfd, SOL_SOCKET, SO_SNDTIMEO, &stTimeval, sizeof(stTimeval))) {
		return -1;
	}

	if (-1 == setsockopt(m_iSockfd, SOL_SOCKET, SO_RCVTIMEO, &stTimeval, sizeof(stTimeval))) {
		return -1;
	}

	if( ::connect(m_iSockfd, (struct sockaddr *)&stAddress, sizeof(stAddress)) < 0) {

		return -1;
	}
	m_bConnect = true;
	return 0;
}

/*
 * 重连socket
 * 参数：
 * sockfd : 重连句柄
 * ip:server　ip
 * port: server Port
 * 返回值：成功true 失败false
 */
int CTcpSocket::TcpReconnect()
{
	if(m_bConnect) {
		::close(m_iSockfd);//关掉原有的连接
		m_iSockfd = -1;
		m_bConnect = false;
		m_iSockfd = socket(AF_INET,SOCK_STREAM,0);
		if(m_iSockfd < 0) {//创建socket失败
			return -1;
		}
	}
	return TcpConnect();
}

/*
 * socket读取数据
 * 参数：
 * recvBuf:保存读取数据buf
 * recvLen:读取长度
 * 返回值：成功返回读取到的长度   失败返回-1  超时－2 读取对象不存在 0
 */

int CTcpSocket::TcpRead(void *const pvRecvBuf, size_t sizeRecvLen)
{
	if(pvRecvBuf == NULL || sizeRecvLen <= 0 || m_iSockfd == -1) {//参数有误
		return -1;
	}
	struct timeval stTimeout;
	stTimeout.tv_sec =0;// m_uiTimeout;
	stTimeout.tv_usec = m_uiTimeout;
	//int sock_buf_size=9012;

if(::setsockopt(m_iSockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&stTimeout, sizeof(stTimeout)) == -1) {//设置读取超时 失败
		return -1;
	}

	//if(::setsockopt(m_iSockfd, SOL_SOCKET,SO_RCVBUF,(char*)&sock_buf_size, sizeof(sock_buf_size)) == -1) {//设置读取超时 失败
	//	return -1;
	//}
	int res = ::recv(m_iSockfd, pvRecvBuf, sizeRecvLen, 0);
	switch(res)
	{
	case -1:
		if(errno == EAGAIN) {//超时
			errno = 0;
			return -2;
		}//else出错
		return -1;
		break;
	case 0://socket 读取对象不存在
		return 0;
		break;
	default:
		if(res < 0)
			return -1;
		break;
	}
	return res;
}

/*
 * socket读取数据
 * 参数：
 * recvBuf:保存读取数据buf
 * recvLen:读取长度
 * 返回值：成功返回接收长度   失败返回-1  超时－2 读取对象不存在 0
 */
int CTcpSocket::TcpReadAll(void *const pvRecvBuf, size_t sizeRecvLen)
{
	if(pvRecvBuf == NULL || sizeRecvLen == 0) {
		return -1;
	}
	int ilen = 0;
	int ilength = 0;
	while((ilength = TcpRead(((char *)pvRecvBuf + ilen), sizeRecvLen - ilen)) > 0)
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


bool CTcpSocket::TcpWrite(const void *pvSendBuf,size_t sizeSendLen)
{
	if(!pvSendBuf || sizeSendLen <= 0) {//参数有误
		return false;
	}
	if(::send(m_iSockfd, pvSendBuf, sizeSendLen,0) != (int)sizeSendLen) {//发送失败
		return false;
	}
	return true;
}


bool CTcpSocket:: TcpClose()
{
	struct linger so_linger;
	so_linger.l_onoff = true;
	so_linger.l_linger = 0;
	setsockopt(m_iSockfd,SOL_SOCKET,SO_LINGER,&so_linger,sizeof(so_linger));
	printf("m_iSockfd=%d is closing \n",m_iSockfd);
	if(m_bConnect) {
		if(m_iSockfd != -1)
		{
			::close(m_iSockfd);
			m_iSockfd=-1;
		}
		m_bConnect = false;
	}
	return true;
}
int CTcpSocket::EncodingBase64(char * pInput, char * pOutput)
{
 int i = 0;
 int loop = 0;
 int remain = 0;
 int iDstLen = 0;
 int iSrcLen = (int)strlen(pInput);

 loop = iSrcLen/3;
 remain = iSrcLen%3;

 // also can encode native char one by one as decode method
 // but because all of char in native string  is to be encoded so encode 3-chars one time is easier.

 for (i=0; i < loop; i++)
 {
  char a1 = (pInput[i*3] >> 2);
  char a2 = ( ((pInput[i*3] & 0x03) << 4) | (pInput[i*3+1] >> 4) );
  char a3 = ( ((pInput[i*3+1] & 0x0F) << 2) | ((pInput[i*3+2] & 0xC0) >> 6) );
  char a4 = (pInput[i*3+2] & 0x3F);

  pOutput[i*4] = AVal(a1);
  pOutput[i*4+1] = AVal(a2);
  pOutput[i*4+2] = AVal(a3);
  pOutput[i*4+3] = AVal(a4);
 }

 iDstLen = i*4;

 if (remain == 1)
 {
  // should pad two equal sign
  i = iSrcLen-1;
  char a1 = (pInput[i] >> 2);
  char a2 = ((pInput[i] & 0x03) << 4);

  pOutput[iDstLen++] = AVal(a1);
  pOutput[iDstLen++] = AVal(a2);
  pOutput[iDstLen++] = '=';
  pOutput[iDstLen++] = '=';
  pOutput[iDstLen] = 0x00;
 }
 else if (remain == 2)
 {
  // should pad one equal sign
  i = iSrcLen-2;
  char a1 = (pInput[i] >> 2);
  char a2 = ( ((pInput[i] & 0x03) << 4) | (pInput[i+1] >> 4));
  char a3 = ( (pInput[i+1] & 0x0F) << 2);

  pOutput[iDstLen++] = AVal(a1);
  pOutput[iDstLen++] = AVal(a2);
  pOutput[iDstLen++] = AVal(a3);
  pOutput[iDstLen++] = '=';
  pOutput[iDstLen] = 0x00;
 }
 else
 {
  // just division by 3
  pOutput[iDstLen] = 0x00;
 }

 return iDstLen;
}
int CTcpSocket::TcpSendEmail(char * smtpServer,int port,char* username,char * password,char* from,char * to,char * subject,char * body)
{

/*
	printf("port=%d,ip=%s\n",port,smtpServer);
	CTcpSocket(port,std::string(smtpServer));

	if(!TcpConnect(port,std::string(smtpServer)))
	{	
		printf("connect email info error\n");
		return -1;
	}

*/
 printf("connect email succefully\n");
 
 char buffer[1024]; 
 char recvBuff[1024*8]; 
 std::string strRcptTo;
 memset(buffer,0,sizeof(buffer));
 memset(recvBuff,0,sizeof(recvBuff));
 printf("from=%s,len %d\n",from,strlen(from));
 sprintf(buffer,"EHLO %s\n",from);//fromΪchar���ݡ��洢���͵�ַ
 printf("buffer=%s,len=%d\n",buffer,strlen(buffer));
 if(!TcpWrite(buffer,strlen(buffer)))
 {
	printf("send EHLO error\n");
	return -1;
 }

 printf("send succefully\n");
 TcpReadAll(recvBuff,1024*8);
 printf("recvBuff=%s\n",recvBuff);

 sprintf(buffer,"AUTH LOGIN\r\n");
 TcpWrite(buffer,strlen(buffer));
 TcpReadAll(recvBuff,1024*8);
 printf("recvBuff=%s\n",recvBuff);


 //USER NAME
 //User name is coded by base64.
 //base64_encode_2(username,buffer);//�Ƚ��û��ʺž���base64����
 EncodingBase64(username,buffer);
 printf("username buffer=%s\n",buffer);
 strcat(buffer,"\r\n");
 TcpWrite(buffer,strlen(buffer));
 TcpReadAll(recvBuff,1024*8);
 printf("recvBuff=%s\n",recvBuff);


 //password coded by base64
 EncodingBase64(password,buffer);
 printf("password buffer=%s\n",buffer);
 strcat(buffer,"\r\n");
 TcpWrite(buffer,strlen(buffer));
 TcpReadAll(recvBuff,1024*8);
 printf("recvBuff=%s\n",recvBuff);



 sprintf(buffer,"MAIL FROM:<%s>\r\n",from);
 TcpWrite(buffer,strlen(buffer));
 TcpReadAll(recvBuff,1024*8);
 printf("recvBuff=%s\n",recvBuff);



int index=0;
char p[100][2048];
char *buf = to;
std::string strBuffer = to;
std::string strAddress;
printf("strBuffer=%s\n",strBuffer.c_str());
int iBegin = 0;
int iPos ;

printf("iBegin=%d\n",iBegin);
while((iPos=strBuffer.find(",",iBegin))!= -1 ) 
{	

	printf("iPos=%d\n",iPos);
	strAddress = strBuffer.substr(iBegin,iPos-iBegin);
	printf("strAddress=%s\n",strAddress.c_str());
	memset(p[index],0,sizeof(p[index]));
	sprintf(p[index],"%s",strAddress.c_str());
	index++;
	sprintf(buffer,"RCPT TO: <%s>\r\n",strAddress.c_str());
	TcpWrite(buffer,strlen(buffer));
	TcpReadAll(recvBuff,1024*8);
	printf("recvBuff=%s\n",recvBuff);
	iBegin = iPos+1;
	printf("iBegin=%d\n",iBegin);
	memset(buffer,0,sizeof(buffer));
}

	strAddress = strBuffer.substr(iBegin);
	memset(p[index],0,sizeof(p[index]));
	sprintf(p[index],"%s",strAddress.c_str());
	index++;
	printf("index=%d\n",index);
	printf("strAddress ttttt=%s\n",strAddress.c_str());
	sprintf(buffer,"RCPT TO: <%s>\r\n",strAddress.c_str());
	TcpWrite(buffer,strlen(buffer));
	TcpReadAll(recvBuff,1024*8);
	printf("recvBuff=%s\n",recvBuff);


/*
while((p[index]=strtok(buf,","))!=NULL) 
{	
	printf("list:%s\n",p[index]);
	sprintf(buffer,"RCPT TO: <%s>\r\n",p[index]);
	TcpWrite(buffer,strlen(buffer));
	TcpReadAll(recvBuff,1024*8);
	printf("recvBuff=%s\n",recvBuff);
	index++;
	buf=NULL; 
}
*/


 printf("===p[0]====%s\n",p[0]);
 printf("===p[1]====%s\n",p[1]);
 printf("===p[2]====%s\n",p[2]);
 
 //memset(buffer,0,sizeof(buffer));
 sprintf(buffer,"DATA\r\n");
 TcpWrite(buffer,strlen(buffer));
 TcpReadAll(recvBuff,1024*8);
 printf("recvBuff=%s\n",recvBuff);

 //DATA from
 sprintf(buffer,"From: %s\r\n",from);
 TcpWrite(buffer,strlen(buffer));

 //DATA to
 for(int i=0;i<index;i++)
 {
 printf("===p[%d]====%s\n",i,p[i]);
 memset(buffer,0,sizeof(buffer));
 sprintf(buffer,"To: %s\r\n",p[i]);
 TcpWrite(buffer,strlen(buffer));
 }
 


  //DATA cc
 //sprintf(buffer,"cc: %s\r\n","suwk@asiainfo.com");
 //TcpWrite(buffer,strlen(buffer));

 //DATA head
 sprintf(buffer,"Subject: %s\n\r\n",subject);
 TcpWrite(buffer,strlen(buffer));


 //DATA body
 sprintf(buffer,"%s\r\n.\r\n",body);
 TcpWrite(buffer,strlen(buffer));
 printf("recvBuff=%s\n",recvBuff);

 strcpy(buffer,"QUIT\r\n");
 TcpWrite(buffer,strlen(buffer));
 TcpReadAll(recvBuff,1024*8);
 printf("recvBuff=%s\n",recvBuff);

 //TcpClose();
 
 return 0;


}


bool CTcpSocket:: TcpCloseFinal()
{
	struct linger so_linger;
	so_linger.l_onoff = true;
	so_linger.l_linger = 0;
	setsockopt(m_iSockfd,SOL_SOCKET,SO_LINGER,&so_linger,sizeof(so_linger));
		if(m_iSockfd != -1)
		{
			::close(m_iSockfd);
			m_iSockfd=-1;
		}
		m_bConnect = false;
	return true;
}

/*
 *
 */
bool CTcpSocket::TcpNewSocket()
{
	if(m_bConnect) {
		::close(m_iSockfd);
		m_bConnect = false;
	}
	m_iSockfd = socket(AF_INET,SOCK_STREAM,0);
	if(m_iSockfd == -1) {
		return false;
	}
	return true;
}

/*
 * set socket ip
 */
void CTcpSocket::TcpSetIp(const std::string& strIp)
{
	m_strIp = strIp;
}

/*
 * set socket port
 */
void CTcpSocket::TcpSetPort(const in_port_t in_portPort)
{
	m_in_portPort = in_portPort;
}

/*
 * set socket timeout
 */
void CTcpSocket::TcpSetTimeout(const u_int uiTimeout)
{
	m_uiTimeout = uiTimeout;
}

bool CTcpSocket::TcpIsConnect()
{
	return m_bConnect;
}

int CTcpSocket::TcpGetSocket()
{
	return m_iSockfd;
}
bool CTcpSocket::TcpSetNoblock()
{
	if (-1 == fcntl(m_iSockfd, F_SETFL, fcntl(m_iSockfd, F_GETFD, 0) | O_NONBLOCK)) {
		fprintf(stderr, "%s\n", strerror(errno));
		return false;
	}
	return true;
}

void CTcpSocket::TcpSslInitParams()
{
	ssl = NULL;
	ssl_ctx = NULL;
	ssl_method = NULL;
	server_cert = NULL;
	SSL_library_init();
	OpenSSL_add_all_algorithms();
	OpenSSL_add_all_ciphers();
}

int CTcpSocket::TcpSslInitEnv()
{
		/*
		if( SSL_library_init()< 0 )
		{
			return -1;
		}

		OpenSSL_add_all_algorithms();
		OpenSSL_add_all_ciphers();
	//SSL_load_error_strings();
		*/

	//ssl_method = const_cast<SSL_METHOD *>(SSLv23_client_method());
	
		ssl_ctx = SSL_CTX_new(SSLv23_client_method());
		//ssl_ctx = SSL_CTX_new(TLSv1_method());
	  				
	  				#if 1
						/* Load the RSA CA certificate into the SSL_CTX structure */
						/* This will allow this client to verify the server's   */
						/* certificate.                             */
						SSL_CTX_load_verify_locations(ssl_ctx, strSslKeyPath.c_str(), NULL); 
						/* Set flag in context to require peer (server) certificate verification */
						SSL_CTX_set_verify(ssl_ctx,SSL_VERIFY_NONE,NULL); //是否要求对端验证 默认 SSL_VERIFY_NONE  ;SSL_VERIFY_PEER
						
						SSL_CTX_set_verify_depth(ssl_ctx,1);
						SSL_CTX_set_default_passwd_cb_userdata(ssl_ctx, (void*)strSslKeyPassWord.c_str());
						//SSL_CTX_set_default_passwd_cb_userdata(ssl_ctx, (void*)"dataMining");
						//读取证书文件
						SSL_CTX_use_certificate_file(ssl_ctx,strSslKeyPath.c_str(),SSL_FILETYPE_PEM);
						//读取密钥文件
						SSL_CTX_use_PrivateKey_file(ssl_ctx,strSslKeyPath.c_str(),SSL_FILETYPE_PEM);
						
						//验证密钥是否与证书一致
						if(!SSL_CTX_check_private_key(ssl_ctx))
						{
								ERR_print_errors_fp(stderr);	
						}
								 
						#endif
		ssl = SSL_new(ssl_ctx);
	//openssl的文档上也明文规定不能将一个SSL指针用于多个线程，所有调用CreateThread函数创建线程，参数设置为SSL指针必然在线程中是互斥的，考虑运用windows开源库pthread改造多线程
		return 0;
}

bool CTcpSocket::TcpSslConnect()
{	
	int s1=SSL_set_fd(ssl,m_iSockfd);					
	int s2=SSL_connect(ssl);
	//printf("s1=%d,s2=%d\n",s1,s2);
	if( s1!=1 || s2!=1 )
	{
		//ERR_print_errors_fp(stderr);
		return false;
	}
	
	return true;

}

int CTcpSocket::TcpSslReadLen( void* buf, int nBytes)
{
        int nleft;
        int nread;
        //char *ptr;
        //ptr = buf;
        nleft = nBytes;
        
        
        do {
        		
                nread=SSL_read(ssl, buf,nleft);
                switch(SSL_get_error(ssl,nread))
                {
                        case SSL_ERROR_NONE://ret >0 read all
                                //printf("SSL_ERROR_NONE\n");
                                nleft = 0;
                                break;
                        case SSL_ERROR_ZERO_RETURN://close
                                //printf("SSL_ERROR_ZERO_RETURN\n");
                                nread=0;//ret 0
                                //goto end;
                                break;
                        case SSL_ERROR_WANT_READ://try again
                                //printf("SSL_ERROR_WANT_READ\n");
                                //if(nread<0)
                                //nread = 0;
                                //break;
                                continue;
                        case SSL_ERROR_WANT_WRITE:
                                //printf("SSL_ERROR_WANT_WRITE\n");//try again
                                break;
                        default:
                        		printf("SSL read problem\n");
                                return -1;
                
                }//end switch
//end:

                nleft=nleft-nread;
				//int sss=SSL_pending(ssl);
        }while( nleft > 0 );

        return (nBytes-nleft);
}


int CTcpSocket::TcpSslWriteLen(void* buf, int nBytes)
{
        int nleft;
        int nwrite ;
        char *ptr;
        
        ptr=(char*)buf;
        nleft=nBytes;
        
        
        do {
                nwrite=SSL_write(ssl, ptr,nleft);
                switch(SSL_get_error(ssl,nwrite))
                {
                        case SSL_ERROR_NONE://ret >0 send all
                                //printf("SSL_ERROR_NONE\n");
                                break;
                        case SSL_ERROR_ZERO_RETURN://close
                                //printf("SSL_ERROR_ZERO_RETURN\n");
                                nwrite=0;//ret 0
                                //goto end;
                                break;
                        case SSL_ERROR_WANT_READ://try again
                                //printf("SSL_ERROR_WANT_READ\n");
                                break;
                        case SSL_ERROR_WANT_WRITE:
                               // printf("SSL_ERROR_WANT_WRITE\n");//try again
                                break;
                        default:
					ERR_print_errors_fp(stderr); 
					printf("%s\n",strerror(errno));
                        		printf("SSL write problem\n");
                                return -1;
                                
                
                }//end switch
//end:
                nleft = nleft-nwrite;
                ptr = ptr + nwrite;
        }while( nleft>0 );

        return (nBytes-nleft);
}

void CTcpSocket::TcpSslDestroy()
{
		SSL_shutdown(ssl);
		SSL_free(ssl);
		TcpClose(); 	  
		SSL_CTX_free(ssl_ctx);
}
