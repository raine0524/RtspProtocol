#ifdef WIN32
#include <WS2tcpip.h>
#include <IPHlpApi.h>
#elif
#endif

#include "NetBase.h"
#include "MetaObject.h"

#pragma comment(lib, "iphlpapi.lib")

std::vector<unsigned long> RZNetPort::m_vPortList;

//////////////////////////////////////////////////////////////////////////
//RZNetPort��
RZNetPort::RZNetPort(NET_PRTTYPE ePrtType)
	:m_port(0),
	 m_ePrtType(ePrtType)
{
}

RZNetPort::RZNetPort(unsigned short port, NET_PRTTYPE ePrtType)
	:m_ePrtType(ePrtType)
{
	//����ָ���˿�ֵ����������캯��ʱ����ֹ��ePrtType����ΪENUM_UNK
	SetPortValue(port);
}

NET_PORTTYPE RZNetPort::GetPortType() const
{
	if (m_port >=0 && m_port <= 1023)			//֪���˿ڷ�ΧΪ0~1023
		return ENUM_WELLKNOWN;
	else if (m_port >= 1024 && m_port <= 49151)		//ע��˿ڷ�ΧΪ1024~49151
		return ENUM_REGISTER;
	else if (m_port >= 49152 && m_port <= 65535)	//��̬�˿ڷ�ΧΪ49152~65535
		return ENUM_DYNAMIC;
	else
		return ENUM_PORTTYPE_UNK;
}

bool RZNetPort::PortOccupied(unsigned long port)
{
	if (port > 65535)
		Log::ERR("Test port is not valid.\n");
	for (std::vector<unsigned long>::const_iterator iter = m_vPortList.begin(); 
			iter != m_vPortList.end(); iter++)
	{
		if (port == *iter)		//��ǰ�˿ں�����ĳ����ʹ�ö˿���ͬ��������ռ��
			return true;
	}
	return false;
}

std::vector<unsigned long> RZNetPort::GetLocalUsedPort(NET_PRTTYPE pt)
{
	if (pt == ENUM_TCP)
		return GetLocalUsedTcpPort();
	else if (pt == ENUM_UDP)
		return GetLocalUsedUdpPort();
	else
		return std::vector<unsigned long>();
}

std::vector<unsigned long> RZNetPort::GetLocalUsedTcpPort()
{
	std::vector<unsigned long> vTcpPortList;

#ifdef WIN32
	ULONG dwRetVal, dwSize = sizeof(MIB_TCPTABLE);
	PMIB_TCPTABLE pTcpTable = 0;
	if ((pTcpTable = (PMIB_TCPTABLE)malloc(dwSize)) == 0)
		Log::ERR("malloc for \'PMIB_TCPTABLE\' failed.\n");

	//һ�����״ε���GetTcpTableʱ���᷵�ش�����ERROR_INSUFFICIENT_BUFFER����ָʾ
	//������̫С���޷�װ���㹻���TcpTable��������ȵ��øú�����ȡTcpTable�Ĵ�С
	if (::GetTcpTable(pTcpTable, &dwSize, true) == ERROR_INSUFFICIENT_BUFFER)
	{
		free(pTcpTable);
		if ((pTcpTable = (PMIB_TCPTABLE)malloc(dwSize)) == 0)
			Log::ERR("malloc for \'PMIB_TCPTABLE\' failed.\n");
	}

	if ((dwRetVal = ::GetTcpTable(pTcpTable, &dwSize, true)) != NO_ERROR)
		Log::ERR("Platform SDK \'GetTcpTable\' failed.\tError Code: %u\n", dwRetVal);
	for (unsigned long i = 0; i < pTcpTable->dwNumEntries; i++)
		vTcpPortList.push_back(pTcpTable->table[i].dwLocalPort);
#endif
	return vTcpPortList;
}

std::vector<unsigned long> RZNetPort::GetLocalUsedUdpPort()
{
	std::vector<unsigned long> vUdpPortList;
#ifdef WIN32
	ULONG dwRetVal, dwSize = sizeof(MIB_UDPTABLE);
	PMIB_UDPTABLE pUdpTable = 0;
	if ((pUdpTable = (PMIB_UDPTABLE)malloc(dwSize)) == 0)
		Log::ERR("malloc for \'PMIB_UDPTABLE\' failed.\n");

	if (::GetUdpTable(pUdpTable, &dwSize, true) == ERROR_INSUFFICIENT_BUFFER)
	{
		free(pUdpTable);
		if ((pUdpTable = (PMIB_UDPTABLE)malloc(dwSize)) == 0)
			Log::ERR("malloc for \'PMIB_UDPTABLE\' failed.\n");
	}

	if ((dwRetVal = ::GetUdpTable(pUdpTable, &dwSize, true)) != NO_ERROR) 
		Log::ERR("Platform SDK \'GetUdpTable\' failed.\tError Code: %u\n", dwRetVal);
	for (unsigned long i = 0; i < pUdpTable->dwNumEntries; i++)
		vUdpPortList.push_back(pUdpTable->table[i].dwLocalPort);
#endif
	return vUdpPortList;
}

