#include "../stdafx.h"
#include "main_frame.h"
#include "login_wnd.h"
#include "ui_menu.h"
#include "ui_common/common_utility.h"
#include "IImageOle.h"
#include "ole_helper.h"
#include "chat_common\comfunc.h"
#include "utils\code_convert.h"
#include "menu_wnd.h"
#include <WinUser.h>
#include "jpeg_file/JpegFile.h"
#include "select_online_user_wnd.h"
#include "select_visitor_wnd.h"
#include <ShlObj.h>
#include <GdiPlus.h>



#pragma comment(lib, "gdiplus.lib")


#if 0

CMainFrame::CMainFrame(CChatManager* manager) :m_manager(manager)
{
	//初始化
	m_pFontBtn = m_pFaceBtn = m_pScreenBtn = pSendMsgBtn = m_pVoiceBtn = NULL;
	m_pRightCommonWordCombo = m_pRightCommonTypeCombo = m_pRightCommonFindCombo = NULL;
	m_pRightCommonWordEdit = m_pRightCommonTypeEdit = m_pRightCommonFindEdit = NULL;
	m_MainCenterRightWND = NULL;
	m_pSendEdit = NULL;

	pOnlineNode = pWaitForAccept = pMySelfeNode = NULL;

	m_recordWaitNumber = 0;

	m_facePathUrl = "<IMG alt=\"\" src=\"face.gif\">";

	m_rightRectWnd = { 0 };

	CCodeConvert f_covet;
	string strTmp = "<html><head>";
	strTmp += STRING_HTML_META;
	strTmp += STRING_HTML_BASE;
	strTmp += STRING_HTML_STYLE;
	strTmp += "</head><body>";
	strTmp += "<div><p><span style=\"font - size:16px; color:#cccccc\">无可显示信息</span></p><div></body>";
	f_covet.Gb2312ToUTF_8(m_defaultUrlInfo, strTmp.c_str(), strTmp.length());
	
	m_savedImageIndex = 1000;
	m_curSelectOptionBtn = 0;
	m_bRecording = false;

	m_pLastOfflineNode = m_pLastOnlineNode = 0;
	memset(m_pManagerBtn, 0, sizeof(m_pManagerBtn));
	m_userListCount = m_recordListCount = 0;
	m_invoteOtherList.clear();
	m_invoteMyselfList.clear();

	//m_pShowImgDlg = NULL;

	pShowImgDlg = NULL;
	m_wndShow = 0;
	m_defalutButtonImage = L"";

	memset(&m_centerChatInfo, 0, sizeof(m_centerChatInfo));
	memset(&m_rightRectMax, 0, sizeof(m_rightRectMax));
}



CMainFrame::~CMainFrame()
{
	delete m_manager;
}

LPCTSTR CMainFrame::GetWindowClassName() const
{
	return _T("CMainFrame");
}

CControlUI* CMainFrame::CreateControl(LPCTSTR pstrClass)
{


	//if (_tcscmp(pstrClass, _T("RichEdit2")) == 0)
	//	return new DuiLib2::CRichEditUI2;
	
	if (_tcscmp(pstrClass, _T("talklist")) == 0)
		return new UserListUI;

	if (_tcscmp(pstrClass, _T("UserList")) == 0) 
		return new UserListUI;



	return NULL;
}

void CMainFrame::OnFinalMessage(HWND hWnd)
{

	WindowImplBase::OnFinalMessage(hWnd);

}


CDuiString CMainFrame::GetSkinFile()
{
	return _T("MainFrame.xml");
}

CDuiString CMainFrame::GetSkinFolder()
{
	return  _T("SkinRes\\");
}



LRESULT CMainFrame::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = FALSE;

	//::RevokeDragDrop(m_hWnd);
	//m_faceList.Reset();

	return 0;
}


LRESULT CMainFrame::OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	// 有时会在收到WM_NCDESTROY后收到wParam为SC_CLOSE的WM_SYSCOMMAND
	if (wParam == SC_CLOSE) 
	{
		PostQuitMessage(0); //防止有模态窗口时，从任务栏关闭会报错
		bHandled = TRUE;
		return 0;
	}

	BOOL bZoomed = ::IsZoomed(*this);
	LRESULT lRes = CWindowWnd::HandleMessage(uMsg, wParam, lParam);

	if (::IsZoomed(*this) != bZoomed) {
		if (!bZoomed) {
			CControlUI* pControl = static_cast<CControlUI*>(m_PaintManager.FindControl(_T("maxBtn")));
			if (pControl) pControl->SetVisible(false);
			pControl = static_cast<CControlUI*>(m_PaintManager.FindControl(_T("restoreBtn")));
			if (pControl) pControl->SetVisible(true);
			
			MoveAndRestoreMsgWnd(0);
			MoveAndRestoreRightFrameControl(0);
		}
		else {
			CControlUI* pControl = static_cast<CControlUI*>(m_PaintManager.FindControl(_T("maxBtn")));
			if (pControl) pControl->SetVisible(true);
			pControl = static_cast<CControlUI*>(m_PaintManager.FindControl(_T("restoreBtn")));
			if (pControl) pControl->SetVisible(false);

			MoveAndRestoreMsgWnd(1);
			MoveAndRestoreRightFrameControl(1);
		}
	}

	return lRes;

}

LRESULT CMainFrame::ResponseDefaultKeyEvent(WPARAM wParam)
{
	if (wParam == VK_RETURN)
	{
		return FALSE;
	}
	else if (wParam == VK_ESCAPE)
	{
		return TRUE;
	}
	return FALSE;
}

LRESULT CMainFrame::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	TNotifyUI msg = {};

	if (uMsg == WM_SHOWWINDOW)
	{
	}
	else if (uMsg == WM_MOUSEMOVE)
		OnMouseMove(uMsg, wParam, lParam);

	else if (uMsg == WM_RBUTTONDOWN)
	{
	}

	else if (uMsg == WM_KILLFOCUS)
	{
	}
	else if (uMsg == WM_SETFOCUS)
	{
	}

	else if (uMsg == WM_LBUTTONDOWN)
	{
	}

	else if (uMsg == WM_KEYDOWN) // 发送消息框的Ctrl+V消息	
	{
		if ((wParam == 'V' || wParam == 'v') && ::GetKeyState(VK_CONTROL) < 0)
		{
			if ((m_pSendEdit != NULL) && m_pSendEdit->IsFocused())
			{
				OnCtrlVEvent();
				return TRUE;
			}
		}

		if ((wParam == 'Q' ||  wParam == 'q') && ::GetKeyState(VK_CONTROL) < 0)
		{
		
			OnBtnScreen(msg);
			return TRUE;
		}



		if (wParam == VK_RETURN)   //m_manager->m_sysConfig->m_nKeySendType == 0)
		{

			int state = ::GetKeyState(VK_CONTROL);

			if (m_manager->m_sysConfig->m_nKeySendType == 0 && ::GetKeyState(VK_CONTROL) >= 0)
			{
				if (m_pSendEdit != NULL && m_pSendEdit->IsFocused())
				{
					OnBtnSendMessage(msg);
					return true;
				}
			}
			if (m_manager->m_sysConfig->m_nKeySendType == 1 && ::GetKeyState(VK_CONTROL) < 0)
			{
				if (m_pSendEdit != NULL && m_pSendEdit->IsFocused())
				{
					OnBtnSendMessage(msg);
					return true;
				}
			}

		}



	}


#if 0
		if (wParam == VK_RETURN  && ::GetKeyState(VK_CONTROL) > 0)   //m_manager->m_sysConfig->m_nKeySendType == 0)
		{
			 
			if (m_manager->m_sysConfig->m_nKeySendType == 0)
			{
				if (m_pSendEdit != NULL && m_pSendEdit->IsFocused())
				{
					OnBtnSendMessage(msg);
					return 0;
				}
			}
			else if (m_manager->m_sysConfig->m_nKeySendType == 1)
			{
				if (m_pSendEdit != NULL && m_pSendEdit->IsFocused())
				{
					return __super::HandleMessage(uMsg, wParam, lParam);
				}
			}	
		
		}
		if (wParam == VK_RETURN && ::GetKeyState(VK_CONTROL) < 0 ) 
		{
			if (m_pSendEdit != NULL && m_pSendEdit->IsFocused() && m_manager->m_sysConfig->m_nKeySendType == 1)
			{
				OnBtnSendMessage(msg);
			}
		}



/*
			if (m_controlKeyState == 0)



			if (m_manager->m_sysConfig->m_nKeySendType == 0 && (m_pSendEdit != NULL) && m_pSendEdit->IsFocused())
			    OnBtnSendMessage(msg);
		}
	
		else  if (wParam == VK_RETURN && m_controlKeyState == 1)
		{
			if (m_manager->m_sysConfig->m_nKeySendType == 1 && (m_pSendEdit != NULL) && m_pSendEdit->IsFocused())
				OnBtnSendMessage(msg);
		}
*/


	}

#endif

//	if (uMsg == WM_NOTIFY && EN_PASTE == ((LPNMHDR)lParam)->code)
//	{
	//	ITextServices * pTextServices = m_pSendEdit->GetTextServices();
	//	if ((UINT)pTextServices == ((LPNMHDR)lParam)->idFrom)
	//	{
	//		AddMsgToSendEdit(((NMRICHEDITOLECALLBACK *)lParam)->lpszText);
//		}
//		if (pTextServices != NULL)
//			pTextServices->Release();
	//}

	//if (uMsg == WM_MENU)
	//{
	//	CControlUI * pControl = (CControlUI *)lParam;
	//	if (pControl != NULL)
	//		this->paint_manager_.SendNotify(pControl, _T("menu_msg"), wParam, lParam);
	//}

	//if (uMsg == FACE_CTRL_SEL)
		//return OnFaceCtrlSel(uMsg, wParam, lParam);

	return __super::HandleMessage(uMsg, wParam, lParam);
}

LRESULT CMainFrame::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (uMsg == WM_FACE_CTRL_SEL)
	{

		OnFaceCtrlSel(uMsg, wParam, lParam);
	}

	else if (uMsg == WM_SCREEN_CAPTURE_SUCCED) //截图完成后的消息操作
	{
		OnCtrlVEvent();
	}

	else if (uMsg == ON_AFTER_CREATED)
	{
		string msg = *(string*)wParam;

		if (Handler_ListMsg == msg)
		    m_pListMsgHandler.isCreated = true;

		else if (Handler_VisitorRelated == msg)
		{
			m_pVisitorRelatedHandler.isCreated = true;
			m_pVisitorRelatedHandler.handler->ShowBrowser(SW_HIDE);
		}
		else if (Handler_WebUrl == msg)
		{
			m_pWebURLHandler.isCreated = true;
			m_pWebURLHandler.handler->ShowBrowser(SW_HIDE);
		}

		if (Handler_ShowImage == msg)
		{
			//m_pShowImageHandler.isCreated = true;
			//m_pShowImageHandler.handler->ShowBrowser(SW_SHOW);
		}
	
	}
	else if (uMsg == ON_AFTER_LOAD)
	{
		string msg = *(string*)wParam;

		if (Handler_ListMsg == msg)
		   m_pListMsgHandler.isLoaded = true;

		else if (Handler_VisitorRelated == msg)
		{
			m_pVisitorRelatedHandler.isLoaded = true;
		}

		else if (Handler_WebUrl == msg)
		{
			m_pWebURLHandler.isLoaded = true;
		}

		if (Handler_ShowImage == msg)
		{
			//m_pShowImageHandler.isLoaded = true;
			//m_pShowImageHandler.handler->ShowBrowser(SW_SHOW);
		}

	}
	else if (uMsg == ON_JS_CALL_MFC)
	{
		JsCallMFC(wParam, lParam);
	}

	else if (uMsg == WM_MENU_START)
	{ 
		WCHAR *name = (WCHAR *)wParam;
		CDuiString conName = name;
		OnMenuEvent(conName);

		delete []name;
	}

	else if (uMsg == WM_DOUBLE_CLICK_SHOW_WND_MSG)
	{
		m_wndShow = false;
		m_frameSmallMenu.SetMenuType(0);

		if (::IsIconic(this->m_hWnd))
			::ShowWindow(this->m_hWnd, SW_RESTORE);
		else
			::ShowWindow(this->m_hWnd, SW_SHOW);
	}
	
	else if (uMsg == WM_GET_CEF_FOCUS_MSG)
	{
		CControlUI *control = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(_T("HorizontalLayoutUI_MiddleButtonBar")));
		if (control != NULL)
			control->SetFocus();
	}




	m_frameSmallMenu.HandleCustomMessage(uMsg, wParam, lParam);

	return 0;
}


LRESULT CMainFrame::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam)
{

	int x = GET_X_LPARAM(lParam);
	int y = GET_Y_LPARAM(lParam);

	return 0;
}


UILIB_RESOURCETYPE CMainFrame::GetResourceType() const
{
	return UILIB_FILE;
}




void CMainFrame::Notify(TNotifyUI& msg)
{

	if (_tcsicmp(msg.sType, DUI_MSGTYPE_WINDOWINIT) == 0)
	{
		OnPrepare(msg);
	}
	else if (_tcsicmp(msg.sType, DUI_MSGTYPE_KILLFOCUS) == 0)
	{
	}

	else if (_tcsicmp(msg.sType, DUI_MSGTYPE_CLICK) == 0)
	{
		OnClick(msg);
		OnSelectChanged(msg);
	}

	else if (_tcsicmp(msg.sType, DUI_MSGTYPE_TIMER) == 0)
	{
		return OnTimer(msg);
	}
	else if (_tcsicmp(msg.sType, DUI_MSGTYPE_SELECTCHANGED) == 0)
	{

	}

	else if (_tcsicmp(msg.sType, DUI_MSGTYPE_ITEMACTIVATE) == 0)
	{
		OnItemActive(msg);

	}

	else if (_tcsicmp(msg.sType, DUI_MSGTYPE_MENU) == 0)
	{
		OnItemRbClick(msg);
	}

	//else if (_tcsicmp(msg.sType, DUI_MSGTYPE_ITEMSELECT) == 0)
	//{
	//	OnItemClick(msg);
	//}

	else if (_tcsicmp(msg.sType, DUI_MSGTYPE_ITEMCLICK) == 0)
	{
		OnItemClick(msg);
	}






}



void CMainFrame::OnTimer(TNotifyUI& msg)
{

	if (msg.pSender == pUserList)
	{
	//	m_PaintManager.KillTimer(pUserList);
		//延时3秒中后再去 去在线用户列表
	//	m_manager->SendTo_GetListChatInfo();

		/*
	//	AddOnlineVisitor(pUserList, NULL, -1);
		m_PaintManager.KillTimer(pUserList);
		list<CUserObject* >::iterator  iter = m_upUser.begin();
		for (; iter != m_upUser.end(); iter++)
		{

			RecvOnline(*iter);

		}
		//UserListUI::Node* tempNode = iter->second;
		m_upUser.clear();
		*/
	}

}

void CMainFrame::OnCloseBtn(TNotifyUI& msg)
{
	m_manager->Exit();
	m_frameSmallMenu.DeleteSmallIcon();
	Close();
	PostQuitMessage(0);


}


void CMainFrame::MoveAndRestoreMsgWnd(int type)
{
	int leftWidth = 0;
	RECT rc = { 0 }, rect = {0};
	RECT sysRect;
	CDuiString formatString;
	int changeType = 0;

	CControlUI *leftLayout = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(_T("VerticalLayoutUI_LeftFrame")));
	CControlUI *lineLayout = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(_T("VerticalLayoutUI_LeftLine1")));
	CControlUI *centerLayout = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(_T("VerticalLayoutUI_CenterFrame")));
	CControlUI *rightLayout = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(_T("VerticalLayoutUI_RightFrame")));
	CControlUI *ShowMsgWnd = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(_T("HorizontalLayoutUI_ShowMsg")));

	CControlUI *ShowRightWebWnd = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(_T("right_tab")));


	CControlUI *optionButtonControl = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(_T("right_option_button_HorizontalLayout")));

	CButtonUI *button0 = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("option_button_5")));
	CButtonUI *button1 = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("option_button_6")));
	CButtonUI *button2 = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("option_button_7")));

	if (type == 0) //max
	{
		GetWindowRect(this->m_hWnd, &sysRect);

		if (sysRect.right >= 1280)
			changeType = 1;

		//设置按钮背景高度
		if (changeType == 1)
		    optionButtonControl->SetAttribute(L"height", L"26");

		leftWidth = leftLayout->GetWidth();
		leftWidth += lineLayout->GetWidth();
		
		int centerWidth = (sysRect.right - leftWidth) / 2;
		int rightWidth = sysRect.right - leftWidth - centerWidth;

		if (m_centerChatInfo.centerFrameWitdh == 0)
		    m_centerChatInfo.centerFrameWitdh = centerLayout->GetWidth();
		if (m_centerChatInfo.showMsgWidth == 0)
		    m_centerChatInfo.showMsgWidth = ShowMsgWnd->GetWidth();

		formatString.Format(_T("%d"), centerWidth);
		centerLayout->SetAttribute(_T("width"), formatString);
		formatString.Format(_T("%d"), rightWidth);
		//rightLayout->SetAttribute(_T("width"), formatString);  //lxh 屏蔽掉 

		//聊天框框
		rc = ShowMsgWnd->GetPos();
		rc.right = rc.left + centerWidth - 2;
		m_pListMsgHandler.handler->MoveBrowser(rc);

		//右侧的订单显示web
		if (m_rightRectWnd.right == 0)
			m_rightRectWnd = ShowRightWebWnd->GetPos();

		rect.left = rc.right + 4;
		rect.right = sysRect.right - 4;
		if (changeType == 1)
			rect.top += m_rightRectWnd.top-26;
		else
			rect.top += m_rightRectWnd.top - 26;

		rect.bottom = sysRect.bottom - 4;
		m_pVisitorRelatedHandler.handler->MoveBrowser(rect);
		if (m_rightRectMax.right == 0)
		    m_rightRectMax = rect;

		//管理中心显示web
		rect.left = m_mainCenterAndRightRect.left;
		rect.right = sysRect.right - 4;
		rect.top = m_mainCenterAndRightRect.top;
		rect.bottom = sysRect.bottom - 4;
		m_pWebURLHandler.handler->MoveBrowser(rect);

		//右侧 option 按钮移动
		if (changeType == 1)
		{
			rect = button0->GetPos();
			rc = { rect.right, rect.top, rect.right + 70, rect.top + 26 };
			button1->SetPos(rc);
			rc = { rc.right, rc.top, rc.right + 70, rc.top + 26 };
			button2->SetPos(rc);
		}
	}
	else
	{
		formatString.Format(_T("%d"), m_centerChatInfo.centerFrameWitdh);
		centerLayout->SetAttribute(_T("width"), formatString);

		rc = ShowMsgWnd->GetPos();
		rc.right = rc.left + m_centerChatInfo.showMsgWidth;
		m_pListMsgHandler.handler->MoveBrowser(rc);

		//设置按钮背景高度
		optionButtonControl->SetAttribute(L"height", L"52");

		//rc = ShowRightWebWnd->GetPos();
		m_pVisitorRelatedHandler.handler->MoveBrowser(m_rightRectWnd);
		m_pWebURLHandler.handler->MoveBrowser(m_mainCenterAndRightRect);

		//右侧 option 按钮移动
		button0 = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("option_button_1")));
		rect = button0->GetPos();
		rc = { rect.left, rect.bottom, rect.left + 70, rect.bottom + 26 };
		button1->SetPos(rc);
		rc = { rc.right, rc.top, rc.right + 70, rc.top + 26 };
		button2->SetPos(rc);
	}
}



void CMainFrame::OnMaxBtn(TNotifyUI& msg)
{

	SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE, 0);

}



void CMainFrame::OnRestoreBtn(TNotifyUI& msg)
{

	SendMessage(WM_SYSCOMMAND, SC_RESTORE, 0);

}


void CMainFrame::OnMinBtn(TNotifyUI& msg)
{

	SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0);
	//this->ShowWindow(FALSE);
	//SetProcessWorkingSetSize(::GetCurrentProcess(), -1, -1);

}



void CMainFrame::InitLibcef(void)
{
	//聊天窗口 初始化
	m_pListMsgHandler.handler = NULL;
	m_pListMsgHandler.handleName = Handler_ListMsg;
	m_pListMsgHandler.isLoaded = false;
	m_pListMsgHandler.isCreated = false;
	//右侧访客窗口
	m_pVisitorRelatedHandler.handler = NULL;
	m_pVisitorRelatedHandler.handleName = Handler_VisitorRelated;
	m_pVisitorRelatedHandler.isLoaded = false;
	m_pVisitorRelatedHandler.isCreated = false;

	
    // 访客历史 访客来电 访客留言 客户管理 统计分析
	m_pWebURLHandler.handler = NULL;
	m_pWebURLHandler.handleName = Handler_WebUrl;
	m_pWebURLHandler.isLoaded = false;
	m_pWebURLHandler.isCreated = false;


	//显示聊天内容的libcef窗口
	m_pListMsgHandler.handler = new ClientHandler();
	m_pListMsgHandler.handler->m_isDisplayRefresh = false;

	m_pVisitorRelatedHandler.handler = new ClientHandler();
	m_pVisitorRelatedHandler.handler->m_isDisplayRefresh = false;

	m_pWebURLHandler.handler = new ClientHandler();
	m_pWebURLHandler.handler->m_isDisplayRefresh = false;


	if (!m_pListMsgHandler.isCreated)
	{
		string localUrl = GetCurrentPath();
		localUrl += ("\\html\\list.html");
		CCodeConvert f_covet;
		string utfUrl;
		f_covet.Gb2312ToUTF_8(utfUrl, localUrl.c_str(), localUrl.length());
		//这里需要根据每个控件的位置  计算起始大小
		//RECT rc = { 308, 144, 308 + 379, 399 + 144 };

		CControlUI *ShowMsgWnd = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(_T("HorizontalLayoutUI_ShowMsg")));

		RECT rect, padRect;
		rect = ShowMsgWnd->GetPos();
		padRect = ShowMsgWnd->GetPadding();
		int width = ShowMsgWnd->GetWidth();
		int height = ShowMsgWnd->GetHeight();

		m_pListMsgHandler.handler->CreateBrowser(this->m_hWnd, rect, utfUrl, Handler_ListMsg);
		//m_pListMsgHandler.handler->CreateBrowser(this->m_hWnd, rect, "www.baidu.com", Handler_ListMsg);

	}

	if (!m_pVisitorRelatedHandler.isCreated)
	{

		CControlUI *ShowMsgWnd = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(_T("right_tab")));

		RECT rect, padRect;
		rect = ShowMsgWnd->GetPos();
		padRect = ShowMsgWnd->GetPadding();

		m_pVisitorRelatedHandler.handler->CreateBrowser(this->m_hWnd, rect, "about:blank", Handler_VisitorRelated);

		m_pVisitorRelatedHandler.handler->ShowBrowser(SW_HIDE);	
	}

	if (!m_pWebURLHandler.isCreated)
	{
		CControlUI *m_MainCenterRightWND = static_cast<CVerticalLayoutUI*>(m_PaintManager.FindControl(_T("VerticalLayoutUI_CenterRight_WebShow")));
     	m_mainCenterAndRightRect = m_MainCenterRightWND->GetPos();
		m_pWebURLHandler.handler->CreateBrowser(this->m_hWnd, m_mainCenterAndRightRect, "about:blank", Handler_WebUrl);
		m_pWebURLHandler.handler->ShowBrowser(SW_HIDE);
	}



	//显示大图窗口
	//m_pShowImageHandler.handler = NULL;
	//m_pShowImageHandler.handleName = Handler_ShowImage;
	//m_pShowImageHandler.isLoaded = false;
	//m_pShowImageHandler.isCreated = false;


	//m_pShowImageHandler.handler = new ClientHandler();
	//m_pShowImageHandler.handler->m_isDisplayRefresh = false;

	


}


