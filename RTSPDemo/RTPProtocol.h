#ifndef	RTPPROTOCOL_H_
#define	RTPPROTOCOL_H_

#include <queue>
#include <stdio.h>

#include "MetaObject.h"
#include "NetConn.h"
#include "NetBase.h"
#include "ConsoleText.h"
#include "Logger.h"
#include "H264RTP2Frame.h"

#pragma pack(1)		//�ļ��������ڵ����нṹ���ֶβ�����Ҳ�����

const unsigned long CSysRecvBufSize = 1024*1024;
const unsigned long CDaySeconds = 24*60*60UL;
const unsigned long CMilli2PicoSec = 1000*1000*1000UL;

class RZRTPAgent;
class RZRTCPAgent;

typedef union
{
	struct
	{
		unsigned char RC : 5;		//���ձ�����������ֵ������ǰ���н��ձ���飨reception report blocks��������
		unsigned char P : 1;			//���λ������λ��ʾ����䣬�������һ���ֽ�ָʾ�����ֽ���������������
		unsigned char V : 2;			//ָ��RTPЭ��İ汾����ǰ�汾��Ϊ2
	} bField;
	unsigned char theWholeByte;
} RTCP_FirstByte;

struct _RTCP_SenderInfo
{
	char NTPTimeStamp[8];
	unsigned long RTPTimeStamp;			//��NTPTimeStamp�ַ�����Ӧ��ʱ���ʾ
	unsigned long PacketCount;				//�ӿ�ʼ����ֱ���������RTCP���ڼ䴫���RTP���ݰ�������
	unsigned long PayloadOctets;			//�ӿ�ʼ����ֱ���������RTCP���ڼ䴫���RTP������Ч�غɵ�����

	_RTCP_SenderInfo()
	{
		::memset(this, 0, sizeof(_RTCP_SenderInfo));
	}
};
typedef struct _RTCP_SenderInfo		RTCP_SenderInfo;

struct _RTCP_ReportBlocks
{
	unsigned long SSRC_n;				//ָ�������������Դ��ͬ��Դ��ʶ��
	unsigned char FractionLost;		//ָ������һ��SR/RR����������RTP���ݰ��Ķ�ʧ�ʣ�=FractionLost/256��
	char CumuLostPackets[3];			//�ӿ�ʼ����ʱ�ۼƶ�ʧ��RTP���ݰ�������
	unsigned long ExtHiestSeqNum;			//��16λ����RTP���ݰ����������к�
	unsigned long InterarrivalJitter;			//���ն�����RTP���ݰ�����ʱ���ͳ�Ʒ������
	unsigned long LSR;				//�ϴ�SRʱ���
	unsigned long DLSR;			//�ϴη���SR�������η���֮���ʱ��

	_RTCP_ReportBlocks()
	{
		::memset(this, 0, sizeof(_RTCP_ReportBlocks));
	}
};
typedef struct _RTCP_ReportBlocks		RTCP_ReportBlocks;

struct _RTCP_SRHeader
{
	RTCP_FirstByte uFirstByte;
	unsigned char PT;				//ָ��ͷ�����ͣ����統���ֵΪ200ʱָ����ǰͷ����SR���͵�ͷ��
	unsigned short length;		//��ʾͷ���ĳ��ȣ�=(length+1)*4 bytes�������Ұ������
	unsigned long SSRC;			//ָ����ǰͷ����ͬ��Դ��ʶ�����������RTPͷ���е�SSRC��ͬ
	RTCP_SenderInfo stSendInfo;
	std::vector<RTCP_ReportBlocks> vReportBlks;

	_RTCP_SRHeader()
		:PT(200),
		length(0),
		SSRC(0),
		stSendInfo(),
		vReportBlks()
	{
		uFirstByte.theWholeByte = 0;
	}
};
typedef struct _RTCP_SRHeader		RTCP_SRHeader;

struct _RTCP_RRHeader
{
	RTCP_FirstByte uFirstByte;		//the same as that of the SR packet except that PT field contains the constant 201
	unsigned char PT;					//& RTCP_SenderInfo structure is omitted
	unsigned short length;
	unsigned long SSRC;
	RTCP_ReportBlocks stReportBlks;

	_RTCP_RRHeader()
		:PT(201),
		stReportBlks()
	{
	}
};
typedef struct _RTCP_RRHeader		RTCP_RRHeader;

struct _RTCP_SDESHeader
{
	RTCP_FirstByte uFirstByte;
	unsigned char PT;
	unsigned short length;
	char chunk[24];

	_RTCP_SDESHeader()
		:PT(202)
	{
	}
};
typedef struct _RTCP_SDESHeader		RTCP_SDESHeader;

struct _RTCP_BYEHeader
{
	RTCP_FirstByte uFirstByte;
	unsigned char PT;
	unsigned short length;
	unsigned long SSRC;

	_RTCP_BYEHeader()
		:PT(203)
	{
	}
};
typedef struct _RTCP_BYEHeader			RTCP_BYEHeader;

