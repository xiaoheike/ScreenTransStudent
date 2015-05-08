// MulticastDlg.cpp : 实现文件
//

#include "stdafx.h"
//#include "ScreenMonitorMFC.h"
#include "MulticastDlg.h"
#include "afxdialogex.h"
#include <io.h>
#include <fcntl.h>

//把下面的函数加到你初始化的地方，然后你就可以使用printf函数了
void InitConsoleWindow()
{
	int nCrt = 0;
	FILE* fp;
	AllocConsole();
	nCrt = _open_osfhandle((long)GetStdHandle(STD_OUTPUT_HANDLE), _O_TEXT);
	fp = _fdopen(nCrt, "w");
	*stdout = *fp;
	setvbuf(stdout, NULL, _IONBF, 0);
}


// CMulticastDlg 对话框

IMPLEMENT_DYNAMIC(CMulticastDlg, CDialogEx)

CMulticastDlg::CMulticastDlg(CWnd* pParent/* = NULL*/)
	: CDialogEx(CMulticastDlg::IDD, pParent)
	, m_isStop(false)
//	, m_addr(addr)
	, m_pBmpCompressData(NULL)
{

}

CMulticastDlg::CMulticastDlg(sockaddr_in addr, SOCKET socketMulticast, /*SOCKET socketMsg,*/ CWnd* pParent)
	: CDialogEx(CMulticastDlg::IDD, pParent)
	, m_socketMulticast(socketMulticast)
//	, m_socketMsg(socketMsg)
	, m_isStop(false)
//	, m_addr(addr)
	, m_pBmpCompressData(NULL)
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
END_MESSAGE_MAP()



// CMulticastDlg 消息处理程序
void CMulticastDlg::SetIsStop(bool isStop)
{
	m_isStop = isStop;
}
	

void CMulticastDlg::SetMulticastSocket(SOCKET multicastSocket)
{
	m_socketMulticast = multicastSocket;
}
void CMulticastDlg::SetScreenData()
{
	int id = 0;
	while (false == m_isStop)
	{
		MULTICASTDATA multicastData;
		memset(&multicastData, 0, sizeof(MULTICASTDATA));
		
		m_socketCenter.RecvDataUDP(m_socketMulticast, (char*)&multicastData, 
			sizeof(MULTICASTDATA));


		switch (multicastData.infoType)
		{
		case 0:// 位图头信息
		{
			int bmpHeadInfoSize = multicastData.bmpHeadInfo.bmiHeader.biSize;
			m_pBMPINFO = (BITMAPINFO*)LocalAlloc(LPTR, bmpHeadInfoSize);
			memcpy(m_pBMPINFO, &multicastData.bmpHeadInfo, bmpHeadInfoSize);
			m_pBmpCompressData = new BYTE[multicastData.bmpCompressSize];
			memset(m_pBmpCompressData, 0, multicastData.bmpCompressSize);
		}
			break;
		case 1: // 位图数据信息
			if (m_pBmpCompressData == NULL)
			{
				int bmpHeadInfoSize = multicastData.bmpHeadInfo.bmiHeader.biSize;
				m_pBMPINFO = (BITMAPINFO*)LocalAlloc(LPTR, bmpHeadInfoSize);
				memcpy(m_pBMPINFO, &multicastData.bmpHeadInfo, bmpHeadInfoSize);

				m_pBmpCompressData = new BYTE[multicastData.bmpCompressSize];
				memset(m_pBmpCompressData, 0, multicastData.bmpCompressSize);

				id = 0;
// 				MessageBox(_T("m_pBmpCompressData == NULL"));
// 				exit(1);
			}
			if (id == multicastData.ID)
			{
				memcpy_s(m_pBmpCompressData + multicastData.beginPos,
					MULTICAST_TRANS_SIZE, multicastData.transData, MULTICAST_TRANS_SIZE);
				id++;
			}
			else
			{
				if (m_pBmpCompressData != NULL)
				{
					delete[] m_pBmpCompressData;
					m_pBmpCompressData = NULL;
				}
				if (m_pBMPINFO != NULL)
				{
					LocalFree(m_pBMPINFO);
					m_pBMPINFO = NULL;
				}
			}
			break;
		case 2: // 接收最后一次发送的数据
		{
			if (id == multicastData.ID)
			{
				unsigned long lastTransSize = multicastData.bmpCompressSize - multicastData.beginPos;
				memcpy_s(m_pBmpCompressData + multicastData.beginPos, lastTransSize,
					multicastData.transData, lastTransSize);
			}
			else
			{
// 				InitConsoleWindow();
// 				printf("bid = %d, mulID = %d\n", id, multicastData.ID);
				multicastData.isShow = false;
				id = 0;
				if (m_pBmpCompressData != NULL)
				{
					delete[] m_pBmpCompressData;
					m_pBmpCompressData = NULL;
				}
				if (m_pBMPINFO != NULL)
				{
					LocalFree(m_pBMPINFO);
					m_pBMPINFO = NULL;
				}
			}
		}
			break;
		default:
			MessageBox(_T("未知的图像数据ID"), _T("提示"), MB_OK);
			CleanData();
			exit(1);
		}
		//判断传送完以后是否可以显示图像
		if (multicastData.isShow)
		{
			BYTE* bmpShowData = UnCompressData(m_pBMPINFO->bmiHeader.biSizeImage, 
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
					m_pBMPINFO->bmiHeader.biWidth,
					m_pBMPINFO->bmiHeader.biHeight,
					bmpShowData, //位图数据
					m_pBMPINFO, //BITMAPINFO 位图信息头
					DIB_RGB_COLORS,
					SRCCOPY
					);
				ReleaseDC(dc);
			}
			delete[] bmpShowData;
			bmpShowData = NULL;
			LocalFree(m_pBMPINFO);
			m_pBMPINFO = NULL;
		}
	}
	CleanData();
}

void CMulticastDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO:  在此处添加消息处理程序代码
	GetClientRect(&m_rectClient);
}

void CMulticastDlg::CleanData()
{
	if (m_pBmpCompressData != NULL)
	{
		delete[] m_pBmpCompressData;
		m_pBmpCompressData = NULL;
	}
	if (m_pBMPINFO != NULL)
	{
		LocalFree(m_pBMPINFO);
		m_pBMPINFO = NULL;
	}
}


BOOL CMulticastDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	// 让模态对话框最大化显示
	ShowWindow(SW_NORMAL);
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常:  OCX 属性页应返回 FALSE
}


void CMulticastDlg::OnClose()
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	CDialogEx::OnClose();
}

BYTE* CMulticastDlg::UnCompressData(uLongf biSizeImage, unsigned long bmpCompressSize)
{
	uLongf unCompressDataLen = (uLongf)((biSizeImage + 12)*(100.1 / 100)) + 1;

	BYTE* pUnCompressData = new BYTE[unCompressDataLen];
	int err = uncompress(pUnCompressData, &unCompressDataLen, 
		m_pBmpCompressData, bmpCompressSize);
	if (err != Z_OK) {
		CString str;
		str.Format(_T("uncompess error = %d,unCompressDataLen = %d, biSizeImage = %d, bmpCompressSize = %d"), 
			err, unCompressDataLen, biSizeImage, bmpCompressSize);
		MessageBox(str);
		delete[] pUnCompressData;
		pUnCompressData = NULL;
		delete[]m_pBmpCompressData;
		m_pBmpCompressData = NULL;
		exit(0);
	}

	BYTE* bmpShowData = new BYTE[unCompressDataLen];
	memcpy(bmpShowData, pUnCompressData, unCompressDataLen);

	delete[] pUnCompressData;
	pUnCompressData = NULL;
	delete[]m_pBmpCompressData;
	m_pBmpCompressData = NULL;
	return bmpShowData;
}


BOOL CMulticastDlg::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO:  在此添加专用代码和/或调用基类
	int width = GetSystemMetrics(SM_CXSCREEN); //获取屏幕的宽度
	int height = GetSystemMetrics(SM_CYSCREEN); //获取屏幕的高度
// 	CRect rect(width, height);
// 	rect.h
	AfxGetMainWnd()->MoveWindow(0, 0, width, height, true);
//	SetWindowPos(0, 0, width, height, true);
//	SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE, 0);
	return CDialogEx::PreCreateWindow(cs);
}

void CMulticastDlg::CloseDlg()
{
	CDialog::OnCancel();
//	return false;
}


void CMulticastDlg::OnNcDestroy()
{
	CDialogEx::OnNcDestroy();

	// TODO:  在此处添加消息处理程序代码
//	delete this;
}