void CMainFrame::SetHandler()
{

	m_manager->SetHandlerMsgs(this);

}

void CMainFrame::OnPrepare(TNotifyUI& msg)
{
	CDuiString nameString = _T("");
	CDuiString typeString[4] = { _T("对话中"), _T("转接中"), _T("邀请中"), _T("内部对话") };
	WCHAR  buf[128] = {0};


	_globalSetting.m_showWnd = 1;
	//cef窗口
	InitLibcef();

	//聊天框中间栏按钮
	m_pFontBtn = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btnFont")));
	m_pFaceBtn = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btnFace")));
	m_pScreenBtn = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("screenshotsbtn")));
	pSendMsgBtn = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("sendMsgBtn")));

	m_pVoiceBtn = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("voiceSendBtn")));


	m_pRightCommonWordCombo = static_cast<CComboUI*>(m_PaintManager.FindControl(_T("right_type_1")));
	m_pRightCommonTypeCombo = static_cast<CComboUI*>(m_PaintManager.FindControl(_T("right_type_2")));
	m_pRightCommonFindCombo = static_cast<CComboUI*>(m_PaintManager.FindControl(_T("right_type_3")));

	m_pRightCommonWordEdit = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("edit_right_type_1")));
	m_pRightCommonTypeEdit = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("edit_right_type_2")));
	m_pRightCommonFindEdit = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("edit_right_type_3")));

	CButtonUI* pButton = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("option_button_1")));
	if (pButton != NULL)
	{
		if (m_defalutButtonImage.IsEmpty())
			m_defalutButtonImage = pButton->GetNormalImage();

		pButton->SetNormalImage(pButton->GetFocusedImage());
	}

	MoveAndRestoreRightFrameControl(1);
		
	//上层管理按钮 设置初始状态
	UpdateTopCenterButtonState(0);

	
	//右下角小图标
	CreateSmallTaskIcon(DEFINE_SMALL_ICON_PATH);
	//m_frameSmallMenu.Init();
	//CDuiString path = GetCurrentPathW();
	//path += DEFINE_SMALL_ICON_PATH;
	//m_frameSmallMenu.CreateSmallIcon(this->m_hWnd, (WCHAR*)path.GetData());

	//表情包初始化
	wstring strPath = ZYM::CPath::GetCurDir() + _T("../bin/") _T("SkinRes\\Face\\FaceConfig.xml");
	m_faceList.LoadConfigFile(strPath.c_str());

	//左侧用户列表显示
    pUserList = static_cast<UserListUI*>(m_PaintManager.FindControl(_T("userlist")));
	pUserList->SetListBKImage(_T("file = 'mainframe\\tree_top.png' corner = '2,1,2,1' fade = '100'"));
	pUserList->SetListName(_T("userlist"));
	pWaitForAccept = NULL;
	pWaitForAccept = pUserList->AddNode(_T("{x 4}{i gameicons.png 18 0}{x 4}等待开始"), 0);
	//先添加自己的位置
	AddMyselfToList(pUserList, &m_manager->m_userInfo);
	//再添加最后一个的位置
	AddOnlineVisitor(pUserList, NULL, -1);



#if 0	
	UserListUI::Node* pCategoryNode = NULL;
	UserListUI::Node* pGameNode = NULL;
	UserListUI::Node* pServerNode = NULL;
	UserListUI::Node* pRoomNode = NULL;
	pCategoryNode = pUserList->AddNode(_T("{x 4}{i gameicons.png 18 3}{x 4}对话列表"));

	nameString.Format(_T("{x 4}{i gameicons.png 18 10}{x 4}等待开始"));
	pGameNode = pUserList->AddNode(nameString, pCategoryNode);

	for (int i = 0; i < 4; ++i)
	{
		nameString.Format(_T("{x 4}{i gameicons.png 18 10}{x 4}测试%d"), i + 1);
		pGameNode = pUserList->AddNode(nameString, pCategoryNode);
		for (int i = 0; i < 3; ++i)
		{
			nameString = _T("{x 4}{i gameicons.png 18 10}{x 4}");
			
			nameString += typeString[i];
		
			pServerNode = pUserList->AddNode(nameString, pGameNode);
			//for (int i = 0; i < 3; ++i)
			//{
			//	pRoomNode = pGameList->AddNode(_T("{x 4}{i gameicons.png 18 10}{x 4}测试房间"), pServerNode);
			//}
		}
	}
#endif

	//发送按钮
	m_pSendEdit = static_cast<CRichEditUI*>(m_PaintManager.FindControl(_T("richSend")));
	m_pSendEdit->SetText(_T(""));
	m_pSendEdit->SetFocus();
	//richedit
	IRichEditOleCallback2* pRichEditOleCallback2 = NULL;
	HRESULT hr = ::CoCreateInstance(CLSID_ImageOle, NULL, CLSCTX_INPROC_SERVER,
		__uuidof(IRichEditOleCallback2), (void**)&pRichEditOleCallback2);
	if (SUCCEEDED(hr))
	{
		pRichEditOleCallback2->SetNotifyHwnd(m_hWnd);
		ITextServices * pTextServices = m_pSendEdit->GetTextServices();
		pRichEditOleCallback2->SetTextServices(pTextServices);
		pTextServices->Release();
		m_pSendEdit->SetOleCallback(pRichEditOleCallback2);
		pRichEditOleCallback2->Release();
	}
	IDropTarget *pdt = m_pSendEdit->GetTxDropTarget();
	hr = ::RegisterDragDrop(m_hWnd, pdt);
	pdt->Release();


	//请求坐席列表
	SendMsgToGetList();

	m_manager->SendTo_GetQuickReply(0);
}




void CMainFrame::OnClick(TNotifyUI& msg)
{
	CDuiString msgName = msg.pSender->GetName();
	int findPos = msgName.Find(_T("managerbutton_"));

	if (msg.pSender == m_pFontBtn)
		OnBtnFont(msg);
	else if (msg.pSender == m_pFaceBtn)
		OnBtnFace(msg);
	else if (msg.pSender == m_pScreenBtn)
		OnBtnScreen(msg);
	else  if (msg.pSender == pSendMsgBtn)
		OnBtnSendMessage(msg);
	else if (msg.pSender == m_pVoiceBtn)
		OnBtnVoice(msg);
	else if (msg.pSender->GetName() == _T("mid_button_1")) //接受
	{
		AcceptChat();
	}
	else if (msg.pSender->GetName() == _T("mid_button_2")) //结束
	{
		m_manager->SendTo_CloseChat(m_curSelectId, CHATCLOSE_USER);
	}
	else if (msg.pSender->GetName() == _T("mid_button_3")) //转接
	{
		m_topWndType = 1;
		//获取在线坐席数
		m_manager->SendTo_GetOnlineUser();
	}
	else if (msg.pSender->GetName() == _T("mid_button_4"))//屏蔽
	{

	}
	else if (msg.pSender->GetName() == _T("mid_button_5"))//邀请评价
	{
	}
	else if (msg.pSender->GetName() == _T("mid_button_6"))//释放
	{
		m_manager->SendTo_ReleaseChat(m_curSelectId);
	}
	else if (msg.pSender->GetName() == _T("mid_button_7")) //邀请协助
	{
		m_topWndType = 2;
		//获取在线坐席数
		m_manager->SendTo_GetOnlineUser();
	}

	else if (msg.pSender->GetName() == _T("fileSendBtn"))
	{
		OnBtnSendFile(msg);
	}

	if (msg.pSender->GetName() == DEF_CLOSE_WND_BUTTON)
	{
		m_wndShow = true;
		m_frameSmallMenu.SetMenuType(1);
		::ShowWindow(this->m_hWnd, SW_HIDE);

		//OnCloseBtn(msg);
	}
	else  if (msg.pSender->GetName() == DEF_MIN_WND_BUTTON)
	{
		OnMinBtn(msg);
	}
	else  if (msg.pSender->GetName() == DEF_RESTORE_WND_BUTTON)
	{

		OnRestoreBtn(msg);
	}
	else  if (msg.pSender->GetName() == DEF_MAX_WND_BUTTON)
	{
		OnMaxBtn(msg);
	}

	else  if (msg.pSender->GetName() == _T("sendMsgBtn_select"))
	{
		OnBtnSelectSendType(msg);
	}


	if (findPos >= 0)
	{

		OnManagerButtonEvent( msg);
	}



}

void CMainFrame::OnSelectChanged(TNotifyUI &msg)
{
	WCHAR OptionBtnName[32] = { 0 };

	CTabLayoutUI* pTabControl = static_cast<CTabLayoutUI*>(m_PaintManager.FindControl(_T("right_tab")));
	//操作tab
	if (pTabControl != NULL)
	{
		//int index = pTabControl->GetCurSel() + 1;

		//这里因为右边屏幕 有7个option选项 
		for (int i = 0; i < 7; i++)
		{
			swprintf_s(OptionBtnName, _T("option_button_%d"), i + 1);

			if (_tcsicmp(msg.pSender->GetName(), OptionBtnName) == 0)
			{

				

				if (pTabControl && pTabControl->GetCurSel() != i)
				{
					pTabControl->SelectItem(i);
					m_curSelectOptionBtn = i;
					ShowRightOptionFrameView(m_curSelectId, m_curSavedSid);

					CButtonUI* pButton = static_cast<CButtonUI*>(m_PaintManager.FindControl(OptionBtnName));
					if (pButton != NULL)
						pButton->SetNormalImage(pButton->GetFocusedImage());
					
					break;
				}
			}
		}
	}



	for (int i = 0; i < 7; i++)
	{
		swprintf_s(OptionBtnName, _T("option_button_%d"), i + 1);
		CButtonUI* pButton = static_cast<CButtonUI*>(m_PaintManager.FindControl(OptionBtnName));
		if (pButton != NULL)
		{
			if (m_curSelectOptionBtn != i)
			{
				if (!m_defalutButtonImage.IsEmpty())
				    pButton->SetNormalImage(m_defalutButtonImage);
			}
		}
	}


}


void CMainFrame::OnItemClick(TNotifyUI &msg)
{
	m_savedClickId = 0;

	if (msg.pSender->GetName() == _T("userlist"))
	{
		if (pUserList->GetItemIndex(msg.pSender) != -1)
		{
			if (_tcscmp(msg.pSender->GetClass(), _T("ListLabelElementUI")) == 0)
			{
				//显示 rightoption 和中间的聊天界面
				HideWebBrowser();

				UserListUI::Node* node = (UserListUI::Node*)msg.pSender->GetTag();
				m_curClickItemNode = node;
				CDuiString str = node->data()._text;
				m_savedClickId = node->data()._uid;
				m_curSavedSid = node->data()._sid;
				
				OnItemClickEvent(m_savedClickId, 0);
			}
		}
	}

	
}

void CMainFrame::OnItemActive(TNotifyUI &msg)
{
	//if (pUserList->GetItemIndex(msg.pSender) != -1)
	if (msg.pSender->GetName() == _T("userlist"))
	{
		if (_tcscmp(msg.pSender->GetClass(), _T("ListLabelElementUI")) == 0) 
		{
			UserListUI::Node* node = (UserListUI::Node*)msg.pSender->GetTag();
			pUserList->ExpandNode(node, !node->data()._expand);

			CDuiString str = node->data()._text;
			unsigned long uid = node->data()._uid;
			string sid = node->data()._sid;

			OnActiveUser(uid,sid);
			  
		}
	}

	else if (msg.pSender->GetName() == _T("talklist"))
	{
		if (_tcscmp(msg.pSender->GetClass(), _T("ListLabelElementUI")) == 0)
		{
			UserListUI::Node* node = (UserListUI::Node*)msg.pSender->GetTag();
			m_pTalkList->ExpandNode(node, !node->data()._expand);
			string sidText = node->data()._sid;

			DoRightShortAnswerList(sidText);
		}
	}
}


void CMainFrame::OnItemRbClick(TNotifyUI &msg)
{
	CDuiString xmlPath = L"";
	unsigned long uid = m_savedClickId;
	string sid = m_curSavedSid;
	CWebUserObject *pWebUser = NULL;
	CUserObject *pUser = NULL;
	int step = 0;
	int height = 0;

	if (msg.pSender->GetName() == L"userlist")
	{
		VISITOR_TYPE  type = CheckIdForTalkType(uid);
		switch (type)
		{//坐席状态
			case HOST_USER_DEFAULT:
			case VISITOR_TALKING_OTHER://对话中
				xmlPath = L"menu\\menu_right_no_id.xml";
				step = 3;
				break;
				
			case HOST_USER_ONLINE:
				xmlPath = L"menu\\menu_right_user_online.xml";
				step = 4;
				break;

			case HOST_USER_OFFLINE:
				step = 4;
				xmlPath = L"menu\\menu_right_user_offline.xml";
				break;

			case HOST_USER_MYSELF:
				step = 8;
				xmlPath = L"menu\\menu_right_myself_node.xml";
				break;
			
			case VISITOR_REQ_ING:
			case VISITOR_TRANING:                       //转接中
			case VISITOR_INVOTING:                      //邀请中
				xmlPath = L"menu\\menu_right_wait_request.xml";
				step = 6;
				break;

			case VISITOR_TALKING_MYSELF:
				xmlPath = L"menu\\menu_right_talk_myself.xml";
				step = 7;
				break;

			case VISITOR_TALKING_HELP_OTHER:           //是别人的协助对象 对话中
				xmlPath = L"menu\\menu_right_talk_invote_other.xml";
				step = 7;
				break;
	
			case VISITOR_IN_TALK_ING:                   //内部对话中	
				xmlPath = L"menu\\menu_right_inline_user_talk.xml";  
				step = 8;
				break;

			case VISITOR_ONLINE_AUTO_INVOTING:          //自动邀请中
			case VISITOR_ONLINE_AUTO_VISITING:          //访问中
				step = 9;
				xmlPath = L"menu\\menu_right_online_talk_visitor.xml";    //在线列表 用户的弹出菜单
				break;

			case VISITOR_ONLINE_AUTO_END:
				xmlPath = L"menu\\menu_right_online_end_visitor.xml";
				step = 9;
				break;

			default:
				break;
		}

	
		CDuiString path = GetCurrentPathW();
		path += L"\\SkinRes\\menu\\check.png";
		CDuiString key;
		switch (m_manager->m_sysConfig->m_nFilterType)
		{
		case VISITORFILTER_ALL:
			key = L"label_right_set_filter_show_all";
			break;

		case VISITORFILTER_MYVISITOR:			//只显示我自己的访客，不显示访问中和已离开的人，在等待接待和显示
			key = L"label_right_set_filter_show_my_visitor";
			break;

		case VISITORFILTER_ALLVISITOR:		//显示全部客服的访客，
			key = L"label_right_set_filter_show_all_history_visitor";
			break;

		case VISITORFILTER_1MINUTES:
			key = L"label_right_set_filter_1_minute_visitor";
			break;

		case VISITORFILTER_3MINUTES:
			key = L"label_right_set_filter_3_minute_visitor";
			break;
		case VISITORFILTER_5MINUTES:
			key = L"label_right_set_filter_5_minute_visitor";
			break;
		case VISITORFILTER_10MINUTES:
			key = L"label_right_set_filter_10_minute_visitor";
			break;
		default:
			key = L"label_right_set_filter_show_all";
			break;
		}

		if (!xmlPath.IsEmpty())
		{
			CMenuWnd* pMenu = new CMenuWnd(m_hMainWnd);
			CPoint cpoint = msg.ptMouse;
		
			ClientToScreen(this->m_hWnd, &cpoint);
			pMenu->SetPath((WCHAR*)xmlPath.GetData());
			RECT sysRect;
			GetWindowRect(m_hWnd, &sysRect);

			height = step * 30;
			if (cpoint.y + height >= sysRect.bottom)
				cpoint.y -= height;

			pMenu->SetAttrData(key, path);

			pMenu->Init(NULL, _T(""), _T("xml"), cpoint);
		}


		   
#if 0
		CheckIdForUerOrWebuser(uid, m_curSavedSid,&pWebUser, &pUser);

		//轮询查找 查找当前选择的 uid是 空 还是坐席 等待中的用户 等等
		if (uid == 0) //既不是访客 也不是坐席
		{
			UserListUI::Node* node = NULL;
			node = (UserListUI::Node*)msg.pSender->GetTag();
			if (node == NULL)
				node = m_curClickItemNode;

			if (CheckItemForOnlineVisitor(node))
			{
				xmlPath = L"menu\\menu_right_online_visitor.xml";    //在线列表 用户的弹出菜单
			}
			else
			{
				xmlPath = L"menu\\menu_right_no_id.xml";
			}
			
		}
		else if (uid == m_manager->m_userInfo.UserInfo.uid) //自己的uid
		{
			xmlPath = L"menu\\menu_right_myself_node.xml";
		}

		else
		{
			if (pWebUser != NULL)
			{
				if (pWebUser->onlineinfo.talkstatus == TALK_STATUS_REQUEST)
				{
					xmlPath = L"menu\\menu_right_wait_request.xml";
				}
			}
			else
			{
				if (pUser == NULL)
					return;

				if (pUser->status == USER_STATUS_OFFLINE)
				{
					//xmlPath = L"menu\\menu_right_4.xml";
					xmlPath = L"menu\\menu_right_user_offline.xml";
					
				}
				else if (pUser->status == USER_STATUS_ONLINE)
				{
					//xmlPath = L"menu\\menu_right_5.xml";

					xmlPath = L"menu\\menu_right_user_online.xml";
				}

			}
		}
#endif

	}
}



void CMainFrame::OnItemSelect(TNotifyUI &msg)
{


}




void CMainFrame::OnHeaderClick(TNotifyUI& msg)
{
}

void CMainFrame::OnMouseEnter(TNotifyUI& msg)
{
}





void CMainFrame::OnBtnFont(TNotifyUI& msg)
{





}

void CMainFrame::OnBtnFace(TNotifyUI& msg)
{

	if (!m_faceSelDlg.m_firstCreate)
	{
		m_faceSelDlg.SetFaceList(&m_faceList);
		if (!::IsWindow((HWND)m_faceSelDlg))
		{
			m_faceSelDlg.Create(this->m_hWnd, NULL, WS_CHILD | WS_POPUP, WS_EX_TOOLWINDOW);


			CPoint cpoint = msg.ptMouse;
			ClientToScreen(this->m_hWnd, &cpoint);
			int cx = 432;
		    int cy = 236;
			int x = cpoint.x - cx / 2;
			int y = cpoint.y - cy-10;

			::SetWindowPos((HWND)m_faceSelDlg, NULL, x, y, cx, cy, NULL);
			::ShowWindow((HWND)m_faceSelDlg, SW_SHOW);

			m_faceSelDlg.Init();

		}
	}
	else
	{
		CPoint cpoint = msg.ptMouse;
		ClientToScreen(this->m_hWnd, &cpoint);
		int cx = 432;
		int cy = 236;
		int x = cpoint.x - cx / 2;
		int y = cpoint.y - cy-10;

		::SetWindowPos((HWND)m_faceSelDlg, NULL, x, y, cx, cy, NULL);
		::ShowWindow((HWND)m_faceSelDlg, SW_SHOW);
	}

	

}

void CMainFrame::OnBtnScreen(TNotifyUI& msg)
{
	if (m_manager)
	{
		m_manager->ScreenCapture(m_hMainWnd);
	}
}

//选择表情后的处理
void CMainFrame::OnFaceCtrlSel(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int nFaceId = m_faceSelDlg.GetSelFaceId();
	int nFaceIndex = m_faceSelDlg.GetSelFaceIndex();
	tstring strFileName = m_faceSelDlg.GetSelFaceFileName();
	if (!strFileName.empty())
	{
		_RichEdit_InsertFace(m_pSendEdit, strFileName.c_str(), nFaceId, nFaceIndex);
		m_pSendEdit->SetFocus();
	}
	((COptionUI *)m_pFaceBtn)->Selected(false);

}



BOOL CMainFrame::_RichEdit_InsertFace(CRichEditUI * pRichEdit, LPCTSTR lpszFileName, int nFaceId, int nFaceIndex)
{
	BOOL bRet = FALSE;

	if (NULL == pRichEdit || NULL == lpszFileName || NULL == *lpszFileName)
		return FALSE;

	ITextServices * pTextServices = pRichEdit->GetTextServices();
	ITextHost * pTextHost = pRichEdit->GetTextHost();
	if (pTextServices != NULL && pTextHost != NULL)
	{
		//if (pRichEdit == m_pRecvEdit)
			//RichEdit_SetStartIndent(pTextServices, 300);
		bRet = RichEdit_InsertFace(pTextServices, pTextHost,
			lpszFileName, nFaceId, nFaceIndex, RGB(255, 255, 255), TRUE, 40);
	}

	if (pTextServices != NULL)
		pTextServices->Release();
	if (pTextHost != NULL)
		pTextHost->Release();

	return bRet;
}

#if 0
void CMainFrame::ReplaceFaceId(string &msg)
{
	string str = msg;
	string url;
	char getStr[32] = { 0 };
	
	while (1)
	{
		url = m_facePathUrl;
		int pos = str.find("[");


		if (pos == -1)//没有找到 插入的图片 
			break;


		int end = str.find("]", pos);
		if (end == -1)
			break;
		string idStr = str.substr(pos + 1, end - pos - 1);
		string reStr = str.substr(pos, end - pos + 1);

		string facePath = GetCurrentPath();
		facePath += ("\\SkinRes\\Face\\");
		facePath += idStr;

		StringReplace(url, "face.gif", facePath);
		StringReplace(str, reStr, url);
		StringReplace(str, "\\", "/");
	} 

	msg = str;

}
#endif

void CMainFrame::OnBtnSendMessage(TNotifyUI& msg)
{
	char getInput[MAX_1024_LEN*10] = {0};
	string msgId = m_manager->GetMsgId();
	string sendMsgData;
	MSG_RECV_TYPE sendUserType;
	MSG_DATA_TYPE sendMsgType;

	//如果当前选择的用户为 空 则不需要发送  测试时暂时屏蔽掉
	sendUserType = GetSendUserType(m_curSelectId);
	if (sendUserType == MSG_RECV_ERROR || m_curSelectId <= 0)
		return;

	ITextServices * pTextServices = m_pSendEdit->GetTextServices();
	tstring strText;
	VectorStringData vecName;

	RichEdit_GetText2(pTextServices, strText);
	if (strText.length() > 0)
	{
		sendMsgType = MSG_DATA_TYPE_IMAGE;

		GetEditImageData((WCHAR*)strText.c_str(), vecName);
	}
	else
	{
		sendMsgType = MSG_DATA_TYPE_TEXT;
		RichEdit_GetText(pTextServices, strText);
	}

	pTextServices->Release();

	if (strText.size() <= 0)
		return;
	m_pSendEdit->SetText(_T(""));
	m_pSendEdit->SetFocus();


	UnicodeToANSI(strText.c_str(), getInput);
	sendMsgData = getInput;
	if (sendMsgType == MSG_DATA_TYPE_TEXT)
	{
	
		//消息测试 暂时先只发给用户  坐席后续加上
		m_manager->SendTo_Msg(m_curSelectId, sendUserType, msgId, MSG_DATA_TYPE_TEXT, sendMsgData);
		
		//理论上只有发送消息成功 才在聊天界面显示  
		//暂时先添加在这里  后面再移到收到消息成功 的回调哪里
		//显示 发送的消息
		ReplaceFaceId(sendMsgData);
		ShowMySelfSendMsg(sendMsgData, sendMsgType, msgId);
	}

	else if (sendMsgType == MSG_DATA_TYPE_IMAGE)
	{
		for (int i = 0; i < vecName.size(); i++)
		{
			m_manager->SendTo_Msg(m_curSelectId, sendUserType, msgId, MSG_DATA_TYPE_IMAGE, vecName[i]);

			sendMsgData = vecName[i];
			//ReplaceImageId(sendMsgData);

			ShowMySelfSendMsg(sendMsgData, sendMsgType, msgId);
		}

	}


}


