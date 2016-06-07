// ScreenCaptureDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScreenCapture.h"
#include "ScreenCaptureDlg.h"
#include <atlimage.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define VALUE_VALID -500
#define VALUE_BOLDER_REDUCE 5

int g_wide = 0;
int g_nei = 0;
int g_gengnei = 0;

CScreenCaptureDlg::CScreenCaptureDlg(CWnd* pParent)
	: CDialog(CScreenCaptureDlg::IDD, pParent)
{
	GdiplusStartupInput gdi;
	ULONG_PTR	gdiToken; 
	GdiplusStartup(&gdiToken,&gdi,NULL);

	m_pngMask.LoadImage(_T("D:\\TQ\\TQSolution\\product\\res\\ScreenCapture\\SC_MASK.png"));
	m_pngDot.LoadImage(_T("D:\\src\\TQSolution\\TQ_UNICODE\\ScreenCapture\\res\\SC_DOT.png"));
	m_pngAction.LoadImage(_T("D:\\TQ\\TQSolution\\product\\res\\ScreenCapture\\SC_ACTION.png"));
	m_pngActionClick.LoadImage(_T("D:\\TQ\\TQSolution\\product\\res\\ScreenCapture\\SC_ACTION_CLICK.png"));
	m_width=GetSystemMetrics(SM_CXSCREEN);
	m_height=GetSystemMetrics(SM_CYSCREEN);

	//获取屏幕分辩率
	m_xScreen = GetSystemMetrics(SM_CXSCREEN);
	m_yScreen = GetSystemMetrics(SM_CYSCREEN);

	//截取屏幕到位图中
	CRect rect(0, 0,m_xScreen,m_yScreen);
	m_pBitmap=CBitmap::FromHandle(CopyScreenToBitmap(&rect));
	m_backGround = m_pBitmap;

	// 初始化为无效值
	m_ptStopMove.x = VALUE_VALID;
	m_ptStopMove.y = VALUE_VALID;
	m_ptDragPos.x = VALUE_VALID;
	m_ptDragPos.y = VALUE_VALID;

	m_bIsNotDrawing = true;
	m_bStartDraw = false;
	m_status = Inital;
	m_eDrawType = NotLabel;
	m_eToolType = NotTool;

	m_hCursor[0]=AfxGetApp()->LoadCursor(IDC_CURSOR_ARROW);
	m_hCursor[1]=AfxGetApp()->LoadCursor(IDC_CURSOR_CROSS);
	m_hCursor[2]=AfxGetApp()->LoadCursor(IDC_CURSOR_DRAG);
	SetCursor(m_hCursor[0]);
}

CScreenCaptureDlg::~CScreenCaptureDlg()
{
	m_backGround->DeleteObject();
	m_pBitmap->DeleteObject();
	int count = m_labels.size();
	for (int i = 0; i < count; i++)
	{
		delete m_labels[i];
	}
	m_labels.clear();
}

void CScreenCaptureDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CScreenCaptureDlg, CDialog)
	//{{AFX_MSG_MAP(CScreenCaptureDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	//ON_WM_ERASEBKGND()
	ON_WM_SETCURSOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CScreenCaptureDlg message handlers

BOOL CScreenCaptureDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	//把对化框设置成全屏顶层窗口
	SetWindowPos(&wndTopMost,0,0,m_xScreen,m_yScreen,SWP_SHOWWINDOW);
			
	m_curSel[0]=AfxGetApp()->LoadCursor(IDC_CURSOR_MOVE);
	m_curSel[1]=AfxGetApp()->LoadCursor(IDC_CURSOR_NWSE);
	m_curSel[2]=AfxGetApp()->LoadCursor(IDC_CURSOR_NS);
	m_curSel[3]=AfxGetApp()->LoadCursor(IDC_CURSOR_NESW);
	m_curSel[4]=AfxGetApp()->LoadCursor(IDC_CURSOR_EW);
	m_curSel[5]=AfxGetApp()->LoadCursor(IDC_CURSOR_EW);
	m_curSel[6]=AfxGetApp()->LoadCursor(IDC_CURSOR_NESW);
	m_curSel[7]=AfxGetApp()->LoadCursor(IDC_CURSOR_NS);
	m_curSel[8]=AfxGetApp()->LoadCursor(IDC_CURSOR_NWSE);

	m_bLeftDown=false;
	m_iCapture=0;

	for(int i=0;i<9;i++)
	{
		m_rcSel[i].left=0;
		m_rcSel[i].top=0;
		m_rcSel[i].right=0;
		m_rcSel[i].bottom=0;
	}

	return TRUE; 
}

void CScreenCaptureDlg::OnPaint() 
{

	if (IsIconic())
	{
		CPaintDC dc(this);

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;
	}
	else
	{
		DrawFrame();

		CDialog::OnPaint();
	}

}

void CScreenCaptureDlg::OnOK() 
{

}

void CScreenCaptureDlg::OnCancel() 
{
	CDialog::OnCancel();
}

void CScreenCaptureDlg::OnMouseMove(UINT nFlags, CPoint point) 
{
	switch(m_status)
	{
	case Inital:
		break;
	case Capturing:
		CapturingDraw(point);
		break;
	case Captured:
		CursorChangeWhenCaptured(point);
		break;
	case Drag:
		DragDraw(point);
		break;
	case Drawing:
		LabelDraw(point);
		break;
	case Draw:
		CursorChangeWhenDraw(point);
		break;
	}

	CDialog::OnMouseMove(nFlags, point);
}

