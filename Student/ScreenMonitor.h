#pragma once
#include <math.h>
//////////////////////////////////////////////////////////////////////////
// ZLIB库
#include "zlib.h"
#include "zconf.h"
#pragma comment(lib,"zdll.lib")
//////////////////////////////////////////////////////////////////////////
#include <afxwin.h>
#include "Comment.h"
#include <iostream>
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
//	void CompressBmpData(BYTE* pBmpOriginalData, UINT sizePerBlock);
//	void SendBmpHeaderInfo();
	void SendBmpData();
	void SendScreenData();
	void SetSocket(SOCKET socketMsg);
	void SetSendFlag(bool isSendFlag);
	void CleanData();
private:
//public:
	int m_bmpHeadTotalSize;
	BITMAPINFO* m_pBMPINFO;
	uLongf m_imgTotalSize;
	BYTE* m_pBmpTransData;
	CSocketCenter m_pSocketCenter;
	bool m_isSendFlag;
	SOCKET m_socketMsg;
//	SOCKET m_socketScreen;
	uLongf  m_compressBmpDataLen;
	bool m_isInitBITMAPINFO;
//	bool m_isSocketConn;
// 	// 第一张桌面图像数据
// 	BYTE* m_pBmpOriDataFirst;
// 	// 第二张桌面图像数据
// 	BYTE* m_pBmpOriDataSecond;
// 	// 每一块有多少的扫描线
// 	uLongf m_heightPerBlock;
// 	// 最后一块的扫描线的数量
// 	uLongf m_heightLastBlock;
// 	// 屏幕宽度单位是像素
// 	uLongf m_widthPerBlock;
// 	// 每块的像素总数
// 	uLongf m_sizePerBlcok;
};

