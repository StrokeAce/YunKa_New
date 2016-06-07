// ScreenCaptureDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ScreenCapture.h"
#include "ScreenCapDlg.h"
#include <atlimage.h>
#include <imagehlp.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define VALUE_VALID -500
#define VALUE_BOLDER_REDUCE 5
#define WM_CAPTURE_FINISHED (WM_USER + 699)
#define SAVE_TYPE_FILE 0
#define SAVE_TYPE_CLIPBOARD 1
#define SAVE_TYPE_BOTH 2

ScreenCapDlg::ScreenCapDlg(CWnd* pParent)
: CDialog(ScreenCapDlg::IDD, pParent)
{
	GdiplusStartupInput gdi;
	ULONG_PTR	gdiToken; 
	GdiplusStartup(&gdiToken,&gdi,NULL);

	CString path = GetModuleDir();
	m_pngMask.LoadImage(path+"\\res\\screen_capture\\sc_mask.png");
	m_pngDot.LoadImage(path+"\\res\\screen_capture\\sc_dot.png");
	m_pngAction.LoadImage(path+"\\res\\screen_capture\\sc_action.png");
	m_pngActionClick.LoadImage(path+"\\res\\screen_capture\\sc_action_click.png");

	m_width=GetSystemMetrics(SM_CXSCREEN);
	m_height=GetSystemMetrics(SM_CYSCREEN);

	//��ȡ��Ļ�ֱ���
	m_xScreen = GetSystemMetrics(SM_CXSCREEN);
	m_yScreen = GetSystemMetrics(SM_CYSCREEN);

	//��ȡ��Ļ��λͼ��
	CRect rect(0, 0,m_xScreen,m_yScreen);
	m_pBitmap=CBitmap::FromHandle(CopyScreenToBitmap(&rect));
	m_backGround = m_pBitmap;

	// ��ʼ��Ϊ��Чֵ
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

ScreenCapDlg::~ScreenCapDlg()
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

void ScreenCapDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(ScreenCapDlg, CDialog)
	//{{AFX_MSG_MAP(ScreenCapDlg)
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
// ScreenCapDlg message handlers

BOOL ScreenCapDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	//�ѶԻ������ó�ȫ�����㴰��
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

void ScreenCapDlg::OnPaint() 
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

void ScreenCapDlg::OnOK()
{
	SaveTo();
	CDialog::OnOK();
}

void ScreenCapDlg::OnCancel()
{
	CDialog::OnCancel();
}

void ScreenCapDlg::OnMouseMove(UINT nFlags, CPoint point) 
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

void ScreenCapDlg::SaveTo(CString strSaveFile, CString strExt)
{
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

	if (m_saveType == SAVE_TYPE_FILE || m_saveType == SAVE_TYPE_BOTH || !strSaveFile.IsEmpty())
	{
		if (strSaveFile.IsEmpty())
			strSaveFile = m_path;
		HRESULT saveSucess = FALSE;
		CImage image;
		image.Attach(BufferBmp);
		if (strExt == "bmp")
			saveSucess = image.Save(strSaveFile, Gdiplus::ImageFormatBMP);
		else if (strExt == "jpg")
			saveSucess = image.Save(strSaveFile, Gdiplus::ImageFormatJPEG);
		else
			saveSucess = image.Save(strSaveFile, Gdiplus::ImageFormatPNG);

		if (saveSucess)
			::SendMessage(m_hwnd, WM_CAPTURE_FINISHED, (WPARAM)strSaveFile.GetBuffer(), 0);
	}

	if (m_saveType == SAVE_TYPE_CLIPBOARD || m_saveType == SAVE_TYPE_BOTH)
	{
		if (::OpenClipboard(m_hWnd))
		{
			::EmptyClipboard();
			::SetClipboardData(CF_BITMAP, BufferBmp);
			::CloseClipboard();

			if (m_saveType == SAVE_TYPE_CLIPBOARD)
			{
				::SendMessage(m_hwnd, WM_CAPTURE_FINISHED, 0, 0);
			}
		}
	}

	BufferDC.DeleteDC();
	BufferBmp.DeleteObject();
	ReleaseDC(dc);

	m_rcSel[0].right=m_rcSel[0].left;
	m_rcSel[0].bottom=m_rcSel[0].top;

	CDialog::OnCancel();
}

void ScreenCapDlg::OnLButtonDown(UINT nFlags, CPoint point) 
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

void ScreenCapDlg::OnLButtonUp(UINT nFlags, CPoint point) 
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
		/*if (PtInRect(m_rcAction[3],point))
		{
			m_status = Draw;
			m_eDrawType = LabelPen;
		}
		if (PtInRect(m_rcAction[4],point))
		{
			m_status = Draw;
			m_eDrawType = LabelText;
		}*/
		if(PtInRect(m_rcAction[5],point))
		{
			m_labels.pop_back();
		}
		if(PtInRect(m_rcAction[6],point))
		{
			CFileDialog fd(FALSE, "PNG", "δ����", 4 | 2, "PNG ͼƬ��ʽ (*.png)|*.png|BMP ͼƬ��ʽ (*.bmp)|*.bmp|JPG ͼƬ��ʽ (*.jpg)|*.jpg||");
			if (fd.DoModal() == IDCANCEL)
				return;
			CString strExt = ".png";
			strExt = fd.GetFileExt();
			strExt.Trim(); 
			strExt.MakeLower();
			CString strSaveFile = fd.GetPathName();
			SaveTo(strSaveFile, strExt);
		}
		if (PtInRect(m_rcAction[7],point))
		{
			m_status = Inital;
			m_eDrawType = NotLabel;
			OnRButtonDown(NULL,NULL);
		}
		if (PtInRect(m_rcAction[8],point))
		{
			OnOK();
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

void ScreenCapDlg::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	if(PtInRect(m_rcSel[0],point))
	{
		if (IsInToolArea(point) < 0)
		{
			OnOK();
		}		
	}

	CDialog::OnLButtonDblClk(nFlags, point);
}

void ScreenCapDlg::OnRButtonDown(UINT nFlags, CPoint point) 
{
	if (!PtInRect(m_rcSel[0],point))
	{
		CDialog::OnCancel();
	}
	
	CDialog::OnRButtonDown(nFlags, point);
}

BOOL ScreenCapDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	return TRUE;
}

