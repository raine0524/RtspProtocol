#include "SessDescribe.h"
#include "Logger.h"

RZSessDescribe::RZSessDescribe()
	:m_stSessDes(),
	 m_stCacheInfo()
{
}

void RZSessDescribe::SetTypeValue(const std::string& _strSDP)
{
	TimeDescription stTimeDes;
	MediaDescription stMediaDes;
	//sdp�ı��еĸ�ʽΪ<type>=<value>["\r\n"]������<type>��һ����ĸ��<value>��һ���ı���
	std::vector<std::string> vTextLine;
	std::vector<std::string> vTextLineList = RZStream::StreamSplit(_strSDP, "\r\n");

	for (std::vector<std::string>::const_iterator iter = vTextLineList.begin();
			iter != vTextLineList.end(); iter++)
	{
		vTextLine.push_back(iter->substr(0, 1));		//����ռ�õ�ǰ�ַ����ĵ�һ���ֽ�
		vTextLine.push_back(iter->substr(2));			//�ӵڶ����ֽڿ�ʼ��ʾ�����͵�ֵ
		//һ�������޷�ʶ������ͣ���ô�򵥺���
		switch(vTextLine[0][0])		//����//**ע�͵����ͱ�ʾ��ѡ��
		{
		case 'v':		m_stSessDes.v	= vTextLine[1];	break;
		case 'o':		m_stSessDes.o	= vTextLine[1];	break;
		case 's':		m_stSessDes.s	= vTextLine[1];	break;
		case 'i':		m_stSessDes.i	= vTextLine[1];	break;		//**
		case 'u':		m_stSessDes.u	= vTextLine[1];	break;		//**
		case 'e':		m_stSessDes.e	= vTextLine[1];	break;		//**
		case 'p':		m_stSessDes.p	= vTextLine[1];	break;		//**
		case 'c':		m_stSessDes.c	= vTextLine[1];	break;		//**
		case 'b':		m_stSessDes.b.push_back(vTextLine[1]);		break;		//**

		case 't':
			{
				//��TimeDescription�ṹ�У����ֶ�t�ض�����
				stTimeDes = TimeDescription();
				stTimeDes.t	= vTextLine[1];
				//����һ���ѵ�������β�ˣ���ֱ���˳�������
				if ((++iter) == vTextLineList.end()) 
				{
					iter--;	
					break; 
				}
				vTextLine.clear();
				vTextLine.push_back(iter->substr(0, 1));
				vTextLine.push_back(iter->substr(2));
				if (vTextLine[0][0] == 'r') 
					stTimeDes.r	= vTextLine[1];		//**
				else 
					iter--;
				m_stSessDes.vtd.push_back(stTimeDes);
				break;
			}

		case 'z':		m_stSessDes.z	= vTextLine[1];	break;		//**
		case 'k':		m_stSessDes.k	= vTextLine[1];	break;		//**
		case 'a':		m_stSessDes.a.push_back(vTextLine[1]);	break;		//**

		case 'm':
			{
				//��MediaDescription�ṹ�У����ֶ�m�ض�����
				stMediaDes = MediaDescription();		//�����
				stMediaDes.m	= vTextLine[1];
				for (iter++; iter != vTextLineList.end(); iter++)
				{
					vTextLine.clear();
					vTextLine.push_back(iter->substr(0, 1));
					vTextLine.push_back(iter->substr(2));
					if (vTextLine[0] == "m")
						break;			//��ʱ�Ѿ�������һ��MediaDescription�ṹ���У�ֱ���˳�
					switch(vTextLine[0][0]) 
					{
					case 'i':		stMediaDes.i		= vTextLine[1];	break;		//**
					case 'c':		stMediaDes.c	= vTextLine[1];	break;		//**
					case 'b':		stMediaDes.b.push_back(vTextLine[1]);	break;		//**
					case 'k':		stMediaDes.k = vTextLine[1];	break;		//**
					case 'a':		stMediaDes.a.push_back(vTextLine[1]);	break;		//**
					}
				}
				m_stSessDes.vmd.push_back(stMediaDes);
				iter--;
				break;
			}
		}
		vTextLine.clear();
	}

	//�������sdp���Ľ�������������������������
	SetAVStreamIndex();
	SetAVStreamID(m_stCacheInfo.pASDescription);
	SetAVStreamID(m_stCacheInfo.pVSDescription);
}

void RZSessDescribe::SetAVStreamIndex()
{
	std::vector<std::string> vTextLine;
	for (std::vector<MediaDescription>::iterator iter = m_stSessDes.vmd.begin();
			iter != m_stSessDes.vmd.end(); iter++)
	{
		//����m��ֵ�Կո���ָ�
		vTextLine = RZStream::StreamSplit(iter->m, " ");
		if (vTextLine[0] == "audio")				//��Ƶ��
			m_stCacheInfo.pASDescription = &(*iter);
		else if (vTextLine[0] == "video")		//��Ƶ��
			m_stCacheInfo.pVSDescription = &(*iter);
		else			//δ֪������������
			;
	}
}

void RZSessDescribe::SetAVStreamID(const MediaDescription* _pMediaDes)
{
	if (_pMediaDes == NULL)
		return;

	std::string strMediaID = "";
	std::vector<std::string> vTextLine;
	for (std::vector<std::string>::const_iterator iter = (_pMediaDes->a).begin(); 
			iter != (_pMediaDes->a).end(); iter++)
	{
		vTextLine = RZStream::StreamSplit(*iter, ":");		//��������ֵ֮����:�ָ�
		if (vTextLine[0] == "control")
		{
			strMediaID = vTextLine[1];
			break;
		}
	}
	if (_pMediaDes == m_stCacheInfo.pASDescription)
		m_stCacheInfo.strAudioID = strMediaID;
	else if (_pMediaDes = m_stCacheInfo.pVSDescription)
		m_stCacheInfo.strVideoID = strMediaID;
	else
		;
}

int RZSessDescribe::GetSampFrequence(STREAM_MEDIA_TYPE _eMedia) const
{
	if (_eMedia == ENUM_MEDIA_UNK)
		Log::ERR("Unknown media type, please set media type correctly.\n");

	int iSampFreq = 0;
	MediaDescription* pStreamDes = NULL;
	if (_eMedia == ENUM_AUDIO)
		pStreamDes = m_stCacheInfo.pASDescription;
	else			//_eMedia == ENUM_VIDEO
		pStreamDes = m_stCacheInfo.pVSDescription;
	std::vector<std::string> vTextLine;
	for (std::vector<std::string>::const_iterator iter = (pStreamDes->a).begin();
			iter != (pStreamDes->a).end(); iter++)
	{
		vTextLine = RZStream::StreamSplit(*iter, ":");
		if (vTextLine[0] == "rtpmap")
		{
			vTextLine = RZStream::StreamSplit(vTextLine[1], "/");
			iSampFreq = RZTypeConvert::StrToInt(vTextLine[1], 10);
		}
	}
	return iSampFreq;
}