typedef enum
{
	ENUM_RTCP_SR			= 200,			//Sender Report�����Ͷ˱���
	ENUM_RTCP_RR			= 201,			//Receiver Report�����ն˱���
	ENUM_RTCP_SDES		= 202,			//Source Description Items��Դ������
	ENUM_RTCP_BYE			= 203,			//��������
	ENUM_RTCP_APP			= 204			//�ض�Ӧ��
} RTCP_HEADER_TYPE;

struct _RTCP_RecvPacket
{
	RTCP_SRHeader stSRHdr;
	//... ������Ӹ������͵�ͷ��

	unsigned long SSRC;
	bool bGetLastPack;			//�Ƿ���ȡ�����һ��RTCP��
	unsigned long ulTimeStamp;		//���յ�����RTCP����ʱ���
	unsigned long ulLSRPackCnt;		//ǰһ��RTCP�������͵�RTP�����ѷ�����

	_RTCP_RecvPacket()
		:stSRHdr(),
		SSRC(0),
		bGetLastPack(false),
		ulTimeStamp(0),
		ulLSRPackCnt(0)
	{
	}
};
typedef struct _RTCP_RecvPacket		RTCP_RecvPacket;

struct _RTCP_SendPacket
{
	RTCP_RRHeader stRRHdr;
	RTCP_SDESHeader stSDESHdr;
	RTCP_BYEHeader stBYEHdr;

	unsigned long nCumuLost;
	unsigned int uLastJitter;

	_RTCP_SendPacket()
		:stRRHdr(),
		 stSDESHdr(),
		 stBYEHdr(),
		 nCumuLost(0),
		 uLastJitter(0)
	{
	}
};
typedef struct _RTCP_SendPacket		RTCP_SendPacket;

class RZRTCPAgent : public RZAgent
{
public:
	RZRTCPAgent(RZRTPAgent*);
	~RZRTCPAgent() {}

public:
	inline void SetLocalSSRC(unsigned long);

private:
	void ThreadProc();
	void ParsePacket(const char*, unsigned long);
	void OnResponseSR(const char*);
	void OnResponseBYE();
	unsigned long GeneSSRC() const;

	void SetReportBlocks();
	unsigned char CalculateFractionLost();
	unsigned int CalculateInterJitter();
	unsigned long CalculateLSRDelay() const;

private:
	RTCP_RecvPacket m_rPacket;
	RTCP_SendPacket m_sPacket;
	RZRTPAgent* m_pRTPAgent;
};

inline void RZRTCPAgent::SetLocalSSRC(unsigned long _ulSSRC)
{
	m_rPacket.SSRC = _ulSSRC;
}

//////////////////////////////////////////////////////////////////////////
//RZCyclePool��
struct _RangeDetect
{
	unsigned long iFocus;
	unsigned long iBeyondCnts;

	_RangeDetect()
		:iFocus(0),
		 iBeyondCnts(0)
	{
	}
};
typedef struct _RangeDetect		RangeDetect;

class RZCyclePool
{
public:
	RZCyclePool(unsigned long _nCycles = 4);
	~RZCyclePool();

public:
	inline void SetFirstFocus(unsigned long);
	void Insert(unsigned long _index, const char* _pSrc, unsigned long _ulSize);
	RZNetStrPool* GetFocusPool() const;
	void UpdateFocusPool();
	std::vector<RZNetStrPool*> GetValidPool() const;

private:
	RZNetStrPool*	m_pNetStrPool;
	RangeDetect		m_stRngDetect;
	int m_iCycles;
};

inline void RZCyclePool::SetFirstFocus(unsigned long _ulSeqNum)
{
	unsigned long index = _ulSeqNum%(m_iCycles*CPoolSlots);
	m_stRngDetect.iFocus = index/CPoolSlots;
}

//////////////////////////////////////////////////////////////////////////
//RZRTPAgent��
typedef union
{
	struct
	{
		/*�ṹ����ڴ�ģ��Ϊ��  ��λ |-V-|-P-|-X-|-CC-| ��λ  */
		unsigned char CC : 4;	//��ʾ�̶�ͷ��������ŵ�CSRC����Ŀ
		unsigned char X : 1;		//��λ��λ��ͷ���������һ����չͷ��
		unsigned char P : 1;		//��λ��λ��RTP����β�����и��ӵ�����ֽڣ��������һ���ֽ�ָ������ֽ���
		unsigned char V : 2;		//��ʾRTP�İ汾
	} bField;
	unsigned char theWholeByte;
} RTPFirstByte;

typedef union
{
	struct
	{
		unsigned char PT : 7;		//��ʾRTP�������ı�������
		unsigned char M : 1;		//λ�Ľ����������ĵ���profile�����е�
	} bField;
	unsigned char theWholeByte;
} RTPSecByte;

struct _RTPHeader			//����Ҫ����RTP����ͷ�������Ѷ��屣��������
{
	RTPFirstByte		uFirByte;
	RTPSecByte		uSecByte;
	unsigned short	SeqNum;
	unsigned long TimeStamp;
	unsigned long SSRC;