void ScreenCapDlg::DrawToolBar(CDC *DC,CDC* bufferDC)
{
	if(m_status == Captured || m_status == Capturing || m_status == Drawing || m_status == Draw || m_status == Drag)
	{
		int x,y,w,h;
		GetCapturedCoord( x,y,w,h);

		// ���ƹ�����
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

		m_rcAction[0].left=posX-222;
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

		m_rcAction[1].left=posX-192;
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

		m_rcAction[2].left=posX-163;
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

		/*m_rcAction[3].left=posX-189;
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
		}*/		

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

	//�滭����
	DC->BitBlt(0,0,m_width,m_height,bufferDC,0,0,SRCCOPY);
}

//������Ļ��λͼ��
HBITMAP ScreenCapDlg::CopyScreenToBitmap(LPRECT lpRect,BOOL bSave)
//lpRect ����ѡ������
{
	HDC       hScrDC, hMemDC;
	// ��Ļ���ڴ��豸������
	HBITMAP    hBitmap, hOldBitmap;
	// λͼ���
	int       nX, nY, nX2, nY2;
	// ѡ����������
	int       nWidth, nHeight;

	// ȷ��ѡ������Ϊ�վ���
	if (IsRectEmpty(lpRect))
		return NULL;
	//Ϊ��Ļ�����豸������
	hScrDC = CreateDC("DISPLAY", NULL, NULL, NULL);

	//Ϊ��Ļ�豸�����������ݵ��ڴ��豸������
	hMemDC = CreateCompatibleDC(hScrDC);
	// ���ѡ����������
	nX = lpRect->left;
	nY = lpRect->top;
	nX2 = lpRect->right;
	nY2 = lpRect->bottom;

	//ȷ��ѡ�������ǿɼ���
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
	// ����һ������Ļ�豸��������ݵ�λͼ
	hBitmap = CreateCompatibleBitmap(hScrDC, nWidth, nHeight);
	// ����λͼѡ���ڴ��豸��������
	hOldBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);
	// ����Ļ�豸�����������ڴ��豸��������
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

	//��� 
	DeleteDC(hScrDC);
	DeleteDC(hMemDC);
	return hBitmap;
}