//请求坐席列表
void CMainFrame::SendMsgToGetList()
{
	CDuiString text;
	WCHAR name[64] = { 0 };

	UserListUI::Node* pUserTalkNode = pMySelfeNode->child(0);

	//这里查找有没有 没有接入的对话 
	list<unsigned long>::iterator iter;
	for (iter = m_manager->m_listEarlyChat.begin(); iter != m_manager->m_listEarlyChat.end(); iter++)
	{
		unsigned long id = *iter;
		if (id > 0)
		{
			CWebUserObject *pWebUser = NULL;

			pWebUser = m_manager->GetWebUserObjectByUid(id);
			if (pWebUser == NULL)
				return;


			ANSIToUnicode(pWebUser->info.name, name);
			//是否来自微信
			if (pWebUser->m_bIsFrWX)
			{
				text.Format(_T("{x 4}{i user_wx.png 1 0}{x 4}%s"), name);
			}
			else
			{
				text.Format(_T("{x 4}{i user_web.png 1 0}{x 4}%s"), name);
			}

			UserListUI::Node* addNode = pUserList->AddNode(text, pWebUser->webuserid, pWebUser->info.sid, pUserTalkNode);
			pUserList->ExpandNode(pUserTalkNode, true);


			m_allVisitorNodeMap.insert(pair<unsigned long, UserListUI::Node*>(pWebUser->webuserid, addNode));
			m_allVisitorUserMap.insert(pair<unsigned long, unsigned long>(pWebUser->webuserid, m_manager->m_userInfo.UserInfo.uid));

		}

	}




	//获取坐席列表
	m_manager->SendTo_GetShareList();

	m_hMainWnd = m_PaintManager.GetPaintWindow();

	//m_PaintManager.SetTimer(pUserList, WM_ADD_ONLINE_DATA_TIMER_ID, DELAY_ADD_ONLINE_DATA_TIME);

}


void CMainFrame::AddOnlineVisitor(UserListUI * ptr, CUserObject *user,int index)
{
	CDuiString nameString, taklString, changeString, acceptString, inTalkString;
	CDuiString onlineString;
	CDuiString strTemp;
	int start = 0;
	//UserListUI::Node* pOnlineNode = NULL;
	UserListUI::Node* pAutoAccept = NULL;
	UserListUI::Node* pVisiting = NULL;
	UserListUI::Node* pOver = NULL;

	nameString.Format(_T("{x 4}{i gameicons.png 18 0}{x 4} 在线访客"));
	//第一个主节点 显示 名称 在线访客
	if (index == -1)
	{
		//number = ptr->GetNodeListNumber(NULL);
		//pOnlineNode = ptr->AddNode(nameString, 39);
		pOnlineNode = ptr->AddNode(nameString,0);
	}

	else
		pOnlineNode = ptr->AddNode(nameString,0, index);

	nameString.Format(_T("{x 4}{i gameicons.png 18 10}{x 4}自动邀请"));
	pAutoAccept = pUserList->AddNode(nameString,0,"", pOnlineNode);

	nameString.Format(_T("{x 4}{i gameicons.png 18 10}{x 4}访问中"));
	pVisiting = pUserList->AddNode(nameString,0, "",pOnlineNode);


	nameString.Format(_T("{x 4}{i gameicons.png 18 10}{x 4}已结束"));
	pOver = pUserList->AddNode(nameString,0,"", pOnlineNode);

}

void CMainFrame::AddHostUserList(UserListUI * ptr, CUserObject *user, int pos)
{
	CDuiString nameString, taklString, changeString, acceptString, inTalkString;
	CDuiString onlineString;
	CDuiString strTemp;

	UserListUI::Node* pUserNameNode = NULL;
	UserListUI::Node* pUserTalkNode = NULL;
	UserListUI::Node* pUserChangeNode = NULL;
	UserListUI::Node* pUserAcceptNode = NULL;
	UserListUI::Node* pUserInTalkNode = NULL;

	if (user->status == USER_STATUS_OFFLINE) //离线
	{
		onlineString = _T("(离线)");
	}
	else  if (user->status == USER_STATUS_ONLINE)                 //在线
	{
		onlineString = _T("(在线)");
	}

	strTemp = AnsiToUnicode(user->UserInfo.nickname);

	nameString.Format(_T("{x 4}{i user_client.png 1 0}{x 4} %s %s"), strTemp.GetData(), onlineString);

	//第一个主节点 显示 名称 在线状态  //pos  需要插入的位置
	pUserNameNode = ptr->AddNode(nameString, user->UserInfo.uid, pos);

	taklString.Format(_T("{x 4}{i gameicons.png 18 10}{x 4}对话中"));
	pUserTalkNode = pUserList->AddNode(taklString,0, "",pUserNameNode);

	changeString.Format(_T("{x 4}{i gameicons.png 18 10}{x 4}转接中"));
	pUserChangeNode = pUserList->AddNode(changeString,0,"", pUserNameNode);

	acceptString.Format(_T("{x 4}{i gameicons.png 18 10}{x 4}邀请中"));
	pUserAcceptNode = pUserList->AddNode(acceptString, 0,"",pUserNameNode);

	if (user->status == USER_STATUS_OFFLINE) //离线
	{
		m_offlineNodeMap.insert(pair<unsigned long, UserListUI::Node*>(user->UserInfo.uid, pUserNameNode));

		//map<unsigned long, UserListUI::Node*>::iterator  iter = m_allVisitorNodeMap.find(user->UserInfo.uid);
		//if (iter != m_allVisitorNodeMap.end())
			//m_allVisitorNodeMap.erase(iter);

		//m_allVisitorNodeMap.insert(pair<unsigned long, UserListUI::Node*>(user->UserInfo.uid, pUserNameNode));

		pUserList->ExpandNode(pUserNameNode, false);
	}
	else  if (user->status == USER_STATUS_ONLINE)     //在线
	{
		m_onlineNodeMap.insert(pair<unsigned long, UserListUI::Node*>(user->UserInfo.uid, pUserNameNode));

		//map<unsigned long, UserListUI::Node*>::iterator  iter = m_allVisitorNodeMap.find(user->UserInfo.uid);
		//if (iter != m_allVisitorNodeMap.end())
			//m_allVisitorNodeMap.erase(iter);
		//m_allVisitorNodeMap.insert(pair<unsigned long, UserListUI::Node*>(user->UserInfo.uid, pUserNameNode));

		pUserList->ExpandNode(pUserNameNode, true);
	}

	pUserList->SetNodeHighlight(pUserNameNode, L"E:\\YunKa\\bin\\SkinRes\\mainframe\\tree_select.bmp");
	

}



void CMainFrame::AddMyselfToList(UserListUI * ptr, CUserObject *user)
{
	CDuiString nameString, taklString, changeString, acceptString, inTalkString;
	CDuiString onlineString;
	CDuiString strTemp;
	CDuiString text;
	WCHAR name[64] = { 0 };
	UserListUI::Node* pUserNameNode = NULL;
	UserListUI::Node* pUserTalkNode = NULL;
	UserListUI::Node* pUserChangeNode = NULL;
	UserListUI::Node* pUserAcceptNode = NULL;
	UserListUI::Node* pUserInTalkNode = NULL;

	if (user->status == USER_STATUS_OFFLINE)  //离线
	{
		onlineString = OFFLINE_TEXT;
	}
	else  if (user->status == USER_STATUS_ONLINE)                   //在线
	{
		onlineString = ONLINE_TEXT;
	}

	strTemp = AnsiToUnicode(user->UserInfo.nickname);

	//nameString.Format(_T("{x 4}{i gameicons.png 18 0}{x 4} %s %s"), strTemp.GetData(), onlineString);

	nameString.Format(_T("{x 4}{i user_client.png 1 0}{x 4} %s %s"), strTemp.GetData(), onlineString);


	//第一个主节点 显示 名称 在线状态
	pUserNameNode = ptr->AddNode(nameString,user->UserInfo.uid);

	taklString.Format(_T("{x 4}{i gameicons.png 18 10}{x 4}对话中"));
	pUserTalkNode = pUserList->AddNode(taklString,0, "",pUserNameNode);

	changeString.Format(_T("{x 4}{i gameicons.png 18 10}{x 4}转接中"));
	pUserChangeNode = pUserList->AddNode(changeString,0,"", pUserNameNode);


	acceptString.Format(_T("{x 4}{i gameicons.png 18 10}{x 4}邀请中"));
	pUserAcceptNode = pUserList->AddNode(acceptString,0,"", pUserNameNode);

	inTalkString.Format(_T("{x 4}{i gameicons.png 18 10}{x 4}内部对话"));
	pUserInTalkNode = pUserList->AddNode(inTalkString,0,"", pUserNameNode);


	pMySelfeNode = pUserNameNode;
	pUserList->ExpandNode(pMySelfeNode, true);

	
	//m_allVisitorNodeMap.insert(pair<unsigned long, UserListUI::Node*>(user->UserInfo.uid, pUserNameNode));
}



void CMainFrame::AddHostUserList(UserListUI * ptr, CUserObject *user)
{
	CDuiString nameString,taklString,changeString,acceptString,inTalkString;
	CDuiString onlineString;
	CDuiString strTemp;
	int index = 0;

	UserListUI::Node* pUserNameNode = NULL;
	UserListUI::Node* pUserTalkNode = NULL;
	UserListUI::Node* pUserChangeNode = NULL;
	UserListUI::Node* pUserAcceptNode = NULL;
	UserListUI::Node* pUserInTalkNode = NULL;

	if (user->status == USER_STATUS_OFFLINE)  //离线
	{
		onlineString = _T("(离线)");

		strTemp = AnsiToUnicode(user->UserInfo.nickname);
		//nameString.Format(_T("{x 4}{i gameicons.png 18 0}{x 4} %s %s"), strTemp.GetData(), onlineString);
		nameString.Format(_T("{x 4}{i user_client.png 1 0} %s %s"), strTemp.GetData(), onlineString);


		if (m_pLastOfflineNode == NULL) // 第一个主节点 显示 名称 在线状态
		{

			if (m_pLastOnlineNode == NULL)
				index = pUserList->GetNodeIndex(pMySelfeNode);
			else
				index = pUserList->GetNodeIndex(m_pLastOnlineNode);

			pUserNameNode = ptr->AddNode(nameString, user->UserInfo.uid,index);
			m_pLastOfflineNode = pUserNameNode;
		}
		else
		{
			index = pUserList->GetNodeIndex(m_pLastOfflineNode);
			pUserNameNode = ptr->AddNode(nameString, user->UserInfo.uid,index);
			m_pLastOfflineNode = pUserNameNode;
		}


	}
	else  if (user->status == USER_STATUS_ONLINE)                   //在线
	{
		onlineString = _T("(在线)");

		strTemp = AnsiToUnicode(user->UserInfo.nickname);
		//nameString.Format(_T("{x 4}{i gameicons.png 18 0}{x 4} %s %s"), strTemp.GetData(), onlineString);
		nameString.Format(_T("{x 4}{i user_client.png 1 0}{x 4} %s %s"), strTemp.GetData(), onlineString);


		if (m_pLastOnlineNode == NULL) // 第一个主节点 显示 名称 在线状态
		{
			index = pUserList->GetNodeIndex(pMySelfeNode);
			pUserNameNode = ptr->AddNode(nameString, user->UserInfo.uid, index);
			m_pLastOnlineNode = pUserNameNode;
		}
		else
		{
			index = pUserList->GetNodeIndex(m_pLastOnlineNode);
			pUserNameNode = ptr->AddNode(nameString, user->UserInfo.uid, index);
			m_pLastOnlineNode = pUserNameNode;
		}



		//m_upUser.push_back(user);
	}


	pUserList->SetNodeHighlight(pUserNameNode, L"E:\\YunKa\\bin\\SkinRes\\mainframe\\tree_select.bmp");

	taklString.Format(_T("{x 4}{i gameicons.png 18 10}{x 4}对话中"));
	//pUserTalkNode = pUserList->AddNode(taklString, user->UserInfo.uid,"", pUserNameNode);
	pUserTalkNode = pUserList->AddNode(taklString, 0, "", pUserNameNode);

	changeString.Format(_T("{x 4}{i gameicons.png 18 10}{x 4}转接中"));
	//pUserChangeNode = pUserList->AddNode(changeString, user->UserInfo.uid,"", pUserNameNode);
	pUserChangeNode = pUserList->AddNode(changeString, 0, "", pUserNameNode);


	acceptString.Format(_T("{x 4}{i gameicons.png 18 10}{x 4}邀请中"));
	//pUserAcceptNode = pUserList->AddNode(acceptString, user->UserInfo.uid,"", pUserNameNode);
	pUserAcceptNode = pUserList->AddNode(acceptString, 0, "", pUserNameNode);


	if (user->status == USER_STATUS_OFFLINE)    //离线
	{
		m_offlineNodeMap.insert(pair<unsigned long, UserListUI::Node*>(user->UserInfo.uid, pUserNameNode));

		//m_allVisitorNodeMap.insert(pair<unsigned long, UserListUI::Node*>(user->UserInfo.uid, pUserNameNode));

		pUserList->ExpandNode(pUserNameNode, false);

	}
	else  if (user->status == USER_STATUS_ONLINE) //在线
	{
		m_onlineNodeMap.insert(pair<unsigned long, UserListUI::Node*>(user->UserInfo.uid, pUserNameNode));
		//m_allVisitorNodeMap.insert(pair<unsigned long, UserListUI::Node*>(user->UserInfo.uid, pUserNameNode));
		pUserList->ExpandNode(pUserNameNode, true);
		
	}

}

void CMainFrame::RecvMsg(IBaseObject* pObj, MSG_FROM_TYPE msgFrom, string msgId, MSG_TYPE msgType, MSG_DATA_TYPE msgDataType, string msgContent,
	string msgTime, CUserObject* pAssistUser, WxMsgBase* msgContentWx)
{
	if (pObj == NULL || !m_pListMsgHandler.isLoaded)
		return;

	//普通消息 
	if (msgType == MSG_TYPE_NORMAL)
	{
		CWebUserObject    *pWebUserObj = NULL;
		CUserObject       *pUserObj = NULL;
		CUserObject       *pAssistObj = NULL;
		unsigned long     userId = 0;

		if (msgFrom == MSG_FROM_CLIENT)
		{
			pUserObj = (CUserObject *)pObj;
			userId = pUserObj->UserInfo.uid;
		}
		else if ( msgFrom == MSG_FROM_ASSIST)
		{
			pWebUserObj = (CWebUserObject *)pObj;
			pAssistObj = (CUserObject *)pAssistUser;
			userId = pWebUserObj->webuserid;
		}
		else if (msgFrom == MSG_FROM_WEBUSER)
		{
			pWebUserObj = (CWebUserObject *)pObj;
			userId = pWebUserObj->webuserid;
		}
		else if (msgFrom == MSG_FROM_SYS)
		{
			if (pObj->m_nEMObType == OBJECT_WEBUSER)
			{
				pWebUserObj = (CWebUserObject *)pObj;
				userId = pWebUserObj->webuserid;
			}
			else
			{
				pUserObj = (CUserObject *)pObj;
				userId = pUserObj->UserInfo.uid;
			}
		}

		if (userId != m_curSelectId)
		{
			g_WriteLog.WriteLog(C_LOG_ERROR,"RecvMsg userId:%lu, m_curSelectId:%lu", userId, m_curSelectId);
			return;
		}

		if (msgContent.length() == 0)
		{
			g_WriteLog.WriteLog(C_LOG_ERROR, "插入空的聊天记录");
			return;
		}

		CefString strCode(msgContent), strUrl("");
		m_pListMsgHandler.handler->GetBrowser()->GetMainFrame()->ExecuteJavaScript(strCode, strUrl, 0);

		//m_pListMsgHandler.handler->GetBrowser()->GetMainFrame()->

	//	m_pListMsgHandler.handler->GetBrowser()->GetMainFrame()->IsFocused();
	}
	//预知消息
	else if (msgType == MSG_TYPE_PREV)
	{

	}

	
}


void CMainFrame::ResultRecvMsg(string msgId, bool bSuccess, string url, unsigned long msgFromUserId,
	unsigned long assistUserId, string filePath, MSG_FROM_TYPE msgFromType, MSG_DATA_TYPE msgDataType)
{
	char strCallJs[MAX_1024_LEN];
	if (bSuccess)
	{
		sprintf(strCallJs, "ResultRecvMsg('%s','%d','%s','unknown','%s','%d','%d','%lu','%lu');",
			msgId.c_str(), bSuccess,url.c_str(), filePath.c_str(), msgFromType, msgDataType,msgFromUserId,assistUserId);
	}
	else
	{
		CCodeConvert convert;
		string sImagePath;
		string imagePath = FullPath("SkinRes\\mainframe\\");
		StringReplace(imagePath, "\\", "/");
		convert.Gb2312ToUTF_8(sImagePath, imagePath.c_str(), imagePath.length());
		sprintf(strCallJs, "ResultRecvMsg('%s','%d','%s','%s','%s','%d','%d','%lu','%lu');",
			msgId.c_str(), bSuccess,url.c_str(),sImagePath.c_str(), "", 
			msgFromType, msgDataType, msgFromUserId,assistUserId);
	}

	CefString strCode(strCallJs), strUrl("");
	m_pListMsgHandler.handler->GetBrowser()->GetMainFrame()->ExecuteJavaScript(strCode, strUrl, 0);
}

void CMainFrame::ResultSendMsg(string msgId, bool bSuccess, unsigned long userId, MSG_RECV_TYPE recvUserType,
								MSG_DATA_TYPE msgDataType, string msg)
{
	char strCallJs[MAX_256_LEN];
	if (bSuccess)
	{
		if (msgDataType == MSG_DATA_TYPE_IMAGE || msgDataType == MSG_DATA_TYPE_VOICE)
		{
			sprintf(strCallJs, "ResultSendMsg('%s','%d','0','%s','%d','%d','%lu');", 
				msgId.c_str(), bSuccess, msg.c_str(), recvUserType, msgDataType, userId);
		}
		else if (msgDataType == MSG_DATA_TYPE_FILE)
		{
			int pos = msg.find_last_of("/");
			string fileName = msg.substr(pos + 1, msg.length() - pos - 1);
			sprintf(strCallJs, "ResultSendMsg('%s','%d','%s','%s','%d','%d','%lu');",
				msgId.c_str(), bSuccess, fileName.c_str(), msg.c_str(), recvUserType, msgDataType, userId);
		}
	}
	else
	{
		CCodeConvert convert;
		string sImagePath;
		string imagePath = FullPath("SkinRes\\mainframe\\");
		StringReplace(imagePath, "\\", "/");
		convert.Gb2312ToUTF_8(sImagePath, imagePath.c_str(), imagePath.length());
		sprintf(strCallJs, "ResultSendMsg('%s','%d','%s','%s','%d','%d','%lu');", msgId.c_str(), bSuccess,
			sImagePath.c_str(), msg.c_str(), recvUserType, msgDataType, userId);
	}

	CefString strCode(strCallJs), strUrl("");
	m_pListMsgHandler.handler->GetBrowser()->GetMainFrame()->ExecuteJavaScript(strCode, strUrl, 0);
}

void CMainFrame::ResultScreenCapture(string imagePath)
{

}

void CMainFrame::OnManagerButtonEvent(TNotifyUI& msg)
{
	char sURL[1024] = {0};
	WCHAR managerName[32] = {0};
	int doEvent = -1;

	for (int i = 0; i < MID_MANAGER_BUTTON_NUM; i++)
	{
		wsprintf(managerName,L"managerbutton_%d",i+1);
		if (msg.pSender->GetName() == managerName)
		{
			if (m_pManagerBtn[i].m_buttonState == 1)
			{
				doEvent = 0;
			}
			break;
		}
	}

	if (doEvent == -1)
		return;

	//接受    
	//筛选访客
	if (msg.pSender->GetName() == _T("managerbutton_1"))
	{
		//AcceptChat();

		CSelectVisitorWnd *dlg = new CSelectVisitorWnd();

		dlg->Create(m_hWnd, _T(""), UI_WNDSTYLE_DIALOG, 0, 0, 0, 0, 0, NULL);
		dlg->CenterWindow();
		dlg->ShowModal();


	}

	//转接
	//内部对话
	else if (msg.pSender->GetName() == _T("managerbutton_2"))
	{
		//m_topWndType = 1;
		//获取在线坐席数
		//m_manager->SendTo_GetOnlineUser();


		m_topWndType = 3;

		//获取在线坐席数
		m_manager->SendTo_GetOnlineUser();


	} 
	//结束  
	//访客历史
	else if (msg.pSender->GetName() == _T("managerbutton_3"))
	{
		//m_manager->SendTo_CloseChat(m_curSelectId, CHATCLOSE_USER);

		sprintf(sURL, m_manager->m_initConfig.webpage_SvrMsg, m_manager->m_vip.c_str(), m_manager->m_login->m_szAuthtoken);
		ShowWebBrowser(sURL);
	}

	//屏蔽
	//访客来电
	else if (msg.pSender->GetName() == _T("managerbutton_4"))
	{
		sprintf(sURL, m_manager->m_initConfig.webpage_querywebphone, m_manager->m_vip.c_str(), m_manager->m_userInfo.UserInfo.uid, m_manager->m_login->m_szAuthtoken);
		ShowWebBrowser(sURL);
	}

	//邀请评价
	//访客留言
	else if (msg.pSender->GetName() == _T("managerbutton_5"))
	{
		sprintf(sURL, m_manager->m_initConfig.webpage_note, m_manager->m_vip.c_str(), m_manager->m_userInfo.UserInfo.uid, m_manager->m_login->m_szAuthtoken);
		ShowWebBrowser(sURL);
	}


	//筛选访客
	//客户管理
	else if (msg.pSender->GetName() == _T("managerbutton_6"))
	{
		//CSelectVisitorWnd *dlg= new CSelectVisitorWnd();

		//dlg->Create(m_hWnd, _T(""), UI_WNDSTYLE_DIALOG, 0, 0, 0, 0, 0, NULL);
		//dlg->CenterWindow();
		//dlg->ShowModal();

		sprintf(sURL, m_manager->m_initConfig.webpage_crm, m_manager->m_vip.c_str(), m_manager->m_login->m_szAuthtoken);
		ShowWebBrowser(sURL);
	}

	//释放会话
	//工单管理 
	else if (msg.pSender->GetName() == _T("managerbutton_7"))
	{
		//m_manager->SendTo_ReleaseChat(m_curSelectId);

		sprintf(sURL, m_manager->m_initConfig.webpage_workbillurl, m_manager->m_vip.c_str(), m_manager->m_login->m_szAuthtoken);
		ShowWebBrowser(sURL);
	}

	//邀请协助
	//管理中心
	else if (msg.pSender->GetName() == _T("managerbutton_8"))
	{
		//m_topWndType = 2;
		//获取在线坐席数
		//m_manager->SendTo_GetOnlineUser();

		sprintf(sURL, m_manager->m_initConfig.webpage_mgmt, m_manager->m_vip.c_str(), m_manager->m_login->m_szAuthtoken);
		ShowWebBrowser(sURL);
	}


#if 0

	//内部对话
	else if (msg.pSender->GetName() == _T("managerbutton_9"))
	{
		m_topWndType = 3;

		//获取在线坐席数
		m_manager->SendTo_GetOnlineUser();

	}
	//访客历史
	else if (msg.pSender->GetName() == _T("managerbutton_10"))
	{

		sprintf(sURL, m_manager->m_initConfig.webpage_SvrMsg, m_manager->m_vip.c_str(), m_manager->m_login->m_szAuthtoken);
		ShowWebBrowser(sURL);
	}


	//访客来电
	else if (msg.pSender->GetName() == _T("managerbutton_11"))
	{

		sprintf(sURL, m_manager->m_initConfig.webpage_querywebphone, m_manager->m_vip.c_str(), m_manager->m_userInfo.UserInfo.uid, m_manager->m_login->m_szAuthtoken);
		ShowWebBrowser(sURL);
	}
	//访客留言
	else if (msg.pSender->GetName() == _T("managerbutton_12"))
	{
		sprintf(sURL, m_manager->m_initConfig.webpage_note, m_manager->m_vip.c_str(), m_manager->m_userInfo.UserInfo.uid, m_manager->m_login->m_szAuthtoken);
		ShowWebBrowser(sURL);
	}
	//客户管理
	else if (msg.pSender->GetName() == _T("managerbutton_13"))
	{
		sprintf(sURL, m_manager->m_initConfig.webpage_crm, m_manager->m_vip.c_str(), m_manager->m_login->m_szAuthtoken);
		ShowWebBrowser(sURL);
	}
	//统计分析
	else if (msg.pSender->GetName() == _T("managerbutton_14"))
	{
		sprintf(sURL, m_manager->m_initConfig.webpage_workbillurl, m_manager->m_vip.c_str(), m_manager->m_login->m_szAuthtoken);
		ShowWebBrowser(sURL);
	}
	//管理中心
	else if (msg.pSender->GetName() == _T("managerbutton_15"))
	{
		sprintf(sURL, m_manager->m_initConfig.webpage_mgmt, m_manager->m_vip.c_str(), m_manager->m_login->m_szAuthtoken);
		ShowWebBrowser(sURL);
	}

#endif

}