	_RTPHeader() 
	{
		::memset(this, 0, sizeof(_RTPHeader));
	}
};
typedef struct _RTPHeader	RTPHeader;

struct _MediaInfo
{
	int iFirstSeq;				//��һ���������кţ������RTSP��Ӧ����RTP-Info��ͷ��
	int iPackNum;			//��������
	bool bRecvAll;			//���Ƿ���ȫ���������
	unsigned long ulSSRC;			//����ͬ��Դ��ʶ��
	int iSampFreq;			//����Ƶ��

	_MediaInfo()
		:iFirstSeq(0),
		 iPackNum(-1),		//��ʼ��Ϊ-1��ָ����δ�յ����һ����
		 bRecvAll(false),
		 ulSSRC(0)
	{
	}
};
typedef struct _MediaInfo			MediaInfo;

struct _LostPackStatis
{
	int iStart, iEnd;		//��������ص���ʼ�����λ��
	unsigned long nRecvPacks;	//ʵ���յ��İ�
	unsigned long nHndleCnt;		//�Ѵ���İ�������
	unsigned long uThresHold;	//��ǰ��ֵ
	std::vector<unsigned long> vLostPack;		//��Ŷ�ʧ�İ������кţ�ת����Ψһ��

	unsigned long ulFirSeqWallClck;		//RTCPЯ���ĵ�һ��ǽ��ʱ�ӣ���λΪ����
	unsigned long ulFirSeqRTPStmp;		//��Ӧ�ĵ�һ��RTPʱ���
	std::vector<unsigned long> vPackDiff;		//the difference D in packet
	RZSemaphore mSemaphore;

	_LostPackStatis()
		:iStart(0),
		 iEnd(0),
		 nRecvPacks(0),
		 nHndleCnt(0),
		 uThresHold(0),
		 vLostPack(),
		 ulFirSeqWallClck(0),
		 ulFirSeqRTPStmp(0),
		 mSemaphore(1, 1)
	{
		vPackDiff.push_back(0);
	}
};
typedef struct _LostPackStatis		LostPackStatis;

class RZRTPAgent
	:public RZAgent,
	 public H264RTP2FrameCallback
{
	//declaration
	friend class RZRTSPAgent;
	friend class RZRTCPAgent;

public:
	RZRTPAgent(STREAM_MEDIA_TYPE _eMediaType = ENUM_MEDIA_UNK);
	~RZRTPAgent();

public:
	inline RZNetPort GetLocalPort() const;
	inline void SetFirstSeq(int);
	inline void SetSSRC(unsigned long);
	inline unsigned long GetHiestSeqNum() const;
	inline bool PackRecvComplete() const;
	void SetMediaType(STREAM_MEDIA_TYPE _eMediaType);
	void SetServerIPAndPort(const RZNetIPAddr&, const std::string&);

private:
	void InitListenPort();
	void ThreadProc();
	void ParsePacket(const char*, unsigned long);
	void RemvPackDiff();
	void RemvLostPack();
	void OnSinglePool(const RZNetStrPool*);
	void FlushCyclePool();
	inline void WriteMediaFile(void* pPacketData, int nPacketLen);
	virtual void OnH264RTP2FrameCallbackFramePacket(H264RTP2Frame*pH264RTP2Frame,void*pPacketData,int nPacketLen,int nKeyFrame);

private:
	STREAM_MEDIA_TYPE		m_eMediaType;		//��ǰ���յ�������������
	MediaInfo					m_stMediaInfo;
	LostPackStatis			m_stLostPackStatis;		//����ͳ��
	RZCyclePool				m_stCyclePool;
	RZRTCPAgent*			m_pRTCPAgent;
	H264RTP2Frame*	m_pRTP2Frame;
	FILE*	m_pFrameFile;
	bool	m_bWantToStop;

	RZConsoleOutput	m_ConsoleOutput;
	static RZSemaphore	m_Semaphore;
};

inline RZNetPort RZRTPAgent::GetLocalPort() const
{
	RZUdpConn* pUdpConn = dynamic_cast<RZUdpConn*>(m_pNetConn);
	return pUdpConn->GetLocalPort();
}

inline bool RZRTPAgent::PackRecvComplete() const
{
	return m_stMediaInfo.bRecvAll;
}

inline void RZRTPAgent::SetFirstSeq(int _iSeq)
{
	m_stMediaInfo.iFirstSeq = _iSeq;
	m_stLostPackStatis.iStart = _iSeq%CPoolSlots;
	m_stCyclePool.SetFirstFocus(_iSeq);
}

inline void RZRTPAgent::SetSSRC(unsigned long _SSRC)
{
	m_stMediaInfo.ulSSRC = _SSRC;
}

inline unsigned long RZRTPAgent::GetHiestSeqNum() const
{
	return (m_stMediaInfo.iFirstSeq+m_stLostPackStatis.nHndleCnt-1);
}

inline void RZRTPAgent::WriteMediaFile(void* pPacketData, int nPacketLen)
{
	::fwrite(pPacketData, sizeof(char), nPacketLen, m_pFrameFile);
}
#endif			//RTPPROTOCOL_H_