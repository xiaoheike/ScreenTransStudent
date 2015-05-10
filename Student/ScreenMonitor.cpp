/******************************************************************* 
FileName: ScreenMonitor.cpp
Author	: xiaoheike
Date	: 2015-05-10 14:43:29
Desc	: 屏幕监控类，实现屏幕监控相关方法，包括屏幕图像数据的传输
		  获取等
*******************************************************************/ 
#include "stdafx.h"
#include "ScreenMonitor.h"
#include <math.h>

CScreenMonitor::CScreenMonitor()
	: m_pBmpTransData(NULL)
	, m_pBitMapInfo(NULL)
	, m_isScreenMonitorEnd(true)
	, m_compressBmpDataLen(0)
{
}

CScreenMonitor::~CScreenMonitor()
{
	CleanData();
}

void CScreenMonitor::CleanData()
{
	DeletepBitMapInfo();

	DeletepBmpTransData();
}

/******************************************************************
Function	: DeletepBmpTransData
Date		: 2015-05-09 21:20:38
Author		: xiaoheike
Parameter	: 无
Return		: void
Desc		: 释放new 的内存
******************************************************************/
void CScreenMonitor::DeletepBmpTransData()
{
	if (m_pBmpTransData != NULL)
	{
		delete[] m_pBmpTransData;
		m_pBmpTransData = NULL;
	}
}

/******************************************************************
Function	: DeletepBitMapInfo
Date		: 2015-05-09 21:21:10
Author		: xiaoheike
Parameter	: 无
Return		: void
Desc		: 释放new 的内存
******************************************************************/
void CScreenMonitor::DeletepBitMapInfo()
{
	if (m_pBitMapInfo != NULL)
	{
		LocalFree(m_pBitMapInfo);
		m_pBitMapInfo = NULL;
	}
}

/******************************************************************
Function	: SetSocketMsg
Date		: 2015-05-09 21:21:24
Author		: xiaoheike
Parameter	: socketMsg--连接教师机的SOCKET
Return		: void
Desc		: 给类成员SOCKET 赋值
******************************************************************/
void CScreenMonitor::SetSocketMsg(SOCKET socketMsg)
{
	m_socketMsg = socketMsg;
	// 	m_socketScreen = socketScreen;
}

/******************************************************************
Function	: GetDeskScreeData
Date		: 2015-05-09 21:22:41
Author		: xiaoheike
Parameter	: 无
Return		: void
Desc		: 获得桌面的图像数据并调用压缩方法
******************************************************************/
void CScreenMonitor::GetDeskScreeData()
{
	CDC memDC; //定义一个内存画布
	CBitmap bmp;
	BITMAP bitmap;
	this->GetDeskScreenDC(memDC, bmp, bitmap);

	this->SetCursorIcon(memDC);

	// 保存原始桌面图像数据的数组
	BYTE* pBmpOriginData = new BYTE[m_imgTotalSize];
	// 将图像的数据拷贝到pBmpOriginalData 中
	if (::GetDIBits(memDC.m_hDC, bmp, 0, bitmap.bmHeight,
		pBmpOriginData, m_pBitMapInfo, DIB_RGB_COLORS) == 0)
	{
		AfxMessageBox(_T("GetDIBits Error"));
		this->DeletepBitMapInfo();
		this->DeletepBmpOriginData(pBmpOriginData);
		return;
	}

	// 压缩图像数据
	this->CompressBmpData(pBmpOriginData);
	this->DeletepBmpOriginData(pBmpOriginData);
	DeleteDC(memDC);
	DeleteObject(bmp);
}

/******************************************************************
Function	: DeletepBmpOriginData
Date		: 2015-05-09 21:23:51
Author		: xiaoheike
Parameter	: pBmpOriginData--将要被释放的指针
Return		: void
Desc		: 释放new 的内存
******************************************************************/
void CScreenMonitor::DeletepBmpOriginData(BYTE* pBmpOriginData)
{
	if (pBmpOriginData != NULL)
	{
		delete[] pBmpOriginData;
		pBmpOriginData = NULL;
	}
}

