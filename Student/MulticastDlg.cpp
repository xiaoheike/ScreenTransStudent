/*******************************************************************
FileName: MulticastDlg.cpp
Author	: xiaoheike
Date	: 2015-05-10 14:46:17
Desc	: 屏幕广播对话框类，实现显示对话框，获得屏幕图像数据以及数据的发送等
*******************************************************************/
#include "stdafx.h"
#include "MulticastDlg.h"

// CMulticastDlg 对话框

IMPLEMENT_DYNAMIC(CMulticastDlg, CDialogEx)

CMulticastDlg::CMulticastDlg(CWnd* pParent/* = NULL*/)
: CDialogEx(CMulticastDlg::IDD, pParent)
, m_isMulticastStop(false)
, m_pBmpTransData(NULL)
{
}

CMulticastDlg::CMulticastDlg(sockaddr_in addr, SOCKET socketMulticast, CWnd* pParent)
: CDialogEx(CMulticastDlg::IDD, pParent)
, m_socketMulticast(socketMulticast)
, m_isMulticastStop(false)
, m_pBmpTransData(NULL)
{
	// 	// 检测是否有内存泄露
	// 	_CrtDumpMemoryLeaks();
	// 	// 内存泄露的位置
	// 	_CrtSetBreakAlloc(919);
}

CMulticastDlg::~CMulticastDlg()
{
	CleanData();
}

void CMulticastDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMulticastDlg, CDialogEx)
	ON_WM_SIZE()
	ON_WM_CLOSE()
	ON_WM_NCDESTROY()
	ON_WM_SYSCOMMAND()
END_MESSAGE_MAP()

// CMulticastDlg 消息处理程序
/******************************************************************
Function	: SetIsMulticastStop
Date		: 2015-05-10 16:02:55
Author		: xiaoheike
Parameter	: isMulticastStop--接收教师机屏幕图像数据停止的标志
Return		: void
Desc		: m_isMulticastStop--true 继续接收教师机屏幕图像数据
m_isMulticastStop--false 接收教师机屏幕图像数据线程结束
******************************************************************/
void CMulticastDlg::SetIsMulticastStop(bool isMulticastStop)
{
	m_isMulticastStop = isMulticastStop;
}

/******************************************************************
Function	: SetSocketMulticast
Date		: 2015-05-10 16:04:25
Author		: xiaoheike
Parameter	: socketMulticast--连接教师机的广播 SOCKET
Return		: void
Desc		: 给广播SOCKET 赋值
******************************************************************/
void CMulticastDlg::SetSocketMulticast(SOCKET socketMulticast)
{
	m_socketMulticast = socketMulticast;
}

/******************************************************************
Function	: SetScreenData
Date		: 2015-05-10 16:05:25
Author		: xiaoheike
Parameter	: 无
Return		: void
Desc		: 接收教师机发来的屏幕图像数据并调用显示方法
******************************************************************/
void CMulticastDlg::SetScreenData()
{
	// 接收到教师机的一幅屏幕图像数据的第几块的序号
	int id = 0;
	while (false == m_isMulticastStop)
	{
		MULTICASTDATA multicastData;
		memset(&multicastData, 0, sizeof(MULTICASTDATA));

		m_socketCenter.RecvDataUDP(m_socketMulticast, (char*)&multicastData,
			sizeof(MULTICASTDATA));

		switch (multicastData.infoType)
		{
		case 1: // 位图数据信息
			SetBmpTransDataNotLast(multicastData, id);
			break;
		case 2: // 接收最后一次发送的数据
			SetBmpTransDataLast(multicastData, id);
			break;
		default:
			MessageBox(_T("未知的图像数据信息"), _T("提示"), MB_OK);
			CleanData();
			exit(1);
		}
		ShowBmp(multicastData);
	}
//	CleanData();
}