void CScreenCaptureDlg::SaveToFile(CString strSaveFile)
{
	CString strExt=".png";
	if(strSaveFile=="")
	{
		CFileDialog fd(FALSE,"PNG","未命名",4|2,"PNG 图片格式 (*.png)|*.png|BMP 图片格式 (*.bmp)|*.bmp|JPG 图片格式 (*.jpg)|*.jpg||");
		if(fd.DoModal()==IDCANCEL)
			return;
		strExt=fd.GetFileExt();
		strExt.Trim();strExt.MakeLower();
		strSaveFile=fd.GetPathName();
	}

	CDC *dc=GetDC();
	CDC BufferDC;
	BufferDC.CreateCompatibleDC(dc);
	CBitmap BufferBmp;
	BufferBmp.CreateCompatibleBitmap(dc,m_rcSel[0].Width(),m_rcSel[0].Height());
	BufferDC.SelectObject(&BufferBmp);
	CDC pdc;
	pdc.CreateCompatibleDC(&BufferDC);
	pdc.SelectObject(m_backGround);
	int count = m_labels.size();
	for(int i = 0; i < count; i++)
	{
		m_draw.DrawObj(&pdc,m_labels[i]);
	}
	BufferDC.BitBlt(0,0,m_rcSel[0].Width(),m_rcSel[0].Height(),&pdc,m_rcSel[0].left,m_rcSel[0].top,SRCCOPY);
	pdc.DeleteDC();

	CImage image;
	image.Attach(BufferBmp);
	if(strExt=="bmp")
		image.Save(strSaveFile, Gdiplus::ImageFormatBMP);
	else if(strExt=="jpg")
		image.Save(strSaveFile, Gdiplus::ImageFormatJPEG);
	else
		image.Save(strSaveFile, Gdiplus::ImageFormatPNG);

	BufferDC.DeleteDC();
	BufferBmp.DeleteObject();
	ReleaseDC(dc);

	m_rcSel[0].right=m_rcSel[0].left;
	m_rcSel[0].bottom=m_rcSel[0].top;

	CDialog::OnCancel();

}

void CScreenCaptureDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	m_ptLButtonDown=point;
	m_bLeftDown=true;
	if(m_status == Captured)
	{
		for(int i=0;i<=8;i++)
		{
			if(PtInRect(m_rcSel[i],point))
			{
				if(i==0)
				{
					m_ptDragPos.x = m_ptLButtonDown.x - m_rcSel[0].left;
					m_ptDragPos.y = m_ptLButtonDown.y - m_rcSel[0].top;
					m_iCapture=9;

					if (IsInToolArea(point) > -1)
					{
						SetCursor(m_hCursor[0]);
					}
					else
					{
						m_status = Drag;
						SetCursor(m_hCursor[2]);
					}					
				}
				else
				{
					m_status = Drag;
					m_iCapture=i;
					SetCursor(m_curSel[i]);
				}
				break;
			}
		}
	}
	else if(m_status == Inital)
	{
		m_status = Capturing;
		m_rcSel[0].left=point.x;
		m_rcSel[0].top=point.y;
		m_rcSel[0].right=point.x;
		m_rcSel[0].bottom=point.y;
	}
	else if(m_status == Draw)
	{
		if (m_bIsNotDrawing && PtInRect(m_rcSel[0],point))
		{

			if (IsInToolArea(point) < 0)
			{
				m_ptStartDraw = point;
				m_bIsNotDrawing = false;
				m_bStartDraw = true;
				m_status = Drawing;
			}
		}
	}

	if (m_status == Captured || m_status == Draw)
	{		
		int index = IsInToolArea(point);
		if (index > -1 && index < 5)
		{
			m_eDrawType = (LableType)index;
		}
		if (index > 4)
		{
			m_eToolType = (ToolType)index;		
		}
	}

	DrawFrame();

	CDialog::OnLButtonDown(nFlags, point);
}

