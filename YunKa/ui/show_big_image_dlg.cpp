#include "../stdafx.h"
#include "show_big_image_dlg.h"
#include "common_utility.h"
#include "utils\code_convert.h"




CShowBigImageDlg::CShowBigImageDlg()
{
	isCreate = false;
	m_msgDataType = MSG_DATA_TYPE_TEXT;
}


CShowBigImageDlg::~CShowBigImageDlg()
{

}




void CShowBigImageDlg::OnPrepare(TNotifyUI& msg)
{

#if 1
	//聊天窗口 初始化
	m_pShowImageHandler.handler = NULL;
	m_pShowImageHandler.handleName = Handler_ShowImage;
	m_pShowImageHandler.isLoaded = false;
	m_pShowImageHandler.isCreated = false;


	m_pShowImageHandler.handler = new ClientHandler();
	m_pShowImageHandler.handler->m_isDisplayRefresh = false;


	if (!m_pShowImageHandler.isCreated)
	{
		CControlUI *PosWnd = static_cast<CHorizontalLayoutUI*>(m_pm.FindControl(_T("show_image_control")));

		RECT rect, padRect;
		rect = PosWnd->GetPos();
		m_pShowImageHandler.handler->CreateBrowser(m_hWnd, rect, "about:blank", Handler_ShowImage);
	}

#endif
}

RECT CShowBigImageDlg::GetPos()
{


	CControlUI *PosWnd = static_cast<CHorizontalLayoutUI*>(m_pm.FindControl(_T("show_image_control")));

	RECT rect, padRect;
	rect = PosWnd->GetPos();

	return rect;
}


void CShowBigImageDlg::Notify(TNotifyUI& msg)
{
	if (_tcsicmp(msg.sType, DUI_MSGTYPE_WINDOWINIT) == 0)
	{
		OnPrepare(msg);
	}

	if (msg.sType == DUI_MSGTYPE_CLICK)
	{
		if (msg.pSender->GetName() == L"closeBtn_show_image" )
		{
			//Close();

			::PostMessage(m_hWnd, WM_HIDE_IMAGE_WND_MSG, NULL, NULL);
			//isCreate = false;
			//PostQuitMessage(0);
		}
	}
	if (msg.sType == DUI_MSGTYPE_ITEMSELECT)
	{

	}
	if (msg.sType == DUI_MSGTYPE_ITEMCLICK)
	{
	
	}
}



LRESULT CShowBigImageDlg::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
	styleValue &= ~WS_CAPTION;
	::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

	m_pm.Init(m_hWnd);
	m_pm.AddPreMessageFilter(this);
	CDialogBuilder builder;

	CControlUI* pRoot = builder.Create(L"showimage\\show_image.xml", (UINT)0, 0, &m_pm);

	//CControlUI* pRoot = builder.Create(L"topwnd\\select_visitor.xml", (UINT)0, 0, &m_pm);
	ASSERT(pRoot && "Failed to parse XML");
	m_pm.AttachDialog(pRoot);
	m_pm.AddNotifier(this);


	return 0;
}

LRESULT CShowBigImageDlg::OnNcActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (::IsIconic(*this)) bHandled = FALSE;
	return (wParam == 0) ? TRUE : FALSE;
}

LRESULT CShowBigImageDlg::OnNcCalcSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}

LRESULT CShowBigImageDlg::OnNcPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}

LRESULT CShowBigImageDlg::OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	POINT pt; pt.x = GET_X_LPARAM(lParam); pt.y = GET_Y_LPARAM(lParam);
	::ScreenToClient(*this, &pt);

	RECT rcClient;
	::GetClientRect(*this, &rcClient);

	RECT rcCaption = m_pm.GetCaptionRect();
	if (pt.x >= rcClient.left + rcCaption.left && pt.x < rcClient.right - rcCaption.right \
		&& pt.y >= rcCaption.top && pt.y < rcCaption.bottom) {
		CControlUI* pControl = static_cast<CControlUI*>(m_pm.FindControl(pt));
		if (pControl && _tcscmp(pControl->GetClass(), _T("ButtonUI")) != 0)
			return HTCAPTION;
	}

	return HTCLIENT;
}