void CMainFrame::ShowMySelfSendMsg(string strMsg, MSG_DATA_TYPE msgType, string msgId)
{
	unsigned long     userId = 0;
	CCodeConvert      f_covet;
	char strJsCode[MAX_1024_LEN] = { 0 };
	char contentMsg[MAX_1024_LEN] = { 0 };
	string  name, msg,imagePath;

	if (strMsg.empty())
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "插入空的聊天记录");
		return;
	}

	string strName = m_manager->m_userInfo.UserInfo.nickname;
	StringReplace(strName, "'", "&#039;");
	StringReplace(strName, "\r\n", "<br>");
	f_covet.Gb2312ToUTF_8(name, strName.c_str(), strName.length());

	StringReplace(strMsg, "'", "&#039;");
	StringReplace(strMsg, "\r\n", "<br>");
	f_covet.Gb2312ToUTF_8(msg, strMsg.c_str(), strMsg.length());

	imagePath = FullPath("SkinRes\\mainframe\\msg_wait.gif");
	StringReplace(imagePath, "\\", "/");
	StringReplace(msg, "\\", "/");

	if (msgType == MSG_DATA_TYPE_IMAGE)
	{
		sprintf(contentMsg, "<img id=\"%s_image\" class=\"wait_image\" src=\"%s\"><img class=\"msg_image\" src=\"%s\" onclick=window.RunMsgList(\"ViewDetails\",\"%s\",\"2\")>",
			msgId.c_str(), imagePath.c_str(), msg.c_str(), msg.c_str());
		msg = contentMsg;
	}
	else if (msgType == MSG_DATA_TYPE_VOICE)
	{
		sprintf(contentMsg, "<img id=\"%s_image\" class=\"wait_image\" src=\"%s\"><audio class=\"msg_voice\" controls=\"controls\" src=\"%s.wav\" type=\"audio/mpeg\"></audio>",
			msgId.c_str(), imagePath.c_str(),m_audioPath.c_str());
		msg = contentMsg;
	}
	else if (msgType == MSG_DATA_TYPE_FILE)
	{
		sprintf(contentMsg, "<img id=\"%s_image\" class=\"wait_image\" src=\"%s\"><span id=\"%s_span\" class=\"file_text\">%s</span>",
			msgId.c_str(), imagePath.c_str(), msgId.c_str(), msg.c_str());
		msg = contentMsg;
	}

	//组合消息
	string head;
	head = "<img id=\"" + msgId + "_head\" class=\"head_image\" src=\"" + m_manager->m_userInfo.m_headPath + "\">";
	f_covet.Gb2312ToUTF_8(head, head.c_str(), head.length());
	string msgTime = GetTimeByMDAndHMS(0);
	sprintf(strJsCode, "AppendMsgToHistory('%d', '%d', '%s', '%s', '%s', '%lu', '%s', '%s', '%s'); ",
		MSG_FROM_SELF, msgType, name.c_str(), msgTime.c_str(), msg.c_str(), userId, head.c_str(), msgId.c_str(), imagePath.c_str());

	if (m_pListMsgHandler.isLoaded)
	{
		CefString strCode(strJsCode), strUrl("");
		m_pListMsgHandler.handler->GetBrowser()->GetMainFrame()->ExecuteJavaScript(strCode, strUrl, 0);
	}

}


void CMainFrame::LoadBrowser(char* url)
{
	if (m_pVisitorRelatedHandler.isCreated)
	{
		if (m_pVisitorRelatedHandler.handler->GetBrowser())
		{
			if (url == NULL)
			{
				m_pVisitorRelatedHandler.handler->LoadString(m_defaultUrlInfo.c_str());
			}
			else
			{
				m_pVisitorRelatedHandler.handler->GetBrowser()->GetMainFrame()->LoadURL(url);
			}
		}
	}
	else 
	{
		RECT rc = { 0, 0, 0, 0 };
		if (url == NULL)
		{
			// 首次创建cef页面时就加载空页面的情况
			m_pVisitorRelatedHandler.handler->CreateBrowser(this->m_hWnd, rc, "about:blank", Handler_VisitorRelated);
		}
		else
		{
			m_pVisitorRelatedHandler.handler->CreateBrowser(this->m_hWnd, rc, url, Handler_VisitorRelated);
		}
	}
}




void CMainFrame::ShowRightOptionFrameView(unsigned long id,string sid)
{
	CWebUserObject *pWebUser = NULL;
	string strFrom, strEnd;
	string strUrl;
	CCodeConvert f_covet;
	string msg;
	char showMsg[1024] = {0};
	int index = m_curSelectOptionBtn;

	if (index == 0)
	{
		m_pVisitorRelatedHandler.handler->ShowBrowser(SW_HIDE);
		return;
	}

	m_pVisitorRelatedHandler.handler->ShowBrowser(SW_SHOW);

	//这里只判断 是不是访客用户 
	if (id == 0)
		pWebUser = m_manager->GetWebUserObjectBySid((char*)sid.c_str());
	else
		pWebUser = m_manager->GetWebUserObjectByUid(id);

	

	if ( (id == 0 && sid.empty()) || pWebUser == NULL || !m_pVisitorRelatedHandler.isCreated)
	{
		LoadBrowser(NULL);
		return;
	}
	int nTransFrom = -1;
	if (pWebUser->m_bIsFrWX)
	{
		nTransFrom = 0;
	}
	else
	{
		nTransFrom = 1;
	}

	_globalSetting.GetCurTimeString(strFrom, strEnd, 7);
	try
	{
		switch (index)
		{
		case TYPESELECT_MSG:

			return;

		case TYPESELECT_INFO:
			if (pWebUser->m_bIsGetInfo)
			{
				if (pWebUser->m_bIsFrWX)
				{
					// 微信用户信息
					string content = CreateClientInfoHtml(pWebUser->m_pWxUserInfo);
					f_covet.Gb2312ToUTF_8(msg, content.c_str(), content.length());
				}
				else
				{
					// web用户信息
					f_covet.Gb2312ToUTF_8(msg, pWebUser->m_strInfoHtml.c_str(), pWebUser->m_strInfoHtml.length());
				}
				m_pVisitorRelatedHandler.handler->LoadString(msg);
			}
			else
			{
				m_pVisitorRelatedHandler.handler->LoadString(m_defaultUrlInfo.c_str());
				if (pWebUser->m_bIsFrWX)
				{
					m_manager->SendGetWxUserInfo(pWebUser->webuserid, pWebUser->chatid);
				}
			}
			break;
		case TYPESELECT_CLIENT:
			_globalSetting.GetCurTimeString(strFrom, strEnd, 30);
			sprintf(showMsg, m_manager->m_initConfig.visitorpage_visitortail, m_manager->m_vip.c_str(),pWebUser->chatid, pWebUser->webuserid, pWebUser->info.sid, 0, nTransFrom, strFrom.c_str(), strEnd.c_str(),m_manager->m_userInfo.UserInfo.uid);
			strUrl = showMsg;
			break;
		case TYPESELECT_CHATID:
			sprintf(showMsg, m_manager->m_initConfig.visitorpage_visitorbill, m_manager->m_vip.c_str(),pWebUser->chatid, pWebUser->webuserid, pWebUser->info.sid, 0, 0, strFrom.c_str(), strEnd.c_str());
			strUrl = showMsg;
			break;
		case TYPESELECT_CLIENTINFO:
			//客户信息
			sprintf(showMsg, m_manager->m_initConfig.visitorpage_visitorinfo, m_manager->m_vip.c_str(),pWebUser->chatid, pWebUser->webuserid, pWebUser->info.sid, 0, 0, strFrom.c_str(), strEnd.c_str(), pWebUser->info.sid);
			strUrl = showMsg;
			break;
		case TYPESELECT_NOTICE:
			//改成下订单
			if (pWebUser->m_bIsFrWX)
			{
				sprintf(showMsg, m_manager->m_initConfig.visitorpage_visitororder, m_manager->m_vip.c_str(),pWebUser->chatid, pWebUser->webuserid, pWebUser->info.sid, 0, 0, strFrom.c_str(), strEnd.c_str(), pWebUser->info.thirdid, pWebUser->m_sWxAppid, 0);
				strUrl = showMsg;
			}	
			else
			{
				string strTmp = "<html><head>";
				strTmp += STRING_HTML_META;
				strTmp += STRING_HTML_BASE;
				strTmp += STRING_HTML_STYLE;
				strTmp += "</head><body>";
				strTmp += "<div><p><span style=\"font - size:16px; color:#cccccc\">目前仅支持微信用户下订单！</span></p><div></body>";
				f_covet.Gb2312ToUTF_8(msg, strTmp.c_str(), strTmp.length());
				m_pVisitorRelatedHandler.handler->LoadString(msg);
			}
			break;

		case TYPESELECT_ORDER:
		//查订单
			//strURL.Format(pApp->m_InitConfig.visitorpage_visitororder, pWebUser->chatid, pWebUser->webuserid, pWebUser->info.sid, 0, 0, strFrom, strEnd, pWebUser->info.thirdid, pWebUser->m_sWxAppid, 1);
			sprintf(showMsg, m_manager->m_initConfig.visitorpage_visitororder, m_manager->m_vip.c_str(), pWebUser->chatid, pWebUser->webuserid, pWebUser->info.sid, 0, 0, strFrom.c_str(), strEnd.c_str(), pWebUser->info.thirdid, pWebUser->m_sWxAppid, 1);
			strUrl = showMsg;


			break;
		default:
			break;
		}

	}
	catch (...)
	{
		strUrl.empty();
	}

	if (!strUrl.empty())
	{
		if (strUrl != "about:blank")
		{
			strUrl += "&token=";
			strUrl += m_manager->m_login->m_szAuthtoken;
		}
		LoadBrowser((char*)strUrl.c_str());
	}
}

void CMainFrame::ShowClearMsg()
{
	//显示聊天界面内容
	if (m_pListMsgHandler.isLoaded)
	{
		CefString strCode("ClearHistory();"), strUrl("");
		m_pListMsgHandler.handler->GetBrowser()->GetMainFrame()->ExecuteJavaScript(strCode, strUrl, 0);

	}

}

void CMainFrame::ChangeShowUserMsgWnd(unsigned long id)
{
	map<unsigned long, UserListUI::Node*>::iterator iter = { 0, NULL };
	UserListUI::Node*  tempNode = NULL;
	CUserObject *pUser = NULL;
	CWebUserObject *pWebUser = NULL;


	if ((m_curSelectId == id && id!= 0) || (id == 0 && m_curSavedSid.length() == 0) )//切换聊天对象显示 
		return;

	CheckIdForUerOrWebuser(id, m_curSavedSid, &pWebUser, &pUser);	
	if (pUser == NULL && pWebUser == NULL)
		return;
	
	ShowClearMsg();
	//先寻找是不是坐席
	if (pUser != NULL) //坐席
	{
		list<ONE_MSG_INFO>::iterator iter = pUser->m_strMsgs.begin();

		for (; iter != pUser->m_strMsgs.end(); iter++)
		{
			ONE_MSG_INFO msgInfo = *iter;
			if (m_pListMsgHandler.isLoaded)
			{
				CefString strCode(msgInfo.msg), strUrl("");
				m_pListMsgHandler.handler->GetBrowser()->GetMainFrame()->ExecuteJavaScript(strCode, strUrl, 0);
			}
		}
	}
	else  //用户
	{
		list<ONE_MSG_INFO>::iterator iter = pWebUser->m_strMsgs.begin();

		for (; iter != pWebUser->m_strMsgs.end(); iter++)
		{
			ONE_MSG_INFO msgInfo = *iter;
			CefString strCode(msgInfo.msg), strUrl("");
			m_pListMsgHandler.handler->GetBrowser()->GetMainFrame()->ExecuteJavaScript(strCode, strUrl, 0);
		}
	}

}
 
//右键菜单的处理 根据控件的名字区分   
//右键菜单弹出处理 
void CMainFrame::OnMenuEvent(CDuiString controlName)
{
	TNotifyUI msg;

	if (controlName.IsEmpty())
		return;

	CWebUserObject *pWebUser = NULL;
	CUserObject   *pUser = NULL;

	CheckIdForUerOrWebuser(m_savedClickId,m_curSavedSid,&pWebUser,&pUser);
//选择 0 enter 发送消息 还是 1 CTRL enter 发送消息  
	if (controlName == L"MenuElement_btn_send_menu_enter")   //按enter 键发送
	{
		if (m_manager->m_sysConfig->m_nKeySendType == 1)
		    m_manager->m_sysConfig->m_nKeySendType = 0;
	}
	else if (controlName == L"MenuElement_btn_send_menu_ctrl_enter") //按Ctrl + enter 键发送
	{
		if (m_manager->m_sysConfig->m_nKeySendType == 0)
		    m_manager->m_sysConfig->m_nKeySendType  =  1;
	}

	//隐藏主窗口
	else if (controlName == L"menu_hide_main_wnd")
	{
		if (m_wndShow == 0)
		{		
			m_wndShow = true;
			m_frameSmallMenu.SetMenuType(1);
			::ShowWindow(this->m_hWnd, SW_HIDE);
		}
		else
		{
			m_wndShow = false;
			m_frameSmallMenu.SetMenuType(0);
		
			if (::IsIconic(this->m_hWnd))	
				::ShowWindow(this->m_hWnd, SW_RESTORE);
			else
				::ShowWindow(this->m_hWnd, SW_SHOW);			
		}
	}
	//上线
	else if (controlName == L"menu_online")
	{

		CreateSmallTaskIcon(DEFINE_SMALL_ICON_PATH);




	}
	//繁忙
	else if (controlName == L"menu_busy")
	{

		CreateSmallTaskIcon(DEFINE_SMALL_BUSY_ICON_PATH);
	}
	//离开
	else if (controlName == L"menu_leave")
	{

		CreateSmallTaskIcon(DEFINE_SMALL_LEAVE_ICON_PATH);
	}
	//切换账号
	else if (controlName == L"menu_logout")
	{
		_globalSetting.m_logoutState = 1;
		OnCloseBtn(msg);
	}
	//系统设置
	else if (controlName == L"menu_sys_set")
	{
		//if (m_hSystemSettings == NULL)
		{
			m_hSystemSettings = new CSystemSettings();
			m_hSystemSettings->m_sysConfig = m_manager->m_sysConfig;
			m_hSystemSettings->Create(m_hWnd, _T(""), UI_WNDSTYLE_DIALOG, 0, 0, 0, 0, 0, NULL);
			m_hSystemSettings->CenterWindow();

			RECT sysRect;
			GetWindowRect(m_hWnd, &sysRect);

			int cx = 800;
			int cy = 500;
			int x = (sysRect.right - cx) / 2;
			int y = (sysRect.bottom - cy) / 2;

			::SetWindowPos((HWND)m_hSystemSettings, NULL, x, y, cx, cy, NULL);
			::ShowWindow((HWND)m_hSystemSettings, SW_SHOW);
		}
		//else
		{
			//m_hSystemSettings->ShowWnd(SW_SHOW);
		}
	}

	//退出
	else if (controlName == L"menu_quit")
	{
		_globalSetting.m_logoutState = 0;
		OnCloseBtn(msg);
	}


	//屏蔽此人
	else if (controlName == L"menu_right_forbid_user")
	{
	}

	//更新好友对象
	else if (controlName == L"menu_right_update_friend")
	{
		
	}

	//查找访客
	else if (controlName == L"menu_right_find_visitor")
	{
	}

	//更改访客名称
	else if (controlName == L"menu_right_change_visitor_name")
	{
	}


	//移除对话
	else if (controlName == L"menu_right_move_chat")
	{
	}

	//屏蔽此人
	else if (controlName == L"menu_right_forbid_user")
	{
	}

	//接受此对话
	else if (controlName == L"menu_right_accept_chat")
	{
		AcceptChat();
	}

	//拒绝此对话
	else if (controlName == L"menu_right_refuse_chat")
	{

		RefuseChat();
	}

	//显示全部
	else if (controlName == L"menu_right_show_all")
	{
		m_manager->m_sysConfig->m_nFilterType = VISITORFILTER_ALL;
	}

	//显示我的全部联系人
	else if (controlName == L"MenuElement_right_my_history_visitor")
	{
		m_manager->m_sysConfig->m_nFilterType = VISITORFILTER_MYVISITOR;
	}

	//只显示全部最近联系人
	else if (controlName == L"MenuElement_right_all_history_visitor")
	{
		m_manager->m_sysConfig->m_nFilterType = VISITORFILTER_ALLVISITOR;
	}

	//只显示访问时间超过1分钟的联系人
	else if (controlName == L"MenuElement_right_1_minute")
	{
		m_manager->m_sysConfig->m_nFilterType = VISITORFILTER_1MINUTES;
	}
	//只显示访问时间超过3分钟的联系人    
	else if (controlName == L"MenuElement_right_3_minute")
	{
		m_manager->m_sysConfig->m_nFilterType = VISITORFILTER_3MINUTES;
	}

	//只显示访问时间超过5分钟的联系人
	else if (controlName == L"MenuElement_right_5_minute")
	{
		m_manager->m_sysConfig->m_nFilterType = VISITORFILTER_5MINUTES;
	}

	//只显示访问时间超过10分钟的联系人
	else if (controlName == L"MenuElement_right_10_minute")
	{
		m_manager->m_sysConfig->m_nFilterType = VISITORFILTER_10MINUTES;
	}

	//用户设置
	else if (controlName == L"MenuElement_right_user_set")
	{
		CSelectVisitorWnd *dlg = new CSelectVisitorWnd();

		dlg->Create(m_hWnd, _T(""), UI_WNDSTYLE_DIALOG, 0, 0, 0, 0, 0, NULL);
		dlg->CenterWindow();
		dlg->ShowModal();
	}


	//自定义邀请
	else if (controlName == L"menu_right_online_visitor_request_set") 
	{

	}
	//邀请对话
	else if (controlName == L"menu_right_online_visitor_request_chat") 
	{

		if (pWebUser != NULL && pWebUser->info.userstatus != USER_STATUS_OFFLINE
			&& (pWebUser->onlineinfo.talkstatus == TALK_STATUS_NO
			|| pWebUser->onlineinfo.talkstatus == TALK_STATUS_AUTOINVITE))
		{

			if (m_manager->SendTo_InviteWebUser(pWebUser, APPLY_ASK, "") == 0)
			{
				map<string, UserListUI::Node*>::iterator iterSid = m_visitorOnlineNode.find(pWebUser->info.sid);
				if (iterSid != m_visitorOnlineNode.end())
				{
					UserListUI::Node* tempNode = iterSid->second;
					m_visitorOnlineNode.erase(iterSid);

					CDuiString text = tempNode->data()._text;


					if (tempNode != NULL && tempNode->data()._level >= 0 && tempNode->data()._level <= 5)
						pUserList->RemoveNode(tempNode);

			
					//加入邀请中列表
					tempNode = pMySelfeNode->child(2);
					UserListUI::Node* AddNode = pUserList->AddNode(text, pWebUser->webuserid, pWebUser->info.sid, tempNode);
					pUserList->ExpandNode(tempNode, true);

					m_visitorOnlineNode.insert(pair<string, UserListUI::Node*>(pWebUser->info.sid, AddNode));

				}



			}

		}

	}
	//直接对话
	else if (controlName == L"menu_right_online_visitor_do_chat") 
	{
		if (pWebUser != NULL && pWebUser->info.userstatus != USER_STATUS_OFFLINE
			&& (pWebUser->onlineinfo.talkstatus == TALK_STATUS_NO
			|| pWebUser->onlineinfo.talkstatus == TALK_STATUS_AUTOINVITE))
		{

			if (m_manager->SendTo_InviteWebUser(pWebUser, APPLY_OPEN, "") == 0)
			{
				map<string, UserListUI::Node*>::iterator iterSid = m_visitorOnlineNode.find(pWebUser->info.sid);
				if (iterSid != m_visitorOnlineNode.end())
				{
					UserListUI::Node* tempNode = iterSid->second;
					m_visitorOnlineNode.erase(iterSid);

					CDuiString text = tempNode->data()._text;


					if (tempNode != NULL && tempNode->data()._level >= 0 && tempNode->data()._level <= 5)
						pUserList->RemoveNode(tempNode);


					//加入对话中列表
					tempNode = pMySelfeNode->child(0);
					UserListUI::Node* AddNode = pUserList->AddNode(text, pWebUser->webuserid, pWebUser->info.sid, tempNode);
					pUserList->ExpandNode(tempNode, true);

					m_visitorOnlineNode.insert(pair<string, UserListUI::Node*>(pWebUser->info.sid, AddNode));

				}
			}


		}
	}

	//邀请留言
	else if (controlName == L"menu_right_online_visitor_request_message")  
	{

		if (pWebUser != NULL && pWebUser->info.userstatus != USER_STATUS_OFFLINE
			&& (pWebUser->onlineinfo.talkstatus == TALK_STATUS_NO
			|| pWebUser->onlineinfo.talkstatus == TALK_STATUS_AUTOINVITE))
		{
			m_manager->SendTo_InviteWebUser(pWebUser, MYGETNOTE, "");
		}
	}

	//发送网页和图片
	else if (controlName == L"menu_right_send_html_image")
	{

	}
	//发送文件
	else if (controlName == L"menu_right_send_file")
	{

	}
	//转接此对话
	else if (controlName == L"menu_right_invote_talk_to_other")
	{
		m_topWndType = 1;
		//获取在线坐席数
		m_manager->SendTo_GetOnlineUser();
	}

	//邀请协助
	else if (controlName == L"menu_right_request_other_help_talk")
	{
		m_topWndType = 2;
		//获取在线坐席数
		m_manager->SendTo_GetOnlineUser();
	}
	//结束对话
	else if (controlName == L"menu_right_finish_talk")
	{
		m_manager->SendTo_CloseChat(m_curSelectId, CHATCLOSE_USER);
	}

	



}