void CScreenCaptureDlg::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if(!m_bLeftDown)
	{
		CDialog::OnLButtonUp(nFlags, point);
		return;
	}

	m_bLeftDown=false;
	m_iCapture=0;

	if(m_status == Capturing)
	{
		if (!(point.x == m_ptLButtonDown.x || point.y == m_ptLButtonDown.y))
		{
			m_status=Captured;
		}
	}
	else if(m_status == Drag)
	{
		m_status=Captured;
	}
	else if(m_status == Drawing)
	{
		m_bStartDraw = false;
		m_bIsNotDrawing = true;
        m_status = Draw;
		
		if (!(point.x == m_ptLButtonDown.x || point.y == m_ptLButtonDown.y))
		{
			if (m_eDrawType == LabelLine)
			{
				LineObj* obj = new LineObj(m_ptStartDraw,m_ptLineEndPos);
				m_labels.push_back(obj);
			}
			else if (m_eDrawType == LabelRectangle)
			{
				RectangleObj* obj = new RectangleObj(m_ptStartDraw,m_ptLineEndPos);
				m_labels.push_back(obj);
			}
			else if (m_eDrawType == LabelEllipse)
			{
				EllipseObj* obj = new EllipseObj(m_ptStartDraw,m_ptLineEndPos);
				m_labels.push_back(obj);
			}
		}		
	}
	else if (m_status == Draw || m_status == Captured)
	{
		if(PtInRect(m_rcAction[0],point))
		{
			m_status = Draw;
			m_eDrawType = LabelRectangle;
		}
		if(PtInRect(m_rcAction[1],point))
		{
			m_status = Draw;
			m_eDrawType = LabelEllipse;
		}
		if (PtInRect(m_rcAction[2],point))
		{
			m_status = Draw;
			m_eDrawType = LabelLine;
		}
		if (PtInRect(m_rcAction[3],point))
		{
			m_status = Draw;
			m_eDrawType = LabelPen;
		}
		if (PtInRect(m_rcAction[4],point))
		{
			m_status = Draw;
			m_eDrawType = LabelText;
		}
		if(PtInRect(m_rcAction[5],point))
		{
			m_labels.pop_back();
		}
		if(PtInRect(m_rcAction[6],point))
		{
			SaveToFile();
		}
		if (PtInRect(m_rcAction[7],point))
		{
			m_status = Inital;
			m_eDrawType = NotLabel;
			OnRButtonDown(NULL,NULL);
		}
	}

	m_eToolType = NotTool;

	int x,y,w,h;
	GetCapturedCoord( x,y,w,h );

	m_rcSel[0].left=x;
	m_rcSel[0].top=y;
	m_rcSel[0].right=x+w;
	m_rcSel[0].bottom=y+h;

	DrawFrame();

	CDialog::OnLButtonUp(nFlags, point);
}

void CScreenCaptureDlg::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	if(PtInRect(m_rcSel[0],point))
	{
		if (IsInToolArea(point) < 0)
		{
			CString strFilename;
			CopyScreenToBitmap(&m_rcSel[0],TRUE);
			OnRButtonDown(NULL,NULL);
			//strFilename.Format("%s%s.png",gDlg->m_strMyDocumentsPath,CTime::GetCurrentTime().Format("%Y%m%d%H%M%S"));
			// SaveToFile(strFilename);
		}		
	}
	
	CDialog::OnLButtonDblClk(nFlags, point);
}

//处理鼠标右键单击 .ESC. 
void CScreenCaptureDlg::OnRButtonDown(UINT nFlags, CPoint point) 
{
	PostQuitMessage(0);
	CDialog::OnRButtonDown(nFlags, point);
}

BOOL CScreenCaptureDlg::OnEraseBkgnd(CDC* pDC) 
{
	BITMAP bmp;
	m_pBitmap->GetBitmap(&bmp);

	CDC dcCompatible;
	dcCompatible.CreateCompatibleDC(pDC);
	dcCompatible.SelectObject(m_pBitmap);

	CRect rect;
	GetClientRect(&rect);
	pDC->BitBlt(0,0,rect.Width(),rect.Height(),&dcCompatible,0,0,SRCCOPY);

	return TRUE;
}

BOOL CScreenCaptureDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	return TRUE;
}