/******************************************************************
Function	: GetDeskScreenDC
Date		: 2015-05-09 21:25:28
Author		: xiaoheike
Parameter	: memDC--CDC类  bmp--位图   bitmap--位图结构体
Return		: void
Desc		: 获得桌面图像
******************************************************************/
void CScreenMonitor::GetDeskScreenDC(CDC &memDC, CBitmap &bmp, BITMAP &bitmap)
{
	CDC* pDeskDC = CWnd::GetDesktopWindow()->GetDC(); //获取桌面画布对象
	int width = GetSystemMetrics(SM_CXSCREEN); //获取屏幕的宽度
	int height = GetSystemMetrics(SM_CYSCREEN); //获取屏幕的高度

	memDC.CreateCompatibleDC(pDeskDC); //创建一个兼容的画布

	bmp.CreateCompatibleBitmap(pDeskDC, width, height); //创建兼容位图
	memDC.SelectObject(&bmp); //选中位图对象

	bmp.GetBitmap(&bitmap);
	memDC.BitBlt(0, 0, width, height, pDeskDC, 0, 0, SRCCOPY);
	m_imgTotalSize = bitmap.bmWidthBytes * bitmap.bmHeight;
	InitBITMAPINFO(bitmap, height, width);

	pDeskDC->DeleteDC();
}

/******************************************************************
Function	: SetCursorIcon
Date		: 2015-05-09 21:27:16
Author		: xiaoheike
Parameter	: memDC--CDC类
Return		: void
Desc		: 获得鼠标信息并绘制到桌面图像中
******************************************************************/
void CScreenMonitor::SetCursorIcon(CDC &memDC)
{
	//获取当前光标及其位置
	HCURSOR hCursor = GetCursor();
	POINT ptCursor;
	GetCursorPos(&ptCursor);
	//获取光标的图标数据
	ICONINFO IconInfo;
	if (GetIconInfo(hCursor, &IconInfo))
	{
		ptCursor.x -= ((int)IconInfo.xHotspot);
		ptCursor.y -= ((int)IconInfo.yHotspot);
		if (IconInfo.hbmMask != NULL)
			DeleteObject(IconInfo.hbmMask);
		if (IconInfo.hbmColor != NULL)
			DeleteObject(IconInfo.hbmColor);
	}
	//在兼容设备描述表上画出该光标
	DrawIconEx(
		memDC.m_hDC,         // handle to device context
		ptCursor.x, ptCursor.y,
		hCursor,         // handle to icon to draw
		0, 0,          // width of the icon
		0,           // index of frame in animated cursor
		NULL,          // handle to background brush
		DI_NORMAL | DI_COMPAT      // icon-drawing flags
		);
}

/******************************************************************
Function	: SendScreenBmpData
Date		: 2015-05-09 21:28:07
Author		: xiaoheike
Parameter	: 无
Return		: void
Desc		: 发送屏幕图像数据到教师机
******************************************************************/
void CScreenMonitor::SendScreenBmpData()
{
	BMPDATA bmpData;
	int count = this->InitBmpData(bmpData);
	for (int i = 0; i < count; i++)
	{
		// 通知教师机准备接收屏幕图像数据
		m_socketCenter.SendReadyInfo(m_socketMsg, BEGINSCREENMONITOR);

		SetBmpData(bmpData, i, count);

		// 发送图像数据到教师机
		m_socketCenter.SendDataTCP(m_socketMsg, (char*)&bmpData, sizeof(BMPDATA));
	}
}

/******************************************************************
Function	: SetBmpData
Date		: 2015-05-09 21:29:46
Author		: xiaoheike
Parameter	: bmpData--需要被赋值的BMPDATA 结构体  i--一幅桌面图像的第几次传输
count--一幅桌面图像需要被传输的次数
Return		: void
Desc		: 将屏幕图像数据赋值到BMPDATA 结构体中
******************************************************************/
void CScreenMonitor::SetBmpData(BMPDATA &bmpData, int i, int count)
{
	UINT beginPos = 0;
	memset(bmpData.screenBmpData, 0, SCREEN_TRANS_SIZE);
	beginPos = i * SCREEN_TRANS_SIZE;
	bmpData.beginPos = beginPos;
	// 发送一幅屏幕图像的最后一块
	if (i == count - 1)
	{
		bmpData.isShow = true;
		bmpData.infoType = 2;
		uLongf last = m_compressBmpDataLen - beginPos;
		memcpy_s(bmpData.screenBmpData, last,
			m_pBmpTransData + beginPos, last);
	}
	else
	{
		memcpy_s(bmpData.screenBmpData, SCREEN_TRANS_SIZE,
			m_pBmpTransData + beginPos, SCREEN_TRANS_SIZE);
	}
}

