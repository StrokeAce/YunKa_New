#include "../stdafx.h"
#include "select_online_user_wnd.h"
#include "common_utility.h"




class CSelectWndBuilderCallback : public IDialogBuilderCallback
{
	CControlUI* CreateControl(LPCTSTR pstrClass)
	{
		if (_tcsicmp(pstrClass, L"OnlineUserList") == 0)
		{
			return new UserListUI;
		}
	
		return NULL;
	}
};


CSelectOnlineUserWnd::CSelectOnlineUserWnd()
{
	_globalSetting.m_currentSelectUserId = 0;
}


CSelectOnlineUserWnd::~CSelectOnlineUserWnd()
{

}




void CSelectOnlineUserWnd::OnPrepare(TNotifyUI& msg)
{
	WCHAR showText[128] = {0};
	CDuiString duiText;

	m_pOnlineUserList = static_cast<UserListUI*>(m_pm.FindControl(_T("onlineSelectList")));

	m_pOnlineUserList->SetListName(_T("onlineSelectList"));



	list<CGroupObject *>::iterator iter = m_pGroup->m_ListGroupInfo.begin();

	for (; iter != m_pGroup->m_ListGroupInfo.end(); iter++)
	{

		ANSIToUnicode((*iter)->strName.c_str(), showText);
		UserListUI::Node*  group = m_pOnlineUserList->AddNode(showText, 0);

		list<CUserObject * >::iterator iteruser = (*iter)->m_ListUserInfo.begin(); //m_pGroup->m_ListUserInfo.begin();
		for (; iteruser != (*iter)->m_ListUserInfo.end(); iteruser++)
		{

			ANSIToUnicode((*iteruser)->UserInfo.nickname, showText);
			duiText = L"{x 12}";
			duiText += showText;
			m_pOnlineUserList->AddNode(showText, (*iteruser)->UserInfo.uid,"", group);
		}
	}
}


void CSelectOnlineUserWnd::Notify(TNotifyUI& msg)
{
	if (_tcsicmp(msg.sType, DUI_MSGTYPE_WINDOWINIT) == 0)
	{
		OnPrepare(msg);
	}

	if (msg.sType == DUI_MSGTYPE_CLICK)
	{
		if (msg.pSender->GetName() == L"closeBtn_chat" || msg.pSender->GetName() == L"cancelChatInsideBtn")
		{
			PostQuitMessage(0);
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
		OnItemClick(msg);
	}
}



LRESULT CSelectOnlineUserWnd::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
	styleValue &= ~WS_CAPTION;
	::SetWindowLong(*this, GWL_STYLE, styleValue  | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

	m_pm.Init(m_hWnd);
	m_pm.AddPreMessageFilter(this);
	CDialogBuilder builder;
	CSelectWndBuilderCallback selectCallback;

	CControlUI* pRoot = builder.Create(L"topwnd\\select_online_user.xml", (UINT)0, &selectCallback, &m_pm);
	ASSERT(pRoot && "Failed to parse XML");
	m_pm.AttachDialog(pRoot);
	m_pm.AddNotifier(this);

	return 0;
}

LRESULT CSelectOnlineUserWnd::OnNcActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (::IsIconic(*this)) bHandled = FALSE;
	return (wParam == 0) ? TRUE : FALSE;
}

LRESULT CSelectOnlineUserWnd::OnNcCalcSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}

LRESULT CSelectOnlineUserWnd::OnNcPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}

LRESULT CSelectOnlineUserWnd::OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
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

LRESULT CSelectOnlineUserWnd::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
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

LRESULT CSelectOnlineUserWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
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

	default:
		bHandled = FALSE;
	}
	if (bHandled) return lRes;
	if (m_pm.MessageHandler(uMsg, wParam, lParam, lRes)) return lRes;
	return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
}

LRESULT CSelectOnlineUserWnd::MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{

	if (uMsg == WM_KEYDOWN) {
		if (wParam == VK_RETURN) {
			CEditUI* pEdit = static_cast<CEditUI*>(m_pm.FindControl(DEF_ACCOUNT_ID_EDIT));
			if (pEdit->GetText().IsEmpty()) pEdit->SetFocus();
			else {
				pEdit = static_cast<CEditUI*>(m_pm.FindControl(DEF_PASSWORD_TEXT_EDIT));
				if (pEdit->GetText().IsEmpty()) pEdit->SetFocus();
				//else Close();
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



LRESULT CSelectOnlineUserWnd::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{



	return 0;
}



void CSelectOnlineUserWnd::OnItemClick(TNotifyUI &msg)
{

	if (msg.pSender->GetName() == _T("onlineSelectList"))
	{
		if (m_pOnlineUserList->GetItemIndex(msg.pSender) != -1)
		{
			if (_tcscmp(msg.pSender->GetClass(), _T("ListLabelElementUI")) == 0)
			{

				UserListUI::Node* node = (UserListUI::Node*)msg.pSender->GetTag();
				CDuiString str = node->data()._text;
				_globalSetting.m_currentSelectUserId = node->data()._uid;

			}
		}

	}

}