void CScreenCaptureDlg::DrawToolBar(CDC *DC,CDC* bufferDC)
{
	if(m_status == Captured || m_status == Capturing || m_status == Drawing || m_status == Draw || m_status == Drag)
	{
		int x,y,w,h;
		GetCapturedCoord( x,y,w,h);

		// 绘制工具栏
		int posX = x + w;
		int posY = y + h;
		if(posY > m_height - 32)
		{
			posY = y - 32;
			if ( y < 32)
			{
				posY = y + h - 32;
			}
		}

		m_rcAction[0].left=posX-273;
		m_rcAction[0].top=posY+2;
		m_rcAction[0].right=m_rcAction[0].left+30;
		m_rcAction[0].bottom=m_rcAction[0].top+30;

		if (m_eDrawType == LabelRectangle)
		{
			m_pngActionClick.DrawImage(bufferDC,m_rcAction[0].left,m_rcAction[0].top,30,30,0,0,30,30);
		}
		else
		{
			m_pngAction.DrawImage(bufferDC,m_rcAction[0].left,m_rcAction[0].top,30,30,0,0,30,30);
		}	

		m_rcAction[1].left=posX-243;
		m_rcAction[1].top=posY+2;
		m_rcAction[1].right=m_rcAction[1].left+29;
		m_rcAction[1].bottom=m_rcAction[1].top+30;

		if (m_eDrawType == LabelEllipse)
		{			
			m_pngActionClick.DrawImage(bufferDC,m_rcAction[1].left,m_rcAction[1].top,29,30,30,0,29,30);
		}
		else
		{
			m_pngAction.DrawImage(bufferDC,m_rcAction[1].left,m_rcAction[1].top,29,30,30,0,29,30);
		}

		m_rcAction[2].left=posX-214;
		m_rcAction[2].top=posY+2;
		m_rcAction[2].right=m_rcAction[2].left+25;
		m_rcAction[2].bottom=m_rcAction[2].top+30;

		if (m_eDrawType == LabelLine)
		{
			m_pngActionClick.DrawImage(bufferDC,m_rcAction[2].left,m_rcAction[2].top,25,30,59,0,25,30);
		}
		else
		{
			m_pngAction.DrawImage(bufferDC,m_rcAction[2].left,m_rcAction[2].top,25,30,59,0,25,30);
		}		

		m_rcAction[3].left=posX-189;
		m_rcAction[3].top=posY+2;
		m_rcAction[3].right=m_rcAction[3].left+26;
		m_rcAction[3].bottom=m_rcAction[3].top+30;
		if (m_eDrawType == LabelPen)
		{
			m_pngActionClick.DrawImage(bufferDC,m_rcAction[3].left,m_rcAction[3].top,26,30,84,0,26,30);
		}
		else
		{
			m_pngAction.DrawImage(bufferDC,m_rcAction[3].left,m_rcAction[3].top,26,30,84,0,26,30);
		}

		m_rcAction[4].left=posX-163;
		m_rcAction[4].top=posY+2;
		m_rcAction[4].right=m_rcAction[4].left+25;
		m_rcAction[4].bottom=m_rcAction[4].top+30;
		if (m_eDrawType == LabelText)
		{
			m_pngActionClick.DrawImage(bufferDC,m_rcAction[4].left,m_rcAction[4].top,25,30,110,0,25,30);
		}
		else
		{
			m_pngAction.DrawImage(bufferDC,m_rcAction[4].left,m_rcAction[4].top,25,30,110,0,25,30);
		}		

		m_rcAction[5].left=posX-138;
		m_rcAction[5].top=posY+2;
		m_rcAction[5].right=m_rcAction[5].left+29;
		m_rcAction[5].bottom=m_rcAction[5].top+30;
		if (m_eToolType == ToolDrop)
		{
			m_pngActionClick.DrawImage(bufferDC,m_rcAction[5].left,m_rcAction[5].top,29,30,135,0,29,30);
		}
		else
		{
			m_pngAction.DrawImage(bufferDC,m_rcAction[5].left,m_rcAction[5].top,29,30,135,0,29,30);
		}
		
		m_rcAction[6].left=posX-109;
		m_rcAction[6].top=posY+2;
		m_rcAction[6].right=m_rcAction[6].left+28;
		m_rcAction[6].bottom=m_rcAction[6].top+30;
		if (m_eToolType == ToolSaveAs)
		{
			m_pngActionClick.DrawImage(bufferDC,m_rcAction[6].left,m_rcAction[6].top,28,30,164,0,28,30);
		}
		else
		{
			m_pngAction.DrawImage(bufferDC,m_rcAction[6].left,m_rcAction[6].top,28,30,164,0,28,30);
		}		
		
		m_rcAction[7].left=posX-81;
		m_rcAction[7].top=posY+2;
		m_rcAction[7].right=m_rcAction[7].left+27;
		m_rcAction[7].bottom=m_rcAction[7].top+30;
		if (m_eToolType == ToolCancel)
		{
			m_pngActionClick.DrawImage(bufferDC,m_rcAction[7].left,m_rcAction[7].top,27,30,192,0,27,30);
		}
		else
		{
			m_pngAction.DrawImage(bufferDC,m_rcAction[7].left,m_rcAction[7].top,27,30,192,0,27,30);
		}

		m_rcAction[8].left=posX-54;
		m_rcAction[8].top=posY+2;
		m_rcAction[8].right=m_rcAction[8].left+54;
		m_rcAction[8].bottom=m_rcAction[8].top+30;
		if (m_eToolType == ToolConfirm)
		{
			m_pngActionClick.DrawImage(bufferDC,m_rcAction[8].left,m_rcAction[8].top,54,30,219,0,54,30);
		}
		else
		{
			m_pngAction.DrawImage(bufferDC,m_rcAction[8].left,m_rcAction[8].top,54,30,219,0,54,30);
		}
	}

	//绘画界面
	DC->BitBlt(0,0,m_width,m_height,bufferDC,0,0,SRCCOPY);
}

//拷贝屏幕到位图中
HBITMAP CScreenCaptureDlg::CopyScreenToBitmap(LPRECT lpRect,BOOL bSave)
//lpRect 代表选定区域
{
	HDC       hScrDC, hMemDC;
	// 屏幕和内存设备描述表
	HBITMAP    hBitmap, hOldBitmap;
	// 位图句柄
	int       nX, nY, nX2, nY2;
	// 选定区域坐标
	int       nWidth, nHeight;

	// 确保选定区域不为空矩形
	if (IsRectEmpty(lpRect))
		return NULL;
	//为屏幕创建设备描述表
	hScrDC = CreateDC("DISPLAY", NULL, NULL, NULL);

	//为屏幕设备描述表创建兼容的内存设备描述表
	hMemDC = CreateCompatibleDC(hScrDC);
	// 获得选定区域坐标
	nX = lpRect->left;
	nY = lpRect->top;
	nX2 = lpRect->right;
	nY2 = lpRect->bottom;

	//确保选定区域是可见的
	if (nX < 0)
		nX = 0;
	if (nY < 0)
		nY = 0;
	if (nX2 > m_xScreen)
		nX2 = m_xScreen;
	if (nY2 > m_yScreen)
		nY2 = m_yScreen;
	nWidth = nX2 - nX;
	nHeight = nY2 - nY;
	// 创建一个与屏幕设备描述表兼容的位图
	hBitmap = CreateCompatibleBitmap(hScrDC, nWidth, nHeight);
	// 把新位图选到内存设备描述表中
	hOldBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);
	// 把屏幕设备描述表拷贝到内存设备描述表中
	if(bSave)
	{
		CDC dcCompatible;
		dcCompatible.CreateCompatibleDC(CDC::FromHandle(hMemDC));
		dcCompatible.SelectObject(m_pBitmap);

		BitBlt(hMemDC, 0, 0, nWidth, nHeight,
			dcCompatible, nX, nY, SRCCOPY);
	}
	else
	{
		BitBlt(hMemDC, 0, 0, nWidth, nHeight,
			hScrDC, nX, nY, SRCCOPY);
	}

	hBitmap = (HBITMAP)SelectObject(hMemDC, hOldBitmap);
	
	//清除 
	DeleteDC(hScrDC);
	DeleteDC(hMemDC);
	return hBitmap;
}

