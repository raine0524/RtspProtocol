#ifndef	RTSPAGENT_H_
#define	RTSPAGENT_H_

#include <map>
#include "MetaObject.h"
#include "NetBase.h"
#include "NetConn.h"
#include "SessDescribe.h"
#include "RTPProtocol.h"

const unsigned short CRTSPDefaultPort = 554;

typedef enum 
{
	ENUM_DESCRIBE = 0, 
	ENUM_ANNOUNCE, 
	ENUM_GET_PARAMETER, 
	ENUM_OPTIONS, 
	ENUM_PAUSE, 
	ENUM_PLAY, 
	ENUM_RECORD, 
	ENUM_REDIRECT, 
	ENUM_SETUP, 
	ENUM_SET_PARAMETER, 
	ENUM_TEARDOWN
} RTSP_METHOD;

typedef enum 
{
	ENUM_ACCEPTAS = 0,
	ENUM_ACCEPTVS,
	ENUM_ACCEPTAVS
	// ...
} RTSP_TASK;

typedef struct
{
	std::string ghCseq;
	std::string ghCacheControl;
	std::string ghConnection;
	std::string ghDate;
	std::string ghVia;
	std::string ghScale;
	std::string ghSession;
	std::string ghSpeed;
	std::string ghTransport;
} GeneralHeader;

typedef struct
{
	std::string rhAllow;
	std::string rhContentType;
	std::string rhPublic;
	std::string rhRange;
	std::string rhRetryAfter;
	std::string rhRTPInfo;
	std::string rhServer;
	std::string rhUnsupported;
	std::string rhWWWAuthenticate;
} ResponseHeader;

typedef struct
{
	std::string ehContentBase;
	std::string ehContentEncoding;
	std::string ehContentLanguage;
	std::string ehContentLength;
	std::string ehContentLocation;
	std::string ehExpires;
	std::string ehLastModified;
} EntityHeader;

typedef struct
{
	//��չͷ���ṹ
} ExtensionHeader;

typedef struct
{
	std::string strStatusCode;					//����������Ӧ����ֻȡ�м��״̬��Ϳ�����
	GeneralHeader			stGrlHdr;			//��ͨ�ñ�ͷ
	ResponseHeader		stResHdr;			//����Ӧ��ͷ
	EntityHeader				stEntHdr;			//��ʵ�屨ͷ
	ExtensionHeader		stExtHdr;			//����չͷ����stub
	std::string strResEntity;						//���ܻ��ж��ֲ�ͬ��ʽ��ʵ�壬ͳһ��string���
} ResponsePacket;

struct _RTSPHdrCache
{
	int iCseq;		//sequence number
	std::string strSession;

	_RTSPHdrCache()
	{
		iCseq = 0;
		strSession = "";
	}
};
typedef struct _RTSPHdrCache			RTSPHdrCache;

class RZRTSPAgent : public RZAgent
{
public:
	RZRTSPAgent();
	~RZRTSPAgent() {}

public:
	//�����ӷ�����->�����������ļ�->��ִ������
	void ConnectToServer(const std::string&, int);
	inline void SetRequestFile(const std::string&);
	void StartTask(RTSP_TASK);

private:
	std::string		GetMethodStr(RTSP_METHOD) const;
	RZReqLine		GetReqLineByMethod(RTSP_METHOD, int);
	std::vector<RZExtraHdr> GetGeneralExtraHdr() const;
	void GetDESCRIBEExtraHdr(std::vector<RZExtraHdr>&) const;
	std::string GetTransHeader(int) const;
	void GetSETUPExtraHdr(std::vector<RZExtraHdr>&, int) const;
	void GetPLAYExtraHdr(std::vector<RZExtraHdr>&) const;

	void ThreadProc();
	void OnDispatchAcceptAVStream();
	std::vector<RTSP_METHOD> GetTaskMethodList() const;
	void SendRequest(RTSP_METHOD, int);
	void ParsePacket(const char*, unsigned long);
	inline int GetCSeq() const;
	inline std::string GetStatusCode() const;
	std::map<std::string, int>					GetRTPFirstSeq() const;
	std::map<std::string, std::string>		GetResTransFields() const;

	void OnResponseOPTIONS();
	void OnResponseDESCRIBE();
	void OnResponseSETUP(int&);
	void OnResponsePLAY();
	void OnResponseTEARDOWN();

private:
	RTSP_TASK			m_eTask;
	std::string				m_strReqFile;
	ResponsePacket	m_stResPacket;
	RTSPHdrCache		m_stHdrCache;
	RZSessDescribe	m_sdpPacket;
	RZRTPAgent*		m_pRTPAgent;
};
typedef class RZRTSPAgent		RTSPAgent;

inline void RZRTSPAgent::SetRequestFile(const std::string& _strFile)
{
	m_strReqFile = _strFile;
}

inline std::string RZRTSPAgent::GetStatusCode() const
{
	return m_stResPacket.strStatusCode;
}

inline int RZRTSPAgent::GetCSeq() const
{
	return RZTypeConvert::StrToInt(m_stResPacket.stGrlHdr.ghCseq, 10);
}
#endif		//RTSPAGENT_H_