#include "system_settings.h"


CSystemSettings::CSystemSettings()
{
}

CSystemSettings::~CSystemSettings()
{
}

void CSystemSettings::Notify(TNotifyUI& msg)
{
	if (_tcsicmp(msg.sType, DUI_MSGTYPE_WINDOWINIT) == 0)
	{
		OnPrepare(msg);
	}

	if (msg.sType == DUI_MSGTYPE_CLICK)
	{
		if (msg.pSender->GetName() == L"close_button")
		{
			Close();
		}
	}
	else if (msg.sType == DUI_MSGTYPE_ITEMSELECT)
	{

	}
	else if (msg.sType == DUI_MSGTYPE_ITEMCLICK)
	{

	}
	else if (msg.sType = DUI_MSGTYPE_SELECTCHANGED)
	{
		OnSelectChanged(msg);
	}
}

LRESULT CSystemSettings::MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
	if (uMsg == WM_KEYDOWN) 
	{
		if (wParam == VK_RETURN) 
		{
			return true;
		}
		else if (wParam == VK_ESCAPE) 
		{
			return true;
		}
	}

	return false;
}

LRESULT CSystemSettings::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
	styleValue &= ~WS_CAPTION;
	::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

	m_PaintManager.Init(m_hWnd);
	m_PaintManager.AddPreMessageFilter(this);
	CDialogBuilder builder;

	CControlUI* pRoot = builder.Create(L"systemsettings\\system_settings.xml", (UINT)0, 0, &m_PaintManager);

	ASSERT(pRoot && "Failed to parse XML");
	m_PaintManager.AttachDialog(pRoot);
	m_PaintManager.AddNotifier(this);
	return 0;
}

LRESULT CSystemSettings::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
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


	lRes = HandleCustomMessage(uMsg, wParam, lParam, bHandled);
	if (bHandled)
		return lRes;

	if (m_PaintManager.MessageHandler(uMsg, wParam, lParam, lRes)) return lRes;
	return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
}

LRESULT CSystemSettings::OnNcActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (::IsIconic(*this)) bHandled = FALSE;
	return (wParam == 0) ? TRUE : FALSE;
}

LRESULT CSystemSettings::OnNcCalcSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}

LRESULT CSystemSettings::OnNcPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}

LRESULT CSystemSettings::OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	POINT pt; pt.x = GET_X_LPARAM(lParam); pt.y = GET_Y_LPARAM(lParam);
	::ScreenToClient(*this, &pt);

	RECT rcClient;
	::GetClientRect(*this, &rcClient);

	RECT rcCaption = m_PaintManager.GetCaptionRect();
	if (pt.x >= rcClient.left + rcCaption.left && pt.x < rcClient.right - rcCaption.right \
		&& pt.y >= rcCaption.top && pt.y < rcCaption.bottom) {
		CControlUI* pControl = static_cast<CControlUI*>(m_PaintManager.FindControl(pt));
		if (pControl && _tcscmp(pControl->GetClass(), _T("ButtonUI")) != 0)
			return HTCAPTION;
	}

	return HTCLIENT;
}

LRESULT CSystemSettings::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	SIZE szRoundCorner = m_PaintManager.GetRoundCorner();
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

LRESULT CSystemSettings::OnMenuHandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}

LRESULT CSystemSettings::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (uMsg == WM_HIDE_IMAGE_WND_MSG)
	{
		::ShowWindow(m_hWnd, SW_HIDE);
	}
	return 0;
}

void CSystemSettings::OnPrepare(TNotifyUI& msg)
{
	//CControlUI *PosWnd = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(_T("show_image_control")));
}

LPCTSTR CSystemSettings::GetWindowClassName() const
{
	return _T("UIShowSystemSettingDlg");
}

UINT CSystemSettings::GetClassStyle() const
{
	return UI_CLASSSTYLE_DIALOG;
}

void CSystemSettings::ShowWnd(int type)
{
	::ShowWindow(m_hWnd, type);
}

void CSystemSettings::OnSelectChanged(TNotifyUI &msg)
{
	WCHAR OptionBtnName[MAX_64_LEN] = { 0 };
	CTabLayoutUI* pTabControl = static_cast<CTabLayoutUI*>(m_PaintManager.FindControl(_T("left_tab")));
	if (pTabControl != NULL)
	{
		for (int i = 0; i < 3; i++)
		{
			swprintf_s(OptionBtnName, _T("top_tab_button_%d"), i + 1);

			if (_tcsicmp(msg.pSender->GetName(), OptionBtnName) == 0)
			{
				if (pTabControl && pTabControl->GetCurSel() != i)
				{
					pTabControl->SelectItem(i);
					break;
				}
			}
		}
	}

	pTabControl = NULL;
	pTabControl = static_cast<CTabLayoutUI*>(m_PaintManager.FindControl(_T("right_frame_1")));
	if (pTabControl != NULL)
	{
		for (int i = 0; i < 5; i++)
		{
			swprintf_s(OptionBtnName, _T("left_tab_1_button_%d"), i + 1);

			if (_tcsicmp(msg.pSender->GetName(), OptionBtnName) == 0)
			{
				if (pTabControl && pTabControl->GetCurSel() != i)
				{
					pTabControl->SelectItem(i);
					break;
				}
			}
		}
	}
}