//重画窗口
void CScreenCaptureDlg::PaintWindow()
{
	//获取当全屏对话框窗口大小
	CRect rect1;
	GetWindowRect(rect1);

	CRgn rgn1,rgn2;
	rgn1.CreateRectRgnIndirect(rect1);

	InvalidateRect(rect1);
}

void CScreenCaptureDlg::DrawFrame()
{
	CDC *dc=GetDC();
	CDC bufferDC;
	bufferDC.CreateCompatibleDC(dc);

	DrawCapturedFrame(dc, &bufferDC);
	DrawToolBar(dc,&bufferDC);

	bufferDC.DeleteDC();
	ReleaseDC(dc);
}

void CScreenCaptureDlg::DrawCapturedFrame(CDC *DC, CDC* bufferDC)
{
	CBitmap BufferBmp;
	BufferBmp.CreateCompatibleBitmap(DC,m_width,m_height);
	bufferDC->SelectObject(&BufferBmp);

	CDC pdc;
	pdc.CreateCompatibleDC(bufferDC);
	pdc.SelectObject(m_backGround);
	bufferDC->BitBlt(0,0,m_width,m_height,&pdc,0,0,SRCCOPY);
	pdc.DeleteDC();

	if(m_rcSel[0].Width()==0 || m_rcSel[0].Height()==0)
	{
		// 截图初始时的绘制
		m_pngMask.DrawImage(bufferDC,0,0,m_width,m_height,0,0,8,8);
	}
	else
	{
		int x,y,w,h;
		GetCapturedCoord(x,y,w,h);

		// 绘制四块遮盖区域
		m_pngMask.DrawImage(bufferDC,0,0,m_width,y,0,0,8,8);
		m_pngMask.DrawImage(bufferDC,0,y+h,m_width,m_height-y-h,0,0,8,8);
		m_pngMask.DrawImage(bufferDC,0,y,x,h,0,0,8,8);
		m_pngMask.DrawImage(bufferDC,x+w,y,m_width-x-w,h,0,0,8,8);

		// 绘制截图边框线
		m_draw.DrawRect(bufferDC, x,y,w,h);

		CRect rc; 
		rc.left=x+2;
		rc.top=y-24;
		rc.right=x+70;
		rc.bottom=y-2;

		if(rc.top<0)
		{
			rc.top=y+2;rc.bottom=y+24;
		}
		// 绘制坐标信息区域
		m_pngMask.DrawImage(bufferDC,rc.left,rc.top,rc.Width(),rc.Height(),0,0,8,8);
		CString stxt;
		stxt.Format("%4d *%4d",w,h);
		m_draw.DrawText(bufferDC,stxt,rc);

		// 绘制工具栏
		int drawPosX = x + w;
		int drawPosY = y + h;
		if(drawPosY > m_height - 32)
		{
			drawPosY = y - 32;
			if ( y < 32)
			{
				drawPosY = y + h - 32;
			}
		}

		x=m_rcSel[0].left;
		y=m_rcSel[0].top;
		w=m_rcSel[0].Width();
		h=m_rcSel[0].Height();

		// 绘制8个拖拽点
		m_pngDot.DrawImage(bufferDC,x-2,y-2);
		m_rcSel[1].left=x-5;m_rcSel[1].top=y-5;

		m_pngDot.DrawImage(bufferDC,x+w/2-2,y-2);
		m_rcSel[2].left=x+w/2-5;m_rcSel[2].top=y-5;

		m_pngDot.DrawImage(bufferDC,x+w-3,y-2);
		m_rcSel[3].left=x+w-5;m_rcSel[3].top=y-5;

		m_pngDot.DrawImage(bufferDC,x-2,y+h/2-2);
		m_rcSel[4].left=x-5;m_rcSel[4].top=y+h/2-5;

		m_pngDot.DrawImage(bufferDC,x+w-3,y+h/2-2);
		m_rcSel[5].left=x+w-5;m_rcSel[5].top=y+h/2-5;

		m_pngDot.DrawImage(bufferDC,x-2,y+h-3);
		m_rcSel[6].left=x-5;m_rcSel[6].top=y+h-5;

		m_pngDot.DrawImage(bufferDC,x+w/2-2,y+h-3);
		m_rcSel[7].left=x+w/2-5;m_rcSel[7].top=y+h-5;

		m_pngDot.DrawImage(bufferDC,x+w-3,y+h-3);
		m_rcSel[8].left=x+w-5;m_rcSel[8].top=y+h-5;

		for(int i=1;i<9;i++)
		{
			m_rcSel[i].right=m_rcSel[i].left+10;
			m_rcSel[i].bottom=m_rcSel[i].top+10;
		}

		int count = m_labels.size();
		for(int i = 0; i < count; i++)
		{
			m_draw.DrawObj(bufferDC,m_labels[i]);
		}
	}

	//清理资源
	BufferBmp.DeleteObject();
	
}

