﻿#include "../stdafx.h"
#include "main_frame.h"
#include "login_wnd.h"
#include "global_setting_define.h"
#include "ui_menu.h"
#include "ui_common/common_utility.h"

#include "IImageOle.h"
#include "ole_helper.h"

#include "chat_common\comfunc.h"
#include "utils\code_convert.h"
#include "menu_wnd.h"

#include <WinUser.h>




CMainFrame::CMainFrame(CChatManager* manager) :m_manager(manager)
{

	//初始化
	m_pFontBtn = m_pFaceBtn = m_pScreenBtn = pSendMsgBtn = NULL;
	m_pSendEdit = NULL;

	pOnlineNode = pWaitForStart = pMySelfeNode = NULL;

	m_recordWaitNumber = 0;

	m_facePathUrl = "<IMG alt=\"\" src=\"face.gif\">";

	m_rightRect = {0};

	CCodeConvert f_covet;
	string strTmp = "<html><head>";
	strTmp += STRING_HTML_META;
	strTmp += STRING_HTML_BASE;
	strTmp += STRING_HTML_STYLE;
	strTmp += "</head><body>";
	strTmp += "<div><p><span style=\"font - size:16px; color:#cccccc\">无可显示信息</span></p><div></body>";
	f_covet.Gb2312ToUTF_8(m_defaultUrlInfo, strTmp.c_str(), strTmp.length());
	
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
		}
		else {
			CControlUI* pControl = static_cast<CControlUI*>(m_PaintManager.FindControl(_T("maxBtn")));
			if (pControl) pControl->SetVisible(true);
			pControl = static_cast<CControlUI*>(m_PaintManager.FindControl(_T("restoreBtn")));
			if (pControl) pControl->SetVisible(false);

			MoveAndRestoreMsgWnd(1);
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
    if (uMsg == WM_FACE_CTRL_SEL)
	{

		OnFaceCtrlSel(uMsg,wParam,lParam);
	}



	if (uMsg == WM_MOUSEMOVE)
		OnMouseMove(uMsg, wParam, lParam);

	if (uMsg == WM_RBUTTONDOWN)
	{
//		LRESULT lRes = __super::HandleMessage(uMsg, wParam, lParam);
//		OnRButtonDown(uMsg, wParam, lParam);
	//	return lRes;
	}

//	if (uMsg == WM_LBUTTONDBLCLK)
//		OnLButtonDblClk(uMsg, wParam, lParam);

//	if ((m_pSendEdit != NULL) && m_pSendEdit->IsFocused()
//		&& (uMsg == WM_KEYDOWN) && (wParam == 'V') && (lParam & VK_CONTROL))	// 发送消息框的Ctrl+V消息
//	{
//		m_pSendEdit->PasteSpecial(CF_TEXT);
//		return TRUE;
//	}

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

	if (uMsg == ON_AFTER_CREATED)
	{
		string msg = *(string*)wParam;

		if (Handler_ListMsg == msg)
		    m_pListMsgHandler.isCreated = true;

		if (Handler_VisitorRelated == msg)
		    m_pVisitorRelatedHandler.isCreated = true;
	}
	else if (uMsg == ON_AFTER_LOAD)
	{
		string msg = *(string*)wParam;

		if (Handler_ListMsg == msg)
		   m_pListMsgHandler.isLoaded = true;

		if (Handler_VisitorRelated == msg)
		    m_pVisitorRelatedHandler.isLoaded = true;
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
	}

	else if (_tcsicmp(msg.sType, DUI_MSGTYPE_TIMER) == 0)
	{
		return OnTimer(msg);
	}
	else if (_tcsicmp(msg.sType, DUI_MSGTYPE_SELECTCHANGED) == 0)
	{
		OnSelectChanged(msg);
	}

	else if (_tcsicmp(msg.sType, DUI_MSGTYPE_ITEMACTIVATE) == 0)
	{
		OnItemActive(msg);

	}

	else if (_tcsicmp(msg.sType, DUI_MSGTYPE_MENU) == 0)
	{
		OnItemRbClick(msg);
	}

	else if (_tcsicmp(msg.sType, DUI_MSGTYPE_ITEMCLICK) == 0)
	{

		OnItemClick(msg);
	}






}



void CMainFrame::OnTimer(TNotifyUI& msg)
{

	if (msg.pSender == pUserList)
	{
		AddOnlineVisitor(pUserList, NULL, -1);
		m_PaintManager.KillTimer(pUserList);
		list<CUserObject* >::iterator  iter = m_upUser.begin();
		for (; iter != m_upUser.end(); iter++)
		{

			//AddUserList(pUserList,*iter);

			RecvOnline(*iter);

		}
		//UserListUI::Node* tempNode = iter->second;


		m_upUser.clear();

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

	CControlUI *leftLayout = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(_T("VerticalLayoutUI_LeftFrame")));
	CControlUI *lineLayout = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(_T("VerticalLayoutUI_LeftLine1")));
	CControlUI *centerLayout = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(_T("VerticalLayoutUI_CenterFrame")));
	CControlUI *rightLayout = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(_T("VerticalLayoutUI_RightFrame")));
	CControlUI *ShowMsgWnd = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(_T("HorizontalLayoutUI_ShowMsg")));

	CControlUI *ShowRightWebWnd = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(_T("right_tab")));


	//m_pVisitorRelatedHandler.handler->CreateBrowser(this->m_hWnd, rect, "www.baidu.com", Handler_VisitorRelated);

	if (type == 0) //max
	{
		leftWidth = leftLayout->GetWidth();
		leftWidth += lineLayout->GetWidth();
		GetWindowRect(this->m_hWnd, &sysRect);
		int centerWidth = (sysRect.right - leftWidth) / 2;
		int rightWidth = sysRect.right - leftWidth - centerWidth;

		m_centerChatInfo.centerFrameWitdh = centerLayout->GetWidth();
		m_centerChatInfo.showMsgWidth = ShowMsgWnd->GetWidth();

		formatString.Format(_T("%d"), centerWidth);
		centerLayout->SetAttribute(_T("width"), formatString);
		formatString.Format(_T("%d"), rightWidth);
		rightLayout->SetAttribute(_T("width"), formatString);

		rc = ShowMsgWnd->GetPos();
		rc.right = rc.left + centerWidth - 2;
		m_pListMsgHandler.handler->MoveBrowser(rc);

		if (m_rightRect.left == 0)
		    m_rightRect = ShowRightWebWnd->GetPos();

		rect.left = rc.right + 4;
		rect.right = sysRect.right - 4;
		rect.top += m_rightRect.top;
		rect.bottom = sysRect.bottom - 4;
		m_pVisitorRelatedHandler.handler->MoveBrowser(rect);

	}
	else
	{
		formatString.Format(_T("%d"), m_centerChatInfo.centerFrameWitdh);
		centerLayout->SetAttribute(_T("width"), formatString);

		rc = ShowMsgWnd->GetPos();
		rc.right = rc.left + m_centerChatInfo.showMsgWidth;
		m_pListMsgHandler.handler->MoveBrowser(rc);

		//rc = ShowRightWebWnd->GetPos();
		m_pVisitorRelatedHandler.handler->MoveBrowser(m_rightRect);
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
	m_pVisitorRelatedHandler.handler = NULL;
	m_pVisitorRelatedHandler.handleName = Handler_VisitorRelated;
	m_pVisitorRelatedHandler.isLoaded = false;
	m_pVisitorRelatedHandler.isCreated = false;

	//显示聊天内容的libcef窗口
	m_pListMsgHandler.handler = new ClientHandler();
	m_pListMsgHandler.handler->m_isDisplayRefresh = false;

	m_pVisitorRelatedHandler.handler = new ClientHandler();
	m_pVisitorRelatedHandler.handler->m_isDisplayRefresh = false;

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
	
	}

}