void CMainFrame::OnCtrlVEvent()
{
	char fileName[MAX_1024_LEN]={0}; 
	WCHAR WName[MAX_1024_LEN] = { 0 };

	if (IsClipboardFormatAvailable(CF_BITMAP))
	{
		if (::OpenClipboard(this->GetHWND()))
		{
			//获得剪贴板数据 
			HBITMAP handle = (HBITMAP)GetClipboardData(CF_BITMAP);
			if (handle != NULL)
            {
				SetCopyFileName(fileName);
				BITMAP bm; // 得到位图对象
				GetObject(handle, sizeof(BITMAP), &bm);
				SaveBitmapToFile(handle, bm, fileName);
				::CloseClipboard(); // 关闭剪贴板
			}
			ANSIToUnicode(fileName,WName);
			_RichEdit_InsertFace(m_pSendEdit, WName, 0, m_savedImageIndex);
		
			//m_editImageMap.insert(pair<unsigned long, WCHAR *>(m_savedImageIndex, WName));
			SaveEditImageData(m_savedImageIndex,WName);
			m_savedImageIndex += 1;
		}
	}
	else if (IsClipboardFormatAvailable(CF_TEXT))
	{
		m_pSendEdit->PasteSpecial(CF_TEXT);
	}


#if 0
	if (IsClipboardFormatAvailable(CF_BITMAP))
	{
		if (::OpenClipboard(this->GetHWND()))
		{
			//获得剪贴板数据 
			HBITMAP handle = (HBITMAP)GetClipboardData(CF_BITMAP);
			if (handle != NULL)
			{
				//HDC hDC = ::GetDC(m_hWnd); // 获取设备环境句柄
				//HDC hdcMem = CreateCompatibleDC(hDC); // 创建与设备相关的内存环境
				//SelectObject(hdcMem, handle); // 选择对象
				BITMAP bm; // 得到位图对象
				GetObject(handle, sizeof(BITMAP), &bm);
				SaveBitmapToFile(handle, bm, "E:\\YunKa\\test.jpg");

				::CloseClipboard(); // 关闭剪贴板

#if 0
				HDC hDC = ::GetDC(m_hWnd); // 获取设备环境句柄
				HDC hdcMem = CreateCompatibleDC(hDC); // 创建与设备相关的内存环境
				SelectObject(hdcMem, handle); // 选择对象
				SetMapMode(hdcMem, GetMapMode(hDC)); // 设置映射模式
				BITMAP bm; // 得到位图对象
				GetObject(handle, sizeof(BITMAP), &bm);
				BitBlt(hDC, 0, 0, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCCOPY); //位图复制
				::ReleaseDC(m_hWnd, hDC); // 释放设备环境句柄
				DeleteDC(hdcMem); // 删除内存环境
				::CloseClipboard(); // 关闭剪贴板
#endif
			}


#if 0

			//获得剪贴板数据 
			HBITMAP handle = (HBITMAP)GetClipboardData(CF_BITMAP);
			CBitmap * bm = CBitmap::FromHandle(handle);
			if (bm)
			{
				CString path = GetImageName();
				SaveBitmapToFile(bm, path);
				BITMAP map;
				bm->GetBitmap(&map);
				int width, height;
				width = map.bmWidth;
				height = map.bmHeight;
				GetDisplaySize(width, height);
				CString html;
				html.Format("<img src=\"%s\" width=%d height=%d>", path, width, height);
				this->InsertBuff(html, false);
				DeleteObject(bm);
			}
			CloseClipboard();

#endif


		}
	}
	else if (IsClipboardFormatAvailable(CF_TEXT))
	{
	}


	m_pSendEdit->PasteSpecial(CF_BITMAP);

	//	m_pSendEdit->PasteSpecial(CF_TEXT);

#endif

}



bool CMainFrame::SaveBitmapToFile(HBITMAP hbitmap,BITMAP bitmap, string lpFileName)
{
	int w = bitmap.bmWidth;
	int h = bitmap.bmHeight;
	DWORD dwCount = w * h;

	byte * p = new byte[bitmap.bmWidthBytes * h];//申请内存保存位图数据ITMAP)m_hBitmap, bitmap.bmbitmap.bmBitss * h, p);
	GetBitmapBits(hbitmap, bitmap.bmWidthBytes * h, p);

	BYTE *lpBits = p;
	for (int i = 0; i < dwCount; i++)
	{
		*(lpBits + i * 3 + 0) = *(p + i * 4 + 0);
		*(lpBits + i * 3 + 1) = *(p + i * 4 + 1);
		*(lpBits + i * 3 + 2) = *(p + i * 4 + 2);
	}

	// we swap red and blue for display, undo that.
	int bb = JpegFile::BGRFromRGB((BYTE *)(lpBits), w, h);

	// save RGB packed buffer to JPG
	BOOL ok = JpegFile::RGBToJpegFile(lpFileName, lpBits, w, h, 1, 75);
	delete[] p;

	return true;
}


MSG_RECV_TYPE CMainFrame::GetSendUserType(unsigned long id)
{
	MSG_RECV_TYPE type = MSG_RECV_CLIENT;
	CUserObject *pUser = NULL;
	CWebUserObject  *pWebUser = NULL;

	if (id == 0 && m_curSavedSid.empty())
		return MSG_RECV_ERROR;

	CheckIdForUerOrWebuser(id, m_curSavedSid, &pWebUser, &pUser);
	if (pUser == NULL)
	{
		if (pWebUser == NULL)
		{
			type = MSG_RECV_ERROR;
		}
		else if(pWebUser->m_bIsFrWX)
		{
			type = MSG_RECV_WX;
		}
		else
		{
			type = MSG_RECV_WEB;
		}

	}

	return type;
}


void CMainFrame::CheckIdForUerOrWebuser(unsigned long id,string sid,CWebUserObject **pWebUser,CUserObject **pUser)
{
	if (id != 0)
		*pUser  = m_manager->GetUserObjectByUid(id);

	if (*pUser == NULL || id == 0)
	{
		*pWebUser = m_manager->GetWebUserObjectByUid(id);
		if (*pWebUser == NULL)
			*pWebUser = m_manager->GetWebUserObjectBySid((char*)sid.c_str());
	}
}



string CMainFrame::CreateClientInfoHtml(WxUserInfo* pWxUser)
{
	char itemFormat[1024] = "<tr><td colspan = 2 width = 100%%><LI><SPAN class = clientnamefont1>%s: </SPAN><SPAN class = clientfont1>%s</SPAN></LI></td></tr>";
	string htmlContent = "信息未获取到";
	char htmlitem[1024] = { 0 };

	CCodeConvert vConvert;
	string getInfo;
	if (pWxUser != NULL)
	{
		htmlContent = "";
		if (!pWxUser->nickname.empty())
		{
			if (pWxUser->LanguageType == LANGUAGE_UTF8)
			{

				vConvert.UTF_8ToAnsi(getInfo, pWxUser->nickname.c_str(), pWxUser->nickname.length());
				sprintf(htmlitem, itemFormat, "昵称", getInfo.c_str());
				htmlContent = htmlitem;
			}
			else
			{
				sprintf(htmlitem,itemFormat, "昵称", pWxUser->nickname.c_str());
				htmlContent = htmlitem;
			}
		}

		if (pWxUser->sex == 1)
		{
			sprintf(htmlitem,itemFormat, "性别", "男");
			htmlContent += htmlitem;
		}
		else if (pWxUser->sex == 2)
		{
			sprintf(htmlitem,itemFormat, "性别", "女");
			htmlContent += htmlitem;
		}
		else
		{
			sprintf(htmlitem,itemFormat, "性别", "未知");
			htmlContent += htmlitem;
		}

		if (!pWxUser->country.empty())
		{
			if (pWxUser->LanguageType == LANGUAGE_UTF8)
			{
				vConvert.UTF_8ToAnsi(getInfo, pWxUser->country.c_str(), pWxUser->country.length());
				sprintf(htmlitem, itemFormat, "国家", getInfo.c_str());
				htmlContent += htmlitem;
			}
			else
			{
				sprintf(htmlitem, itemFormat, "国家", pWxUser->country.c_str());
				htmlContent += htmlitem;
			}
		}
		else
		{
			sprintf(htmlitem, itemFormat, "国家", "未知");
			htmlContent += htmlitem;
		}

		if (!pWxUser->province.empty())
		{
			if (pWxUser->LanguageType == LANGUAGE_UTF8)
			{
				vConvert.UTF_8ToAnsi(getInfo, pWxUser->province.c_str(), pWxUser->province.length());
				sprintf(htmlitem, itemFormat, "省份", getInfo.c_str());
				htmlContent += htmlitem;
			}
			else
			{
				sprintf(htmlitem, itemFormat, "省份", pWxUser->province.c_str());
				htmlContent += htmlitem;
			}
		}
		else
		{
			sprintf(htmlitem, itemFormat, "省份", "未知");
			htmlContent += htmlitem;
		}

		if (!pWxUser->city.empty())
		{
			if (pWxUser->LanguageType == LANGUAGE_UTF8)
			{
				vConvert.UTF_8ToAnsi(getInfo, pWxUser->city.c_str(), pWxUser->city.length());
				sprintf(htmlitem, itemFormat, "城市",getInfo.c_str());
				htmlContent += htmlitem;
			}
			else
			{
				sprintf(htmlitem, itemFormat, "城市", pWxUser->city.c_str());
				htmlContent += htmlitem;
			}
		}
		else
		{
			sprintf(htmlitem, itemFormat, "城市", "未知");
			htmlContent += htmlitem;
		}

		{
			char subscribe_time[256];
			FormatTime(pWxUser->subscribe_time, subscribe_time);
			sprintf(htmlitem, itemFormat, "关注时间", subscribe_time);
			htmlContent += htmlitem;
		}

		if (!pWxUser->language.empty())
		{
			sprintf(htmlitem, itemFormat, "语言", pWxUser->language.c_str());
			htmlContent += htmlitem;
		}
		else
		{
			sprintf(htmlitem, itemFormat, "语言", "未知");
			htmlContent += htmlitem;
		}

		if (!pWxUser->openid.empty())
		{
			sprintf(htmlitem, itemFormat, "标识", pWxUser->openid.c_str());
			htmlContent += htmlitem;
		}
		else
		{
			sprintf(htmlitem, itemFormat, "标识", "未知");
			htmlContent += htmlitem;
		}

		if (!pWxUser->fromwxname.empty())
		{
			sprintf(htmlitem, itemFormat, "来源", pWxUser->fromwxname.c_str());
			htmlContent += htmlitem;
		}
		else
		{
			sprintf(htmlitem, itemFormat, "来源", "未知");
			htmlContent += htmlitem;
		}
		/*if (!pWxUser->ullCometimes >= 0)
		{
			char strCometimes[32] = {0};;
			sprintf(strCometimes,"%u", pWxUser->ullCometimes);
			sprintf(htmlitem, itemFormat, "来访次数", strCometimes);
			htmlContent += htmlitem;
		}
		else
		{
			sprintf(htmlitem, itemFormat, "来访次数", "未知");
			htmlContent += htmlitem;
		}*/

		if (!pWxUser->headimgurl.empty())
		{
			char itemFormat1[1024] = "<tr><td colspan = 2><LI><SPAN class = clientnamefont1>%s: </SPAN><br><img src=\"%s\" width = 320 height=320></td></tr>";
		    sprintf(htmlitem, itemFormat1, "头像", pWxUser->headimgurl.c_str());
			htmlContent += htmlitem;
		}
		else
		{
			sprintf(htmlitem, itemFormat, "头像", "未知");
		}
	}

	return htmlContent;
}

extern "C" _declspec(dllimport) int StopRecordWAV();
extern "C" _declspec(dllimport) void WAVToAMR(const char* wavPath);

void CMainFrame::OnBtnVoice(TNotifyUI& msg)
{
	MSG_RECV_TYPE sendUserType = GetSendUserType(m_curSelectId);
	if (sendUserType == MSG_RECV_ERROR || m_curSelectId <= 0 || sendUserType == MSG_RECV_WEB || sendUserType == MSG_RECV_CLIENT)
	{
		// 提示不能给该类型用户发语音消息
	}
	else
	{
		char strJsCode[MAX_256_LEN];
		sprintf(strJsCode, "StartRecordAudio(\"%lu\",\"%d\");", m_curSelectId, MSG_RECV_WX);
		CefString strCode(strJsCode), strUrl("");
		m_pListMsgHandler.handler->GetBrowser()->GetMainFrame()->ExecuteJavaScript(strCode, strUrl, 0);
	}
}


//0 max 1 retore
void CMainFrame::MoveAndRestoreRightFrameControl(int type)
{
	int width = 0;

	CControlUI *rightLayout = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(_T("HorizontalLayout_RightFrameOption")));
	if (rightLayout)
		width = rightLayout->GetWidth()-15;
	
	
	RECT rect = m_pRightCommonWordCombo->GetPos();
	if (type == 1)
	{
		if (m_rightRectWnd.right > 0)
		    width = m_rightRectWnd.right - m_rightRectWnd.left - 15;
	}	
	else
		width = m_rightRectMax.right - m_rightRectMax.left - 15;

	rect.right = rect.left + width;
	m_pRightCommonWordCombo->SetPos(rect);

	rect = m_pRightCommonTypeCombo->GetPos();
	rect.right = rect.left + width;
	m_pRightCommonTypeCombo->SetPos(rect);

	rect = m_pRightCommonFindCombo->GetPos();
	rect.right = rect.left + width;
	m_pRightCommonFindCombo->SetPos(rect);

}

void CMainFrame::InitRightTalkList()
{
	WCHAR addTitle[1024] = { 0 };
	WCHAR title[1024] = { 0 };
	char savedStr[128] = {0};

	m_pTalkList = static_cast<UserListUI*>(m_PaintManager.FindControl(_T("talklist")));
	m_pTalkList->SetListName(_T("talklist"));

	for (int i = 0; i < m_savedShortAnswer.size(); i++)
	{

		SHORT_ANSWER_STRUCT data = m_savedShortAnswer[i];

		ANSIToUnicode(data.key, title);
		sprintf(savedStr,"%d", i );
		UserListUI::Node*  TalkList = m_pTalkList->AddNode(title, 0, savedStr);
		for (int j = 0; j < data.m_value.size(); j++)
		{
			SHORT_ANSWER_DATA ansData = data.m_value[j];
			ANSIToUnicode(ansData.title, title);
			
			wsprintf(addTitle, _T("{x 12}%s"), title);

			sprintf(savedStr, "%d-%d", i,j);
			m_pTalkList->AddNode(addTitle, 0, savedStr, TalkList);

		}

		if (i == 0)
			m_pTalkList->ExpandNode(TalkList, true);
		else
		    m_pTalkList->ExpandNode(TalkList, false);
	}

}

void CMainFrame::JsCallMFC(WPARAM wParam, LPARAM lParam)
{
	if (wParam == JS_CALL_RESTART_SESSION)
	{
		m_manager->RestartSession(lParam);
		delete[](char*)lParam;
	}
	else if (wParam == JS_CALL_START_RECORD)
	{
		CODE_RECORD_AUDIO result = StartRecordAudio();
		if (result != CODE_AUDIO_SUCCESS)
		{
			string strJsCode = "CancelRecord();";
			CefString strCode(strJsCode), strUrl("");
			m_pListMsgHandler.handler->GetBrowser()->GetMainFrame()->ExecuteJavaScript(strCode, strUrl, 0);
			return;
		}
	}
	else if (wParam == JS_CALL_SEND_AUDIO)
	{
		SEND_FILE_PARAMS* sendFile = (SEND_FILE_PARAMS*)lParam;
		unsigned long userId = sendFile->userId;
		MSG_RECV_TYPE recvUserType = (MSG_RECV_TYPE)sendFile->recvUserType;
		delete sendFile;
		
		m_bRecording = false;
		CODE_RECORD_AUDIO code = (CODE_RECORD_AUDIO)StopRecordWAV();

		// 停止录制,并保存文件
		if (StopRecordWAV() == CODE_AUDIO_LITTLE_TIME)
		{
			// 录音时间太短，录音无效
			return;
		}

		// 转码
		WAVToAMR(m_audioPath.c_str());

		// 发送文件的路径url编码
		CCodeConvert convert;
		string encodeFilePath;
		encodeFilePath = convert.URLEncode(m_audioPath.c_str());
		string msgId = m_manager->GetMsgId();

		string imagePath = FullPath("SkinRes\\mainframe\\msg_wait.gif");
		StringReplace(imagePath, "\\", "/");

		ShowMySelfSendMsg(m_audioPath + ".wav", MSG_DATA_TYPE_VOICE, msgId);

		m_manager->SendTo_Msg(userId, MSG_RECV_WX, msgId, MSG_DATA_TYPE_VOICE, m_audioPath);
	}
	else if (wParam == JS_CALL_CANCEL_RECORD)
	{
		// 取消录制
		CancelRecordAudio();
	}
	else if (wParam == JS_CALL_RESEND_FILE)
	{
		RESEND_FILE_PARAMS* reSendFile = (RESEND_FILE_PARAMS*)lParam;
		unsigned long userId = reSendFile->userId;
		MSG_RECV_TYPE recvUserType = (MSG_RECV_TYPE)reSendFile->recvUserType;
		string filaPath = reSendFile->filaPath;
		MSG_DATA_TYPE msgDataType = (MSG_DATA_TYPE)reSendFile->msgDataType;
		string msgId = reSendFile->msgId;
		delete reSendFile;

		m_manager->SendTo_Msg(userId, recvUserType, msgId, msgDataType, filaPath);
	}
	else if (wParam == JS_CALL_RERECV_FILE)
	{
		RERECV_FILE_PARAMS* reRecvParams = (RERECV_FILE_PARAMS*)lParam;
		string url = reRecvParams->url;
		MSG_FROM_TYPE msgFromUserType = (MSG_FROM_TYPE)reRecvParams->msgFromUserType;
		string msgId = reRecvParams->msgId;
		MSG_DATA_TYPE msgDataType = (MSG_DATA_TYPE)reRecvParams->msgDataType;
		unsigned long msgFromUserId = reRecvParams->msgFromUserId;
		unsigned long assistUserId = reRecvParams->assistUserId;
		delete reRecvParams;

		m_manager->ReRecv_Msg(url, msgFromUserType, msgId, msgDataType, msgFromUserId, assistUserId, 0);
	}
	else if (wParam == JS_CALL_VIEW_DETAILS)
	{
		VIEW_DETAILS_PARAMS* viewDetails = (VIEW_DETAILS_PARAMS*)lParam;
		ShowBigImage(viewDetails->url, viewDetails->msgDataType);
		delete viewDetails;
	}
}

extern "C" _declspec(dllimport) void CancelRecordWAV();
extern "C" _declspec(dllimport) int StartRecordWAV(const char* voicePath, const char* voiceName); // 开始录制

CODE_RECORD_AUDIO CMainFrame::StartRecordAudio()
{
	if (!m_bRecording)
	{
		string voicePath = FullPath("temp\\");
		string voiceName = GetTimeString();
		m_audioPath = voicePath + voiceName;
		StringReplace(m_audioPath, "\\", "/");
		CODE_RECORD_AUDIO bSuccess = (CODE_RECORD_AUDIO)StartRecordWAV(voicePath.c_str(), voiceName.c_str());
		if (bSuccess == CODE_AUDIO_SUCCESS)
			m_bRecording = true;
		return bSuccess;
	}
	else
	{
		return CODE_AUDIO_IS_RECORDING;
	}
}

void CMainFrame::CancelRecordAudio()
{
	m_bRecording = false;
	CancelRecordWAV();
}

void CMainFrame::OnBtnSelectSendType(TNotifyUI& msg)
{

	CDuiString xmlPath = L"mainframe\\btn_send_menu.xml";

	if (!xmlPath.IsEmpty())
	{  
		CMenuWnd* pMenu = new CMenuWnd(m_hMainWnd);
		CPoint cpoint = msg.ptMouse;
		cpoint.y -= 50;
		ClientToScreen(this->m_hWnd, &cpoint);
		CDuiString path = GetCurrentPathW();
		path += L"\\SkinRes\\menu\\check.png";
		if (m_manager->m_sysConfig->m_nKeySendType == 0)
		{
			pMenu->SetAttrData(L"MenuElement_btn_send_select_enter", path);
		}
		else
		{
			pMenu->SetAttrData(L"MenuElement_btn_send_select_ctrl_enter", path);
		}
		pMenu->SetPath((WCHAR*)xmlPath.GetData());
		pMenu->Init(NULL, _T(""), _T("xml"), cpoint);
	}
}



void CMainFrame::ShowWebBrowser(char *url)
{

	m_pListMsgHandler.handler->ShowBrowser(SW_HIDE);
	m_pVisitorRelatedHandler.handler->ShowBrowser(SW_HIDE);

	m_pWebURLHandler.handler->GetBrowser()->GetMainFrame()->LoadURL(url);
	m_pWebURLHandler.handler->ShowBrowser(SW_SHOW);

}

void CMainFrame::HideWebBrowser()
{

	m_pWebURLHandler.handler->ShowBrowser(SW_HIDE);

	m_pListMsgHandler.handler->ShowBrowser(SW_SHOW);
	m_pVisitorRelatedHandler.handler->ShowBrowser(SW_SHOW);




}


void CMainFrame::OnBtnSendFile(TNotifyUI& msg)
{
	TCHAR pszPath[1024];
	BROWSEINFO bi;
	bi.hwndOwner = this->GetHWND();
	bi.pidlRoot = NULL;
	bi.pszDisplayName = NULL;
	bi.lpszTitle = TEXT("请选择需要发送的文件");
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_STATUSTEXT | BIF_BROWSEINCLUDEFILES;;
	bi.lpfn = NULL;
	bi.lParam = 0;
	bi.iImage = 0;

	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
	if (pidl == NULL)
	{
		return;
	}

	if (SHGetPathFromIDList(pidl, pszPath))
	{
		char getInput[MAX_1024_LEN];
		UnicodeToANSI(pszPath, getInput);
		MSG_RECV_TYPE sendUserType = GetSendUserType(m_curSelectId);
		if (sendUserType == MSG_RECV_ERROR || m_curSelectId <= 0)
			return;

		string msgId = m_manager->GetMsgId();

		ShowMySelfSendMsg("发送文件.....", MSG_DATA_TYPE_FILE, msgId);

		m_manager->SendTo_Msg(m_curSelectId, sendUserType, msgId, MSG_DATA_TYPE_FILE,
			getInput);
	}

}



