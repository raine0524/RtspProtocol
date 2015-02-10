#include <stdlib.h>
#include "NetConn.h"
#include "Logger.h"

#ifdef	WIN32
#pragma comment(lib, "ws2_32.lib")
#endif

//////////////////////////////////////////////////////////////////////////
//RZNetConn��
RZNetConn::RZNetConn()
	:m_iSockFile(-1), 
	 m_nIPAddr(),
	 m_nPort(ENUM_PRT_UNK),
	 m_uBufSize(CRecvBufSize)
{
	InitNetConn();
}

RZNetConn::~RZNetConn()
{
#ifdef WIN32
	if (m_iSockFile != -1)		//�׽�����Ч
		::closesocket(m_iSockFile);		//�ر��׽���
	if (WSACleanup() == SOCKET_ERROR)
		Log::ERR("clean up resource used by socket file failed.\n");
#endif
	free(m_rBuffer);		//�ͷŻ�������ռ��Դ
}

void RZNetConn::InitNetConn()
{
	if ((m_rBuffer = (char*)malloc(m_uBufSize)) == 0)		//���뻺����
		Log::ERR("malloc for net connection failed.\n");
	::memset(m_rBuffer, 0, m_uBufSize);
	ClearReadSet();		//��ն�����

#ifdef WIN32
	InitWinSockDLL();
#endif
}

void RZNetConn::SetLocalBufSize(unsigned long uBufSize)
{
	void* v = realloc(m_rBuffer, uBufSize);
	if (v == 0)
		Log::ERR("realloc for rBuffer failed.\n");
	m_rBuffer = (char*)v;
	m_uBufSize = uBufSize;
}

int RZNetConn::GetSysRecvBufSize() const
{
	if (!SocketValid())
		Log::ERR("Please create a socket file first.\n");

#ifdef WIN32
	int optVal, optLen = sizeof(int);
	if (::getsockopt(m_iSockFile, SOL_SOCKET, SO_RCVBUF, 
								(char*)&optVal, &optLen) == SOCKET_ERROR)
	{
		Log::ERR("Platform SDK \'getsockopt\' called failed.\tError Code: %d\n", 
							WSAGetLastError());
	}
	return optVal;
#endif
}

void RZNetConn::SetSysRecvBufSize(int iBufSize)
{
	if (!SocketValid())
		Log::ERR("Please create a socket file first.\n");

#ifdef WIN32
	if (::setsockopt(m_iSockFile, SOL_SOCKET, SO_RCVBUF, 
								(const char*)&iBufSize, sizeof(int)) == SOCKET_ERROR)
	{
		Log::ERR("Platform SDK \'setsockopt\' called failed.\tError Code: %d\n", 
							WSAGetLastError());
	}
#endif
}

int RZNetConn::WaitForPeerResponse(const WAIT_MODE& wm, long milliSec /* = 0 */) const
{
	if (wm != ENUM_SYN && wm != ENUM_ASYN)
		Log::ERR("Select incorrect wait mode.\n");

	fd_set readySet = m_readSet;
	struct timeval tv, *ptv;
	int iRet;
	if (wm == ENUM_SYN)		//ͬ��ģʽ
		ptv = NULL;
	else		//�첽ģʽ
	{
		tv.tv_sec = 0;
		tv.tv_usec = milliSec*1000;		//_milliSec��ʾ���룬��tv_usec�ֶα�ʾ΢��
		ptv = &tv;
	}
#ifdef	WIN32
	if ((iRet = ::select(m_maxFd+1, &readySet, NULL, NULL, ptv)) == SOCKET_ERROR)
	{
		Log::ERR("Platform SDK \'Select\' called failed.\tError Code: %d", WSAGetLastError());
	}
#endif
	return iRet;
}

#ifdef WIN32
void RZNetConn::InitWinSockDLL()
{
	WSADATA wsa;
	::memset(&wsa, 0, sizeof(WSADATA));
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		Log::ERR("Initialize socket file failed.\n");
}
#endif

void RZNetConn::CreateSocket()
{
	if (m_nPort.GetPrtType() != ENUM_TCP && m_nPort.GetPrtType() != ENUM_UDP)
		Log::ERR("Please set transport protocol first.\n");

#ifdef WIN32
	int iType, iProtocol, iOptVal = 1;
	if (m_nPort.GetPrtType() == ENUM_TCP)			//TCP
	{
		iType = SOCK_STREAM;
		iProtocol = IPPROTO_TCP;
	} 
	else			//UDP
	{
		iType = SOCK_DGRAM;
		iProtocol = IPPROTO_UDP;
	}
	if ((m_iSockFile = ::socket(AF_INET,iType, iProtocol)) == INVALID_SOCKET)
		Log::ERR("Platform SDK \'socket\' called failed.\tError Code: %d\n", WSAGetLastError());

	if (::setsockopt(m_iSockFile, SOL_SOCKET, SO_REUSEADDR, (const char*)&iOptVal, sizeof(int)) < 0)
		Log::ERR("Platform SDK \'setsockopt\' called failed.\tError Code: %d\n", WSAGetLastError());
#endif
}

//////////////////////////////////////////////////////////////////////////
//RZTcpConn��
RZTcpConn::RZTcpConn()
	:RZNetConn()
{
	m_nPort.SetPrtType(ENUM_TCP);
}

