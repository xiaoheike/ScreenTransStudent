
// StudentDlg.h : 头文件
//

#pragma once
#include "ControlCenter.h"
#define WM_SHOWTASKICON WM_USER + 4
#define IDM_SHOW_WINDOW WM_USER + 5
// CStudentDlg 对话框
class CStudentDlg : public CDialogEx
{
// 构造
public:
	CStudentDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_STUDENT_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnShowTaskIcon(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedOk();

	bool IsEdtDataEmpty();

	void InitNofIconData();

	static DWORD WINAPI OnBeginServer(LPVOID self);
//	void BeginServer();
//	afx_msg void OnShowDlg();
public:
private:
	CString m_stuName;
	// 学生的班级
	CString m_stuClass;
	// 学生的学号
	CString m_stuID;
	// 学生的专业
	CString m_stuMajor;
	NOTIFYICONDATA m_notifyIconData;
	CControlCenter*  m_pCtrlCenter;
public:
	afx_msg void OnBnClickedCancel();
	afx_msg void OnDestroy();
};