void CMainFrame::UpdateTopCenterButtonState(unsigned long id)
{
	CDuiString nameString = _T("");

	CUserObject *pUser = NULL;
	CWebUserObject  *pWebUser = NULL;

	CheckIdForUerOrWebuser(id, m_curSavedSid, &pWebUser, &pUser);
	if ((id == 0 && m_curSavedSid.length() == 0) || pUser != NULL)  //上层管理按钮 设置初始状态  选择的是 坐席 或者其他节点
	{
		for (int i = 0; i < MID_MANAGER_BUTTON_NUM; i++)
		{
			nameString.Format(_T("managerbutton_%d"), i + 1);
			if (m_pManagerBtn[i].m_pManagerBtn == NULL)
			{

					m_pManagerBtn[i].m_pManagerBtn = static_cast<CButtonUI*>(m_PaintManager.FindControl(nameString));
					StrCpyW(m_pManagerBtn[i].normalImage, m_pManagerBtn[i].m_pManagerBtn->GetNormalImage());
					StrCpyW(m_pManagerBtn[i].hotImage, m_pManagerBtn[i].m_pManagerBtn->GetHotImage());
					StrCpyW(m_pManagerBtn[i].pushedImage, m_pManagerBtn[i].m_pManagerBtn->GetPushedImage());
				
		



				//StrCpyW(m_pManagerBtn[i].normalImage, m_pManagerBtn[i].m_pManagerBtn->GetHotImage());
				//StrCpyW(m_pManagerBtn[i].hotImage, m_pManagerBtn[i].m_pManagerBtn->GetNormalImage());
				//StrCpyW(m_pManagerBtn[i].pushedImage, m_pManagerBtn[i].m_pManagerBtn->GetNormalImage());
			}
		}
		return;
	}
	
	//以下是访客 节点
	if (pWebUser == NULL)
		return;

	//如果在等待列表 显示 接受 屏蔽 邀请评价   这几个按钮
	map<unsigned long, UserListUI::Node*>::iterator iter = m_waitVizitorMap.find(id);
	if (iter != m_waitVizitorMap.end() || pWebUser->onlineinfo.talkstatus == TALK_STATUS_REQUEST)
	{

		for (int i = 0; i < MID_MANAGER_BUTTON_NUM; i++)
		{
			//if (i == 1 || i == 2|| i == 4 || i == 5)
			//{
			//	SetManagerButtonState(i, 0);
			//}
			//else
			//{
				SetManagerButtonState(i, 1);
			//}
		}
	}
	else
	{
		VISITOR_TYPE  type = CheckIdForTalkType(id);
		switch (type)
		{
		case VISITOR_REQ_ING:
			for (int i = 0; i < MID_MANAGER_BUTTON_NUM; i++)
			{
				//if (i == 1 || i == 2 || i == 4 || i == 5)
				//{
				//	SetManagerButtonState(i, 0);
				//}
				//else
				//{
				SetManagerButtonState(i, 1);
				//}
			}
			break;
		case VISITOR_TALKING_MYSELF:
			for (int i = 0; i < MID_MANAGER_BUTTON_NUM; i++)
			{
				//if (i == 0)
				//{
				////	SetManagerButtonState(i, 0);
				//}
				//else
				//{
					SetManagerButtonState(i, 1);
				//}
			}

			break;


		case VISITOR_TALKING_OTHER:
		case VISITOR_TALKING_HELP_OTHER:
			for (int i = 0; i < MID_MANAGER_BUTTON_NUM; i++)
			{
				//if (i==0||i == 1 || i == 2 || i == 4 || i == 5)
				//{
				//	SetManagerButtonState(i, 0);
				//}
				//else
				//{
					SetManagerButtonState(i, 1);
				//}
			}
			break;

		case VISITOR_TRANING:
		case VISITOR_INVOTING:

			for (int i = 0; i < MID_MANAGER_BUTTON_NUM; i++)
			{
				//if ( i == 1 || i == 2 || i == 4 || i == 5)
				//{
				//	SetManagerButtonState(i, 0);
				///}
				//else
				//{
					SetManagerButtonState(i, 1);
				//}
			}
			break;

		case VISITOR_IN_TALK_ING:
			for (int i = 0; i < MID_MANAGER_BUTTON_NUM; i++)
			{
				///if (i == 3 || i >= 6)
				//{
				//	SetManagerButtonState(i, 1);
				//}
				//else
				//{
					SetManagerButtonState(i, 1);
				//}
			}
			break;

			
		case VISITOR_ONLINE_AUTO_INVOTING:
		case VISITOR_ONLINE_AUTO_VISITING:
		case VISITOR_ONLINE_AUTO_END:

			for (int i = 0; i < MID_MANAGER_BUTTON_NUM; i++)
			{
				//if (i == 0 || i == 1 || i == 2 || i == 4 || i == 5)
				//{
				//	SetManagerButtonState(i, 0);
				//}
				//else
				//{
					SetManagerButtonState(i, 1);
				//}
			}

			break;

		default:

			for (int i = 0; i < MID_MANAGER_BUTTON_NUM; i++)
			{
				//if (i == 3 || i >= 6)
				//{
				//	SetManagerButtonState(i, 1);
				//}
				//else
				//{
				//	SetManagerButtonState(i, 0);
				//}
			}
			break;
		}



#if 0
		TREENODEENUM  type = CheckIdForNodeType(id);
		switch (type)
		{
		case MYSELF_CHILD_1:
			for (int i = 0; i < MID_MANAGER_BUTTON_NUM; i++)
			{
				if (i == 0)
				{
					m_pManagerBtn[i].m_pManagerBtn->SetNormalImage(m_pManagerBtn[i].normalImage);
					m_pManagerBtn[i].m_pManagerBtn->SetHotImage(m_pManagerBtn[i].normalImage);
					m_pManagerBtn[i].m_pManagerBtn->SetPushedImage(m_pManagerBtn[i].normalImage);
					m_pManagerBtn[i].m_buttonState = 0;
				}
				else
				{
					m_pManagerBtn[i].m_pManagerBtn->SetNormalImage(m_pManagerBtn[i].pushedImage);
					m_pManagerBtn[i].m_pManagerBtn->SetHotImage(m_pManagerBtn[i].hotImage);
					m_pManagerBtn[i].m_pManagerBtn->SetPushedImage(m_pManagerBtn[i].hotImage);
					m_pManagerBtn[i].m_buttonState = 1;
				}
			}
			break;
		case MYSELF_CHILD_2:
		case MYSELF_CHILD_3:
			for (int i = 0; i < MID_MANAGER_BUTTON_NUM; i++)
			{
				if (i == 1 || i==2 || i==6 || i==7)
				{
					m_pManagerBtn[i].m_pManagerBtn->SetNormalImage(m_pManagerBtn[i].normalImage);
					m_pManagerBtn[i].m_pManagerBtn->SetHotImage(m_pManagerBtn[i].normalImage);
					m_pManagerBtn[i].m_pManagerBtn->SetPushedImage(m_pManagerBtn[i].normalImage);
					m_pManagerBtn[i].m_buttonState = 0;
				}
				else
				{
					m_pManagerBtn[i].m_pManagerBtn->SetNormalImage(m_pManagerBtn[i].pushedImage);
					m_pManagerBtn[i].m_pManagerBtn->SetHotImage(m_pManagerBtn[i].hotImage);
					m_pManagerBtn[i].m_pManagerBtn->SetPushedImage(m_pManagerBtn[i].hotImage);
					m_pManagerBtn[i].m_buttonState = 1;
				}
			}
			break;
		case MYSELF_CHILD_4:

			break;
		case OTHER_CHILD_1:
		case MYSELF_CHILD_ACTIVE_1:
			for (int i = 0; i < MID_MANAGER_BUTTON_NUM; i++)
			{
				if (i == 0 || i == 1 || i == 2 || i == 6 || i == 7)
				{
					m_pManagerBtn[i].m_pManagerBtn->SetNormalImage(m_pManagerBtn[i].normalImage);
					m_pManagerBtn[i].m_pManagerBtn->SetHotImage(m_pManagerBtn[i].normalImage);
					m_pManagerBtn[i].m_pManagerBtn->SetPushedImage(m_pManagerBtn[i].normalImage);
					m_pManagerBtn[i].m_buttonState = 0;
				}
				else
				{
					m_pManagerBtn[i].m_pManagerBtn->SetNormalImage(m_pManagerBtn[i].pushedImage);
					m_pManagerBtn[i].m_pManagerBtn->SetHotImage(m_pManagerBtn[i].hotImage);
					m_pManagerBtn[i].m_pManagerBtn->SetPushedImage(m_pManagerBtn[i].hotImage);
					m_pManagerBtn[i].m_buttonState = 1;
				}
			}
			break;
		case OTHER_CHILD_2:

			break;
		case OTHER_CHILD_3:

			break;
		default:

			break;
		}
#endif

	}


}

void CMainFrame::RecvWebUserInfo(CWebUserObject* pWebUser, WEBUSER_INFO_NOTIFY_TYPE type)
{
	unsigned fatherId = 0;
	UserListUI::Node* tempNode = NULL;
	UserListUI::Node* fatherNode = NULL;
	CDuiString text;
	WCHAR name[64] = { 0 };
	ANSIToUnicode(pWebUser->info.name, name);
	int textType = 0;
	int fatherType = 0;

	UserListUI::Node* AddFatherNode = NULL;

	if (pWebUser->webuserid == 0)
		return;
	
	//1 更新用户名字  2 更新用户类型
	if (type == NOTIFY_IS_WX || type == NOTIFY_NAME)
	{
		//先查看是否是实在等待列表 然后查看是否在 会话列表  最后查看是否在 在线列表
		map<unsigned long, UserListUI::Node*>::iterator iterNode = m_waitVizitorMap.find(pWebUser->webuserid);
		if (iterNode == m_waitVizitorMap.end())
		{
			map<unsigned long, unsigned long>::iterator iter = m_allVisitorUserMap.find(pWebUser->webuserid);
			if (iter == m_allVisitorUserMap.end())
				return;

			fatherId = iter->second;

			map<unsigned long, UserListUI::Node*>::iterator iterUser = m_onlineNodeMap.find(fatherId);
			if (iterUser == m_onlineNodeMap.end())
				return;

			fatherNode = iterUser->second;

			iterNode = m_allVisitorNodeMap.find(pWebUser->webuserid);
			if (iterNode == m_allVisitorNodeMap.end())
				return;
			tempNode = iterNode->second;
			m_allVisitorNodeMap.erase(iterNode);
		}
		else
		{
			fatherNode = pWaitForAccept;
			fatherId = 0;
			tempNode = iterNode->second;
			m_waitVizitorMap.erase(iterNode);
			fatherType = 1;
		}

		if (tempNode != NULL && tempNode->data()._level >= 0 && tempNode->data()._level <= 5)
		{
			pUserList->RemoveNode(tempNode);
		}
	}
	else if (type == NOTIFY_ID) //更新webuerid
	{
		//先删除 此sid所在的 node 
		//pMySelfeNode->child

		if (pWebUser->onlineinfo.talkstatus == TALK_STATUS_TALK)
		{

			UserListUI::Node* tempNode = pMySelfeNode->child(0);
			for (int i = 0; i < tempNode->num_children(); i++)
			{
				UserListUI::Node* node = tempNode->child(i);
				string sid = node->data()._sid;
				if (strcmp(pWebUser->info.sid, sid.c_str()) == 0)
				{
					pUserList->RemoveNode(node);
					break;
				}
			}

			fatherNode = pMySelfeNode;
			fatherId = m_manager->m_userInfo.UserInfo.uid;

			fatherType = 0;
		}
		else
		{
			map<string, UserListUI::Node*>::iterator iterSid = m_visitorOnlineNode.find(pWebUser->info.sid);
			if (iterSid != m_visitorOnlineNode.end())
			{
				UserListUI::Node* tempNode = iterSid->second;
				m_visitorOnlineNode.erase(iterSid);

				if (tempNode != NULL && tempNode->data()._level >= 0 && tempNode->data()._level <= 5)
					pUserList->RemoveNode(tempNode);
			}

		
			fatherNode = pOnlineNode;
			fatherId = 0;

			fatherType = 3;
		}

		
	}
	


	textType = CheckIdForInvoteMyselfOrOther(pWebUser->webuserid);

	if (textType == 0)
	{
		//是否来自微信
		if (pWebUser->m_bIsFrWX)
		{
			text.Format(_T("{x 4}{i user_wx.png 1 0}{x 4}%s"), name);
		}
		else
		{
			text.Format(_T("{x 4}{i user_web.png 1 0}{x 4}%s"), name);
		}
	}
	else  if (textType > 0)
	{
		if (pWebUser->m_bIsFrWX)
		{
			text.Format(_T("{x 4}{i gameicons.png 18 16}{i user_wx.png 1 0}{x 4}%s"), name);
		}
		else
		{
			text.Format(_T("{x 4}{i gameicons.png 18 16}{i user_web.png 1 0}{x 4}%s"), name);
		}
	}

	if (fatherType == 1)
		AddFatherNode = fatherNode;
	else  if (fatherType == 0)
	{
		if (pWebUser->onlineinfo.talkstatus == TALK_STATUS_TALK)
		{
			AddFatherNode = fatherNode->child(0);
		}
		else if (pWebUser->onlineinfo.talkstatus == TALK_STATUS_TRANSFER)
		{
			AddFatherNode = fatherNode->child(1);
		}
		else if (pWebUser->onlineinfo.talkstatus == TALK_STATUS_INVITE)
		{
			AddFatherNode = fatherNode->child(2);
		
		}
		else if (pWebUser->onlineinfo.talkstatus == TALK_STATUS_NO)
		{
			//AddFatherNode = fatherNode;
			//fatherId = 0;
			return;
		}
	}

	else  if (fatherType == 3)
	{
		if (pWebUser->onlineinfo.talkstatus == TALK_STATUS_AUTOINVITE)
		{
			AddFatherNode = fatherNode->child(0);
		}
		else if (pWebUser->onlineinfo.talkstatus == TALK_STATUS_REQUEST)
		{
			AddFatherNode = fatherNode->child(1);
		}


	}



	UserListUI::Node* addNode = pUserList->AddNode(text, pWebUser->webuserid, pWebUser->info.sid, AddFatherNode);
	pUserList->ExpandNode(AddFatherNode, true);
	if (fatherType == 1)
	    m_waitVizitorMap.insert(pair<unsigned long, UserListUI::Node*>(pWebUser->webuserid, addNode));
	else
		m_allVisitorNodeMap.insert(pair<unsigned long, UserListUI::Node*>(pWebUser->webuserid, addNode));

	m_allVisitorUserMap.insert(pair<unsigned long, unsigned long>(pWebUser->webuserid, fatherId));


}

void CMainFrame::OnActiveUser(unsigned long id,string sid)
{
	CWebUserObject *pWebUser = m_manager->GetWebUserObjectByUid(id);
	if (pWebUser == NULL)
		pWebUser = m_manager->GetWebUserObjectBySid((char*)sid.c_str());

	int type = -1;
	if (id > 0)
	{
		map<unsigned long, UserListUI::Node*>::iterator iter = m_waitVizitorMap.find(id);
		if (iter != m_waitVizitorMap.end())
		{
			m_manager->SendTo_AcceptChat(id);
		}
		else
		{    
			//接受邀请 的操作
			list<unsigned long >::iterator iterList =   m_acceptingsUserList.begin();
			for (; iterList != m_acceptingsUserList.end(); iterList++)
			{
				if (id == *iterList)
				{
					//在邀请列表里删除 
					m_acceptingsUserList.erase(iterList);
					type = 0;
					break;
				}
			}
			iterList = m_transferUserList.begin();
			for (; iterList != m_transferUserList.end(); iterList++)
			{
				if (id == *iterList)
				{
					//在转接列表里删除 
					m_transferUserList.erase(iterList);
					type = 1;
					break;
				}
			}
		}
	}
	if (type == 0)
	{
		if (pWebUser == NULL)
			return;

		unsigned long  userid = GetInviteUserid(pWebUser->webuserid);
		m_manager->SendTo_InviteUserResult(pWebUser, userid, true);
		//然后加到对话中 
		map<unsigned long, UserListUI::Node*>::iterator iter = m_allVisitorNodeMap.find(pWebUser->webuserid);
		//没有找到
		if (iter == m_allVisitorNodeMap.end())
		{
			return;
		}
		UserListUI::Node *tempNode = iter->second;
		CDuiString text = L"";
		//先在用户的对话列表中删除 
		if (tempNode != NULL && tempNode->data()._level >= 0 && tempNode->data()._level <= 5)
			pUserList->RemoveNode(tempNode);

		m_allVisitorNodeMap.erase(iter);
		WCHAR name[64] = { 0 };
		ANSIToUnicode(pWebUser->info.name, name);
		if (pWebUser->m_bIsFrWX)
		{
			text.Format(_T("{x 4}{i gameicons.png 18 16}{i user_wx.png 1 0}{x 4}%s"), name);
		}
		else
		{
			text.Format(_T("{x 4}{i gameicons.png 18 16}{i user_web.png 1 0}{x 4}%s"), name);
		}
		UserListUI::Node *tempChildNode = pMySelfeNode->child(0);
		UserListUI::Node * addNode = pUserList->AddNode(text, pWebUser->webuserid,pWebUser->info.sid, tempChildNode);
		m_allVisitorNodeMap.insert(pair<unsigned long, UserListUI::Node*>(pWebUser->webuserid, addNode));

		pUserList->ExpandNode(tempChildNode, true);
		m_invoteOtherList.push_back(pWebUser->webuserid);
	}
	else if (type == 1)
	{
		m_manager->SendTo_TransferUserResult(pWebUser, true);
		//然后加到对话中 
		map<unsigned long, UserListUI::Node*>::iterator iter = m_allVisitorNodeMap.find(pWebUser->webuserid);
		//没有找到
		if (iter == m_allVisitorNodeMap.end())
		{
			return;
		}
		UserListUI::Node *tempNode = iter->second;
		CDuiString text;
		WCHAR name[64] = { 0 };
		ANSIToUnicode(pWebUser->info.name, name);

		//先在用户的对话列表中删除 
		if (tempNode != NULL && tempNode->data()._level >= 0 && tempNode->data()._level <= 5)
			pUserList->RemoveNode(tempNode);

		m_allVisitorNodeMap.erase(iter);

		int type = CheckIdForInvoteMyselfOrOther(pWebUser->webuserid);
		if (type > 0)
			ReleaseChatIdForInvoteMyselfOrOther(pWebUser->webuserid);
		//是否来自微信
		if (pWebUser->m_bIsFrWX)
		{
			text.Format(_T("{x 4}{i user_wx.png 1 0}{x 4}%s"), name);
		}
		else
		{
			text.Format(_T("{x 4}{i user_web.png 1 0}{x 4}%s"), name);
		}

		//先删掉 转接中的用户 然后放入对话中
		UserListUI::Node *tempChildNode = pMySelfeNode->child(0);
		UserListUI::Node * addNode = pUserList->AddNode(text, pWebUser->webuserid, pWebUser->info.sid, tempChildNode);
		m_allVisitorNodeMap.insert(pair<unsigned long, UserListUI::Node*>(pWebUser->webuserid, addNode));

		pUserList->ExpandNode(tempChildNode,true);
	}
}

//获取在线 坐席
void CMainFrame::RecvOnlineUsers(CGroupObject* pGroup)
{
	WCHAR showText[128] = { 0 };
	CDuiString duiText;
	UserListUI::Node* currentNode = NULL;
	int type = -1;
	CSelectOnlineUserWnd *dlg = new CSelectOnlineUserWnd();
	dlg->m_pGroup = pGroup;
	dlg->Create(m_hWnd, _T(""), UI_WNDSTYLE_DIALOG, 0, 0, 0, 0, 0, NULL);
	dlg->CenterWindow();
	int state = dlg->ShowModal();
	if (state == 1)
	{
		m_selectUserId = _globalSetting.m_currentSelectUserId;

		if (m_selectUserId == m_manager->m_userInfo.UserInfo.uid)
		{
			MessageBox(this->GetHWND(), L"不能选择自己", L"不能选择自己", 0);
			return;
		}

		CUserObject	*pUser = m_manager->GetUserObjectByUid(m_selectUserId);
		CWebUserObject *pWebUser = m_manager->GetWebUserObjectByUid(m_curSelectId);

		if (m_topWndType == 1 )  //转接 
		{
			if (m_curSelectId > 0 && m_selectUserId>0)
			{
				if (pWebUser != NULL)
					m_manager->SendTo_TransferRequestUser(pWebUser, m_selectUserId);
			}
		}
		else if (m_topWndType == 2 )  // 协助
		{
			if (pWebUser != NULL && (m_curSelectId > 0 && m_selectUserId>0))
				m_manager->SendTo_InviteUser(pWebUser, m_selectUserId);
		}
		else if (m_topWndType == 3 )  //内部对话
		{
			if (m_selectUserId <= 0)
				return;

			if (pUser == NULL)
			{
				list<CGroupObject *>::iterator iter = pGroup->m_ListGroupInfo.begin();

				for (; iter != pGroup->m_ListGroupInfo.end(); iter++)
				{

					ANSIToUnicode((*iter)->strName.c_str(), showText);
		
					list<CUserObject * >::iterator iteruser = (*iter)->m_ListUserInfo.begin(); //m_pGroup->m_ListUserInfo.begin();
					for (; iteruser != (*iter)->m_ListUserInfo.end(); iteruser++)
					{
						if (m_selectUserId == (*iteruser)->UserInfo.uid)
						{	
							pUser = (*iteruser);
							break;
						}
					}
				}
			}

			if (pUser == NULL)
				return;

			if (pUser->m_bFriend)
			{
				if (MessageBox(this->GetHWND(), L"该用户已经为您的友好对象! 您可以直接和他交流, 是否现在交流！", L"询问", MB_YESNO) != IDYES)
					return;

			}
			else if (pUser->m_bInnerTalk)
			{
				if (MessageBox(this->GetHWND(), L"该用户已经为您的内部对话对象! 您可以直接和他交流, 是否现在交流！", L"询问", MB_YESNO) != IDYES)
					return;
			}

			else
			{
				CDuiString strTemp = AnsiToUnicode(pUser->UserInfo.nickname);
				CDuiString nameString;
				nameString.Format(_T("{x 4}{i user_client.png 1 0}{x 4} %s"), strTemp.GetData());
				UserListUI::Node* addNode = pMySelfeNode->child(3);

				currentNode = pUserList->AddNode(nameString.GetData(), m_selectUserId,"", addNode);

				pUserList->ExpandNode(addNode,true);

				m_allVisitorNodeMap.insert(pair<unsigned long, UserListUI::Node*>(m_selectUserId, currentNode));
				m_allVisitorUserMap.insert(pair<unsigned long, unsigned long>(m_selectUserId, m_manager->m_userInfo.UserInfo.uid));

			}

			map<unsigned long, UserListUI::Node*>::iterator iter = m_onlineNodeMap.find(m_selectUserId);
			if (iter == m_onlineNodeMap.end())
			{
				iter = m_offlineNodeMap.find(pUser->UserInfo.uid);

				if (iter == m_offlineNodeMap.end())
				{
					type = 0;
				}
				
			}
			UserListUI::Node* node = NULL;
			if (type == 0)
				node = currentNode;
			else
				node = iter->second;
			
			pUserList->SelectNode(node);
			OnItemClickEvent(m_selectUserId, -1);
		}
	}
}



void CMainFrame::OnSelectUser(unsigned long id)
{


}



UserListUI::Node* CMainFrame::GetOneUserNode(unsigned long id)
{
	UserListUI::Node* returnNode = NULL;
	map<unsigned long, UserListUI::Node*>::iterator  iter = m_allVisitorNodeMap.find(id);
	if (iter == m_allVisitorNodeMap.end())
	{

		iter = m_onlineNodeMap.find(id);
		if (iter == m_onlineNodeMap.end())
		{
			iter = m_offlineNodeMap.find(id);
			if (iter == m_offlineNodeMap.end())
			{
				return returnNode;
			}
		}
	}
		
	returnNode = iter->second;
	return returnNode;
}

void CMainFrame::DeleteOneUserNode(unsigned long id)
{
	UserListUI::Node* returnNode = NULL;
	map<unsigned long, UserListUI::Node*>::iterator  iter = m_allVisitorNodeMap.find(id);
	if (iter == m_allVisitorNodeMap.end())
		return ;
	m_allVisitorNodeMap.erase(iter);
	return;
}




/********************  回调接口的处理    start ***********    ********************************************************************************************************************************/
/*****回调函数**************************************************************************************/

void CMainFrame::RecvShareListCount(int len)
{
	m_userListCount = len;
	m_manager->SendTo_GetAllUserInfo();
}

//回调过来的 坐席信息
void CMainFrame::RecvUserInfo(CUserObject* pWebUser)
{
	AddHostUserList(pUserList, pWebUser);
	m_recordListCount += 1;

	if (m_recordListCount == m_userListCount)
	{

		//获取会话消息
		m_manager->SendTo_GetListChatInfo();
		m_manager->StartLoginVisitor();	
	}


}


// 收到更新用户的在线状态
void CMainFrame::RecvUserStatus(CUserObject* pUser)
{




}

// 坐席上线消息
void CMainFrame::RecvOnline(IBaseObject* pObj)
{
	if (pObj->m_nEMObType == OBJECT_USER)
	{
		CUserObject* pUser = (CUserObject*)pObj;
		HostUserOnlineAndOffline(pUser,true);
	}

	else if (pObj->m_nEMObType == OBJECT_WEBUSER)
	{
		CWebUserObject* pWebUser = (CWebUserObject*)pObj;
		
		VisitorUserOnlineAndOffline(pWebUser, true);
	}

}

// 坐席下线消息
void CMainFrame::RecvOffline(IBaseObject* pObj)
{
	if (pObj->m_nEMObType == OBJECT_USER)
	{
		CUserObject* pUser = (CUserObject*)pObj;
		HostUserOnlineAndOffline(pUser, false);
	}

	else if (pObj->m_nEMObType == OBJECT_WEBUSER)
	{
		CWebUserObject* pWebUser = (CWebUserObject*)pObj;
		VisitorUserOnlineAndOffline(pWebUser,false);
	}
}