void RZTcpConn::BuildConnection()
{
	if (!m_nIPAddr.IPAddrValid() || !m_nPort.PortValid())		//ֻҪ����֮һ��Ч����ô�޷���������
		Log::ERR("IP Address or Port is not valid, can not build connection.\n");

	CreateSocket();		//�����׽���
	AppSockInReadSet();
#ifdef WIN32
	//����׽��ֵ�ַ�ṹ
	struct sockaddr_in stPeerAddr;
	::memset(&stPeerAddr, 0, sizeof(stPeerAddr));
	stPeerAddr.sin_family = AF_INET;
	stPeerAddr.sin_addr.S_un.S_addr = m_nIPAddr.GetULIPAddr();
	stPeerAddr.sin_port = ::htons(m_nPort.GetPortValue());

	if (::connect(m_iSockFile, (sockaddr*)&stPeerAddr, sizeof(stPeerAddr)) ==SOCKET_ERROR) 
		Log::ERR("Platform SDK \'connect\' called failed.\tError Code: %d\n", WSAGetLastError());
#endif
}

void RZTcpConn::SendDataToPeer(const char* buf, int size)
{
	if (!SocketValid())
		Log::ERR("Can not send data, Please BuildConnection first.\n");

#ifdef WIN32
	if (::send(m_iSockFile, buf, size, 0) == SOCKET_ERROR)
		Log::ERR("Platform SDK \'send\' called failed.\tErrorCode: %d\n", WSAGetLastError());
#endif
}

NetString RZTcpConn::RecvDataFromPeer()
{
	if (!SocketValid())
		Log::ERR("Can not receive data, Please BuildConnection first.\n");
	
	NetString stNetStr(m_rBuffer);
	::memset(m_rBuffer, 0, m_uBufSize);
#ifdef WIN32
	if ((stNetStr.iSize = ::recv(m_iSockFile, m_rBuffer, m_uBufSize, 0)) == SOCKET_ERROR)
		Log::ERR("Platform SDK \'recv\' called failed.\tError Code: %d\n", WSAGetLastError());
#endif

	return stNetStr;
}

//////////////////////////////////////////////////////////////////////////
//RZUdpConn��
RZUdpConn::RZUdpConn()
	:RZNetConn(), 
	 m_uLocalPort(ENUM_UDP),
	 m_iSize(sizeof(sockaddr_in))
{
	m_nPort.SetPrtType(ENUM_UDP);
	::memset(&m_lPeerAddr, 0, sizeof(sockaddr_in));
}

void RZUdpConn::BindLocalPort(const RZNetPort& port)
{
	if (!port.PortValid())
		Log::ERR("Port is not valid, set local listen port failed.\n");
	m_uLocalPort = port;

	CreateSocket();		//�����׽���
#ifdef WIN32
	struct sockaddr_in localAddr;
	::memset(&localAddr, 0, sizeof(localAddr));
	localAddr.sin_family = AF_INET;
	localAddr.sin_addr.S_un.S_addr = ::htonl(INADDR_ANY);		//�����κε�ַ���������ݰ�
	localAddr.sin_port = ::htons(m_uLocalPort.GetPortValue());
	//�򵥰󶨾Ϳ����ˣ�UDP�ǡ������ӡ�Э��
	if (::bind(m_iSockFile, (sockaddr*)&localAddr, sizeof(localAddr)) == SOCKET_ERROR)
		Log::ERR("Platform SDK \'bind\' called failed.\tError Code: %d\n", ::WSAGetLastError());
#endif
}

void RZUdpConn::BuildConnection()
{
	if (!m_uLocalPort.PortValid())
		Log::ERR("Local listen port has not binded, please call \'InitLocalPort\' first.\n");
	if (!m_nIPAddr.IPAddrValid() || !m_nPort.PortValid())		//ֻҪ����֮һ��Ч����ô�޷���������
		Log::ERR("IP Address or Port is not valid, can not build connection.\n");

#ifdef WIN32
	::memset(&m_stPeerAddr, 0, sizeof(m_stPeerAddr));
	m_stPeerAddr.sin_family = AF_INET;
	m_stPeerAddr.sin_addr.S_un.S_addr = m_nIPAddr.GetULIPAddr();
	m_stPeerAddr.sin_port = ::htons(m_nPort.GetPortValue());
#endif
}

void RZUdpConn::SendDataToPeer(const char* buf, int size)
{
	if (!SocketValid())
		Log::ERR("Can not send data, Please BuildConnection first.\n");

#ifdef WIN32
	if (::sendto(m_iSockFile, buf, size, 0, (sockaddr*)&m_stPeerAddr, sizeof(sockaddr)) == SOCKET_ERROR)
		Log::ERR("Platform SDK \'sendto\' called failed.\tError Code: %d\n", WSAGetLastError());
#endif
}

NetString RZUdpConn::RecvDataFromPeer()
{
	if (!SocketValid())
		Log::ERR("Can not receive data, Please BuildConnection first.\n");

	NetString stNetStr(m_rBuffer);
	::memset(m_rBuffer, 0, m_uBufSize);
#ifdef WIN32
	stNetStr.iSize = ::recvfrom(m_iSockFile, m_rBuffer, m_uBufSize, 0,
													(sockaddr*)&m_lPeerAddr, &m_iSize);
	if (stNetStr.iSize == SOCKET_ERROR)
		Log::ERR("Platform SDK \'recvfrom\' called failed.\tError Code: %d\n", WSAGetLastError());
#endif

	return stNetStr;
}