/******************************************************************
Function	: ShowBmp
Date		: 2015-05-10 16:06:48
Author		: xiaoheike
Parameter	: multicastData--屏幕图像数据的结构体
Return		: void
Desc		: 将接收到的来自教师机的屏幕图像数据显示到对话框
******************************************************************/
void CMulticastDlg::ShowBmp(MULTICASTDATA &multicastData)
{
	//判断传送完以后是否可以显示图像
	if (multicastData.isShow)
	{
		BYTE* bmpShowData = UnCompressData(m_pBitMapInfo->bmiHeader.biSizeImage,
			multicastData.bmpCompressSize);
		CDC* dc = GetDC();
		if (dc != NULL)
		{
			::StretchDIBits(dc->m_hDC,
				0,
				0,
				m_rectClient.Width(),
				m_rectClient.Height(),
				0,
				0,
				m_pBitMapInfo->bmiHeader.biWidth,
				m_pBitMapInfo->bmiHeader.biHeight,
				bmpShowData, //位图数据
				m_pBitMapInfo, //BITMAPINFO 位图信息头
				DIB_RGB_COLORS,
				SRCCOPY
				);
			ReleaseDC(dc);
		}
		delete[] bmpShowData;
		bmpShowData = NULL;
		DeletepBitMapInfo();
	}
}

/******************************************************************
Function	: SetBmpTransDataNotLast
Date		: 2015-05-10 16:08:21
Author		: xiaoheike
Parameter	: multicastData--屏幕图像数据的结构体
id--一幅屏幕图像数据传输顺序的序号
Return		: void
Desc		: 接收来自教师机的屏幕图像数据（不是一幅图像的最后一次）
******************************************************************/
void CMulticastDlg::SetBmpTransDataNotLast(MULTICASTDATA &multicastData, int& id)
{
	if (m_pBmpTransData == NULL)
	{
		int bmpHeadInfoSize = multicastData.bmpHeadInfo.bmiHeader.biSize;
		m_pBitMapInfo = (BITMAPINFO*)LocalAlloc(LPTR, bmpHeadInfoSize);
		memcpy(m_pBitMapInfo, &multicastData.bmpHeadInfo, bmpHeadInfoSize);

		m_pBmpTransData = new BYTE[multicastData.bmpCompressSize];
		memset(m_pBmpTransData, 0, multicastData.bmpCompressSize);

		id = 0;
	}
	// 如果不相等说明数据包丢失了
	if (id == multicastData.ID)
	{
		memcpy_s(m_pBmpTransData + multicastData.beginPos,
			MULTICAST_TRANS_SIZE, multicastData.transData, MULTICAST_TRANS_SIZE);
		id++;
	}
	else
	{
		DeletepBmpTransData();

		DeletepBitMapInfo();
	}
	//	return id;
}

/******************************************************************
Function	: SetBmpTransDataLast
Date		: 2015-05-10 16:10:59
Author		: xiaoheike
Parameter	: multicastData--屏幕图像数据的结构体
id--一幅屏幕图像数据传输顺序的序号
Return		: void
Desc		: 接收来自教师机的屏幕图像数据（一幅图像的最后一次）
******************************************************************/
void CMulticastDlg::SetBmpTransDataLast(MULTICASTDATA &multicastData, int& id)
{
	if (id == multicastData.ID)
	{
		unsigned long lastTransSize = multicastData.bmpCompressSize - multicastData.beginPos;
		memcpy_s(m_pBmpTransData + multicastData.beginPos, lastTransSize,
			multicastData.transData, lastTransSize);
	}
	else
	{
		multicastData.isShow = false;
		id = 0;
		DeletepBmpTransData();

		DeletepBitMapInfo();
	}
}

/******************************************************************
Function	: DeletepBitMapInfo
Date		: 2015-05-10 16:11:27
Author		: xiaoheike
Parameter	: 无
Return		: void
Desc		: 释放new 的内存
******************************************************************/
void CMulticastDlg::DeletepBitMapInfo()
{
	if (m_pBitMapInfo != NULL)
	{
		LocalFree(m_pBitMapInfo);
		m_pBitMapInfo = NULL;
	}
}

/******************************************************************
Function	: DeletepBmpTransData
Date		: 2015-05-10 16:11:43
Author		: xiaoheike
Parameter	: 无
Return		: void
Desc		: 释放new 的内存
******************************************************************/
void CMulticastDlg::DeletepBmpTransData()
{
	if (m_pBmpTransData != NULL)
	{
		delete[] m_pBmpTransData;
		m_pBmpTransData = NULL;
	}
}