void CMainFrame::SetHandler()
{

	m_manager->SetHandlerMsgs(this);

}

void CMainFrame::OnPrepare(TNotifyUI& msg)
{
	CDuiString nameString = _T("");
	CDuiString typeString[4] = { _T("对话中"), _T("转接中"), _T("邀请中"), _T("内部对话") };

	//cef窗口
	InitLibcef();



	//聊天框中间栏按钮
	m_pFontBtn = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btnFont")));
	m_pFaceBtn = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btnFace")));
	m_pScreenBtn = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("screenshotsbtn")));
	pSendMsgBtn = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("sendMsgBtn")));


	//上层管理按钮 设置初始状态
	for (int i = 0; i < MID_MANAGER_BUTTON_NUM; i++)
	{
		nameString.Format(_T("managerbutton_%d"),i+1);
		m_pManagerBtn[i].m_pManagerBtn = NULL;
		m_pManagerBtn[i].m_pManagerBtn = static_cast<CButtonUI*>(m_PaintManager.FindControl(nameString));

		CDuiString str = m_pManagerBtn[i].m_pManagerBtn->GetNormalImage();
		//m_pManagerBtn[i].normalImage =  m_pManagerBtn[i].m_pManagerBtn->GetNormalImage();
		//m_pManagerBtn[i].hotImage = m_pManagerBtn[i].m_pManagerBtn->GetHotImage();
		//m_pManagerBtn[i].pushedImage = m_pManagerBtn[i].m_pManagerBtn->GetPushedImage();

		StrCpyW(m_pManagerBtn[i].hotImage, m_pManagerBtn[i].m_pManagerBtn->GetHotImage());
		StrCpyW(m_pManagerBtn[i].pushedImage, m_pManagerBtn[i].m_pManagerBtn->GetPushedImage());

		//筛选访客按钮
		if (i == 5  || i >= 8 )
		{
			m_pManagerBtn[i].m_pManagerBtn->SetNormalImage(m_pManagerBtn[i].pushedImage);
		}
		else
		{
			m_pManagerBtn[i].m_pManagerBtn->SetHotImage(str);
			m_pManagerBtn[i].m_pManagerBtn->SetPushedImage(str);
		}
	}

	//右下角小图标
	m_frameSmallMenu.Init();
	m_frameSmallMenu.CreateSmallIcon(this->m_hWnd, DEFINE_SMALL_ICON_PATH);

	//表情包初始化
	wstring strPath = ZYM::CPath::GetCurDir() + _T("../bin/") _T("SkinRes\\Face\\FaceConfig.xml");
	m_faceList.LoadConfigFile(strPath.c_str());

	//左侧用户列表显示
    pUserList = static_cast<UserListUI*>(m_PaintManager.FindControl(_T("userlist")));
	pWaitForAccept = NULL;
	pWaitForStart = pUserList->AddNode(_T("{x 4}{i gameicons.png 18 0}{x 4}等待开始"),0);
	pWaitForAccept = pUserList->AddNode(_T("{x 4}{i gameicons.png 18 16}{x 4}等待应答"),0,pWaitForStart);


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

	if (msg.pSender->GetName() == DEF_CLOSE_WND_BUTTON)
	{
		OnCloseBtn(msg);
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

	else  if (msg.pSender->GetName() == L"acceptbutton")
	{

		///CLoginWnd* pLoginFrame = new CLoginWnd();
		//pLoginFrame->Create(NULL, _T(""), UI_WNDSTYLE_DIALOG, 0, 0, 0, 0, 0, NULL);
		//pLoginFrame->CenterWindow();
		//int result = pLoginFrame->ShowModal();

		//return;

		//CMenuWnd* pMenu = new CMenuWnd(m_hWnd);
		//CPoint point = msg.ptMouse;

		//CMenuWnd* pMenu = new CMenuWnd(m_hWnd);
		//CPoint point = msg.ptMouse;
		//ClientToScreen(m_hWnd, &point);
		//pMenu->Init(NULL, _T(""), _T("xml"), point);



		//pMenu->Create(NULL, _T(""), WS_POPUP, 0, 0, 0, 0, 0, NULL);
		//pMenu->CenterWindow();
		//pMenu->ShowModal();

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
		int index = pTabControl->GetCurSel() + 1;

		//这里因为右边屏幕 有7个option选项 
		for (int i = 0; i < 7; i++)
		{
			swprintf_s(OptionBtnName, _T("option_button_%d"), i + 1);

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


void CMainFrame::OnItemClick(TNotifyUI &msg)
{
	UserListUI* pUserList = static_cast<UserListUI*>(m_PaintManager.FindControl(_T("userlist")));
	if (pUserList->GetItemIndex(msg.pSender) != -1)
	{
		if (_tcscmp(msg.pSender->GetClass(), _T("ListLabelElementUI")) == 0)
		{
			UserListUI::Node* node = (UserListUI::Node*)msg.pSender->GetTag();
			CDuiString str = node->data()._text;
			unsigned long id = node->data()._uid;

			if (id > 0)
			{
				//
				if (m_checkId != id)//切换聊天对象显示 
				{
					ChangeShowUserMsgWnd(id);
				}

				m_checkId = id;
				map<unsigned long, UserListUI::Node*>::iterator iter = m_waitVizitorMap.find(id);
				if (iter != m_waitVizitorMap.end())
				{
					m_pManagerBtn[0].m_pManagerBtn->SetNormalImage(m_pManagerBtn[0].pushedImage);
					m_pManagerBtn[0].m_pManagerBtn->SetHotImage(m_pManagerBtn[0].hotImage);
					m_pManagerBtn[0].m_pManagerBtn->SetPushedImage(m_pManagerBtn[0].pushedImage);

					m_pManagerBtn[3].m_pManagerBtn->SetNormalImage(m_pManagerBtn[3].pushedImage);
					m_pManagerBtn[3].m_pManagerBtn->SetHotImage(m_pManagerBtn[3].hotImage);
					m_pManagerBtn[3].m_pManagerBtn->SetPushedImage(m_pManagerBtn[3].pushedImage);

					m_pManagerBtn[4].m_pManagerBtn->SetNormalImage(m_pManagerBtn[4].pushedImage);
					m_pManagerBtn[4].m_pManagerBtn->SetHotImage(m_pManagerBtn[4].hotImage);
					m_pManagerBtn[4].m_pManagerBtn->SetPushedImage(m_pManagerBtn[4].pushedImage);
				}	




			}
		}
	}
}

void CMainFrame::OnItemActive(TNotifyUI &msg)
{
	UserListUI* pUserList = static_cast<UserListUI*>(m_PaintManager.FindControl(_T("userlist")));
	if (pUserList->GetItemIndex(msg.pSender) != -1)
	{
		if (_tcscmp(msg.pSender->GetClass(), _T("ListLabelElementUI")) == 0) 
		{
			UserListUI::Node* node = (UserListUI::Node*)msg.pSender->GetTag();
			pUserList->ExpandNode(node, !node->data()._expand);

			CDuiString str = node->data()._text;
			unsigned long uid = node->data()._uid;

			if (uid > 0)
			{
				map<unsigned long, UserListUI::Node*>::iterator iter = m_waitVizitorMap.find(uid);
				if (iter != m_waitVizitorMap.end())
				{
					OnSendToAcceptChat(uid);
				}
			}

			//list<CDuiString>::iterator iter;
			//for (iter = m_waitVizitorList.begin(); iter != m_waitVizitorList.end(); iter++)
			//{
				//如果 点击的list是 等待列表里面的 做请求接受操作


				/*
				if (*iter == str)
				{
					pUserList->RemoveNode(node);

					UserListUI::Node* addNode = pMySelfeNode->child(0);
					pUserList->AddNode(str, uid,addNode);
					pUserList->ExpandNode(addNode, true);

					m_waitVizitorList.erase(iter);
					break;
				}
				*/
			//}

#if 0
			if (node->data()._level == 3)
			{
				COptionUI* pControl = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("roomswitch")));
				if (pControl) {
					CHorizontalLayoutUI* pH = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(_T("roomswitchpanel")));
					if (pH) pH->SetVisible(true);
					pControl->SetText(node->parent()->parent()->data()._text);
					pControl->Activate();
				}
			}
#endif
		}
	}
}


