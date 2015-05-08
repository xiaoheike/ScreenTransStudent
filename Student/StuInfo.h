#pragma once
#include "Comment.h"
class CStuInfo
{
public:
	CStuInfo();
	~CStuInfo();
	void SendStuInfo(SOCKET socket, STUINFODATA& stuInfo);
private:
//	CSocketCenter m_socketCenter;
};