BOOL CScreenCaptureDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message==WM_KEYDOWN)
	{
		if(pMsg->wParam==VK_ESCAPE && m_status == Captured)
		{
			if(m_status == Captured)
			{
				OnRButtonDown(NULL,NULL);
				return TRUE;
			}
			else
			{
				CDialog::OnCancel();
				return TRUE;
			}
		}	
	}
	return CDialog::PreTranslateMessage(pMsg);
}

void CScreenCaptureDlg::CapturingDraw(CPoint point)
{
	m_rcSel[0].right=point.x;
	m_rcSel[0].bottom=point.y;
	
	DrawFrame();	
}

void CScreenCaptureDlg::CursorChangeWhenCaptured(CPoint point)
{
	if(m_iCapture==0)
	{
		if (PtInRect(m_rcSel[0],point))
		{
			if (IsInToolArea(point) > -1)
			{
				SetCursor(m_hCursor[0]);
			}
			else
			{
				SetCursor(m_curSel[0]);
			}			
		}
		else
		{
			bool isOk = false;
			for(int i=8;i>=0;i--)
			{
				if(PtInRect(m_rcSel[i],point))
				{
					SetCursor(m_curSel[i]);
					isOk = true;
					break;
				}
			}
			if (!isOk)
			{
				SetCursor(m_hCursor[0]);
			}
		}
	}
}

void CScreenCaptureDlg::CursorChangeWhenDraw(CPoint point)
{
	if(m_iCapture==0)
	{
		if (PtInRect(m_rcSel[0],point))
		{
			if (IsInToolArea(point) > -1)
			{
				SetCursor(m_hCursor[0]);
			}
			else
			{
				SetCursor(m_hCursor[1]);
			}			
		}
		else
		{
			bool isOk = false;
			for(int i=8;i>=0;i--)
			{
				if(m_status == Captured && PtInRect(m_rcSel[i],point))
				{
					SetCursor(m_curSel[i]);
					isOk = true;
					break;
				}
			}
			if (!isOk)
			{
				SetCursor(m_hCursor[0]);
			}
		}
	}
}

void CScreenCaptureDlg::DragDraw( CPoint point )
{
	if(m_iCapture==9)
		SetCursor(m_hCursor[2]);
	else
		SetCursor(m_curSel[m_iCapture]);
	int x=point.x-m_ptLButtonDown.x;
	int y=point.y-m_ptLButtonDown.y;

	switch(m_iCapture)
	{
	case 1:
		m_rcSel[0].left+=x;
		m_rcSel[0].top+=y;
		break;
	case 2:
		m_rcSel[0].top+=y;
		break;
	case 3:
		m_rcSel[0].top+=y;
		m_rcSel[0].right+=x;
		break;
	case 4:
		m_rcSel[0].left+=x;
		break;
	case 5:
		m_rcSel[0].right+=x;
		break;
	case 6:
		m_rcSel[0].left+=x;
		m_rcSel[0].bottom+=y;
		break;
	case 7:
		m_rcSel[0].bottom+=y;
		break;
	case 8:
		m_rcSel[0].right+=x;
		m_rcSel[0].bottom+=y;
		break;
	case 9:
		MoveToSide(x,y,point);
		break;
	}
	m_ptLButtonDown=point;
	
	DrawFrame();
}

void CScreenCaptureDlg::LabelDraw( CPoint point )
{
	if (m_bStartDraw)
	{
		CDC *dc=GetDC();
		CDC bufferDC;
		bufferDC.CreateCompatibleDC(dc);

		DrawCapturedFrame(dc, &bufferDC);		

		m_ptLineEndPos = point;
		if (!PtInRect(m_rcSel[0],point))
		{
			CPoint lineEndPos = GetEndPos(point);
			m_ptLineEndPos.x = lineEndPos.x;
			m_ptLineEndPos.y = lineEndPos.y;		
		}

		if (m_eDrawType == LabelLine)
		{
			LineObj obj(m_ptStartDraw,m_ptLineEndPos);
			m_draw.DrawObj(&bufferDC,&obj);
		}
		else if (m_eDrawType == LabelRectangle)
		{
			RectangleObj obj(m_ptStartDraw,m_ptLineEndPos);
			m_draw.DrawObj(&bufferDC,&obj);
		}
		else if (m_eDrawType == LabelEllipse)
		{
			EllipseObj obj(m_ptStartDraw,m_ptLineEndPos);
			m_draw.DrawObj(&bufferDC,&obj);
		}

		DrawToolBar(dc,&bufferDC);

		bufferDC.DeleteDC();
		ReleaseDC(dc);
	}
}

