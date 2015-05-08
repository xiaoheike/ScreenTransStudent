#include "stdafx.h"
#include "ScreenMonitor.h"
#include <math.h>
//////////////////////////////////////////////////////////////////////////
// zlib库 需要使用其中的压缩图片功能
#include "zlib.h"
#include "zconf.h"
#pragma comment(lib,"zdll.lib")
//////////////////////////////////////////////////////////////////////////
#include <afxwin.h>
#include <io.h>
#include <fcntl.h>

/*
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
*/


CScreenMonitor::CScreenMonitor()
	: m_pBmpTransData(NULL)
	, m_pBMPINFO(NULL)
	, m_isSendFlag(true)
	, m_isInitBITMAPINFO(true)
//	, m_socketScreen(INVALID_SOCKET)
//	, m_isSocketConn(true)
// 	, m_pBmpOriDataFirst(NULL)
// 	, m_pBmpOriDataSecond(NULL)
// 	, m_heightPerBlock(0)
// 	, m_heightLastBlock(0)
// 	, m_widthPerBlock(0)
// 	, m_sizePerBlcok(0)
{
}


CScreenMonitor::~CScreenMonitor()
{
	CleanData();
// 	if (m_socketMsg != INVALID_SOCKET)
// 	{
// 		closesocket(m_socketMsg);
// 		m_socketMsg = INVALID_SOCKET;
// 	}
}

void CScreenMonitor::CleanData()
{
	if (m_pBMPINFO != NULL)
	{
		LocalFree(m_pBMPINFO);
		m_pBMPINFO = NULL;
	}

	if (m_pBmpTransData != NULL)
	{
		delete[] m_pBmpTransData;
		m_pBmpTransData = NULL;
	}

// 	if (m_socketScreen != INVALID_SOCKET)
// 	{
// 		closesocket(m_socketScreen);
// 		m_socketScreen = NULL;
// 	}

}
void CScreenMonitor::SetSocket(SOCKET socketMsg)
{
	m_socketMsg = socketMsg;
// 	m_socketScreen = socketScreen;
}


/*
	获得桌面的截图数据并进行调用压缩函数
*/
void CScreenMonitor::GetDeskScreeData()
{
	CDC* pDeskDC = CWnd::GetDesktopWindow()->GetDC(); //获取桌面画布对象
	int width = GetSystemMetrics(SM_CXSCREEN); //获取屏幕的宽度
	int height = GetSystemMetrics(SM_CYSCREEN); //获取屏幕的高度
	CDC memDC; //定义一个内存画布
	memDC.CreateCompatibleDC(pDeskDC); //创建一个兼容的画布
	CBitmap bmp;
	bmp.CreateCompatibleBitmap(pDeskDC, width, height); //创建兼容位图
	memDC.SelectObject(&bmp); //选中位图对象
	BITMAP bitmap;
	bmp.GetBitmap(&bitmap);

	memDC.BitBlt(0, 0, width, height, pDeskDC, 0, 0, SRCCOPY);
	m_imgTotalSize = bitmap.bmWidthBytes * bitmap.bmHeight;

//  	if (true == m_isInitBITMAPINFO)
//  	{
		// 初始化截图图像头信息
		InitBITMAPINFO(bitmap, height, width);
//  		m_isInitBITMAPINFO = false;
//  	}
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
	// 桌面截图的原始数据数组
	BYTE* pBmpOriginalData = new BYTE[m_imgTotalSize];
	// 将图像的数据拷贝到pBmpOriginalData 中
	if (::GetDIBits(memDC.m_hDC, bmp, 0, bitmap.bmHeight,
				pBmpOriginalData, m_pBMPINFO, DIB_RGB_COLORS) == 0)
	{
		AfxMessageBox(_T("GetDIBits Error"));
		delete[] pBmpOriginalData;
		pBmpOriginalData = NULL;
		LocalFree(m_pBMPINFO);
		m_pBMPINFO = NULL;
		return;
	}

	// 压缩截图数据
	CompressBmpData(pBmpOriginalData);

	delete[] pBmpOriginalData;
	pBmpOriginalData = NULL;
	pDeskDC->DeleteDC();
	DeleteDC(memDC);
	DeleteObject(bmp);
}