void CMainFrame::RecvChatInfo(CWebUserObject* pWebUser, CUserObject* pUser)
{
	CDuiString text;
	WCHAR name[64] = { 0 };
	ANSIToUnicode(pWebUser->info.name, name);

	//查找是否在 在线列表里面
	FindVisitorFromOnlineNode(pWebUser);

	//首先得找一下 webuser id  如果已经在 数列表里 则先删除
	UserListUI::Node* tempNode = GetOneUserNode(pWebUser->webuserid);
	if (tempNode != NULL)
	{
		if (tempNode->data()._level >= 0 && tempNode->data()._level <= 5)
			pUserList->RemoveNode(tempNode);

		DeleteOneUserNode(pWebUser->webuserid);

		map<unsigned long, unsigned long >::iterator  iter = m_allVisitorUserMap.find(pWebUser->webuserid);
		if (iter != m_allVisitorUserMap.end())
			m_allVisitorUserMap.erase(iter);
	}

	//是否来自微信
	if (pWebUser->m_bIsFrWX)
	{
		text.Format(_T("{x 4}{i user_wx.png 1 0}{x 4}%s"), name);
	}
	else
	{
		text.Format(_T("{x 4}{i user_web.png 1 0}{x 4}%s"), name);
	}

	//先判断类型
	//为接收的用户 在等待列表
	if (pWebUser->onlineinfo.talkstatus == TALK_STATUS_REQUEST)
	{
		//添加等待列表
		UserListUI::Node* tempNode = pUserList->AddNode(text, pWebUser->webuserid, pWebUser->info.sid, pWaitForAccept);
		pUserList->ExpandNode(pWaitForAccept, true);

		m_waitVizitorMap.insert(pair<unsigned long, UserListUI::Node*>(pWebUser->webuserid, tempNode));

		m_allVisitorUserMap.insert(pair<unsigned long, unsigned long>(pWebUser->webuserid, 0));

		//m_allVisitorNodeMap.insert(pair<unsigned long, UserListUI::Node*>(pWebUser->webuserid, tempNode));
	}
	//已接收的在会话列表
	else if (pWebUser->onlineinfo.talkstatus == TALK_STATUS_TALK)
	{
		if (pUser == NULL)
		{
			g_WriteLog.WriteLog(C_LOG_ERROR, "RecvChatInfo error: pUser = NULL, Print pWebUser state:%d ----webUid = %ul ", pWebUser->onlineinfo.talkstatus, pWebUser->webuserid);
			return;
		}


		//如果是自己的id 则加到自己下面
		if (pUser->UserInfo.uid == m_manager->m_userInfo.UserInfo.uid)
		{
			UserListUI::Node* tempNode = pMySelfeNode->child(0);
			UserListUI::Node* addNode = pUserList->AddNode(text, pWebUser->webuserid, pWebUser->info.sid, tempNode);
			pUserList->ExpandNode(tempNode, true);

			m_allVisitorNodeMap.insert(pair<unsigned long, UserListUI::Node*>(pWebUser->webuserid, addNode));
		}
		else //加到其他人列表底下
		{
			UserListUI::Node* tempNode = GetOneUserNode(pUser->UserInfo.uid);
			if (tempNode == NULL)
				return;

			UserListUI::Node* child = tempNode->child(0);

			UserListUI::Node* addNode = pUserList->AddNode(text, pWebUser->webuserid, pWebUser->info.sid, child);
			pUserList->ExpandNode(tempNode, true);
			pUserList->ExpandNode(child, true);

			m_allVisitorNodeMap.insert(pair<unsigned long, UserListUI::Node*>(pWebUser->webuserid, addNode));

		}

		m_allVisitorUserMap.insert(pair<unsigned long, unsigned long>(pWebUser->webuserid, pUser->UserInfo.uid));
	}


	else if (pWebUser->onlineinfo.talkstatus == TALK_STATUS_NO)
	{
		//暂时先不处理 状态是0 的状况
		g_WriteLog.WriteLog(C_LOG_ERROR, "RecvChatInfo error: talkstatus = 0 ");
		return;

		//FindVisitorFromOnlineNode(pWebUser);

		//加入在线访客的列表
		tempNode = pOnlineNode->child(1);
		UserListUI::Node* AddNode = pUserList->AddNode(text, pWebUser->webuserid, pWebUser->info.sid, tempNode);
		pUserList->ExpandNode(tempNode, true);

		m_visitorOnlineNode.insert(pair<string, UserListUI::Node*>(pWebUser->info.sid, AddNode));
	}

}

//请求接受的回调
void CMainFrame::RecvAcceptChat(CWebUserObject* pWebUser, CUserObject* pUser)
{
	UserListUI::Node *tempNode = NULL;
	CDuiString text;
	unsigned long uid = 0;
	UserListUI::Node* addNode = NULL;
	int type = -1;

	if (pWebUser == NULL)
		return;


	map<unsigned long, unsigned long > ::iterator iterlong = m_allVisitorUserMap.find(pWebUser->webuserid);
	if (iterlong != m_allVisitorUserMap.end())
	{
		m_allVisitorUserMap.erase(iterlong);
	}


	map<unsigned long, UserListUI::Node*>::iterator iter = m_waitVizitorMap.find(pWebUser->webuserid);
	if (iter != m_waitVizitorMap.end())
	{
		tempNode = iter->second;
		text = tempNode->data()._text;
		uid = tempNode->data()._uid;
		//当前选择 的用户就是 激活的用户
		m_curSelectId = uid;

		type = 0;

		m_waitVizitorMap.erase(iter);
	}
	else
	{
		iter = m_allVisitorNodeMap.find(pWebUser->webuserid);
		if (iter != m_allVisitorNodeMap.end())
		{
			tempNode = iter->second;
			//text = tempNode->data()._text;
			uid = tempNode->data()._uid;
			//当前选择 的用户就是 激活的用户
			m_curSelectId = uid;
			type = 1;

			WCHAR name[64] = { 0 };
			ANSIToUnicode(pWebUser->info.name, name);
			//是否来自微信
			if (pWebUser->m_bIsFrWX)
			{
				text.Format(_T("{x 4}{i user_wx.png 1 0}{x 4}%s"), name);
			}
			else
			{
				text.Format(_T("{x 4}{i user_web.png 1 0}{x 4}%s"), name);
			}
		}

		m_allVisitorNodeMap.erase(iter);
	}

	//需要从等待列表删除 这个用户
	if (tempNode != NULL && tempNode->data()._level >= 0 && tempNode->data()._level <= 5)
		pUserList->RemoveNode(tempNode);

	if (pUser == NULL)
	{
		return;
	}

	//如果回调返回的user uid和自己的相同 则加到自己回话底下
	if (pUser->UserInfo.uid == m_manager->m_userInfo.UserInfo.uid)
	{
		//需要 加到那个底下  还得判断状态 暂时先加第一个
		addNode = pMySelfeNode->child(0);
	}
	//否则寻找其他座席回话
	//1 先寻找 在线的坐席，2 在寻找离线坐席
	else
	{
		map<unsigned long, UserListUI::Node*>::iterator iter2 = m_onlineNodeMap.find(pUser->UserInfo.uid);
		if (iter2 == m_onlineNodeMap.end())
		{
			iter2 = m_offlineNodeMap.find(pUser->UserInfo.uid);

			if (iter2 == m_offlineNodeMap.end())
				return;
		}

		//需要 加到那个底下  还得判断状态 暂时先加第一个
		UserListUI::Node* parantNode = iter2->second;
		addNode = parantNode->child(0);
	}

	//添加显示 list
	UserListUI::Node* currentNode = pUserList->AddNode(text, uid, pWebUser->info.sid, addNode);
	pUserList->ExpandNode(addNode, true);

	//插入 用户map 同时删除 等等map
	m_allVisitorNodeMap.insert(pair<unsigned long, UserListUI::Node*>(uid, currentNode));
	m_allVisitorUserMap.insert(pair<unsigned long, unsigned long>(pWebUser->webuserid, pUser->UserInfo.uid));

	if (pUser->UserInfo.uid == m_manager->m_userInfo.UserInfo.uid)
	{

		pUserList->SelectNode(currentNode);
		OnItemClickEvent(pWebUser->webuserid, -1);
	}


}

void CMainFrame::RecvCloseChat(CWebUserObject* pWebUser)
{
	UserListUI::Node *tempNode = NULL;
	int type = 0;
	unsigned long uid = 0;
	CDuiString text;
	WCHAR name[64] = { 0 };
	ANSIToUnicode(pWebUser->info.name, name);
	map<unsigned long, UserListUI::Node*>::iterator iter = m_waitVizitorMap.find(pWebUser->webuserid);

	ReleaseChatIdForInvoteMyselfOrOther(pWebUser->webuserid);

	//没有找到
	if (iter == m_waitVizitorMap.end())
	{
		iter = m_allVisitorNodeMap.find(pWebUser->webuserid);

		if (iter == m_allVisitorNodeMap.end())
			goto lable;

		type = 1;
	}
	tempNode = iter->second;

	if (tempNode != NULL && tempNode->data()._level >= 0 && tempNode->data()._level <= 5)
	{
		//从坐席列表底下删除 然后加入等待列表
		pUserList->RemoveNode(tempNode);
	}



	//从 会话中 删除 当前访客 信息
	if (type == 0)
		m_waitVizitorMap.erase(iter);
	else
		m_allVisitorNodeMap.erase(iter);

//最后一步 将退出的用户 加到在线访客中  已结束的列表中
lable:


	//是否来自微信
	if (pWebUser->m_bIsFrWX)
	{
		text.Format(_T("{x 4}{i user_wx.png 1 0}{x 4}%s"), name);
	}
	else
	{
		text.Format(_T("{x 4}{i user_web.png 1 0}{x 4}%s"), name);
	}

	FindVisitorFromOnlineNode(pWebUser);
	UserListUI::Node* fatherNode = pOnlineNode->child(1);
	UserListUI::Node* AddNode = pUserList->AddNode(text, pWebUser->webuserid, pWebUser->info.sid, fatherNode);
	pUserList->ExpandNode(fatherNode,true);

	m_visitorOnlineNode.insert(pair<string, UserListUI::Node*>(pWebUser->info.sid, AddNode));


}

void CMainFrame::RecvReleaseChat(CWebUserObject* pWebUser)
{
	UserListUI::Node *tempNode = NULL;

	ReleaseChatIdForInvoteMyselfOrOther(pWebUser->webuserid);

	

	map<unsigned long, UserListUI::Node*>::iterator iter = m_allVisitorNodeMap.find(pWebUser->webuserid);
	//没有找到
	if (iter != m_allVisitorNodeMap.end())
	{

		tempNode = iter->second;
		m_allVisitorNodeMap.erase(iter);

		if (tempNode != NULL && tempNode->data()._level >= 0 && tempNode->data()._level <= 5)
		{
			//从坐席列表底下删除 然后加入等待列表
			pUserList->RemoveNode(tempNode);
		}

	}
	

	CWebUserObject *webuser = pWebUser;
	webuser->onlineinfo.talkstatus = TALK_STATUS_REQUEST;
	RecvChatInfo(pWebUser, NULL);



	//从 会话中 删除 当前访客 信息
	map<unsigned long, unsigned long>::iterator iterLong = m_allVisitorUserMap.find(pWebUser->webuserid);
	if (iterLong == m_allVisitorUserMap.end())
		return;
	m_allVisitorUserMap.erase(iterLong);

}



//收到邀请协助的 申请  邀请协助的 回调函数
void CMainFrame::RecvInviteUser(CWebUserObject* pWebUser, unsigned long uid)
{
	//CUserObject	*pUser = m_manager->GetUserObjectByUid(9692111);
	//CWebUserObject *pWebUser = m_manager->GetWebUserObjectByUid(m_curSelectId);
	CDuiString text = L"";
	unsigned long id = 0;
	string sid;
	WCHAR name[64] = { 0 };
	ANSIToUnicode(pWebUser->info.name, name);

	if (pWebUser == NULL)
		return;

	if (pWebUser->webuserid == 0)
		return;

	InsertInviteUserid(pWebUser->webuserid, uid);

	int type = -1;

	//放入邀请列表
	//
	list<unsigned long >::iterator iterList = m_acceptingsUserList.begin();
	for (; iterList != m_acceptingsUserList.end(); iterList++)
	{
		if (pWebUser->webuserid == *iterList)
		{
			type = 0;
			break;
		}
	}

	if (type != 0)
		m_acceptingsUserList.push_back(pWebUser->webuserid);


	map<unsigned long, UserListUI::Node*>::iterator iter = m_allVisitorNodeMap.find(pWebUser->webuserid);
	//没有找到
	if (iter != m_allVisitorNodeMap.end())
	{
		UserListUI::Node *tempNode = iter->second;
		if (tempNode != NULL)
		{
			text = tempNode->data()._text;
			id = tempNode->data()._uid;
			sid = tempNode->data()._sid;

		}

		//先在用户的对话列表中删除 
		if (tempNode != NULL && tempNode->data()._level >= 0 && tempNode->data()._level <= 5)
			pUserList->RemoveNode(tempNode);
		m_allVisitorNodeMap.erase(iter);
	}
	else
	{
		if (pWebUser->m_bIsFrWX)
		{
			text.Format(_T("{x 4}{i user_wx.png 1 0}{x 4}%s"), name);
		}
		else
		{
			text.Format(_T("{x 4}{i user_web.png 1 0}{x 4}%s"), name);
		}
		id = pWebUser->webuserid;
		sid = pWebUser->info.sid;
	}

	//然后显示 在邀请列表中
	UserListUI::Node* ChildNode = pMySelfeNode->child(2);
	//int index = 0;
	//int childNum = ChildNode->num_children;
	//if (childNum == 0)
	//	index = pUserList->GetNodeIndex(ChildNode);
	//else
	//{
	//	UserListUI::Node* childNodeTemp = ChildNode->child(childNum - 1);
	//	index = pUserList->GetNodeIndex(childNodeTemp);
	//}

	UserListUI::Node*currentNode = pUserList->AddNode(text, id,sid, ChildNode);

	pUserList->ExpandNode(ChildNode, true);
	m_allVisitorNodeMap.insert(pair<unsigned long, UserListUI::Node*>(id, currentNode));


}

//邀请协助 做的最后一次 回调

void CMainFrame::ResultInviteUser(CWebUserObject* pWebUser, unsigned long uid, RESULT_STATUS status)
{
	if (pWebUser == NULL || uid == 0)
		return;

	map<unsigned long, UserListUI::Node*>::iterator iter = m_allVisitorNodeMap.find(pWebUser->webuserid);
	if (iter != m_allVisitorNodeMap.end())
	{
		UserListUI::Node *tempNode = iter->second;

		//先在用户的对话列表中删除 
		if (tempNode != NULL && tempNode->data()._level >= 0 && tempNode->data()._level <= 5)
			pUserList->RemoveNode(tempNode);

		m_allVisitorNodeMap.erase(iter);
	}

	if (status == INVITE_ACCEPT)
	{
		//然后在添加
		CDuiString text;
		WCHAR name[64] = { 0 };
		ANSIToUnicode(pWebUser->info.name, name);
		if (pWebUser->m_bIsFrWX)
		{
			text.Format(_T("{x 4}{i gameicons.png 18 16}{i user_wx.png 1 0}{x 4}%s"), name);
		}
		else
		{
			text.Format(_T("{x 4}{i gameicons.png 18 16}{i user_web.png 1 0}{x 4}%s"), name);
		}
	
		m_invoteMyselfList.push_back(pWebUser->webuserid);
		UserListUI::Node *tempChildNode = pMySelfeNode->child(0);

		UserListUI::Node * addNode = pUserList->AddNode(text, pWebUser->webuserid, pWebUser->info.sid, tempChildNode);
		m_allVisitorNodeMap.insert(pair<unsigned long, UserListUI::Node*>(pWebUser->webuserid, addNode));

		pUserList->ExpandNode(tempChildNode, true);

	}

	else  if (status == INVITE_REFUSE)
	{



	}

	else if (status == INVITE_ING)
	{

	}



}


//邀请  转接的 回调函数
void CMainFrame::RecvTransferUser(CWebUserObject* pWebUser)
{
	CDuiString text = L"";
	unsigned long id = 0;
	string sid;
	WCHAR name[64] = { 0 };
	ANSIToUnicode(pWebUser->info.name, name);

	int type = -1;

	if (pWebUser == NULL)
		return;

	//放入转接列表
	//
	list<unsigned long >::iterator iterList = m_transferUserList.begin();
	for (; iterList != m_transferUserList.end(); iterList++)
	{
		if (pWebUser->webuserid == *iterList)
		{
			type = 0;
			break;
		}
	}

	if (type != 0)
		m_transferUserList.push_back(pWebUser->webuserid);


	map<unsigned long, UserListUI::Node*>::iterator iter = m_allVisitorNodeMap.find(pWebUser->webuserid);
	//没有找到
	if (iter != m_allVisitorNodeMap.end())
	{
		UserListUI::Node *tempNode = iter->second;

		if (tempNode != NULL)
		{
			text = tempNode->data()._text;
			id = tempNode->data()._uid;
			sid = tempNode->data()._sid;
		}

		//先在用户的对话列表中删除 
		if (tempNode != NULL && tempNode->data()._level >= 0 && tempNode->data()._level <= 5)
			pUserList->RemoveNode(tempNode);

		m_allVisitorNodeMap.erase(iter);

	}
	else
	{

		if (pWebUser->m_bIsFrWX)
		{
			text.Format(_T("{x 4}{i user_wx.png 1 0}{x 4}%s"), name);
		}
		else
		{
			text.Format(_T("{x 4}{i user_web.png 1 0}{x 4}%s"), name);
		}
		id = pWebUser->webuserid;
		sid = pWebUser->info.sid;
	}

	//然后显示 在转接列表中
	UserListUI::Node* ChildNode = pMySelfeNode->child(1);

	UserListUI::Node*currentNode = pUserList->AddNode(text, id,sid, ChildNode);

	pUserList->ExpandNode(ChildNode, true);
	m_allVisitorNodeMap.insert(pair<unsigned long, UserListUI::Node*>(id, currentNode));


}

void CMainFrame::ResultInviteWebUser(CWebUserObject* pWebUser, bool bAgree)
{
	//拒绝处理






}

//邀请转接的 最后一次 回调      
void CMainFrame::ResultTransferUser(CWebUserObject* pWebUser, CUserObject* pUser, RESULT_STATUS status)
{
	CDuiString text;
	WCHAR name[64] = { 0 };
	ANSIToUnicode(pWebUser->info.name, name);

	map<unsigned long, UserListUI::Node*>::iterator iter = m_allVisitorNodeMap.find(pWebUser->webuserid);
	//没有找到
	if (iter == m_allVisitorNodeMap.end())
	{
		return;
	}

	if (status == INVITE_ACCEPT)
	{
		UserListUI::Node *tempNode = iter->second;

		text = tempNode->data()._text;

#if 0
		//判定当前的转接对象 是不是在 协助列表里
		int type = CheckIdForInvoteMyselfOrOther(pWebUser->webuserid);
		if (type > 0)
			ReleaseChatIdForInvoteMyselfOrOther(pWebUser->webuserid);

		//是否来自微信
		if (pWebUser->m_bIsFrWX)
		{
			text.Format(_T("{x 4}{i user_wx.png 1 0}{x 4}%s"), name);
		}
		else
		{
			text.Format(_T("{x 4}{i user_web.png 1 0}{x 4}%s"), name);
		}
#endif


		//先在用户的对话列表中删除 
		if (tempNode != NULL && tempNode->data()._level >= 0 && tempNode->data()._level <= 5)
			pUserList->RemoveNode(tempNode);

		m_allVisitorNodeMap.erase(iter);


		//然后在添加  在接受的用户中 对话中 加上webuser

		map<unsigned long, UserListUI::Node*>::iterator  iter = m_onlineNodeMap.find(pUser->UserInfo.uid);
		if (iter == m_onlineNodeMap.end())
		{
			iter = m_offlineNodeMap.find(pUser->UserInfo.uid);
			if (iter == m_offlineNodeMap.end())
				return;
		}

		tempNode = iter->second;

		UserListUI::Node *tempChildNode = tempNode->child(1);

		UserListUI::Node * addNode = pUserList->AddNode(text, pWebUser->webuserid, pWebUser->info.sid, tempChildNode);
		m_allVisitorNodeMap.insert(pair<unsigned long, UserListUI::Node*>(pWebUser->webuserid, addNode));

		pUserList->ExpandNode(tempChildNode, true);

	}

	else  if (status == INVITE_REFUSE)
	{



	}







}



//拒绝 接受会话
void CMainFrame::RecvWebUserInInvite(CWebUserObject* pWebUser, CUserObject* pInviteUser)
{


	//直接将 web用户放入 访问者列表
	map<string, UserListUI::Node*>::iterator iterSid = m_visitorOnlineNode.find(pWebUser->info.sid);
	if (iterSid == m_visitorOnlineNode.end())
	{
		return;
	}
	UserListUI::Node* tempNode = iterSid->second;
	m_visitorOnlineNode.erase(iterSid);
	pUserList->RemoveNode(tempNode);

	UserListUI::Node* addNode = pOnlineNode->child(1);
	m_visitorOnlineNode.insert(pair<string, UserListUI::Node*>(pWebUser->info.sid, addNode));


}



/********************  回调接口的处理   end    ***********    ********************************************************************************************************************************/

//判定当前的用户id 处于那种状态底下
VISITOR_TYPE  CMainFrame::CheckIdForTalkType(unsigned long id)
{
	VISITOR_TYPE type = DEFAULT_POS;
	CUserObject *pUser = NULL;
	CWebUserObject  *pWebUser = NULL;
	UserListUI::Node* tempNode = NULL;

	CheckIdForUerOrWebuser(id, m_curSavedSid, &pWebUser, &pUser);	

	if (id == 0 && m_curSavedSid.empty())
		return type;

	//这一部分是 坐席位置判断
	//先判断是不是 坐席 然后判断 状态
	if (pUser != NULL)
	{
		//先查找是不是 自己
		if (id == m_manager->m_userInfo.UserInfo.uid)
			type = HOST_USER_MYSELF;
		   
		else
		{
			map<unsigned long, UserListUI::Node*>::iterator iter = m_onlineNodeMap.find(id);
			if (iter != m_onlineNodeMap.end())
				type = HOST_USER_ONLINE;
			else
			{
				iter = m_offlineNodeMap.find(pUser->UserInfo.uid);

				if (iter == m_offlineNodeMap.end())
				{
					type = HOST_USER_DEFAULT;
				}
				else
					type = HOST_USER_OFFLINE;
			}

		}
		return type;
	}

	//这一部分是 访客位置判断
	if (pWebUser == NULL)
	{
		//这里多做一次 判断 因为有时候 根据sid 不能取到 webuser 的信息
		map<string, UserListUI::Node*>::iterator iterSid = m_visitorOnlineNode.find(m_curSavedSid);
		if (iterSid != m_visitorOnlineNode.end())
		{
			tempNode = iterSid->second;
		}

		//查找 用户 在 在线访客的哪个节点下面
		for (int j = 0; j < 3; j++)
		{
			UserListUI::Node *tempFatherNode = pOnlineNode->child(j);
			int num = tempFatherNode->num_children();
			for (int i = 0; i < num; i++)
			{
				if (tempFatherNode->child(i) == tempNode)
				{

					type = (VISITOR_TYPE)(VISITOR_ONLINE_AUTO_INVOTING + j);
					//if (j==0)
					//    type = VISITOR_ONLINE_AUTO_VISITING;
					//else
					//	type = VISITOR_ONLINE_AUTO_END;			
					break;
				}
			}
		}
		return type;
	}
		

	if (pWebUser->onlineinfo.talkstatus == TALK_STATUS_NO)   //在线访客 访问中  或者   已结束
	{
		map<string, UserListUI::Node*>::iterator iterSid = m_visitorOnlineNode.find(pWebUser->info.sid);
		if (iterSid != m_visitorOnlineNode.end())
		{
			tempNode = iterSid->second;
		}

		//查找 用户 在 在线访客的哪个节点下面
		for (int j = 0; j < 3; j++)
		{
			UserListUI::Node *tempFatherNode = pOnlineNode->child(j);
			int num = tempFatherNode->num_children();
			for (int i = 0; i < num; i++)
			{
				if (tempFatherNode->child(i) == tempNode)
				{

					type = (VISITOR_TYPE)(VISITOR_ONLINE_AUTO_INVOTING + j);
					//if (j==0)
					//    type = VISITOR_ONLINE_AUTO_VISITING;
					//else
					//	type = VISITOR_ONLINE_AUTO_END;			
					break;
				}
			}
		}
		//return  type;
		//type = VISITOR_ONLINE_AUTO_VISITING;  //或者已结束     这两个 对上层的按钮处理是一样的
	}
	else if (pWebUser->onlineinfo.talkstatus == TALK_STATUS_AUTOINVITE) //自动邀请中
	{
		type = VISITOR_ONLINE_AUTO_INVOTING;
	}


	
	else if (pWebUser->onlineinfo.talkstatus == TALK_STATUS_REQUEST)  //请求中
	{
		type = VISITOR_REQ_ING;
	}

	else if (pWebUser->onlineinfo.talkstatus == TALK_STATUS_TALK)
	{
		unsigned  id = 0;
		map<unsigned long, unsigned long >::iterator  iter = m_allVisitorUserMap.find(pWebUser->webuserid);
		if (iter != m_allVisitorUserMap.end())
			id = iter->second;


		if (id == m_manager->m_userInfo.UserInfo.uid) //自己底下
		{
			list<unsigned long>::iterator iterList = m_invoteOtherList.begin();
			for (; iterList != m_invoteOtherList.end(); iterList++)
			{
				if (*iterList == pWebUser->webuserid)
				{
					type = VISITOR_TALKING_HELP_OTHER; // VISITOR_TALKING_MYSELF;
				}
			}

			if (type == DEFAULT_POS)
				type = VISITOR_TALKING_MYSELF;
		}
		else
		{

			type = VISITOR_TALKING_OTHER;
		}
	}

	else if (pWebUser->onlineinfo.talkstatus == TALK_STATUS_INVITE)
	{
		type = VISITOR_INVOTING;
	}

	else if (pWebUser->onlineinfo.talkstatus == TALK_STATUS_TRANSFER)
	{
		type = VISITOR_TRANING;
	}



	return  type;


}