void CMainFrame::OnItemRbClick(TNotifyUI &msg)
{
	if (msg.pSender->GetName() == L"userlist")
	{
		CMenuWnd2* pMenu = new CMenuWnd2();
		if (pMenu == NULL) { return; }
		POINT pt = { msg.ptMouse.x, msg.ptMouse.y };
		//::ClientToScreen(*this, &pt);
		pMenu->SetXMLPath(L"menu\\menu_right.xml");
		pMenu->Init(msg.pSender, pt);



		//CMenuWnd* pMenu = new CMenuWnd(this->GetHWND());
		//CPoint cpoint = msg.ptMouse;
		//pMenu->SetPath(L"menutext.xml");

		//pMenu->Init(NULL, _T(""), _T("xml"), cpoint);


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

			RECT rcBtn = m_pFaceBtn->GetPos();
			::ClientToScreen(this->m_hWnd, (LPPOINT)&rcBtn);

			int cx = 432;
		    int cy = 236;
			int x = rcBtn.left - cx / 2;
			int y = rcBtn.top - cy;

			::SetWindowPos((HWND)m_faceSelDlg, NULL, x, y, cx, cy, NULL);
			::ShowWindow((HWND)m_faceSelDlg, SW_SHOW);

			m_faceSelDlg.Init();

		}
	}
	else
	{
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

void CMainFrame::OnBtnSendMessage(TNotifyUI& msg)
{
	char getInput[MAX_1024_LEN] = {0};
	string msgId = m_manager->GetMsgId();
	string sendMsgData;
	int sendMsgType = 0;

	//如果当前选择的用户为 空 则不需要发送
	//if (m_checkId <= 0)
		//return;

	ITextServices * pTextServices = m_pSendEdit->GetTextServices();
	tstring strText;
	RichEdit_GetText(pTextServices, strText);
	pTextServices->Release();

	if (strText.size() <= 0)
		return;
	m_pSendEdit->SetText(_T(""));
	m_pSendEdit->SetFocus();


	UnicodeToANSI(strText.c_str(), getInput);
	sendMsgData = getInput;


	//sendMsgData = "<IMG alt=\"\" src=\"http://sysimages.tq.cn/clientimages/face_v1.0/images/0.gif\">";

	//消息测试 暂时先只发给用户  坐席后续加上
	m_manager->SendTo_Msg(m_checkId, USER_TYPE_WX, msgId, MSG_DATA_TYPE_TEXT, (char*)sendMsgData.c_str());

	//理论上只有发送消息成功 才在聊天界面显示  
	//暂时先添加在这里  后面再移到收到消息成功 的回调哪里
	//显示 发送的消息
	
	//sendMsgData

	//int faceId = atoi(getInput);
	//m_faceSelDlg.GetFileNameById(faceId, strText);


	ReplaceFaceId(sendMsgData);
	ShowMySelfSendMsg(sendMsgData);
}


//请求坐席列表
void CMainFrame::SendMsgToGetList()
{



	//先请求自己的信息 
	m_mySelfInfo = m_manager->GetMySelfUserInfo();


	AddMyselfToList(pUserList, m_mySelfInfo);


	//AddOnlineVisitor(pUserList,NULL,-1);
	//获取坐席列表
	m_manager->SendTo_GetShareList();


	m_hMainWnd = m_PaintManager.GetPaintWindow();


	m_PaintManager.SetTimer(pUserList, WM_ADD_ONLINE_DATA_TIMER_ID, DELAY_ADD_ONLINE_DATA_TIME);

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
	pAutoAccept = pUserList->AddNode(nameString,0, pOnlineNode);

	nameString.Format(_T("{x 4}{i gameicons.png 18 10}{x 4}访问中"));
	pVisiting = pUserList->AddNode(nameString,0, pOnlineNode);


	nameString.Format(_T("{x 4}{i gameicons.png 18 10}{x 4}已结束"));
	pOver = pUserList->AddNode(nameString,0, pOnlineNode);

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

	if (user->status == STATUS_OFFLINE) //离线
	{
		onlineString = _T("(离线)");
	}
	else  if (user->status == STATUS_ONLINE)                 //在线
	{
		onlineString = _T("(在线)");
	}

	strTemp = AnsiToUnicode(user->UserInfo.nickname);

	nameString.Format(_T("{x 4}{i user_client.png 1 0}{x 4} %s %s"), strTemp.GetData(), onlineString);

	//第一个主节点 显示 名称 在线状态  //pos  需要插入的位置
	pUserNameNode = ptr->AddNode(nameString, user->UserInfo.uid, pos);

	taklString.Format(_T("{x 4}{i gameicons.png 18 10}{x 4}对话中"));
	pUserTalkNode = pUserList->AddNode(taklString,0, pUserNameNode);

	changeString.Format(_T("{x 4}{i gameicons.png 18 10}{x 4}转接中"));
	pUserChangeNode = pUserList->AddNode(changeString,0, pUserNameNode);

	acceptString.Format(_T("{x 4}{i gameicons.png 18 10}{x 4}邀请中"));
	pUserAcceptNode = pUserList->AddNode(acceptString, 0,pUserNameNode);

	if (user->status == STATUS_OFFLINE) //离线
	{
		m_offlineNodeMap.insert(pair<unsigned long, UserListUI::Node*>(user->UserInfo.uid, pUserNameNode));

		pUserList->ExpandNode(pUserNameNode, false);
	}
	else  if (user->status == STATUS_ONLINE)     //在线
	{
		m_onlineNodeMap.insert(pair<unsigned long, UserListUI::Node*>(user->UserInfo.uid, pUserNameNode));
		pUserList->ExpandNode(pUserNameNode, true);
	}


	
		

}



void CMainFrame::AddMyselfToList(UserListUI * ptr, CUserObject *user)
{
	CDuiString nameString, taklString, changeString, acceptString, inTalkString;
	CDuiString onlineString;
	CDuiString strTemp;

	UserListUI::Node* pUserNameNode = NULL;
	UserListUI::Node* pUserTalkNode = NULL;
	UserListUI::Node* pUserChangeNode = NULL;
	UserListUI::Node* pUserAcceptNode = NULL;
	UserListUI::Node* pUserInTalkNode = NULL;

	if (user->status == STATUS_OFFLINE)  //离线
	{
		onlineString = OFFLINE_TEXT;
	}
	else  if (user->status == STATUS_ONLINE)                   //在线
	{
		onlineString = ONLINE_TEXT;
	}

	strTemp = AnsiToUnicode(user->UserInfo.nickname);

	//nameString.Format(_T("{x 4}{i gameicons.png 18 0}{x 4} %s %s"), strTemp.GetData(), onlineString);

	nameString.Format(_T("{x 4}{i user_client.png 1 0}{x 4} %s %s"), strTemp.GetData(), onlineString);



	

	//第一个主节点 显示 名称 在线状态
	pUserNameNode = ptr->AddNode(nameString,user->UserInfo.uid);

	taklString.Format(_T("{x 4}{i gameicons.png 18 10}{x 4}对话中"));
	pUserTalkNode = pUserList->AddNode(taklString,0, pUserNameNode);

	changeString.Format(_T("{x 4}{i gameicons.png 18 10}{x 4}转接中"));
	pUserChangeNode = pUserList->AddNode(changeString,0, pUserNameNode);


	acceptString.Format(_T("{x 4}{i gameicons.png 18 10}{x 4}邀请中"));
	pUserAcceptNode = pUserList->AddNode(acceptString,0, pUserNameNode);

	inTalkString.Format(_T("{x 4}{i gameicons.png 18 10}{x 4}内部对话"));
	pUserInTalkNode = pUserList->AddNode(inTalkString,0, pUserNameNode);


	pMySelfeNode = pUserNameNode;
	pUserList->ExpandNode(pMySelfeNode, true);

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

	if (user->status == STATUS_OFFLINE)  //离线
	{
		onlineString = _T("(离线)");

		strTemp = AnsiToUnicode(user->UserInfo.nickname);
		//nameString.Format(_T("{x 4}{i gameicons.png 18 0}{x 4} %s %s"), strTemp.GetData(), onlineString);
		nameString.Format(_T("{x 4}{i user_client.png 1 0} %s %s"), strTemp.GetData(), onlineString);

		//第一个主节点 显示 名称 在线状态
		pUserNameNode = ptr->AddNode(nameString, user->UserInfo.uid);
	}
	else  if (user->status == STATUS_ONLINE)                   //在线
	{
		//onlineString = _T("(在线)");
		onlineString = _T("(离线)");

		strTemp = AnsiToUnicode(user->UserInfo.nickname);
		//nameString.Format(_T("{x 4}{i gameicons.png 18 0}{x 4} %s %s"), strTemp.GetData(), onlineString);
		nameString.Format(_T("{x 4}{i user_client.png 1 0}{x 4} %s %s"), strTemp.GetData(), onlineString);

		pUserNameNode = ptr->AddNode(nameString, user->UserInfo.uid);
		
		m_upUser.push_back(user);

		/*
		if (m_onlineNodeMap.size() > 0)
		{
			map<unsigned int, UserListUI::Node*>::iterator iter = m_onlineNodeMap.end();
			UserListUI::Node* mapNode = iter->second;
			index = pUserList->GetNodeIndex(mapNode);
		}
		else
		{
			index = pUserList->GetNodeIndex(pMySelfeNode);
		}
		pUserNameNode = ptr->AddNode(nameString,index);
		*/
	

	}


	taklString.Format(_T("{x 4}{i gameicons.png 18 10}{x 4}对话中"));
	pUserTalkNode = pUserList->AddNode(taklString, user->UserInfo.uid, pUserNameNode);

	changeString.Format(_T("{x 4}{i gameicons.png 18 10}{x 4}转接中"));
	pUserChangeNode = pUserList->AddNode(changeString, user->UserInfo.uid, pUserNameNode);


	acceptString.Format(_T("{x 4}{i gameicons.png 18 10}{x 4}邀请中"));
	pUserAcceptNode = pUserList->AddNode(acceptString, user->UserInfo.uid, pUserNameNode);


	if ( 1 /* user->status == STATUS_OFFLINE */)    //离线
	{
		m_offlineNodeMap.insert(pair<unsigned long, UserListUI::Node*>(user->UserInfo.uid, pUserNameNode));

	}
	else  if (user->status == STATUS_ONLINE) //在线
	{
		m_onlineNodeMap.insert(pair<unsigned long, UserListUI::Node*>(user->UserInfo.uid, pUserNameNode));
		
	}

	pUserList->ExpandNode(pUserNameNode, false);

}

void CMainFrame::OnSendToAcceptChat(unsigned long webUserid)
{
	m_manager->SendTo_AcceptChat(webUserid);
}
void CMainFrame::OnSendToReleaseChat(unsigned long webUserid)
{
	m_manager->SendTo_ReleaseChat(webUserid);
}
void CMainFrame::OnSendToCloseChat(unsigned long webUserid)
{
	m_manager->SendTo_CloseChat(webUserid, CHATCLOSE_USER);
}


/*****回调函数**************************************************************************************/

//回调过来的 坐席信息
void CMainFrame::RecvUserInfo(CUserObject* pWebUser)
{

	//m_PaintManager.KillTimer(pUserList);
	AddHostUserList(pUserList, pWebUser);
	//m_PaintManager.SetTimer(pUserList, WM_ADD_ONLINE_DATA_TIMER_ID, DELAY_ADD_ONLINE_DATA_TIME);



}


// 收到更新用户的在线状态
void CMainFrame::RecvUserStatus(CUserObject* pUser)
{




}

// 坐席上线消息
void CMainFrame::RecvOnline(CUserObject* pUser)
{
	int index = 0;
	if (pUser->status == STATUS_ONLINE)  //当前坐席是离线状态 同时过来的状态是上线状态
	{
		//先删除当前的离线坐席 list 再添加上线的坐席状态

		map<unsigned long, UserListUI::Node*>::iterator  iter = m_offlineNodeMap.find(pUser->UserInfo.uid);
		if (iter == m_offlineNodeMap.end())
			return;

		UserListUI::Node* tempNode = iter->second;

		//先删除
		pUserList->RemoveNode(tempNode);
		m_offlineNodeMap.erase(iter);

		//index += pUserList->GetNodeIndex(pWaitForStart);
		if (m_onlineNodeMap.size() > 0)
		{
			iter = m_onlineNodeMap.end();
			UserListUI::Node* mapNode = iter->second;
			index = pUserList->GetNodeIndex(mapNode);
		}
		else
			index = pUserList->GetNodeIndex(pMySelfeNode);

		//再添加
		AddHostUserList(pUserList, pUser, index);

	}



}

// 坐席下线消息
void CMainFrame::RecvOffline(CUserObject* pUser)
{
	int index = 0;
	if (pUser->status == STATUS_OFFLINE)  //当前坐席是在线状态 同时过来的状态是离线状态
	{
		//先删除当前的在线 坐席 list   再添加离线的坐席状态

		map<unsigned long, UserListUI::Node*>::iterator  iter = m_onlineNodeMap.find(pUser->UserInfo.uid);
		if (iter == m_onlineNodeMap.end())
			return;

		UserListUI::Node* tempNode = iter->second;

		//先删除
		pUserList->RemoveNode(tempNode);
		m_onlineNodeMap.erase(iter);


		//index = pUserList->GetNodeIndex(pMySelfeNode);// +pUserList->GetNodeIndex(pWaitForStart);

		if (m_onlineNodeMap.size() > 0)
		{
			iter = m_onlineNodeMap.end();

			UserListUI::Node* mapNode = iter->second;
			index = pUserList->GetNodeIndex(mapNode);
		}
		else
			index = pUserList->GetNodeIndex(pMySelfeNode);


		//再添加
		AddHostUserList(pUserList, pUser, index);
	}


}



void CMainFrame::RecvChatInfo(CWebUserObject* pWebUser)
{
	CDuiString text;
	WCHAR name[64] = { 0 };
	ANSIToUnicode(pWebUser->info.name, name);

	//先判断类型
	//为接收的用户 在等待列表
	if (pWebUser->onlineinfo.talkstatus = TALKSTATUS_REQUEST)
	{
		//添加等待列表

		//是否来自微信
		if (pWebUser->m_bIsFrWX)
		{
			text.Format(_T("{x 4}{i user_wx.png 1 0}{x 4}%s"), name);
		}
		else
		{
			text.Format(_T("{x 4}{i user_web.png 1 0}{x 4}%s"), name);
		}


		UserListUI::Node* tempNode = pUserList->AddNode(text, pWebUser->webuserid, pWaitForAccept);
		pUserList->ExpandNode(pWaitForAccept, true);
		//m_waitVizitorList.push_back(text);

		m_waitVizitorMap.insert(pair<unsigned long, UserListUI::Node*>(pWebUser->webuserid, tempNode));

		//m_recordWaitNumber++;
		//m_waitVizitorList.insert(pair<unsigned int, UserListUI::Node*>(m_recordWaitNumber, text));


	}
	//已接收的在会话列表
	else if (pWebUser->onlineinfo.talkstatus = TALKSTATUS_TALK)
	{

		map<unsigned long, UserListUI::Node*>::iterator  iter = m_onlineNodeMap.find(pWebUser->info.uid);
		UserListUI::Node* tempNode = iter->second;
		UserListUI::Node* child = NULL;
		int num = tempNode->num_children();

		for (int i = 0; i < num; i++)
		{
			child = tempNode->child(i);
		}

		pUserList->AddNode(text, pWebUser->webuserid, child);
		pUserList->ExpandNode(tempNode, true);
	}

}


//请求接受的回调
void CMainFrame::RecvAcceptChat(CUserObject* pUser, CWebUserObject* pWebUser)
{
	UserListUI::Node *tempNode = NULL;
	CDuiString text;
	unsigned long uid = 0;
	UserListUI::Node* addNode = NULL;

	map<unsigned long, UserListUI::Node*>::iterator iter = m_waitVizitorMap.find(pWebUser->webuserid);
	if (iter != m_waitVizitorMap.end())
	{
		tempNode = iter->second;
		text = tempNode->data()._text;
		uid = tempNode->data()._uid;
		//当前选择 的用户就是 激活的用户
		m_checkId = uid;
	}
	else
		return;

	//需要从等待列表删除 这个用户
	pUserList->RemoveNode(tempNode);
	//如果回调返回的user uid和自己的相同 则加到自己回话底下
 	if (pUser->UserInfo.uid == m_mySelfInfo->UserInfo.uid)
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
	UserListUI::Node* currentNode = pUserList->AddNode(text, uid, addNode);
	pUserList->ExpandNode(addNode, true);
	//插入 用户map 同时删除 等等map
	m_allVisitorNodeMap.insert(pair<unsigned long, UserListUI::Node*>(uid, currentNode));
	m_waitVizitorMap.erase(iter);


	//显示聊天界面内容
	ShowClearMsg();

	char str[MAX_1024_LEN] = {0};
	sprintf(str, "%s接受了%s邀请的对话", pUser->UserInfo.nickname, pWebUser->info.name);

	//string stime = GetTimeByMDAndHMS
	AddToMsgList(pWebUser, str, "");
	

#if 0
	if (pWebUser->m_strMsgs.empty())
	{
		string msgid = m_manager->GetMsgId();
		int len = strlen(pWebUser->info.name);
		if (len > 0)
		{
	
			CCodeConvert f_covet;
			string msg = pUser->UserInfo.nickname;
			msg += "的消息记录";
			string name;
			f_covet.Gb2312ToUTF_8(name, msg.c_str(), msg.length());
			ONE_MSG_INFO ongMsg;
			ongMsg.msgId = msgid;
			char strJsCode[MAX_256_LEN];
			sprintf(strJsCode,"AppendMsgToHistory('%d','%d','%s','%s','%s','%s','%s','%s');",
				/*系统提示消息 3 MSG_TYPE_SYS */3, MSG_DATA_TYPE_TEXT, "", "", name.c_str(), "0", "unused", msgid);

			m_pListMsgHandler.handler->GetBrowser()->GetMainFrame()->ExecuteJavaScript(strJsCode, "", 0);
			
		}

		
	}
#endif

}

void CMainFrame::RecvCloseChat(CWebUserObject* pWebUser)
{
	UserListUI::Node *tempNode = NULL;
	int type = 0;
	unsigned long uid = 0;
	map<unsigned long, UserListUI::Node*>::iterator iter = m_waitVizitorMap.find(pWebUser->webuserid);

	//没有找到
	if (iter == m_waitVizitorMap.end())
	{
		iter = m_allVisitorNodeMap.find(pWebUser->webuserid);

		if (iter == m_allVisitorNodeMap.end())
			return;

		type = 1;
	}
	tempNode = iter->second;
	pUserList->RemoveNode(tempNode);

	if (type == 0)
		m_waitVizitorMap.erase(iter);
	else
		m_allVisitorNodeMap.erase(iter);

}

void CMainFrame::RecvReleaseChat(CWebUserObject* pWebUser)
{
	UserListUI::Node *tempNode = NULL;

	//map<unsigned long, UserListUI::Node*>::iterator iter = m_waitVizitorMap.find(pWebUser->webuserid);

	map<unsigned long, UserListUI::Node*>::iterator iter = m_allVisitorNodeMap.find(pWebUser->webuserid);
	//没有找到
	if (iter == m_allVisitorNodeMap.end())
	{
		return;
	}
	tempNode = iter->second;
	CDuiString text = tempNode->data()._text;


	//从坐席列表底下删除 然后加入等待列表
	pUserList->RemoveNode(tempNode);

	CWebUserObject *webuser = pWebUser;
	webuser->onlineinfo.talkstatus = TALKSTATUS_REQUEST;
	RecvChatInfo(pWebUser);

	m_allVisitorNodeMap.erase(iter);

}


void CMainFrame::RecvMsg(IBaseObject* pObj, MSG_FROM_TYPE msgFrom, string msgId, MSG_TYPE msgType, MSG_DATA_TYPE msgDataType, string msgContent,
	string msgTime, CUserObject* pAssistUser, WxMsgBase* msgContentWx, string msgExt)
{
	CWebUserObject    *pWebUserObj = NULL;
	CUserObject       *pUserObj = NULL;
	string            headPath = "";
	unsigned long     userId = 0;
	CCodeConvert      f_covet;

	char strJsCode[MAX_1024_LEN] = {0};
	string  name,msg;


	if (pObj == NULL)
		return;
	if (msgContent.length() == 0)
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "插入空的聊天记录");
		return;
	}

	int urlPos = msgContent.find("用户头像地址:");
	int urlPos1 = msgContent.find("user_headimgurl:");
	int urlPos2 = msgContent.find(">立即评价</a>");
	if (urlPos > -1 || urlPos1 > -1 || urlPos2 > -1)
	{
		return;
	}
	
	if (msgFrom == MSG_FROM_CLIENT)
	{
	}
	else if (msgFrom == MSG_FROM_WEBUSER)   //微信或者web用户
	{

		pWebUserObj = (CWebUserObject *)pObj;

		userId = pWebUserObj->webuserid;
		string strName = pWebUserObj->info.name;
		StringReplace(strName, "\\", "\\\\");
		StringReplace(strName,  "'", "&#039;");
		StringReplace(strName, "\r\n", "<br>");
		f_covet.Gb2312ToUTF_8(name, strName.c_str(), strName.length());

		StringReplace(msgContent, "\\", "\\\\");
		StringReplace(msgContent, "'", "&#039;");
		StringReplace(msgContent, "\r\n", "<br>");

		//这里需要把收到的内容做一下 还原
		ReplaceFaceId(msgContent);
		f_covet.Gb2312ToUTF_8(msg, msgContent.c_str(), msgContent.length());

		// 微信用户发来的
		if (pWebUserObj->m_bIsFrWX)
		{
			if (pWebUserObj->m_pWxUserInfo != NULL && !pWebUserObj->m_pWxUserInfo->headimgurl.empty())
			{
				headPath = pWebUserObj->m_pWxUserInfo->headimgurl;
			}
			else
			{
				// 当没有头像时，说明没有收到userinfo，主动去获取，包括token也去获取一次
				//m_pFrame->GetWxUserInfoAndToken(pWebUser);
			}
		}
	}

	//如果收到的消息不是 当前所选用户的id的，暂时屏蔽，后面需要记录起来，等到选择到后显示出来
	if (userId != m_checkId)
	{
		return;
	}


	if (headPath.empty())
	{
		// 没有取到头像时，显示默认头像
		string defaultHead = FullPath("res\\headimages\\default.png");

		StringReplace(defaultHead, "\\", "/");
		f_covet.Gb2312ToUTF_8(headPath, defaultHead.c_str(), defaultHead.length());
	}
	//组合消息
	sprintf(strJsCode, "AppendMsgToHistory('%d', '%d', '%s', '%s', '%s', '%lu', '%s', '%s', '%d'); ",
		msgType,
		msgDataType, name.c_str(), msgTime.c_str(), msg.c_str(), userId, headPath.c_str(), msgId, msgDataType );

	if (m_pListMsgHandler.isLoaded)
	{
		CefString strCode(strJsCode), strUrl("");
		m_pListMsgHandler.handler->GetBrowser()->GetMainFrame()->ExecuteJavaScript(strCode, strUrl, 0);
	}




}


