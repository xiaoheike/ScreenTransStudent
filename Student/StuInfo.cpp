#include "stdafx.h"
#include "StuInfo.h"
#include "SocketCenter.h"


CStuInfo::CStuInfo()
{
}


CStuInfo::~CStuInfo()
{
}

void CStuInfo::SendStuInfo(SOCKET socket, STUINFODATA& stuInfo)
{
	// 通知客户端请求已经接收到了
	CSocketCenter socketCenter;
	socketCenter.SendReadyInfo(socket, STUDENTINFO);
//	int t = sizeof(STUINFODATA);
	socketCenter.SendDataTCP(socket, (char*)&stuInfo, sizeof(STUINFODATA));
}
