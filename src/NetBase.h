#ifndef	NETBASE_H_
#define	NETBASE_H_

#ifdef WIN32
#include <WinSock2.h>
#endif

#include <string>
#include <vector>
#include "Logger.h"

const unsigned long CRPRangeStart = 2048;			//�������ѡ��Ķ˿ڷ�Χ
const unsigned long CRPRangeEnd = 48512;
const unsigned long CDPRangeStart = 49256;
const unsigned long CDPRangeEnd = 65256;

typedef enum 
{
	ENUM_TCP = 0,
	ENUM_UDP, 
	ENUM_PRT_UNK		//δ֪����
} NET_PRTTYPE;

typedef enum 
{
	ENUM_WELLKNOWN = 0,		//֪���˿ڣ���0~1023���ṩ��һЩ֪������ʹ��
	ENUM_REGISTER,					//ע��˿ڣ���1024~49151���ṩ���û����̻�Ӧ�ó���ʹ��
	ENUM_DYNAMIC,				//��̬�˿ڣ���49152~65535�����̶������ĳ�ַ��񣬶�̬����
	ENUM_PORTTYPE_UNK
} NET_PORTTYPE;

typedef enum 
{
	ENUM_RTSP = 0, 
	//...
} APP_PRTTYPE;

class RZNetPort 
{
public:
	RZNetPort(NET_PRTTYPE);
	RZNetPort(unsigned short, NET_PRTTYPE);
	~RZNetPort() {}

public:
	inline bool PortValid() const;
	inline NET_PRTTYPE GetPrtType() const;				//��ȡЭ������
	inline void SetPrtType(NET_PRTTYPE);				//����Э������
	inline unsigned short GetPortValue() const;		//ȡ�ö˿ڵ�ֵ
	void SetPortValue(unsigned short);						//���ö˿ڵ�ֵ
	void RandSelectValidPort(NET_PORTTYPE);

private:
	bool PortOccupied(unsigned long);
	NET_PORTTYPE GetPortType() const;

	//////////////////////////////////////////////////////////////////////////
	inline void UpdateSystemOccupiedPort();
	std::vector<unsigned long> GetLocalUsedPort(NET_PRTTYPE);
	std::vector<unsigned long> GetLocalUsedTcpPort();
	std::vector<unsigned long> GetLocalUsedUdpPort();

private:
	unsigned short	m_port;
	NET_PRTTYPE	m_ePrtType;		//�˿ڶ�Ӧ��Э������

	//���ӳ־û����󣬱���ϵͳ��ǰռ�ö˿�
	static std::vector<unsigned long> m_vPortList;
};

inline bool RZNetPort::PortValid() const
{
	if (m_port == 0)		//�˿�Ϊ0��ʾ������
		return false;
	return true;
}

inline NET_PRTTYPE RZNetPort::GetPrtType() const
{
	return m_ePrtType;
}

inline void RZNetPort::SetPrtType(NET_PRTTYPE ePrtType)
{
	if (ePrtType == ENUM_PRT_UNK)
		Log::ERR("Unvalid Protocol Type. Please set protocol type first.\n");
	m_ePrtType = ePrtType;
}

inline unsigned short RZNetPort::GetPortValue() const
{
	if (m_port == 0)
		Log::ERR("Current port is not valid, Please set a valid port first.\n");
	return m_port;
}

inline void RZNetPort::UpdateSystemOccupiedPort()
{
	m_vPortList = GetLocalUsedPort(m_ePrtType);
}

//////////////////////////////////////////////////////////////////////////
class RZNetIPAddr 
{
public:
	RZNetIPAddr();
	RZNetIPAddr(const std::string&);
	~RZNetIPAddr() {}

public:
	//ȡ��IP��ַ�ĵ��ʮ���ƴ���ʽ���޷���������ʽ
	inline unsigned long GetULIPAddr() const;
	inline std::string GetSTRIPAddr() const;

	void SetIPAddr(const std::string&);
	inline bool IPAddrValid() const;

private:
	inline void InitIPAddr(const std::string&);
	bool IPAddrLegal(const std::string&) const;

private:
	unsigned long	m_uIPAddr;
	std::string			m_strIPAddr;
};

inline void RZNetIPAddr::InitIPAddr(const std::string& str)
{
	m_strIPAddr = str;
#ifdef WIN32
	m_uIPAddr = ::inet_addr(str.c_str());
#endif
}

inline bool RZNetIPAddr::IPAddrValid() const
{
	if (m_uIPAddr != 0)
		return true;
	else
		return false;
}

inline unsigned long RZNetIPAddr::GetULIPAddr() const
{
	if (m_uIPAddr == 0)
		Log::ERR("Please set a valid IP Address first.\n");
	return m_uIPAddr;
}

inline std::string RZNetIPAddr::GetSTRIPAddr() const
{
	if (m_uIPAddr == 0)
		Log::ERR("Please set a valid IP Address first.\n");
	return m_strIPAddr;
}

//////////////////////////////////////////////////////////////////////////
class RZReqLine
{
public:
	RZReqLine(APP_PRTTYPE);
	~RZReqLine() {}

public:
	inline void SetReqLine(const std::string&, const std::string&);
	inline std::string GetReqLine() const;

private:
	//һ�������еĸ�ʽΪ<method> <uri> <version>
	std::string m_strMethod;
	std::string m_strUri;
	std::string m_strVersion;
};

inline void RZReqLine::SetReqLine(const std::string& strMethod, const std::string& strUri)
{
	m_strMethod = strMethod;
	m_strUri = strUri;
}

inline std::string RZReqLine::GetReqLine() const
{
	return m_strMethod+" "+m_strUri+" "+m_strVersion;
}

//////////////////////////////////////////////////////////////////////////
class RZExtraHdr 
{
public:
	RZExtraHdr();
	~RZExtraHdr() {}

public:
	inline void SetExtraHdr(const std::string&, const std::string&);
	inline std::string GetExtraHdr() const;

private:
	//���ӱ�ͷ�ĸ�ʽΪ<header name>: <header data>
	std::string m_strHdrName;
	std::string m_strHdrData;
};

inline void RZExtraHdr::SetExtraHdr(const std::string& strHdrName, const std::string& strHdrData)
{
	m_strHdrName = strHdrName;
	m_strHdrData = strHdrData;
}

inline std::string RZExtraHdr::GetExtraHdr() const
{
	return m_strHdrName+": "+m_strHdrData;
}

//////////////////////////////////////////////////////////////////////////
class RZReqPacket 
{
public:
	RZReqPacket(APP_PRTTYPE);
	~RZReqPacket() {}

public:
	inline void FillPacket(const RZReqLine&, const std::vector<RZExtraHdr>&);
	std::string GetRequestString() const;

private:
	//һ���������һ�������кͶ������ͷ
	RZReqLine m_rLine;
	std::vector<RZExtraHdr> m_vExtHdrList;
};

inline void RZReqPacket::FillPacket(const RZReqLine& rLine, const std::vector<RZExtraHdr>& vHdrList)
{
	m_rLine = rLine;
	m_vExtHdrList = vHdrList;
}
#endif		//NETBASE_H_