void CMainFrame::ResultRecvMsg(string msgId, bool bSuccess)
{
}

void CMainFrame::ResultSendMsg(string msgId, bool bSuccess)
{
}

void CMainFrame::ResultScreenCapture(string imagePath)
{

}

void CMainFrame::OnManagerButtonEvent(TNotifyUI& msg)
{
	//CDuiString msgName = msg.pSender->GetName();
	//int findPos = msgName.Find(_T("managerbutton_"));

	//结束
	if (msg.pSender->GetName() == _T("managerbutton_3"))
	{
		m_manager->SendTo_CloseChat(m_checkId, CHATCLOSE_USER);

	}
	//释放会话
	if (msg.pSender->GetName() == _T("managerbutton_7"))
	{
		m_manager->SendTo_ReleaseChat(m_checkId);

	}

}

void CMainFrame::ShowMySelfSendMsg(string strMsg)
{
	string            headPath = "";
	unsigned long     userId = 0;
	CCodeConvert      f_covet;
	char tempStr[256] = {0};
	char strJsCode[MAX_1024_LEN] = { 0 };
	string  name, msg ;
	string defaultHead;

	if (strMsg.empty())
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "插入空的聊天记录");
		return;
	}

	int urlPos = strMsg.find("用户头像地址:");
	int urlPos1 = strMsg.find("user_headimgurl:");
	int urlPos2 = strMsg.find(">立即评价</a>");
	if (urlPos > -1 || urlPos1 > -1 || urlPos2 > -1)
	{
		return;
	}
	string strName = m_mySelfInfo->UserInfo.nickname;	
	StringReplace(strName, "\\", "\\\\");
	StringReplace(strName, "'", "&#039;");
	StringReplace(strName, "\r\n", "<br>");
	f_covet.Gb2312ToUTF_8(name, strName.c_str(), strName.length());

	StringReplace(strMsg, "\\", "\\\\");
	StringReplace(strMsg, "'", "&#039;");
	StringReplace(strMsg, "\r\n", "<br>");
	f_covet.Gb2312ToUTF_8(msg, strMsg.c_str(), strMsg.length());

	userId = m_mySelfInfo->UserInfo.uid;
	defaultHead = FullPath("res\\headimage\\");
	sprintf(tempStr,"%d.png",userId);
	defaultHead += tempStr;

	if (!_globalSetting.FindFileExist((char*)defaultHead.c_str()))
	{
		defaultHead = FullPath("res\\headimages\\default.png");
	}

	StringReplace(defaultHead, "\\", "/");
	f_covet.Gb2312ToUTF_8(headPath, defaultHead.c_str(), defaultHead.length());

	//组合消息
	string msgTime = GetTimeByMDAndHMS(0);
	string msgId = m_manager->GetMsgId();
	sprintf(strJsCode, "AppendMsgToHistory('%d', '%d', '%s', '%s', '%s', '%lu', '%s', '%s', '%d'); ",
		5,1, name.c_str(), msgTime.c_str(), msg.c_str(), userId, headPath.c_str(), msgId, MSG_FROM_CLIENT);

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


