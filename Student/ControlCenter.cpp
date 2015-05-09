/*******************************************************************
FileName: ControlCenter.cpp
Author	: xiaoheike
Date	: 2015-05-09 10:27:18
Desc	: 接收来自教师机的消息并进行分配处理
*******************************************************************/
#include "stdafx.h"
#include "ControlCenter.h"
#include "StuInfo.h"

/******************************************************************
Function	: CControlCenter
Date		: 2015-05-09 10:28:59
Author		: xiaoheike
Parameter	: stuInfo--自定义结构体，学生的信息
Return		: 无
Desc		: 构造方法
******************************************************************/
CControlCenter::CControlCenter(STUINFODATA stuInfo)
: m_port(0)
, m_socketMsg(INVALID_SOCKET)
, m_pScreenMonitor(NULL)
, m_stuInfo(stuInfo)
, m_pMulticastDlg(NULL)
{
}

CControlCenter::~CControlCenter()
{
	DeleteSocketMsg();

	DeletepScreenMonitor();

	DeleteSocketMulticast();

	DeletepMulticastDlg();
}

/******************************************************************
Function	: DeletepMulticastDlg
Date		: 2015-05-09 16:52:40
Author		: xiaoheike
Parameter	: 无
Return		: void
Desc		: 释放new 分配的内存
******************************************************************/
void CControlCenter::DeletepMulticastDlg()
{
	if (m_pMulticastDlg != NULL)
	{
		delete m_pMulticastDlg;
		m_pMulticastDlg = NULL;
	}
}

/******************************************************************
Function	: DeleteSocketMulticast
Date		: 2015-05-09 16:53:34
Author		: xiaoheike
Parameter	: 无
Return		: void
Desc		: 关闭SOCKET 连接
******************************************************************/
void CControlCenter::DeleteSocketMulticast()
{
	if (m_socketMulticast != INVALID_SOCKET)
	{
		closesocket(m_socketMulticast);
		m_socketMulticast = NULL;
	}
}

/******************************************************************
Function	: DeletepScreenMonitor
Date		: 2015-05-09 16:54:11
Author		: xiaoheike
Parameter	: 无
Return		: void
Desc		: 释放new 分配的内存
******************************************************************/
void CControlCenter::DeletepScreenMonitor()
{
	if (m_pScreenMonitor != NULL)
	{
		delete m_pScreenMonitor;
		m_pScreenMonitor = NULL;
	}
}

/******************************************************************
Function	: DeleteSocketMsg
Date		: 2015-05-09 16:55:02
Author		: xiaoheike
Parameter	: 无
Return		: void
Desc		: 关闭SOCKET 连接
******************************************************************/
void CControlCenter::DeleteSocketMsg()
{
	if (m_socketMsg != INVALID_SOCKET)
	{
		closesocket(m_socketMsg);
		m_socketMsg = INVALID_SOCKET;
	}
}

/******************************************************************
Function	: SetConnectInfo
Date		: 2015-05-09 10:39:10
Author		: xiaoheike
Parameter	: IP--IP地址
port--端口号
Return		: void
Desc		: 设置IP 地址和端口的信息
******************************************************************/
void CControlCenter::SetConnectInfo(char* IP, int port)
{
	strcpy_s(m_IP, IP);
	m_port = port;
}

/******************************************************************
Function	: ConnectClient
Date		: 2015-05-09 10:40:37
Author		: xiaoheike
Parameter	: 无
Return		: void
Desc		: 连接教师机，得到SOCKET
******************************************************************/
void CControlCenter::ConnectClient()
{
	while (true)
	{
		m_socketMsg = m_socketCenter.InitSocket(m_IP, m_port);
		if (m_socketMsg == SOCKET_ERROR)
		{
			Sleep(3000);
			continue;
		}
		else
		{
			break;
		}
	}
	RecvRequest();
}

/******************************************************************
Function	: RecvRequest
Date		: 2015-05-09 10:48:24
Author		: xiaoheike
Parameter	: 无
Return		: void
Desc		: 不断监听来处教师机的请求
******************************************************************/
void CControlCenter::RecvRequest()
{
	// 监听教师机广播的SOCKET（IP 组播技术）
	m_socketMulticast = m_socketCenter.InitMulticastSocket(MULTICAST_TRANS_PORT,
		MULTICAST_IP);

	MSGTYPE msgType;
	while (true)
	{
		memset(&msgType, 0, sizeof(MSGTYPE));
		// 不断监听来自客户端的请求
		int result = m_socketCenter.RecvDataTCP(m_socketMsg,
			(char*)&msgType, sizeof(MSGTYPE));
		if (result == 0)
		{
			AfxMessageBox(_T("result = 0"));
			break;
		}
		this->ExecuteRequest(msgType);
	}
}

/******************************************************************
Function	: ExecuteRequest
Date		: 2015-05-09 10:50:44
Author		: xiaoheike
Parameter	: msgType--自定义结构体，消息的信息
Return		: void
Desc		: 执行接收到的请求
******************************************************************/
void CControlCenter::ExecuteRequest(MSGTYPE msgType)
{
	switch (msgType.msgID)
	{
	case STUDENTINFO:
		SendStuInfo();
		break;
	case BEGINSCREENMONITOR:
		BeginScreenMonitor();
		break;
	case ENDSCREENMONITOR:
		EndScreenMonitor();
		break;
	case BEGINMULTICAST:
		BeginMulticast();
		break;
	case ENDMULTICAST:
		EndMulticast();
		break;
	default:
		AfxMessageBox(_T("Unkonw message"));
		return;
	}
}

