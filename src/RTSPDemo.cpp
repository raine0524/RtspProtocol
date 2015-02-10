// RTSPDemo.cpp : 定义控制台应用程序的入口点。
//

#include "RTSPAgent.h"

const std::string CServerAddr = "192.168.1.136";
const std::string CRequestFile = "sample.mp3";

int main(int argc, char* argv[])
{
	HOSTENT* host = NULL;
	host = gethostbyname("video.fjtu.com.cn");
	if (!host)
		return EXIT_FAILURE;
	std::string strIP;
	int nPort;
	sockaddr_in sa;
	memcpy(&sa.sin_addr.S_un.S_addr, host->h_addr_list[0], host->h_length);
	strIP = inet_ntoa(sa.sin_addr);
	nPort = ntohs(sa.sin_port);

	RTSPAgent rAgent;
	rAgent.ConnectToServer(strIP, nPort);
	rAgent.SetRequestFile("vs01/yhhy/yhhy_01.rm");
	rAgent.StartTask(ENUM_ACCEPTAS);

	RZTime::Sleep(1000);
	RZThread::WaitPeerThreadStop(rAgent);
	return 0;
}