/*
	发送截图数据到客户端
*/
void CScreenMonitor::SendBmpData()
{
	BMPDATA bmpData;
	memset(&bmpData, 0, sizeof(BMPDATA));
	// 发送图像数据
	bmpData.infoType = 1;
	int count = int(ceil(double(m_compressBmpDataLen) / SCREEN_TRANS_SIZE));

	memcpy(&bmpData.bmpHeadInfo, m_pBMPINFO, m_bmpHeadTotalSize);
	bmpData.bmpCompressSize = m_compressBmpDataLen;


	bmpData.isShow = false;
	UINT beginPos;
	for (int i = 0; i < count; i++)
	{
		// 通知客户端可以开始接收屏幕数据了
		m_pSocketCenter.SendReadyInfo(m_socketMsg, SCREENDATA);


		memset(bmpData.transData, 0, SCREEN_TRANS_SIZE);
		beginPos = i * SCREEN_TRANS_SIZE;
		bmpData.beginPos = beginPos;
		if (i == count - 1) // 最后一次发送数据
		{
			bmpData.isShow = true;
			bmpData.infoType = 2;
			uLongf last = m_compressBmpDataLen - beginPos;
			memcpy_s(bmpData.transData, last,
				m_pBmpTransData + beginPos, last);
		}
		else
		{
			memcpy_s(bmpData.transData, SCREEN_TRANS_SIZE,
				m_pBmpTransData + beginPos, SCREEN_TRANS_SIZE);
		}
		//发送数据
		m_pSocketCenter.SendDataTCP(m_socketMsg, (char*)&bmpData, sizeof(BMPDATA));
	}
}

/*
	发送截图数据到客户端的线程能否停止的标示位
	input:
		isSendFlag--true运行线程，false停止纯种
*/
void CScreenMonitor::SetSendFlag(bool isSendFlag)
{
//	m_isInitBITMAPINFO = true;
	m_isSendFlag = isSendFlag;
}

/*
	不断发送数据到客户端的线程的主体函数
*/
void CScreenMonitor::SendScreenData()
{
	while (true == m_isSendFlag)
	{
		this->GetDeskScreeData();
		SendBmpData();
		Sleep(300);
	}
//	delete this;
	CleanData();
	
}

/*
	压缩截图数据
	input:
		pBmpOriginalData--图像的数据信息
*/
void CScreenMonitor::CompressBmpData(BYTE* pBmpOriginalData)
{
	BYTE* pCompressData = NULL;
	m_compressBmpDataLen = 0;
	// 需要一个足够大的空间
	m_compressBmpDataLen = (uLongf)((m_imgTotalSize + 12)*(100.1 / 100)) + 1;

	pCompressData = new BYTE[m_compressBmpDataLen];
	// 将数据进行压缩并保存到pCompressData 中
	int err = compress(pCompressData, &m_compressBmpDataLen, pBmpOriginalData, m_imgTotalSize);

	if (err != Z_OK) {
// 		InitConsoleWindow();
// 		printf("compess error: %d", err);
		exit(1);
	}

// 	InitConsoleWindow();
// 	printf("\r\norignal size: %d, compressed size : %d\r\n", 
// 			m_imgTotalSize, m_compressBmpDataLen);
// 	cout << "orignal size: " << m_imgTotalSize
// 		<< " , compressed size : " << m_compressBmpDataLen << '\n';
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

/*
	初始化截图的信息头结构体 BITMAPINFO
	input:
		bitmap--截图的大小等信息
		height--桌面的高
		width--桌面的宽
*/
void CScreenMonitor::InitBITMAPINFO(BITMAP &bitmap, int height, int width)
{
//	m_imgTotalSize = bitmap.bmWidthBytes * bitmap.bmHeight;
	// 每一块的扫描线的数量
// 	m_heightPerBlock = height / BLOCKNUM;
// 	// 最后一块的扫描线的数量
// 	m_heightLastBlock = height - m_heightPerBlock * (BLOCKNUM - 1);
// 	m_widthPerBlock = width;
// 	m_sizePerBlcok = m_heightPerBlock * m_widthPerBlock;
// 	InitConsoleWindow();
// 	printf("\nm_heightPerBlock = %d, m_heightLastBlock = %d, height = %d\n",
// 		m_heightPerBlock, m_heightLastBlock, height);
	double paletteSize = 0; //记录调色板大小
	if (bitmap.bmBitsPixel < 16) //判断是否为真彩色位图
	{
		//paletteSize = pow(2.0, (double)bitmap.bmBitsPixel*sizeof(RGBQUAD));
		paletteSize = (1 << bitmap.bmBitsPixel)*sizeof(RGBQUAD);
	}
	m_bmpHeadTotalSize = (int)paletteSize + sizeof(BITMAPINFO);

	m_pBMPINFO = (BITMAPINFO*)LocalAlloc(LPTR, m_bmpHeadTotalSize);
	m_pBMPINFO->bmiHeader.biBitCount = bitmap.bmBitsPixel;
	m_pBMPINFO->bmiHeader.biClrImportant = 0;
	m_pBMPINFO->bmiHeader.biCompression = 0;
	m_pBMPINFO->bmiHeader.biHeight = height;
	m_pBMPINFO->bmiHeader.biPlanes = bitmap.bmPlanes;
	m_pBMPINFO->bmiHeader.biSize = m_bmpHeadTotalSize;//sizeof(BITMAPINFO);
	m_pBMPINFO->bmiHeader.biSizeImage = m_imgTotalSize;
	m_pBMPINFO->bmiHeader.biWidth = width;
	m_pBMPINFO->bmiHeader.biXPelsPerMeter = 0;
	m_pBMPINFO->bmiHeader.biYPelsPerMeter = 0;
}