void CMainFrame::ShowRightFrameView(int index)
{
	CWebUserObject *pWebUser = NULL;
	string strFrom, strEnd;
	string strUrl;

	pWebUser = m_manager->GetWebUserObjectByUid(m_checkId);

	if (m_checkId == 0 || pWebUser == NULL || !m_pVisitorRelatedHandler.isCreated)
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

	switch (index)
	{
	case TYPESELECT_INFO:
		if (pWebUser->m_bIsGetInfo)
		{
			CCodeConvert f_covet;
			string msg;
			if (pWebUser->m_bIsFrWX)
			{
				// 微信用户信息
				//CString html;
				//CString content = CreateClientInfoHtml(pWebUser->m_pWxUserInfo);
				//html.Format(Format_ClientInfo_Html, content, content.GetLength());
				//f_covet.Gb2312ToUTF_8(msg, html, html.GetLength());
			}
			else
			{
				// web用户信息
				f_covet.Gb2312ToUTF_8(msg, pWebUser->m_strInfoHtml.c_str(), pWebUser->m_strInfoHtml.length());
			}
			m_pVisitorRelatedHandler.handler->LoadString(msg.c_str());
		}
		else
		{
			m_pVisitorRelatedHandler.handler->LoadString(m_defaultUrlInfo.c_str());
			if (pWebUser->m_bIsFrWX)
			{
				//m_pFrame->GetWxUserInfo(pWebUser->webuserid, pWebUser->chatid);
			}
		}
		break;
	case TYPESELECT_CLIENT:
		_globalSetting.GetCurTimeString(strFrom, strEnd, 30);
		//globalGetCurTimeString(strFrom, strEnd, 30);
		if (m_manager->m_sysConfig->m_sStrServer == "tcp01.tq.cn" || m_manager->m_sysConfig->m_sStrServer == "211.151.52.48")//公网使用原来的链接
		{

		}	
			//strURL.Format(pApp->m_InitConfig.visitorpage_visitortail, pWebUser->chatid, pWebUser->webuserid, pWebUser->info.sid, 0, nTransFrom, strFrom, strEnd,/*pWebUser->floatadminuid*/ pApp->m_UserInfo.UserInfo.uid);
		else//公司内部使用新开发修改的链接
			//strURL.Format(pApp->m_InitConfig.visitorpage_visitortail, pWebUser->chatid, pWebUser->info.sid, 0, nTransFrom);

		break;
	case TYPESELECT_CHATID:

		//回话订单
		//strURL.Format(pApp->m_InitConfig.visitorpage_visitorbill, pWebUser->chatid, pWebUser->webuserid, pWebUser->info.sid, 0, 0, strFrom, strEnd);

		break;
	case TYPESELECT_CLIENTINFO:

		//客户信息
		//if (pApp->m_SysConfig.strServer == "tcp01.tq.cn" || pApp->m_SysConfig.strServer == "211.151.52.48")
		//	strURL.Format(pApp->m_InitConfig.visitorpage_visitorinfo, pWebUser->chatid, pWebUser->webuserid, pWebUser->info.sid, 0, 0, strFrom, strEnd, pWebUser->info.sid);
		//else
		//	strURL.Format(pApp->m_InitConfig.visitorpage_visitorinfo, pWebUser->chatid, "", 0, 0, strFrom, strEnd, pWebUser->info.thirdid, pWebUser->info.sid);


		break;

	case TYPESELECT_NOTICE:


#if 0
		//改成下订单
		if (pWebUser->m_bIsFrWX)
			strURL.Format(pApp->m_InitConfig.visitorpage_visitororder, pWebUser->chatid, pWebUser->webuserid, pWebUser->info.sid, 0, 0, strFrom, strEnd, pWebUser->info.thirdid, pWebUser->m_sWxAppid, 0);
		else
		{
			CCodeConvert f_covet;
			strTmp = "<html><head>";
			strTmp += STRING_HTML_META;
			strTmp += STRING_HTML_BASE;
			strTmp += STRING_HTML_STYLE;
			strTmp += "</head><body>";
			strTmp += "<div><p><span style=\"font - size:16px; color:#cccccc\">目前仅支持微信用户下订单！</span></p><div></body>";
			string msg;
			f_covet.Gb2312ToUTF_8(msg, strTmp, strTmp.GetLength());
			m_pVisitorRelatedHandler.handler->LoadString(msg.c_str());
		}
#endif


		break;

	case TYPESELECT_ORDER:

		//查订单
		//strURL.Format(pApp->m_InitConfig.visitorpage_visitororder, pWebUser->chatid, pWebUser->webuserid, pWebUser->info.sid, 0, 0, strFrom, strEnd, pWebUser->info.thirdid, pWebUser->m_sWxAppid, 1);


		break;
	default:
		break;
	}

	if (strUrl.empty())
	{
		if (strUrl != "about:blank")
		{
			strUrl += "&token=";
			//strURL += m_manager->GetAuthTokenString();
		}

		string url;
		//CCodeConvert convert;
		//g_WriteLog.WriteLog(C_LOG_TRACE, "OnEMTabDown--SelectType:%d,url:%s", cmd, strURL);
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





void CMainFrame::AddToMsgList(CUserObject *pUser, string strMsg, string strTime,  int userType,
	int msgType, int msgDataType, string msgId)
{
	string            headPath = "";
	unsigned long     userId = 0;
	CCodeConvert      f_covet;

	char strJsCode[MAX_1024_LEN] = { 0 };
	string  name, msg;


	if (pUser == NULL)
		return;
	if (strMsg.length() == 0)
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "插入空的聊天记录");
		return;
	}

	int urlPos = strMsg.find("用户头像地址:");
	int urlPos1 = strMsg.find("user_headimgurl:");
	int urlPos2 = strMsg.find(">立即评价</a>");
	if (urlPos > -1 || urlPos1 > -1 || urlPos2 > -1)
	{
		return;
	}

	userId = pUser->UserInfo.uid;
	string strName = pUser->UserInfo.nickname;
	StringReplace(strName, "\\", "\\\\");
	StringReplace(strName, "'", "&#039;");
	StringReplace(strName, "\r\n", "<br>");
	f_covet.Gb2312ToUTF_8(name, strName.c_str(), strName.length());

	StringReplace(strMsg, "\\", "\\\\");
	StringReplace(strMsg, "'", "&#039;");
	StringReplace(strMsg, "\r\n", "<br>");

	//这里需要把收到的内容做一下 还原
	ReplaceFaceId(strMsg);
	f_covet.Gb2312ToUTF_8(msg, strMsg.c_str(), strMsg.length());

	if (headPath.empty())
	{
		// 没有取到头像时，显示默认头像
		string defaultHead = FullPath("res\\headimages\\default.png");

		StringReplace(defaultHead, "\\", "/");
		f_covet.Gb2312ToUTF_8(headPath, defaultHead.c_str(), defaultHead.length());
	}
	//组合消息
	sprintf(strJsCode, "AppendMsgToHistory('%d', '%d', '%s', '%s', '%s', '%lu', '%s', '%s', '%d'); ",
		msgType,
		msgDataType, name.c_str(), strTime.c_str(), msg.c_str(), userId, headPath.c_str(), msgId, userType);

	if (m_pListMsgHandler.isLoaded)
	{
		CefString strCode(strJsCode), strUrl("");
		m_pListMsgHandler.handler->GetBrowser()->GetMainFrame()->ExecuteJavaScript(strCode, strUrl, 0);
	}

}
void CMainFrame::AddToMsgList(CWebUserObject *pWebUser, string strMsg, string strTime, int userType,
	int msgType , int msgDataType , CUserObject* pUser , string msgId )
{
	//string            headPath = "";
	//unsigned long     userId = 0;
	//CCodeConvert      f_covet;

	//char strJsCode[MAX_1024_LEN] = { 0 };
	//string  name, msg;


	//if (pWebUser == NULL)
	//	return;
	//if (strMsg.length() == 0)
	//{
	//	g_WriteLog.WriteLog(C_LOG_ERROR, "插入空的聊天记录");
	//	return;
	//}

	//int urlPos = strMsg.find("用户头像地址:");
	//int urlPos1 = strMsg.find("user_headimgurl:");
	//int urlPos2 = strMsg.find(">立即评价</a>");
	//if (urlPos > -1 || urlPos1 > -1 || urlPos2 > -1)
	//{
	//	return;
	//}

	//userId = pWebUser->webuserid;
	//string strName = pWebUser->info.name;
	//StringReplace(strName, "\\", "\\\\");
	//StringReplace(strName, "'", "&#039;");
	//StringReplace(strName, "\r\n", "<br>");
	//f_covet.Gb2312ToUTF_8(name, strName.c_str(), strName.length());

	//StringReplace(strMsg, "\\", "\\\\");
	//StringReplace(strMsg, "'", "&#039;");
	//StringReplace(strMsg, "\r\n", "<br>");

	////这里需要把收到的内容做一下 还原
	//ReplaceFaceId(strMsg);
	//f_covet.Gb2312ToUTF_8(msg, strMsg.c_str(), strMsg.length());

	//// 微信用户发来的
	//if (pWebUser->m_bIsFrWX)
	//{
	//	if (pWebUser->m_pWxUserInfo != NULL && !pWebUser->m_pWxUserInfo->headimgurl.empty())
	//	{
	//		headPath = pWebUser->m_pWxUserInfo->headimgurl;
	//	}
	//	else
	//	{
	//		// 当没有头像时，说明没有收到userinfo，主动去获取，包括token也去获取一次
	//		//m_pFrame->GetWxUserInfoAndToken(pWebUser);
	//	}
	//}

	//if (headPath.empty())
	//{
	//	// 没有取到头像时，显示默认头像
	//	string defaultHead = FullPath("res\\headimages\\default.png");

	//	StringReplace(defaultHead, "\\", "/");
	//	f_covet.Gb2312ToUTF_8(headPath, defaultHead.c_str(), defaultHead.length());
	//}
	////组合消息
	//sprintf(strJsCode, "AppendMsgToHistory('%d', '%d', '%s', '%s', '%s', '%lu', '%s', '%s', '%d'); ",
	//	msgType,msgDataType, name.c_str(), strTime.c_str(), msg.c_str(), userId, headPath.c_str(), msgId, userType);

	if (m_pListMsgHandler.isLoaded)
	{
		CefString strCode(strMsg), strUrl("");
		m_pListMsgHandler.handler->GetBrowser()->GetMainFrame()->ExecuteJavaScript(strCode, strUrl, 0);
	}

}


void CMainFrame::ChangeShowUserMsgWnd(unsigned long id)
{
	map<unsigned long, UserListUI::Node*>::iterator iter = { 0, NULL };
	UserListUI::Node*  tempNode = NULL;
	CUserObject *pUser = NULL;
	CWebUserObject *pWebUser = NULL;


	if (m_checkId == id || id == 0)//切换聊天对象显示 
		return;

	pUser = m_manager->GetUserObjectByUid(id);

	if (pUser == NULL)
	{
		pWebUser = m_manager->GetWebUserObjectByUid(id);

		if (pWebUser == NULL)
			return;
	}

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

	/*
	iter = m_onlineNodeMap.find(id);
	if (iter != m_onlineNodeMap.end())
	{
		tempNode = iter->second;
	}
	else
	{
		iter = m_offlineNodeMap.find(id);
		if (iter != m_offlineNodeMap.end())
		{
			tempNode = iter->second;
		}
	}
	*/
}

void CMainFrame::ResultInviteUser(CWebUserObject* pWebUser, CUserObject* pUser, bool bSuccess)
{

}

void CMainFrame::ResultTransferUser(CWebUserObject* pWebUser, CUserObject* pUser, bool bSuccess)
{

}