//�ػ�����
void ScreenCapDlg::PaintWindow()
{
	//��ȡ��ȫ���Ի��򴰿ڴ�С
	CRect rect1;
	GetWindowRect(rect1);

	CRgn rgn1,rgn2;
	rgn1.CreateRectRgnIndirect(rect1);

	InvalidateRect(rect1);
}

void ScreenCapDlg::SetCapInfo(HWND hWnd, int saveType, LPSTR path)
{
	m_hwnd = hWnd;
	m_path = path;
	m_saveType = saveType;
}

void ScreenCapDlg::DrawFrame()
{
	CDC *dc=GetDC();
	CDC bufferDC;
	bufferDC.CreateCompatibleDC(dc);

	DrawCapturedFrame(dc, &bufferDC);
	DrawToolBar(dc,&bufferDC);

	bufferDC.DeleteDC();
	ReleaseDC(dc);
}

void ScreenCapDlg::DrawCapturedFrame(CDC *DC, CDC* bufferDC)
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
		// ��ͼ��ʼʱ�Ļ���
		m_pngMask.DrawImage(bufferDC,0,0,m_width,m_height,0,0,8,8);
	}
	else
	{
		int x,y,w,h;
		GetCapturedCoord(x,y,w,h);

		// �����Ŀ��ڸ�����
		m_pngMask.DrawImage(bufferDC,0,0,m_width,y,0,0,8,8);
		m_pngMask.DrawImage(bufferDC,0,y+h,m_width,m_height-y-h,0,0,8,8);
		m_pngMask.DrawImage(bufferDC,0,y,x,h,0,0,8,8);
		m_pngMask.DrawImage(bufferDC,x+w,y,m_width-x-w,h,0,0,8,8);

		// ���ƽ�ͼ�߿���
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
		// ����������Ϣ����
		m_pngMask.DrawImage(bufferDC,rc.left,rc.top,rc.Width(),rc.Height(),0,0,8,8);
		CString stxt;
		stxt.Format("%4d *%4d",w,h);
		m_draw.DrawText(bufferDC,stxt,rc);

		// ���ƹ�����
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

		// ����8����ק��
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

	//������Դ
	BufferBmp.DeleteObject();
}

BOOL ScreenCapDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message==WM_KEYDOWN)
	{
		if(pMsg->wParam==VK_ESCAPE)
		{
			CDialog::OnCancel();
			return TRUE;
		}	
	}
	return CDialog::PreTranslateMessage(pMsg);
}

void ScreenCapDlg::CapturingDraw(CPoint point)
{
	m_rcSel[0].right=point.x;
	m_rcSel[0].bottom=point.y;

	DrawFrame();	
}

void ScreenCapDlg::CursorChangeWhenCaptured(CPoint point)
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

void ScreenCapDlg::CursorChangeWhenDraw(CPoint point)
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

void ScreenCapDlg::DragDraw( CPoint point )
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

void ScreenCapDlg::LabelDraw( CPoint point )
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