//判定当前的用户id 处于那种状态底下
TREENODEENUM  CMainFrame::CheckIdForNodeType(unsigned long id)
{
	TREENODEENUM type = DEFAULT_TYPE;

	//先判断是不是 在自己底下的状态
	map<unsigned long, unsigned long>::iterator iterLong = m_allVisitorUserMap.find(id);
	if (iterLong == m_allVisitorUserMap.end())
		return type;

	unsigned long fatherId = iterLong->second;
	if (fatherId == 0)
		return type;

	UserListUI::Node* fatherNode;
	UserListUI::Node* childNode;

	//先找到自己的node根节点
	map<unsigned long, UserListUI::Node*>::iterator iter = m_allVisitorNodeMap.find(id);
	//没有找到
	if (iter == m_allVisitorNodeMap.end())
		return type;

	childNode = iter->second;
	if (childNode == NULL)
		return type;

	//是自己的孩子 
	if (fatherId == m_manager->m_userInfo.UserInfo.uid)
	{
		for (int i = 0; i < 4; i++)
		{
			UserListUI::Node *tempNode = pMySelfeNode->child(i);
			for (int j = 0; i < tempNode->num_children(); i++)
			{
				if (childNode == tempNode->child(j))
				{
					type = (TREENODEENUM)(i + MYSELF_CHILD_1);

					list<unsigned long>::iterator iterList = m_invoteOtherList.begin();
					for (; iterList != m_invoteOtherList.end(); iterList++)
					{
						if (*iterList == id)
						{
							type = MYSELF_CHILD_ACTIVE_1;
							break;
						}


					}

					break;
				}
			}

		}
	}
	else //别人的孩子
	{
		//先找 别人的node根节点
		iter = m_onlineNodeMap.find(fatherId);
		if (iter == m_onlineNodeMap.end())
		{
			iter = m_offlineNodeMap.find(fatherId);
			if (iter == m_offlineNodeMap.end())
				return type;
		}
		fatherNode = iter->second;
		for (int i = 0; i < 3; i++)
		{
			UserListUI::Node *tempNode = fatherNode->child(i);
			for (int j = 0; i < tempNode->num_children(); i++)
			{
				if (childNode == tempNode->child(j))
				{
					type = (TREENODEENUM)(i + OTHER_CHILD_1);
					break;
				}
			}

		}


	}
	return type;
}


void CMainFrame::OnItemClickEvent(unsigned long id,int type)
{
	if (id >= 0)
	{
		if (m_curSelectId != id || type == -1 || (id == 0 && m_curSavedSid.length() > 0) )//切换聊天对象显示 
		{

			ChangeShowUserMsgWnd(id);

			ShowRightOptionFrameView(id,m_curSavedSid);
		}
		m_curSelectId = id;
	}

	//更新 上层的按钮状态
	UpdateTopCenterButtonState(id);

}




void CMainFrame::HostUserOnlineAndOffline(CUserObject* pUser, bool type)
{
	int index = 0;
	if (type == true)
	{

		if (pUser->status == USER_STATUS_ONLINE)  //当前坐席是离线状态 同时过来的状态是上线状态
		{
			//先删除当前的离线坐席 list 再添加上线的坐席状态

			map<unsigned long, UserListUI::Node*>::iterator  iter = m_offlineNodeMap.find(pUser->UserInfo.uid);
			if (iter == m_offlineNodeMap.end())
				return;

			UserListUI::Node* tempNode = iter->second;


			//先删除
			if (tempNode != NULL && tempNode->data()._level >= 0 && tempNode->data()._level <= 5)
				pUserList->RemoveNode(tempNode);

			m_offlineNodeMap.erase(iter);

			if (m_onlineNodeMap.size() > 0)
			{
				iter = m_onlineNodeMap.end();
				iter--;

				UserListUI::Node* mapNode = iter->second;
				index = pUserList->GetNodeIndex(mapNode);
			}
			else
				index = pUserList->GetNodeIndex(pMySelfeNode);

			//再添加
			AddHostUserList(pUserList, pUser, index);
		}

	}
	else
	{
		int index = 0;
		if (pUser->status == USER_STATUS_OFFLINE)  //当前坐席是在线状态 同时过来的状态是离线状态
		{
			//先删除当前的在线 坐席 list   再添加离线的坐席状态

			map<unsigned long, UserListUI::Node*>::iterator  iter = m_onlineNodeMap.find(pUser->UserInfo.uid);
			if (iter == m_onlineNodeMap.end())
				return;


			UserListUI::Node* tempNode = iter->second;
			//先删除
			if (tempNode != NULL && tempNode->data()._level >= 0 && tempNode->data()._level <= 5)
				pUserList->RemoveNode(tempNode);

			m_onlineNodeMap.erase(iter);


			if (m_offlineNodeMap.size() > 0)
			{
				iter = m_offlineNodeMap.end();
				iter--;

				UserListUI::Node* mapNode = iter->second;
				index = pUserList->GetNodeIndex(mapNode);
			}
			else
				index = pUserList->GetNodeIndex(pMySelfeNode);

			//再添加
			AddHostUserList(pUserList, pUser, index);
		}

	}
}

void CMainFrame::VisitorUserOnlineAndOffline(CWebUserObject* pWebUser, bool type)
{
	CDuiString text;
	UserListUI::Node* tempNode = NULL;
	WCHAR name[64] = { 0 };
	ANSIToUnicode(pWebUser->info.name, name);
	char id[64] = { 0 };

	//是否来自微信
	if (pWebUser->m_bIsFrWX)
	{
		text.Format(_T("{x 4}{i user_wx.png 1 0}{x 4}%s"), name);
	}
	else
	{
		text.Format(_T("{x 4}{i user_web.png 1 0}{x 4}%s"), name);
	}

	if (type == true)
	{
		FindVisitorFromOnlineNode(pWebUser);
		if (pWebUser->onlineinfo.talkstatus == TALK_STATUS_NO)
		{
			tempNode = pOnlineNode->child(1);
		}
		else if (pWebUser->onlineinfo.talkstatus == TALK_STATUS_AUTOINVITE)
		{
			tempNode = pOnlineNode->child(0);
		}
		else
			return;


		//加入在线访客的列表
		UserListUI::Node* AddNode = pUserList->AddNode(text, pWebUser->webuserid, pWebUser->info.sid, tempNode);
		pUserList->ExpandNode(tempNode, true);

		m_visitorOnlineNode.insert(pair<string, UserListUI::Node*>(pWebUser->info.sid, AddNode));
	}
	else
	{
		FindVisitorFromOnlineNode(pWebUser);
		UserListUI::Node* addNode = pOnlineNode->child(2);

		//加入在线访客的列表
		UserListUI::Node* AddNode = pUserList->AddNode(text, pWebUser->webuserid, pWebUser->info.sid, addNode);
		pUserList->ExpandNode(addNode, true);

		if (pWebUser->webuserid > 0)
		{
			//sprintf(id, "%ul", pWebUser->webuserid);
			//m_visitorOnlineNode.insert(pair<string, UserListUI::Node*>(id, addNode));

			m_visitorOnlineNode.insert(pair<string, UserListUI::Node*>(pWebUser->info.sid, AddNode));
		}
		else
		{
			m_visitorOnlineNode.insert(pair<string, UserListUI::Node*>(pWebUser->info.sid, AddNode));
		}


	
	}

}


void CMainFrame::FindVisitorFromOnlineNode(CWebUserObject* pWebUser)
{
	UserListUI::Node* tempNode = NULL;

	//先查看是否是实在等待列表 然后查看是否在 会话列表  最后查看是否在 在线列表
	if (pWebUser->webuserid > 0)
	{

		map<unsigned long, UserListUI::Node*>::iterator iterNode = m_waitVizitorMap.find(pWebUser->webuserid);
		if (iterNode == m_waitVizitorMap.end())
		{
			iterNode = m_allVisitorNodeMap.find(pWebUser->webuserid);
			if (iterNode != m_allVisitorNodeMap.end())
			{
				tempNode = iterNode->second;
				m_allVisitorNodeMap.erase(iterNode);
			}
		}
		else
		{
			tempNode = iterNode->second;
			m_waitVizitorMap.erase(iterNode);
		}


		if (tempNode != NULL && tempNode->data()._level>=0)
		{
			pUserList->RemoveNode(tempNode);
		}


	}

	map<string, UserListUI::Node*>::iterator iterSid = m_visitorOnlineNode.find(pWebUser->info.sid);
	if (iterSid != m_visitorOnlineNode.end())
	{
		tempNode = iterSid->second;
		m_visitorOnlineNode.erase(iterSid);

		if (tempNode != NULL && tempNode->data()._level >= 0 && tempNode->data()._level <= 5)
		pUserList->RemoveNode(tempNode);

	}


}


BOOL CMainFrame::CheckItemForOnlineVisitor(UserListUI::Node *curNode)
{
	for (int i = 0; i < 2; i++)
	{
		UserListUI::Node *fatherNode = pOnlineNode->child(i);

		int num = fatherNode->num_children();
		for (int j = 0; j < num; j++)
		{

			UserListUI::Node *node = fatherNode->child(j);
			if (curNode == node)
				return true;

		}

	}



	return false;
}

//接受 
void CMainFrame::AcceptChat()
{
	OnActiveUser(m_curSelectId, m_curSavedSid);

	/*
	if (m_curSelectId > 0)
	{
		map<unsigned long, UserListUI::Node*>::iterator iter = m_waitVizitorMap.find(m_curSelectId);
		if (iter != m_waitVizitorMap.end())
		{
			m_manager->SendTo_AcceptChat(m_curSelectId);
		}
	}
	*/
}

void CMainFrame::RefuseChat()
{
	CUserObject	*pUser = NULL;
	unsigned long userid = 0;
	if (m_curSelectId > 0)
	{

		CWebUserObject *pWebUser = m_manager->GetWebUserObjectByUid(m_curSelectId);
		if (pWebUser != NULL)
		{

			switch (pWebUser->onlineinfo.talkstatus)
			{
			case TALK_STATUS_REQUEST:

				break;

			case TALK_STATUS_TRANSFER:
				m_manager->SendTo_TransferUserResult(pWebUser, false);
				break;

			case TALK_STATUS_INVITE:
				userid = GetInviteUserid(pWebUser->webuserid);
				m_manager->SendTo_InviteUserResult(pWebUser, userid, false);
				break;


			}


			///m_manager->SendToRefuseChat()

		}

	}
}


void CMainFrame::ShowBigImage(string url, MSG_DATA_TYPE msgDataType)
{

	if (pShowImgDlg == NULL)
	{

		pShowImgDlg = new CShowBigImageDlg();
		pShowImgDlg->Create(m_hWnd, _T(""), UI_WNDSTYLE_DIALOG, 0, 0, 0, 0, 0, NULL);
		pShowImgDlg->CenterWindow();
	
		RECT sysRect;
		GetWindowRect(m_hWnd, &sysRect);
	
		int cx = 800;// +x;
		int cy = 600;// +y;
		int x = (sysRect.right - cx) / 2;
		int y = (sysRect.bottom - cy) / 2;


		::SetWindowPos((HWND)pShowImgDlg, NULL, x, y, cx, cy, NULL);
		::ShowWindow((HWND)pShowImgDlg, SW_SHOW);
		pShowImgDlg->isCreate = true;
		pShowImgDlg->ShowBigImage(url, msgDataType);

	}

	else
	{
		pShowImgDlg->ShowWnd(SW_SHOW);

		
		pShowImgDlg->ShowBigImage(url, msgDataType);


	}


}


//0 为不可能 1为可用

void CMainFrame::SetManagerButtonState(int i,int type)
{
	if (type == 0)
	{

		m_pManagerBtn[i].m_pManagerBtn->SetNormalImage(m_pManagerBtn[i].normalImage);
		m_pManagerBtn[i].m_pManagerBtn->SetHotImage(m_pManagerBtn[i].normalImage);
		m_pManagerBtn[i].m_pManagerBtn->SetPushedImage(m_pManagerBtn[i].normalImage);
		m_pManagerBtn[i].m_buttonState = 0;


	}
	else
	{
	

			m_pManagerBtn[i].m_pManagerBtn->SetNormalImage(m_pManagerBtn[i].pushedImage);
			m_pManagerBtn[i].m_pManagerBtn->SetHotImage(m_pManagerBtn[i].hotImage);
			m_pManagerBtn[i].m_pManagerBtn->SetPushedImage(m_pManagerBtn[i].hotImage);
			m_pManagerBtn[i].m_buttonState = 1;
		

	
	}

}


void CMainFrame::CreateSmallTaskIcon(WCHAR *name)
{
	m_frameSmallMenu.DeleteSmallIcon();

	m_frameSmallMenu.Init();
	CDuiString path = GetCurrentPathW();
	path += name;// DEFINE_SMALL_ICON_PATH;pe
	m_frameSmallMenu.CreateSmallIcon(this->m_hWnd, (WCHAR*)path.GetData());
}


void CMainFrame::OnCancel()
{



}


int CMainFrame::CheckIdForInvoteMyselfOrOther(unsigned long id)
{
	int textType = 0;

	list<unsigned long>::iterator  iterList = m_invoteMyselfList.begin();
	for (; iterList != m_invoteMyselfList.end(); iterList++)
	{
		if (*iterList == id)
		{
			textType = 1;
			break;
		}
	}
	iterList = m_invoteOtherList.begin();
	for (; iterList != m_invoteOtherList.end(); iterList++)
	{
		if (*iterList == id)
		{
			textType = 2;
			break;
		}
	}

	return textType;
}

int CMainFrame::ReleaseChatIdForInvoteMyselfOrOther(unsigned long id)
{

	list<unsigned long>::iterator iterList = m_invoteOtherList.begin();
	for (; iterList != m_invoteOtherList.end(); iterList++)
	{
		if (*iterList == id)
		{
			m_invoteOtherList.erase(iterList);
			break;
		}
	}

	iterList = m_invoteMyselfList.begin();
	for (; iterList != m_invoteMyselfList.end(); iterList++)
	{
		if (*iterList == id)
		{
			m_invoteMyselfList.erase(iterList);
			break;
		}
	}

	return 0;
}

void CMainFrame::RecvQuickReply(string quickReply)
{
	unsigned long curitemid = 1000;
	unsigned long id = 0;


	//解析xml
	CMarkupXml   xml((char*)quickReply.c_str());
	m_savedShortAnswer.clear();
    ParseGroup(xml,id,curitemid);

	InitRightTalkList();

}


int CMainFrame::ParseGroup(CMarkupXml &xml, int id, int curitemid)
{
	int type;
	unsigned long groupid, uid, parentid, tb_flag;
	char title[MAX_256_LEN + 1];
	int m, n = 0;
	KEYWORDGROUP_INFO *pKeyWordGroupInfo;


	while (xml.FindChildElem("item"))
	{
		groupid = (unsigned long)atol((char*)xml.GetChildAttrib("id").c_str());
		uid = (unsigned long)atol((char*)xml.GetChildAttrib("uin").c_str());
		parentid = (unsigned long)atol((char*)xml.GetChildAttrib("parent_sort_id").c_str());
		type = atoi(xml.GetChildAttrib("type").c_str());
		strncpy(title, xml.GetChildAttrib("text").c_str(), MAX_256_LEN);
		tb_flag = atoi(xml.GetChildAttrib("tb_flag").c_str());

		pKeyWordGroupInfo = AddKeyWordGroupInfo(groupid, 0, 0, id, parentid, type, title);

		memset(m_answerData.key, 0, sizeof(m_answerData.key));
		m_answerData.m_value.clear();

		xml.IntoElem();
		//m = ParseGroup(xml, id, curitemid);
		//curitemid += m;
		//n += m;
		switch (type)
		{
		case 0:
			m = ParseGroupItem(xml, pKeyWordGroupInfo, "Quickreply", 0, curitemid);
			curitemid += m;
			n += m;
			break;
		case 1:
			if (strlen(pKeyWordGroupInfo->name) <= 0)
				sprintf(pKeyWordGroupInfo->name, "%s", "推送网址");

			m = ParseGroupItem(xml, pKeyWordGroupInfo, "SendURL", 1, curitemid);
			curitemid += m;
			n += m;
			break;
		case 2:
			m = ParseGroupItem(xml, pKeyWordGroupInfo, "SendFile", 2, curitemid);
			curitemid += m;
			n += m;
			break;
		}
		xml.OutOfElem();


		if (groupid > 0)
		{
			strcpy(m_answerData.key, title);
			m_savedShortAnswer.push_back(m_answerData);
		}
		else if (groupid == 0)
		{
			strcpy(m_answerData.key, xml.GetChildAttrib("title").c_str());
			m_pushWebUrl.push_back(m_answerData);
		}
	
	}

	return n;
}

int CMainFrame::ParseGroupItem(CMarkupXml &xml, KEYWORDGROUP_INFO *pKeyWordGroupInfo, char *sKey, int type, int curitemid)
{
	if (pKeyWordGroupInfo == NULL || sKey == NULL)
		return 0;

	unsigned long itemid;
	char title[MAX_256_LEN + 1];
	char memo[MAX_4096_LEN + 1];
	int num = 0;
	int is_shortcut;
	char shotkey[MAX_256_LEN + 1];


	while (xml.FindChildElem(sKey))
	{
		pKeyWordGroupInfo->type = type;

		itemid = (unsigned long)atol(xml.GetChildAttrib("id").c_str());
		if (itemid == 0)
			itemid = curitemid++;

		is_shortcut = (unsigned long)atol(xml.GetChildAttrib("is_shortcut").c_str());

		strncpy(title, xml.GetChildAttrib("title").c_str(), MAX_256_LEN);
		strncpy(memo, xml.GetChildData().c_str(), MAX_4096_LEN);
		strncpy(shotkey, xml.GetChildAttrib("keyboard").c_str(), MAX_256_LEN);
		SHORT_ANSWER_DATA data;
		strcpy(data.title, title);
		strcpy(data.value, memo);

		m_answerData.m_value.push_back(data);

#if 0
		KEYWORD_INFO *pKeyInfo = AddKeyWordInfo(itemid, 0, 0, pKeyWordGroupInfo->userid, pKeyWordGroupInfo->id, type, title, memo);
		//		if( is_shortcut != 0)
		if (strlen(shotkey) > 0)
		{
			//			pKeyInfo->hotkey = is_shortcut;
			pKeyInfo->hotkey = 1;
			strncpy(shotkey, xml.GetChildAttrib("keyboard").c_str(), MAX_256_LEN);
		//	ParseHotKeyString(shotkey, (BOOL&)pKeyInfo->ctrl, (BOOL&)pKeyInfo->alt, (BOOL&)pKeyInfo->shift, pKeyInfo->code);
		}
#endif
		num++;
	}

	return num;
}

KEYWORD_INFO *CMainFrame::AddKeyWordInfo(unsigned long id, int sort, unsigned long compid,unsigned long uid, unsigned long groupid, unsigned char type,char *name, char *memo)
{
	KEYWORD_INFO *pInfo = NULL;


	if (pInfo == NULL)
	{
		pInfo = new KEYWORD_INFO;
		memset(pInfo, '\0', sizeof(KEYWORD_INFO));

		pInfo->id = id;
	}

	pInfo->sort = sort;
	pInfo->compid = compid;
	pInfo->userid = uid;
	pInfo->groupid = groupid;
	pInfo->type = type;

	strcpy(pInfo->name, name);
	strcpy(pInfo->memo, memo);


	return pInfo;
}

KEYWORDGROUP_INFO *CMainFrame::AddKeyWordGroupInfo(unsigned long id, int sort, unsigned long compid, unsigned long uid, unsigned long parentid, unsigned char type, char *name)
{
	KEYWORDGROUP_INFO *pInfo = NULL;

	if (pInfo == NULL)
	{
		pInfo = new KEYWORDGROUP_INFO;
		memset(pInfo, '\0', sizeof(KEYWORDGROUP_INFO));

		pInfo->id = id;
	}

	pInfo->sort = sort;
	pInfo->compid = compid;
	pInfo->userid = uid;
	pInfo->parentid = parentid;
	pInfo->type = type;
	strcpy(pInfo->name, name);

	return pInfo;
}


void CMainFrame::DoRightShortAnswerList(string str)
{
	int key = 0, value = 0;
	WCHAR text[1024] = {0};

	int index = str.find("-");
	if (index == -1)
		return;
	
	string keyText = str.substr(0, index);
	index += 1;
	string valueText = str.substr(index, str.length() - index);


	key = atoi(keyText.c_str());
	value = atoi(valueText.c_str());

	if (key >= m_savedShortAnswer.size() || value >= m_savedShortAnswer[key].m_value.size())
		return;

	string getString = m_savedShortAnswer[key].m_value[value].value;

	ANSIToUnicode(getString.c_str(),text);
	m_pSendEdit->SetText(text);
	m_pSendEdit->SetFocus();


}


void CMainFrame::InsertInviteUserid(unsigned long webUserid, unsigned long id)
{

	map<unsigned long, unsigned long >::iterator iter = m_recvUserObjMap.find(webUserid);
	if (iter != m_recvUserObjMap.end())
	{
		m_recvUserObjMap.erase(iter);
	}

	m_recvUserObjMap.insert(pair<unsigned long, unsigned long >(webUserid, id));

}

unsigned long  CMainFrame::GetInviteUserid(unsigned long webUserid)
{
	unsigned long  id = 0;

	map<unsigned long, unsigned long >::iterator iter = m_recvUserObjMap.find(webUserid);
	if (iter != m_recvUserObjMap.end())
	{
		id = iter->second;
	}

	return id;
}


#endif