/******************************************************************
Function	: InitBmpData
Date		: 2015-05-09 21:32:51
Author		: xiaoheike
Parameter	: bmpData--桌面图像数据的结构体
Return		: int--一幅桌面图像需要被传输的次数
Desc		: 为桌面图像结构体BMPDATA初始化一些必要信息
******************************************************************/
int CScreenMonitor::InitBmpData(BMPDATA &bmpData)
{
	memset(&bmpData, 0, sizeof(BMPDATA));
	// infoType = 1 发送的不是一幅桌面图像的最后一块
	// infoType = 2 发送的是一幅桌面图像的最后地块
	bmpData.infoType = 1;
	int count = int(ceil(double(m_compressBmpDataLen) / SCREEN_TRANS_SIZE));

	memcpy(&bmpData.bmpHeadInfo, m_pBitMapInfo, m_bmpHeadTotalSize);
	bmpData.bmpCompressSize = m_compressBmpDataLen;

	bmpData.isShow = false;
	return count;
}

/******************************************************************
Function	: SetIsScreenMonitorEnd
Date		: 2015-05-09 21:34:35
Author		: xiaoheike
Parameter	: isScreenMonitorEnd
Return		: void
Desc		: 发送图像数据到教师机的线程是否结束的标志
isScreenMonitorEnd--true运行线程，false停止线程
******************************************************************/
void CScreenMonitor::SetIsScreenMonitorEnd(bool isScreenMonitorEnd)
{
	//	m_isInitBITMAPINFO = true;
	m_isScreenMonitorEnd = isScreenMonitorEnd;
}

/******************************************************************
Function	: SendScreenData
Date		: 2015-05-09 21:35:48
Author		: xiaoheike
Parameter	: 无
Return		: void
Desc		: 不断发送数据到教师机
******************************************************************/
void CScreenMonitor::SendScreenData()
{
	while (true == m_isScreenMonitorEnd)
	{
		this->GetDeskScreeData();
		SendScreenBmpData();
		Sleep(300);
	}
	//	delete this;
	CleanData();
}

/******************************************************************
Function	: CompressBmpData
Date		: 2015-05-09 21:37:17
Author		: xiaoheike
Parameter	: pBmpOriginalData--桌面图像的数据
Return		: void
Desc		: 使用ZLIB 库压缩桌面图像数据
******************************************************************/
void CScreenMonitor::CompressBmpData(BYTE* pBmpOriginalData)
{
	// 需要一个足够大的空间
	m_compressBmpDataLen = (uLongf)((m_imgTotalSize + 12)*(100.1 / 100)) + 1;

	BYTE* pCompressData = new BYTE[m_compressBmpDataLen];
	// 将桌面图像数据压缩到pCompressData 中
	int err = compress(pCompressData, &m_compressBmpDataLen,
		pBmpOriginalData, m_imgTotalSize);

	if (err != Z_OK) {
		exit(1);
	}
	if (m_pBmpTransData != NULL)
	{
		delete[] m_pBmpTransData;
		m_pBmpTransData = NULL;
	}

	// 将压缩后的数据保存到m_pBmpTransData 中
	m_pBmpTransData = new BYTE[m_compressBmpDataLen];
	memcpy(m_pBmpTransData, pCompressData, m_compressBmpDataLen);
	delete[] pCompressData;
	pCompressData = NULL;
}

/******************************************************************
Function	: InitBITMAPINFO
Date		: 2015-05-09 21:40:20
Author		: xiaoheike
Parameter	: bitmap--图像的大小等信息  height--桌面的高
width--桌面的宽
Return		: void
Desc		: 初始化图像的信息头结构体 BITMAPINFO
******************************************************************/
void CScreenMonitor::InitBITMAPINFO(BITMAP &bitmap, int height, int width)
{
	double paletteSize = 0; //记录调色板大小
	if (bitmap.bmBitsPixel < 16) //判断是否为真彩色位图
	{
		paletteSize = (1 << bitmap.bmBitsPixel)*sizeof(RGBQUAD);
	}
	m_bmpHeadTotalSize = (int)paletteSize + sizeof(BITMAPINFO);

	m_pBitMapInfo = (BITMAPINFO*)LocalAlloc(LPTR, m_bmpHeadTotalSize);
	m_pBitMapInfo->bmiHeader.biBitCount = bitmap.bmBitsPixel;
	m_pBitMapInfo->bmiHeader.biClrImportant = 0;
	m_pBitMapInfo->bmiHeader.biCompression = 0;
	m_pBitMapInfo->bmiHeader.biHeight = height;
	m_pBitMapInfo->bmiHeader.biPlanes = bitmap.bmPlanes;
	m_pBitMapInfo->bmiHeader.biSize = m_bmpHeadTotalSize;//sizeof(BITMAPINFO);
	m_pBitMapInfo->bmiHeader.biSizeImage = m_imgTotalSize;
	m_pBitMapInfo->bmiHeader.biWidth = width;
	m_pBitMapInfo->bmiHeader.biXPelsPerMeter = 0;
	m_pBitMapInfo->bmiHeader.biYPelsPerMeter = 0;
}