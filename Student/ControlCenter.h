#pragma once
#include <stdio.h>
#include "Comment.h"
#include "SocketCenter.h"
#include "ScreenMonitor.h"
//#include "Disk.h"
//#include "Multicast.h"
#include "MulticastDlg.h"
#include "ws2def.h"
class CControlCenter
{
public:
	CControlCenter();
	CControlCenter(STUINFODATA stuInfo);
	~CControlCenter();
	void ConnectClient();
	void SetConnectInfo(char* IP, int port);
	void RecvRequest();
//	void ExecuteRequest(MSGINFO msgInfo);
	void ExecuteRequest(MSGTYPE msgInfo);

	void EndMulticast();

	static DWORD WINAPI OnSendScreenData(LPVOID self);
//	static DWORD WINAPI DownFile(LPVOID self);
//	static DWORD WINAPI OnSetScreenData(LPVOID self);
//	void SetScreenData();
	static DWORD WINAPI OnMulticastDlg(LPVOID self);
	void MulticastDlg();
	static DWORD WINAPI OnShowDlg(LPVOID self);
private:
	SOCKET m_socketMsg;
	CSocketCenter m_socketCenter;
	char m_IP[MAX_PATH];
	int m_port;
	CScreenMonitor m_screenMonitor;
//	CDisk m_disk;
//	HANDLE m_hScreenThread;
// 	bool m_isCreateScreenMonitor;
	CScreenMonitor* m_pScreenMonitor;
//	CMulticast m_multicast;
	CMulticastDlg* m_multicastDlg;
	SOCKET m_socketMulticast;
	sockaddr_in m_addr;
	HANDLE m_hMulticast;
	STUINFODATA m_stuInfo;
	HANDLE m_hScreenDlg;
};

