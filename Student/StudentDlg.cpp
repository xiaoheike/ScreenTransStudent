
// StudentDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Student.h"
#include "StudentDlg.h"
#include "afxdialogex.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CStudentDlg 对话框



CStudentDlg::CStudentDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CStudentDlg::IDD, pParent)
	, m_stuName(_T("陈允进"))
	, m_stuClass(_T("软件五班"))
	, m_stuID(_T("20112798"))
	, m_stuMajor(_T("软件工程"))
	, m_pCtrlCenter(NULL)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CStudentDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDINAME, m_stuName);
	DDX_Text(pDX, IDC_EDTCLASS, m_stuClass);
	DDX_Text(pDX, IDC_EDTID, m_stuID);
	DDX_Text(pDX, IDC_EDTMAJOR, m_stuMajor);
}

BEGIN_MESSAGE_MAP(CStudentDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CStudentDlg::OnBnClickedOk)
	ON_MESSAGE(WM_SHOWTASKICON, &CStudentDlg::OnShowTaskIcon)
//	ON_COMMAND(IDM_SHOW_WINDOW, &CStudentDlg::OnShowDlg)
	ON_BN_CLICKED(IDCANCEL, &CStudentDlg::OnBnClickedCancel)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CStudentDlg 消息处理程序

BOOL CStudentDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO:  在此添加额外的初始化代码
	// 在托盘显示图标
	InitNofIconData();
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CStudentDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CStudentDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CStudentDlg::OnBnClickedOk()
{

	if (false == IsEdtDataEmpty())
	{
//		IsEdtDataEmpty();
		// 隐藏窗口
		ShowWindow(SW_HIDE);
		STUINFODATA stuInfoData;
		wcscpy_s(stuInfoData.m_stuName, m_stuName);
		wcscpy_s(stuInfoData.m_stuID, m_stuID);
		wcscpy_s(stuInfoData.m_stuMajor, m_stuMajor);
		wcscpy_s(stuInfoData.m_stuClass, m_stuClass);

		m_pCtrlCenter = new CControlCenter(stuInfoData);
		// 创建一个线程用于监听教师机的请求
		::CloseHandle(CreateThread(NULL, 0, OnBeginServer, (LPVOID)m_pCtrlCenter, 0, 0));
	}

}

DWORD WINAPI CStudentDlg::OnBeginServer(LPVOID self)
{
	CControlCenter* pCtrlCenter = (CControlCenter*)self;
	pCtrlCenter->SetConnectInfo(CONNECT_IP, CONNECT_PORT);
	pCtrlCenter->ConnectClient();
//	pCtrlCenter->RecvRequest();
 	delete pCtrlCenter;
	pCtrlCenter = NULL;
	return 0;
}


void CStudentDlg::InitNofIconData()
{
	m_notifyIconData.cbSize = (DWORD)sizeof(NOTIFYICONDATA);
	m_notifyIconData.hWnd = m_hWnd;
	m_notifyIconData.uID = IDI_TASKBARICON;
	m_notifyIconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	m_notifyIconData.uCallbackMessage = WM_SHOWTASKICON; // 自定义托盘菜单消息
	m_notifyIconData.hIcon = ::LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_TASKBARICON));
	wcscpy_s(m_notifyIconData.szTip, _T("电子教室系统"));
	Shell_NotifyIcon(NIM_ADD, &m_notifyIconData); // 在托盘区添加图标
}

LRESULT CStudentDlg::OnShowTaskIcon(WPARAM wParam, LPARAM lParam)
{
	// 判断图标ID是否相符
	if (wParam != IDI_TASKBARICON) 
		return 1;
	switch (lParam)
	{
	case WM_RBUTTONUP:  // 按鼠标右键
	{
		CPoint point;
		GetCursorPos(&point);
		CMenu menu;
		menu.CreatePopupMenu();
//		menu.AppendMenu(MF_STRING, IDM_SHOW_WINDOW, _T("显示"));
		menu.AppendMenu(MF_STRING, WM_DESTROY, _T("退出"));
		SetForegroundWindow();  // 解决菜单失去焦点不消失的BUG
		menu.TrackPopupMenu(TPM_LEFTALIGN, point.x, point.y, this);
		menu.DestroyMenu();
	}
		break;
// 	case WM_LBUTTONDBLCLK: // 双击鼠标左键
// 		OnShowDlg();
	}
	return 0;
}

// void CStudentDlg::OnShowDlg()
// {
// 	if (IsWindowVisible())
// 	{
// 		OnBnClickedCancel();
// 
// // 		SendMessage(WM_SYSCOMMAND, SC_RESTORE, (LPARAM)m_hWnd);
// 	}
// 	else
// 		ShowWindow(SW_SHOW);
// }

bool CStudentDlg::IsEdtDataEmpty()
{
	bool isEmpty =  false;
	UpdateData(TRUE);
// 	isEmpty = false;
	CString warmStr(_T(""));
	if (m_stuName.IsEmpty())
	{
		warmStr += _T("姓名不能为空");
		isEmpty = true;
	}
	if (m_stuID.IsEmpty())
	{
		warmStr += _T("学号不能为空");
		isEmpty = true;
	}
	if (m_stuMajor.IsEmpty())
	{
		warmStr += _T("专业不能为空");
		isEmpty = true;
	}
	if (m_stuClass.IsEmpty())
	{
		warmStr += _T("班级不能为空");
		isEmpty = true;
	}
	if (true == isEmpty)
	{
		MessageBoxW(warmStr);
	}
	return isEmpty;
}


void CStudentDlg::OnBnClickedCancel()
{
	// TODO:  在此添加控件通知处理程序代码
	if (m_pCtrlCenter != NULL)
	{
		delete m_pCtrlCenter;
		m_pCtrlCenter = NULL;
	}
	Shell_NotifyIcon(NIM_DELETE, &m_notifyIconData); // 在托盘区添加图标
	CDialogEx::OnCancel();
}


void CStudentDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO:  在此处添加消息处理程序代码
	Shell_NotifyIcon(NIM_DELETE, &m_notifyIconData); // 在托盘区添加图标
}
