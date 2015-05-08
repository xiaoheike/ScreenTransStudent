#pragma once
#include "Comment.h"
#include <stdio.h>
class CSocketCenter
{
public:
	CSocketCenter();
// 	CSocketCenter(SOCKET socket);
	~CSocketCenter();
	//SOCKET InitSocket(char* ip, int port);
	SOCKET InitSocket(char* ip, int port);
	int SendDataTCP(SOCKET socket, const char* buf, int bytes);
	int RecvDataTCP(SOCKET socket, char* buf, int bytes);
	void SendReadyInfo(SOCKET socket, int msgID);
	SOCKET InitMulticastSocket(int port, char* multicastIp);
	int RecvDataUDP(SOCKET socket, char* getBuf, int recvBytes);
	void Cleanup();
private:
//	SOCKET m_socketConn;
};