/******************************************************************
Function	: BeginMulticast
Date		: 2015-05-09 16:56:18
Author		: xiaoheike
Parameter	: 无
Return		: void
Desc		: 接收来自教师机的屏幕图像数据并调用显示方法
******************************************************************/
void CControlCenter::BeginMulticast()
{
	DeletepMulticastDlg();
	m_pMulticastDlg = new CMulticastDlg();
	m_pMulticastDlg->SetMulticastSocket(m_socketMulticast);
	m_pMulticastDlg->SetIsStop(false);
	CloseHandle(::CreateThread(0, 0, OnShowMulticastDlg, (LPVOID)m_pMulticastDlg, 0, NULL));
	m_hMulticast = ::CreateThread(0, 0, OnBeginMulticast,
		(LPVOID)m_pMulticastDlg, 0, NULL);
}

/******************************************************************
Function	: OnBeginMulticast
Date		: 2015-05-09 17:12:14
Author		: xiaoheike
Parameter	: self--CMulticast 类对象
Return		: DWORD WINAPI
Desc		: 教师机广播线程运行方法
******************************************************************/
DWORD WINAPI CControlCenter::OnBeginMulticast(LPVOID self)
{
	CMulticastDlg* multicastDlg = (CMulticastDlg*)self;
	multicastDlg->SetScreenData();
	//	multicastDlg->DoModal();
	return 0;
}

/******************************************************************
Function	: OnShowMulticastDlg
Date		: 2015-05-09 17:23:20
Author		: xiaoheike
Parameter	: self--CMulticastDlg 类对象
Return		: DWORD WINAPI
Desc		: 显示教师机广播模态对话框的线程运行方法
******************************************************************/
DWORD WINAPI CControlCenter::OnShowMulticastDlg(LPVOID self)
{
	CMulticastDlg* multicastDlg = (CMulticastDlg*)self;
	multicastDlg->DoModal();
	// 	multicastDlg->Create(IDD_MULTICAST);
	// 	multicastDlg->ShowWindow(SW_NORMAL);
	return 0;
}

/******************************************************************
Function	: EndMulticast
Date		: 2015-05-09 16:59:31
Author		: xiaoheike
Parameter	: 无
Return		: void
Desc		: 停止接收来自教师机的屏幕图像数据
to-do       : 线程会上面SOCKET 的RecvFrom 上而导致线程无法结束
******************************************************************/
void CControlCenter::EndMulticast()
{
	m_pMulticastDlg->SetIsStop(true);
	DWORD exitCode = 0;
	// 关闭模态对话框
	m_pMulticastDlg->CloseDlg();

	GetExitCodeThread(m_hMulticast, &exitCode);
	while (STILL_ACTIVE == exitCode)
	{
		Sleep(100);
		GetExitCodeThread(m_hMulticast, &exitCode);
	}
	//	TerminateThread(m_hMulticast, exitCode);

	CloseHandle(m_hMulticast);
	m_hMulticast = NULL;
	DeletepMulticastDlg();
	//	delete m_pMulticastDlg;
	//	m_pMulticastDlg = NULL;
}

/******************************************************************
Function	: BeginScreenBmpData
Date		: 2015-05-09 11:05:34
Author		: xiaoheike
Parameter	: 无
Return		: void
Desc		: 发送当前机子的屏幕图像数据到教师机
******************************************************************/
void CControlCenter::BeginScreenMonitor()
{
	DeletepScreenMonitor();
	m_pScreenMonitor = new CScreenMonitor();
	m_pScreenMonitor->SetSocket(m_socketMsg);
	m_pScreenMonitor->SetSendFlag(true);
	m_hScreenDlg = (::CreateThread(0, 0, OnBeginScreenMonitor,
		(LPVOID)m_pScreenMonitor, 0, NULL));
}

/******************************************************************
Function	: OnBeginScreenMonitor
Date		: 2015-05-09 17:20:57
Author		: xiaoheike
Parameter	: self--CScreenMonitor 类对象
Return		: DWORD WINAPI
Desc		: 学生机屏幕监控线程运行方法
******************************************************************/
DWORD WINAPI CControlCenter::OnBeginScreenMonitor(LPVOID self)
{
	CScreenMonitor* screenThread = (CScreenMonitor*)self;
	screenThread->SendScreenData();
	return 0;
}

/******************************************************************
Function	: EndScreenMonitor
Date		: 2015-05-09 16:57:10
Author		: xiaoheike
Parameter	: 无
Return		: void
Desc		: 停止发送本机的屏幕图像数据到教师机
******************************************************************/
void CControlCenter::EndScreenMonitor()
{
	m_pScreenMonitor->SetSendFlag(false);
	DWORD exitCode = 0;
	::GetExitCodeThread(m_hScreenDlg, &exitCode);
	while (exitCode == STILL_ACTIVE)
	{
		Sleep(10);
		::GetExitCodeThread(m_hScreenDlg, &exitCode);
	}
	CloseHandle(m_hScreenDlg);
	delete m_pScreenMonitor;
	m_pScreenMonitor = NULL;
}

/******************************************************************
Function	: SendStuInfo
Date		: 2015-05-09 17:02:05
Author		: xiaoheike
Parameter	: 无
Return		: void
Desc		: 发送登录的学生的信息到教师端
******************************************************************/
void CControlCenter::SendStuInfo()
{
	CStuInfo StuInfo;
	StuInfo.SendStuInfo(m_socketMsg, m_stuInfo);
}