void ScreenCapDlg::MoveToSide( int moveX,int moveY, CPoint point )
{
	// ����
	if( moveX < 0 )
	{
		// 1. m_ptStopMove.x == VALUE_VALID ��ͼ��δ����������ͣ���������������
		// 2. ���ص���һ������ֹͣʱ�������ʱ�����Ʋ���Ч
		if (m_ptStopMove.x == VALUE_VALID || (m_ptStopMove.x != VALUE_VALID && m_ptStopMove.x >= point.x))
		{
			// δ����߽�ʱ���������ƽ�ͼ��
			if(m_rcSel[0].left > 0)
			{
				// ���Ʋ��ܳ�����߽磬������ͣ��
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
	// ����ͬ��
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
	// ����
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
	// ����
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

FloatPoint* ScreenCapDlg::GetLineIntersection( Line RefLine1, Line RefLine2 ,LineType type)
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
		// ���±���������ཻ���ֵ�������±������˵Ĳ��㽻�㣬��굱ǰ�����±�������ʱ����Ľ���Ҳ����
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

CPoint ScreenCapDlg::GetEndPos( CPoint point )
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

int ScreenCapDlg::IsInToolArea(CPoint point)
{
	for (int i = 0; i < 9; i++)
	{
		// �±�3��4�������겻�������
		if (i != 3 && i != 4)
		{
			if (PtInRect(m_rcAction[i], point))
			{
				return i;
			}
		}		
	}
	return -1;
}

void ScreenCapDlg::GetCapturedCoord( int& x, int& y, int& w, int& h )
{
	// ���Ž�ͼʱ��Ҫת��
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

CString ScreenCapDlg::GetModuleDir()
{
	HMODULE module = GetModuleHandle(0); 
	char pFileName[MAX_PATH]; 
	GetModuleFileName(module, pFileName, MAX_PATH); 

	CString csFullPath(pFileName); 
	int nPos = csFullPath.ReverseFind( _T('\\') );
	if( nPos < 0 ) 
		return CString(""); 
	else 
		return csFullPath.Left( nPos );
}

CString ScreenCapDlg::OnGetTodayDate() 
{ 
	CString strToday;

	CTime tm;
	int yy;
	int mm;
	int dd;
	int hh;
	int nn;
	int ss;

	yy = tm.GetCurrentTime().GetYear();
	mm = tm.GetCurrentTime().GetMonth();
	dd = tm.GetCurrentTime().GetDay();
	hh = tm.GetCurrentTime().GetHour();
	nn = tm.GetCurrentTime().GetMinute();
	ss = tm.GetCurrentTime().GetSecond();

	CString Today_year=_T( " "); 
	CString Today_month=_T( " "); 
	CString Today_day=_T( " "); 
	CString Today_hour=_T( " "); 
	CString Today_minute=_T( " "); 
	CString Today_second=_T( " "); 

	//��������CString������ֵ�� 
	Today_year.Format( "%d",yy); 
	Today_month.Format( "%d",mm); 
	Today_day.Format( "%d",dd);
	Today_hour.Format( "%d",hh); 
	Today_minute.Format( "%d",nn); 
	Today_second.Format( "%d",ss); 

	//����·�ֻ��һλ��������ǰ����� "0 "�� 
	if(Today_month.GetLength()==1) 
	{ 
		Today_month=_T( "0")+Today_month; 
	} 

	//�������ֻ��һλ��������ǰ����� "0 "�� 
	if(Today_day.GetLength()==1) 
	{ 
		Today_day=_T( "0")+Today_day; 
	} 

	if(Today_hour.GetLength()==1) 
	{ 
		Today_hour=_T( "0")+Today_hour; 
	} 

	if(Today_minute.GetLength()==1) 
	{ 
		Today_minute=_T( "0")+Today_minute; 
	} 

	if(Today_second.GetLength()==1) 
	{ 
		Today_second=_T( "0")+Today_second; 
	} 

	strToday = Today_year + Today_month + Today_day + Today_hour + Today_minute + Today_second;

	//���ؽ���� 
	return strToday; 
} 

_declspec(dllexport) void StartCaptureScreen(HWND hWnd, int saveType, LPSTR path)
{  
	AFX_MANAGE_STATE(AfxGetStaticModuleState());  

	if(hWnd == NULL)
		return;

	ScreenCapDlg* pScreenCapDlg = new ScreenCapDlg();
	pScreenCapDlg->SetCapInfo(hWnd,saveType,path);
	INT_PTR nResponse = pScreenCapDlg->DoModal();
	if (nResponse != IDOK)
	{
		if (pScreenCapDlg != NULL)
		{
			delete pScreenCapDlg;
			pScreenCapDlg = NULL;
		}		
	}
	return;
}