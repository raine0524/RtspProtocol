#ifndef	METAOBJECT_H_
#define	METAOBJECT_H_

#include <string>
#include <vector>
#include <queue>

#include "NetConn.h"

const unsigned long CPoolSlots = 64;
const unsigned long CPoolSlotSize = 4096;
const unsigned long CHour2MilliSec = 60*60*1000UL;
const unsigned long CMin2MilliSec = 60*1000UL;
const unsigned long CSec2MilliSec = 1000UL;

typedef enum 
{
	ENUM_AUDIO = 0,
	ENUM_VIDEO,
	ENUM_MEDIA_UNK
} STREAM_MEDIA_TYPE;

//Singleton Pattern
class RZGlobalInit
{
private:
	RZGlobalInit();
	~RZGlobalInit() {}

private:
	static RZGlobalInit m_gi;
};

class RZStream
{
public:
	//��Ҫ�õ��Ĵ��ָ��
	static std::vector<std::string> StreamSplit(const std::string&, const char*);

private:
	RZStream() {}
	~RZStream() {}
};

class RZTypeConvert 
{
public:
	static int StrToInt(const std::string&, int base);
	static std::string IntToString(int);
	
private:
	RZTypeConvert() {}
	~RZTypeConvert() {}
};

class RZTime
{
public:
	static unsigned long GetTimeStamp();
	static unsigned long GetWallClockTime();
	static void Sleep(unsigned long);

private:
	RZTime() {}
	~RZTime() {}
};

class RZBitmap
{
public:
	RZBitmap(unsigned long nBits);
	~RZBitmap();

public:
	inline void Clear();
	inline void SetBit(unsigned long);
	unsigned long BitCounts() const;
	bool GetBit(unsigned long) const;

private:
	int CountOnByte(char) const;

private:
	char* m_pBitmap;
	unsigned long m_nBytes;		//��ռ�ֽ�
	unsigned long m_nBits;			//λ�ĸ���
};

inline void RZBitmap::Clear()
{
	::memset(m_pBitmap, 0, m_nBytes);
}

inline void RZBitmap::SetBit(unsigned long bit)
{
	if (bit >= m_nBits)
		Log::ERR("The bit need to be set is Out of Range.\n");
	m_pBitmap[bit/8] |= 1<<(bit%8);
}

struct _BufferPool
{
	char* pBufferPool;
	unsigned long* pSize;

	_BufferPool()
	{
		::memset(this, 0, sizeof(_BufferPool));
	}
};
typedef struct _BufferPool		BufferPool;

class RZNetStrPool
{
public:
	RZNetStrPool(unsigned long nSlots = CPoolSlots);
	~RZNetStrPool();

public:
	inline void Clear();
	void Insert(unsigned long, const char*, unsigned long);
	inline BufferPool GetBufferPool() const;
	inline unsigned long GetItems() const;

private:
	BufferPool m_stBufPool;
	unsigned long m_nItems;
	unsigned long m_nSlots;
};

inline void RZNetStrPool::Clear()
{
	::memset(m_stBufPool.pBufferPool, 0, m_nSlots*CPoolSlotSize);
	::memset(m_stBufPool.pSize, 0, m_nSlots*sizeof(unsigned long));
	m_nItems = 0;
}

inline BufferPool RZNetStrPool::GetBufferPool() const
{
	return m_stBufPool;
}

inline unsigned long RZNetStrPool::GetItems() const
{
	return m_nItems;
}

class RZSemaphore
{
public:
	RZSemaphore(long init, long ulMax);
	~RZSemaphore();

public:
	void Wait(WAIT_MODE wm = ENUM_SYN, unsigned long ulMilliSeconds = 0);
	void Release();

private:
#ifdef WIN32
	HANDLE m_hSemaphore;
#endif
};

class RZThread
{
public:
	void StartThread();
	static void WaitPeerThreadStop(const RZThread&);
	inline unsigned long GetCurrentThreadID() const;

private:
#ifdef	WIN32
	HANDLE m_hThread;
	static unsigned long InitThreadProc(void* lpdwThreadParam);
#endif

private:
	unsigned long m_ulThreadID;

protected:
	RZThread();
	virtual ~RZThread();

protected:
	virtual void ThreadProc() = 0;		//�߳�������
};

inline unsigned long RZThread::GetCurrentThreadID() const
{
	return m_ulThreadID;
}

class RZAgent : public RZThread
{
public:
	void SetLocalPort(const RZNetPort&);
	virtual void ConnetToPeer(const RZNetIPAddr&, const RZNetPort&);

protected:
	RZAgent(RZNetConn*);
	virtual ~RZAgent();

protected:
	//"ThreadProc" is still a protected pure abstract function!
	virtual int		RecvPeerData(WAIT_MODE _eWaitMode = ENUM_SYN, long _milliSec = 0);
	virtual void	ParsePacket(const char*, unsigned long) = 0;

protected:
	RZNetConn* m_pNetConn;
};
#endif		//METAOBJECT_H_