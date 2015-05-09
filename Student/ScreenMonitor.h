#pragma once
#include <math.h>
//////////////////////////////////////////////////////////////////////////
// zlib库 需要使用其中的压缩图片功能
#include "zlib.h"
#include "zconf.h"
#pragma comment(lib,"zdll.lib")
//////////////////////////////////////////////////////////////////////////
#include <afxwin.h>
#include "Comment.h"
//#include <iostream>
#include "SocketCenter.h"
using namespace std;
class CScreenMonitor
{
public:
	CScreenMonitor();

	~CScreenMonitor();

	void GetDeskScreeData();

	void InitBITMAPINFO(BITMAP &bitmap, int height, int width);

	void CompressBmpData(BYTE* pBmpOriginalData);

	void SendBmpData();

	void SendScreenData();

	void SetSocket(SOCKET socketMsg);

	void SetSendFlag(bool isSendFlag);

	void CleanData();
private:
	int m_bmpHeadTotalSize;
	BITMAPINFO* m_pBMPINFO;
	uLongf m_imgTotalSize;
	BYTE* m_pBmpTransData;
	CSocketCenter m_pSocketCenter;
	bool m_isSendFlag;
	SOCKET m_socketMsg;
	uLongf  m_compressBmpDataLen;
//	bool m_isInitBITMAPINFO;
};