void RZNetPort::RandSelectValidPort(NET_PORTTYPE pt)
{
	if (m_ePrtType == ENUM_PRT_UNK)
		Log::ERR("Unvalid Protocol Type. Please set protocol type first.\n");
	//ֻ����ע����߶�̬�˿��н������ѡ��
	if (pt != ENUM_REGISTER && pt != ENUM_DYNAMIC)
		Log::ERR("Please set a valid port type, then call this function.\n");

	int randNum;
	unsigned long rangeStart, rangeEnd;
	if (pt == ENUM_REGISTER)		//ʹ��ע��˿�
	{
		rangeStart = CRPRangeStart;
		rangeEnd = CRPRangeEnd;
	} 
	else //ʹ�ö�̬�˿�
	{
		rangeStart = CDPRangeStart;
		rangeEnd = CDPRangeEnd;
	}

	UpdateSystemOccupiedPort();			//���µ�ǰϵͳռ�ö˿��б�
	do
	{
		randNum = rand()%(rangeEnd-rangeStart+1)+rangeStart;
		if (randNum%2 == 1)		//�˿�Ϊ������RTPЭ��Լ��ʹ��ż���˿ڣ�
			randNum++;
	} while(PortOccupied(randNum));
	m_port = randNum;
}

void RZNetPort::SetPortValue(unsigned short port)
{
	if (m_ePrtType == ENUM_PRT_UNK)
		Log::ERR("Unvalid Protocol Type. Please set protocol type first.\n");

	UpdateSystemOccupiedPort();			//����ϵͳռ�ö˿�
	if (PortOccupied(port))			//�˿ڱ�ռ��
		m_port = 0;
	else
		m_port = port;
}

//////////////////////////////////////////////////////////////////////////
//RZNetIPAddr��
RZNetIPAddr::RZNetIPAddr()
	:m_uIPAddr(0),
	 m_strIPAddr("")
{
}

RZNetIPAddr::RZNetIPAddr(const std::string& str)
{
	if (!IPAddrLegal(str))		//IP��ַ��ʽ�Ƿ�
		Log::ERR("Input an unvalid IP Address.\n");
	InitIPAddr(str);
}

bool RZNetIPAddr::IPAddrLegal(const std::string& str) const
{
	//�����ʮ����IP��ַ����"."���зָ�����е�ԭ���ַ�������vector����
	std::vector<std::string> vAtomList = RZStream::StreamSplit(str, ".");
	for (std::vector<std::string>::const_iterator iter = vAtomList.begin(); 
			iter != vAtomList.end(); iter++)
	{
		int iSubField = RZTypeConvert::StrToInt(*iter, 10);		//��ʮ������ʽת��
		if (iSubField < 0 || iSubField > 255)		//ÿ��������0~255֮��
			return false;
	}
	return true;
}

void RZNetIPAddr::SetIPAddr(const std::string& str)
{
	if (!IPAddrLegal(str))		//IP��ַ��ʽ�Ƿ�
		Log::ERR("Input an unvalid IP Address.\n");
	InitIPAddr(str);
}

//////////////////////////////////////////////////////////////////////////
//RZReqLine��
RZReqLine::RZReqLine(APP_PRTTYPE eAppPt)
	:m_strMethod(""),
	m_strUri("")
{
	switch (eAppPt)
	{
	case ENUM_RTSP:	m_strVersion = "RTSP/1.0";break;
		//...
	default:
		Log::ERR("Unknown Application Protocol Type.\n");
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
//RZExtraHdr��
RZExtraHdr::RZExtraHdr()
	:m_strHdrName(""),
	 m_strHdrData("")
{
}

//////////////////////////////////////////////////////////////////////////
//RZReqPacket��
RZReqPacket::RZReqPacket(APP_PRTTYPE eAppPt)
	:m_rLine(eAppPt),
	 m_vExtHdrList()
{
}

std::string RZReqPacket::GetRequestString() const
{
	//һ�����󴮵ĸ�ʽΪ��
	 //<request line>\r\n
	 //<extra header>\r\n
	 //more extra header
	 //\r\n
	 //
	if (m_rLine.GetReqLine() == "")
		Log::ERR("Request Line can not be null.\n");
	std::string str = m_rLine.GetReqLine()+"\r\n";
	for (std::vector<RZExtraHdr>::const_iterator iter = m_vExtHdrList.begin();
			iter != m_vExtHdrList.end(); iter++)
	{
		if (iter->GetExtraHdr() == "")
			continue;		//ѹ��յĸ�������ͷʱ������str�м�������\r\n
		str += iter->GetExtraHdr()+"\r\n";
	}
	str += "\r\n";		//����������־
	return str;
}