void CScreenCaptureDlg::MoveToSide( int moveX,int moveY, CPoint point )
{
	// 左移
	if( moveX < 0 )
	{
		// 1. m_ptStopMove.x == VALUE_VALID 截图框未发生过右移停靠的情况，可左移
		// 2. 鼠标回到上一次右移停止时的坐标点时，左移才生效
		if (m_ptStopMove.x == VALUE_VALID || (m_ptStopMove.x != VALUE_VALID && m_ptStopMove.x >= point.x))
		{
			// 未到左边界时，才能左移截图框
			if(m_rcSel[0].left > 0)
			{
				// 左移不能超出左边界，超出就停靠
				if (m_rcSel[0].left + moveX <= 0)
				{
					m_rcSel[0].right -= m_rcSel[0].left;
					m_rcSel[0].left = 0;
					if (m_ptStopMove.x == VALUE_VALID)
					{
						m_ptStopMove.x = m_ptDragPos.x;
					}
				}
				else
				{
					m_rcSel[0].left += moveX;
					m_rcSel[0].right += moveX;
					if ((m_ptStopMove.x != VALUE_VALID && m_ptStopMove.x >= point.x))
					{
						m_ptStopMove.x = VALUE_VALID;
					}
				}
			}
		}								
	}
	// 右移同理
	else if(moveX > 0)
	{
		if (m_ptStopMove.x == VALUE_VALID || (m_ptStopMove.x != VALUE_VALID && m_ptStopMove.x <= point.x))
		{
			if(m_rcSel[0].right < m_width)
			{
				if (m_rcSel[0].right + moveX >= m_width)
				{
					m_rcSel[0].left += m_width - m_rcSel[0].right;
					m_rcSel[0].right = m_width;
					if (m_ptStopMove.x == VALUE_VALID)
					{
						m_ptStopMove.x = m_width - m_rcSel[0].Width() + m_ptDragPos.x;
					}
				}
				else
				{
					m_rcSel[0].left += moveX;
					m_rcSel[0].right += moveX;
					if ((m_ptStopMove.x != VALUE_VALID && m_ptStopMove.x <= point.x))
					{
						m_ptStopMove.x = VALUE_VALID;
					}
				}
			}
		}									
	}
	// 上移
	if ( moveY < 0 )
	{
		if (m_ptStopMove.y == VALUE_VALID || (m_ptStopMove.y != VALUE_VALID && m_ptStopMove.y >= point.y))
		{
			if(m_rcSel[0].top > 0)
			{
				if (m_rcSel[0].top + moveY <= 0)
				{
					m_rcSel[0].bottom -= m_rcSel[0].top;
					m_rcSel[0].top = 0;
					if (m_ptStopMove.y == VALUE_VALID)
					{
						m_ptStopMove.y = m_ptDragPos.y;
					}
				}
				else
				{
					m_rcSel[0].top += moveY;
					m_rcSel[0].bottom += moveY;
					if ((m_ptStopMove.y != VALUE_VALID && m_ptStopMove.y >= point.y))
					{
						m_ptStopMove.y = VALUE_VALID;
					}
				}
			}
		}
	}
	// 下移
	else if( moveY > 0 )
	{
		if (m_ptStopMove.y == VALUE_VALID || (m_ptStopMove.y != VALUE_VALID && m_ptStopMove.y <= point.y))
		{
			if(m_rcSel[0].bottom < m_height)
			{
				if (m_rcSel[0].bottom + moveY >= m_height)
				{
					m_rcSel[0].top += m_height - m_rcSel[0].bottom;
					m_rcSel[0].bottom = m_height;
					if (m_ptStopMove.y == VALUE_VALID)
					{
						m_ptStopMove.y = m_height - m_rcSel[0].Height() + m_ptDragPos.y;
					}
				}
				else
				{
					m_rcSel[0].top += moveY;
					m_rcSel[0].bottom += moveY;
					if ((m_ptStopMove.y != VALUE_VALID && m_ptStopMove.y <= point.y))
					{
						m_ptStopMove.y = VALUE_VALID;
					}
				}
			}
		}
	}
}

FloatPoint* CScreenCaptureDlg::GetLineIntersection( Line RefLine1, Line RefLine2 ,LineType type)
{
	FloatPoint* Intersection=new FloatPoint;
	float K1,K2,Delta_X1,Delta_Y1,Delta_X2,Delta_Y2;
	Delta_X1=RefLine1.End.x-RefLine1.Start.x;
	Delta_Y1=RefLine1.End.y-RefLine1.Start.y;
	Delta_X2=RefLine2.End.x-RefLine2.Start.x;
	Delta_Y2=RefLine2.End.y-RefLine2.Start.y;

	if (Delta_X1==0)
	{
		if (Delta_X2==0)
		{
			delete Intersection;
			return NULL;
		}
		else
		{
			K2=(float)Delta_Y2/((float)(Delta_X2));
			Intersection->x=RefLine1.Start.x;
			Intersection->y=K2*(Intersection->x- RefLine2.Start.x)+ RefLine2.Start.y ;
		}
	}
	else if (Delta_X2==0)
	{
		K1=(float)Delta_Y1/((float)(Delta_X1));
		Intersection->x=RefLine2.Start.x;
		Intersection->y=K1*(Intersection->x- RefLine1.Start.x)+ RefLine1.Start.y ;
	}
	else
	{
		K1=(float)Delta_Y1/((float)(Delta_X1));
		K2=(float)Delta_Y2/((float)(Delta_X2));
		if(K1==K2) 
		{
			delete Intersection;
			return NULL;
		}
		else
		{
			Intersection->x=(RefLine2.Start.y-RefLine1.Start.y-(K2*RefLine2.Start.x-K1*RefLine1.Start.x))/(K1-K2);
			Intersection->y=K1*(Intersection->x-RefLine1.Start.x)+RefLine1.Start.y;
		}
	}
	if (Intersection)
	{
		// 与下边线求出的相交点的值，超出下边线两端的不算交点，鼠标当前点在下边线以上时求出的交点也不算
		if (type == BottomLine && (RefLine1.End.y < RefLine2.Start.y ||Intersection->x < RefLine2.Start.x || Intersection->x > RefLine2.End.x))
		{
			delete Intersection;
			return NULL;
		}
		else if (type == RightLine && ( RefLine1.End.x < RefLine2.Start.x || Intersection->y < RefLine2.Start.y || Intersection->y > RefLine2.End.y))
		{
			delete Intersection;
			return NULL;
		}
		else if (type == LeftLine && ( RefLine1.End.x > RefLine2.Start.x || Intersection->y < RefLine2.Start.y || Intersection->y > RefLine2.End.y))
		{
			delete Intersection;
			return NULL;
		}
		else if(type == TopLine && (RefLine1.End.y > RefLine2.Start.y || Intersection->x < RefLine2.Start.x || Intersection->x > RefLine2.End.x))
		{
			delete Intersection;
			return NULL;
		}		
	}
	return Intersection;
}

