#include "../stdafx.h"
#include "chat_inside_wnd.h"
#include "common_utility.h"





CChatInsideWnd::CChatInsideWnd()
{

}


CChatInsideWnd::~CChatInsideWnd()
{

}




void CChatInsideWnd::OnPrepare(TNotifyUI& msg)
{


}


void CChatInsideWnd::Notify(TNotifyUI& msg)
{
	if (_tcsicmp(msg.sType, DUI_MSGTYPE_WINDOWINIT) == 0)
	{

	}

	if (msg.sType == DUI_MSGTYPE_CLICK)
	{
		if (msg.pSender->GetName() == L"closeBtn_chat" || msg.pSender->GetName() == L"cancelChatInsideBtn")
		{
			Close();
		}
		else if (msg.pSender->GetName() == L"conformChatInsideBtn")
		{
			Close();
		}
	}
	if (msg.sType == DUI_MSGTYPE_ITEMSELECT)
	{

	}
	if (msg.sType == DUI_MSGTYPE_ITEMCLICK)
	{

	}
}



LRESULT CChatInsideWnd::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
	styleValue &= ~WS_CAPTION;
	::SetWindowLong(*this, GWL_STYLE, styleValue  | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

	m_pm.Init(m_hWnd);
	m_pm.AddPreMessageFilter(this);
	CDialogBuilder builder;

	CControlUI* pRoot = builder.Create(L"topwnd\\chat_inside.xml", (UINT)0, 0, &m_pm);
	ASSERT(pRoot && "Failed to parse XML");
	m_pm.AttachDialog(pRoot);
	m_pm.AddNotifier(this);

	return 0;
}

LRESULT CChatInsideWnd::OnNcActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (::IsIconic(*this)) bHandled = FALSE;
	return (wParam == 0) ? TRUE : FALSE;
}

LRESULT CChatInsideWnd::OnNcCalcSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}

LRESULT CChatInsideWnd::OnNcPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}

LRESULT CChatInsideWnd::OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
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

LRESULT CChatInsideWnd::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
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

LRESULT CChatInsideWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
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
	case WM_MY_MESSAGE_NOTIFYICON:    HandleCustomMessage(uMsg, wParam, lParam, bHandled); break;
	default:
		bHandled = FALSE;
	}
	if (bHandled) return lRes;
	if (m_pm.MessageHandler(uMsg, wParam, lParam, lRes)) return lRes;
	return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
}

LRESULT CChatInsideWnd::MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{

	if (uMsg == WM_KEYDOWN) {
		if (wParam == VK_RETURN) {
			CEditUI* pEdit = static_cast<CEditUI*>(m_pm.FindControl(DEF_ACCOUNT_ID_EDIT));
			if (pEdit->GetText().IsEmpty()) pEdit->SetFocus();
			else {
				pEdit = static_cast<CEditUI*>(m_pm.FindControl(DEF_PASSWORD_TEXT_EDIT));
				if (pEdit->GetText().IsEmpty()) pEdit->SetFocus();
				else Close();
			}
			return true;
		}
		else if (wParam == VK_ESCAPE) {
			//PostQuitMessage(0);
			return true;
		}

	}

	return false;
}



LRESULT CChatInsideWnd::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{



	return 0;
}