/******************************************************************
Function	: OnSize
Date		: 2015-05-10 16:12:57
Author		: xiaoheike
Parameter	:
Return		: void
Desc		: 响应WM_SIZE 消息获得对话框的客户区尺寸
******************************************************************/
void CMulticastDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO:  在此处添加消息处理程序代码
	GetClientRect(&m_rectClient);
}

void CMulticastDlg::CleanData()
{
	DeletepBmpTransData();

	DeletepBitMapInfo();
}

/******************************************************************
Function	: OnInitDialog
Date		: 2015-05-10 16:12:06
Author		: xiaoheike
Parameter	: 无
Return		: BOOL
Desc		: 响应WM_InitDialog 消息对话框创建后第一个调用的方法，让对话框全屏显示并且屏幕
关闭按钮
******************************************************************/
BOOL CMulticastDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	// 让模态对话框最大化显示
	ShowWindow(SW_NORMAL);
	// 使对话框的关闭按钮无效
	CMenu *pSysMenu = GetSystemMenu(FALSE);
	ASSERT(pSysMenu != NULL);
	pSysMenu->EnableMenuItem(SC_CLOSE, MF_DISABLED);
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常:  OCX 属性页应返回 FALSE
}

void CMulticastDlg::OnClose()
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	CDialogEx::OnClose();
}

/******************************************************************
Function	: UnCompressData
Date		: 2015-05-10 16:14:13
Author		: xiaoheike
Parameter	: biSizeImage--图像数据未压缩前的大小
bmpCompressSize--图像数据压缩后的大小
Return		: BYTE*
Desc		: 解缩接收到来自教师机的屏幕图像数据
******************************************************************/
BYTE* CMulticastDlg::UnCompressData(uLongf biSizeImage, unsigned long bmpCompressSize)
{
	uLongf unCompressDataLen = (uLongf)((biSizeImage + 12)*(100.1 / 100)) + 1;

	BYTE* pUnCompressData = new BYTE[unCompressDataLen];
	int err = uncompress(pUnCompressData, &unCompressDataLen,
		m_pBmpTransData, bmpCompressSize);
	if (err != Z_OK) {
		CString str;
		str.Format(_T("uncompess error = %d,unCompressDataLen = %d, biSizeImage = %d, bmpCompressSize = %d"),
			err, unCompressDataLen, biSizeImage, bmpCompressSize);
		MessageBox(str);
		delete[] pUnCompressData;
		pUnCompressData = NULL;
		delete[]m_pBmpTransData;
		m_pBmpTransData = NULL;
		exit(0);
	}

	BYTE* bmpShowData = new BYTE[unCompressDataLen];
	memcpy(bmpShowData, pUnCompressData, unCompressDataLen);

	delete[] pUnCompressData;
	pUnCompressData = NULL;
	DeletepBmpTransData();
	// 	delete[]m_pBmpTransData;
	// 	m_pBmpTransData = NULL;
	return bmpShowData;
}

/******************************************************************
Function	: CloseModalDlg
Date		: 2015-05-10 15:55:00
Author		: xiaoheike
Parameter	: 无
Return		: void
Desc		: 关闭Modal 对话框
******************************************************************/
void CMulticastDlg::CloseModalDlg()
{
	CDialog::OnCancel();
}

/******************************************************************
Function	: OnNcDestroy
Date		: 2015-05-10 15:54:09
Author		: xiaoheike
Parameter	: 无
Return		: void
Desc		: 可以在这个方法中释放自己的 内存(delete this)
******************************************************************/
void CMulticastDlg::OnNcDestroy()
{
	CDialogEx::OnNcDestroy();
}

/******************************************************************
Function	: OnSysCommand
Date		: 2015-05-10 15:55:37
Author		: xiaoheike
Parameter	: nID--
Return		:
Desc		: 响应WM_SYSCOMMAND 消息，在方法内添加代码屏蔽ALT+F4快捷键
******************************************************************/
void CMulticastDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	// 如果是ALT + F4 则不处理这个消息
	if ((nID & 0xFFF0) != SC_CLOSE)
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}