LRESULT CShowBigImageDlg::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	SIZE szRoundCorner = m_pm.GetRoundCorner();
	if (!::IsIconic(*this) && (szRoundCorner.cx != 0 || szRoundCorner.cy != 0)) {
		CDuiRect rcWnd;
		::GetWindowRect(*this, &rcWnd);
		rcWnd.Offset(-rcWnd.left, -rcWnd.top);
		rcWnd.right++; rcWnd.bottom++;
		HRGN hRgn = ::CreateRoundRectRgn(rcWnd.left, rcWnd.top, rcWnd.right, rcWnd.bottom, szRoundCorner.cx, szRoundCorner.cy);
		::SetWindowRgn(*this, hRgn, TRUE);
		::DeleteObject(hRgn);
	}

	bHandled = FALSE;
	return 0;
}

LRESULT CShowBigImageDlg::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lRes = 0;
	BOOL bHandled = TRUE;
	switch (uMsg) {
	case WM_CREATE:        lRes = OnCreate(uMsg, wParam, lParam, bHandled); break;
	case WM_NCACTIVATE:    lRes = OnNcActivate(uMsg, wParam, lParam, bHandled); break;
	case WM_NCCALCSIZE:    lRes = OnNcCalcSize(uMsg, wParam, lParam, bHandled); break;
	case WM_NCPAINT:       lRes = OnNcPaint(uMsg, wParam, lParam, bHandled); break;
	case WM_NCHITTEST:     lRes = OnNcHitTest(uMsg, wParam, lParam, bHandled); break;
	case WM_SIZE:          lRes = OnSize(uMsg, wParam, lParam, bHandled); break;
	case WM_MY_MESSAGE_NOTIFYICON:    HandleCustomMessage(uMsg, wParam, lParam,bHandled); break;
	default:
		bHandled = FALSE;
	}
	if (bHandled) return lRes;


	lRes = HandleCustomMessage(uMsg, wParam, lParam, bHandled);
	if (bHandled) 
		return lRes;

	if (m_pm.MessageHandler(uMsg, wParam, lParam, lRes)) return lRes;
	return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
}

LRESULT CShowBigImageDlg::MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{



	if (uMsg == WM_KEYDOWN) {
		if (wParam == VK_RETURN) {
			return true;
		}
		else if (wParam == VK_ESCAPE) {
			//PostQuitMessage(0);
			return true;
		}

	}

	return false;
}


void CShowBigImageDlg::ShowBigImage(string url, MSG_DATA_TYPE msgDataType)
{
	if (m_pShowImageHandler.isCreated)
	{
		if (msgDataType == MSG_DATA_TYPE_LOCATION)
		{
			m_pShowImageHandler.handler->GetBrowser()->GetMainFrame()->LoadURL(url);
		}
		else if (msgDataType == MSG_DATA_TYPE_IMAGE)
		{
			m_pShowImageHandler.handler->GetBrowser()->GetMainFrame()->LoadURL(url);
		}
	}
	else
	{
		m_url = url;
		m_msgDataType = msgDataType;
	}
}


void CShowBigImageDlg::ShowWnd(int type)
{
	m_pShowImageHandler.handler->ShowBrowser(type);

	::ShowWindow(m_hWnd, type);
}

LRESULT CShowBigImageDlg::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (uMsg == WM_HIDE_IMAGE_WND_MSG)
	{
		//if (m_pShowImgDlg)
			//m_pShowImgDlg->ShowWindow(SW_HIDE);
		//::ShowWindow(m_hWnd, SW_HIDE);
		ShowWnd(SW_HIDE);
	}
	


    if (uMsg == ON_AFTER_CREATED)
	{
		string msg = *(string*)wParam;

		if (Handler_ShowImage == msg)
			m_pShowImageHandler.isCreated = true;

		if (m_msgDataType == MSG_DATA_TYPE_LOCATION)
		{
			m_pShowImageHandler.handler->GetBrowser()->GetMainFrame()->LoadURL(m_url);
		}
		else if (m_msgDataType == MSG_DATA_TYPE_IMAGE)
		{
			m_pShowImageHandler.handler->GetBrowser()->GetMainFrame()->LoadURL(m_url);
		}
	}
	else if (uMsg == ON_AFTER_LOAD)
	{
		string msg = *(string*)wParam;
		if (Handler_ShowImage == msg)
			m_pShowImageHandler.isLoaded = true;
	}


	return 0;
}