CPoint CScreenCaptureDlg::GetEndPos( CPoint point )
{
	CPoint intersectPoint = point;

	if (m_eDrawType == LabelLine)
	{
		Line curLine;
		curLine.Start.x = m_ptStartDraw.x;
		curLine.Start.y = m_ptStartDraw.y;
		curLine.End.x = point.x;
		curLine.End.y = point.y;

		Line left;
		left.Start.x = m_rcSel[0].left;
		left.Start.y = m_rcSel[0].top;
		left.End.x = m_rcSel[0].left;
		left.End.y = m_rcSel[0].bottom;

		Line right;
		right.Start.x = m_rcSel[0].right;
		right.Start.y = m_rcSel[0].top;
		right.End.x = m_rcSel[0].right;
		right.End.y = m_rcSel[0].bottom;

		Line top;
		top.Start.x = m_rcSel[0].left;
		top.Start.y = m_rcSel[0].top;
		top.End.x = m_rcSel[0].right;
		top.End.y = m_rcSel[0].top;

		Line bottom;
		bottom.Start.x = m_rcSel[0].left;
		bottom.Start.y = m_rcSel[0].bottom;
		bottom.End.x = m_rcSel[0].right;
		bottom.End.y = m_rcSel[0].bottom;

		FloatPoint* Intersect = GetLineIntersection(curLine,bottom,BottomLine);
		if (Intersect)
		{
			intersectPoint.x = Intersect->x;
			intersectPoint.y = Intersect->y - VALUE_BOLDER_REDUCE;
			delete Intersect;
			return intersectPoint;
		}
		Intersect = GetLineIntersection(curLine,right,RightLine);
		if (Intersect)
		{
			intersectPoint.x = Intersect->x - VALUE_BOLDER_REDUCE;
			intersectPoint.y = Intersect->y;
			delete Intersect;
			return intersectPoint;
		}
		Intersect = GetLineIntersection(curLine,left,LeftLine);
		if (Intersect)
		{
			intersectPoint.x = Intersect->x + VALUE_BOLDER_REDUCE;
			intersectPoint.y = Intersect->y;
			delete Intersect;
			return intersectPoint;
		}
		Intersect = GetLineIntersection(curLine,top,TopLine);
		if (Intersect)
		{
			intersectPoint.x = Intersect->x;
			intersectPoint.y = Intersect->y + VALUE_BOLDER_REDUCE;
			delete Intersect;
			return intersectPoint;
		}
	}
	else if (m_eDrawType == LabelRectangle || m_eDrawType == LabelEllipse)
	{
		if (point.x > m_rcSel[0].right)
		{
			intersectPoint.x = m_rcSel[0].right - VALUE_BOLDER_REDUCE;
		}
		if (point.x < m_rcSel[0].left)
		{
			intersectPoint.x = m_rcSel[0].left + VALUE_BOLDER_REDUCE;
		}
		if (point.y < m_rcSel[0].top)
		{
			intersectPoint.y = m_rcSel[0].top + VALUE_BOLDER_REDUCE;
		}
		if (point.y > m_rcSel[0].bottom)
		{
			intersectPoint.y = m_rcSel[0].bottom - VALUE_BOLDER_REDUCE;
		}
	}
	
	return intersectPoint;
}

int CScreenCaptureDlg::IsInToolArea(CPoint point)
{
	for (int i = 0; i < 9; i++)
	{
		if (PtInRect(m_rcAction[i], point))
		{
			return i;
		}
	}
	return -1;
}

void CScreenCaptureDlg::GetCapturedCoord( int& x, int& y, int& w, int& h )
{
	// 反着截图时需要转换
	x=m_rcSel[0].left;
	y=m_rcSel[0].top;
	w=m_rcSel[0].Width();
	h=m_rcSel[0].Height();
	if(x>m_rcSel[0].right)
		x=m_rcSel[0].right;
	if(y>m_rcSel[0].bottom)
		y=m_rcSel[0].bottom;
	if(w<0)
		w=-w;
	if(h<0)
		h=-h;
}