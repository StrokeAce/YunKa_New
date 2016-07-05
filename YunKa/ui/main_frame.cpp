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



CMainFrame::CMainFrame(CChatManager* manager) :m_manager(manager)
{

	//初始化
	m_pFontBtn = m_pFaceBtn = m_pScreenBtn = pSendMsgBtn = m_pVoiceBtn = NULL;
	m_pRightCommonWordCombo = m_pRightCommonTypeCombo = m_pRightCommonFindCombo = NULL;
	m_pRightCommonWordEdit = m_pRightCommonTypeEdit = m_pRightCommonFindEdit = NULL;
	m_MainCenterRightWND = NULL;
	m_pSendEdit = NULL;

	pOnlineNode = pWaitForStart = pMySelfeNode = NULL;

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
	m_activeList.clear();

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

	if (uMsg == WM_KEYDOWN) // 发送消息框的Ctrl+V消息	
	{
        if ((wParam == 'V') && ::GetKeyState(VK_CONTROL) < 0)
		{
			if ((m_pSendEdit != NULL) && m_pSendEdit->IsFocused())
			{
				OnCtrlVEvent();
				return TRUE;
			}
		}

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

	if (uMsg == WM_SCREEN_CAPTURE_SUCCED) //截图完成后的消息操作
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
		   
	}
	else if (uMsg == ON_JS_CALL_MFC)
	{
		JsCallMFC(wParam, lParam);
	}

	if (uMsg == WM_MENU_START)
	{ 
		WCHAR *name = (WCHAR *)wParam;
		CDuiString conName = name;
		OnMenuEvent(conName);

		delete []name;
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
		//rightLayout->SetAttribute(_T("width"), formatString);  //lxh 屏蔽掉 

		rc = ShowMsgWnd->GetPos();
		rc.right = rc.left + centerWidth - 2;
		m_pListMsgHandler.handler->MoveBrowser(rc);

		if (m_rightRectWnd.left == 0)
			m_rightRectWnd = ShowRightWebWnd->GetPos();

		rect.left = rc.right + 4;
		rect.right = sysRect.right - 4;
		rect.top += m_rightRectWnd.top;
		rect.bottom = sysRect.bottom - 4;
		m_pVisitorRelatedHandler.handler->MoveBrowser(rect);
		m_rightRectMax = rect;


		rect.left = m_mainCenterAndRightRect.left;
		rect.right = sysRect.right - 4;
		rect.top = m_mainCenterAndRightRect.top;
		rect.bottom = sysRect.bottom - 4;

		m_pWebURLHandler.handler->MoveBrowser(rect);


	}
	else
	{
		formatString.Format(_T("%d"), m_centerChatInfo.centerFrameWitdh);
		centerLayout->SetAttribute(_T("width"), formatString);

		rc = ShowMsgWnd->GetPos();
		rc.right = rc.left + m_centerChatInfo.showMsgWidth;
		m_pListMsgHandler.handler->MoveBrowser(rc);

		//rc = ShowRightWebWnd->GetPos();
		m_pVisitorRelatedHandler.handler->MoveBrowser(m_rightRectWnd);
		m_pWebURLHandler.handler->MoveBrowser(m_mainCenterAndRightRect);
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

	//cef窗口
	InitLibcef();

	//ClearFile("*.jpg");


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

	MoveAndRestoreRightFrameControl(1);
		


	//上层管理按钮 设置初始状态
	UpdateTopCenterButtonState(0);

	//右下角小图标
	m_frameSmallMenu.Init();
	m_frameSmallMenu.CreateSmallIcon(this->m_hWnd, DEFINE_SMALL_ICON_PATH);

	//表情包初始化
	wstring strPath = ZYM::CPath::GetCurDir() + _T("../bin/") _T("SkinRes\\Face\\FaceConfig.xml");
	m_faceList.LoadConfigFile(strPath.c_str());

	//左侧用户列表显示
    pUserList = static_cast<UserListUI*>(m_PaintManager.FindControl(_T("userlist")));
	pUserList->SetListBKImage(_T("file = 'mainframe\\tree_top.png' corner = '2,1,2,1' fade = '100'"));
	pUserList->SetListName(_T("userlist"));
	pWaitForAccept = NULL;
	pWaitForStart = pUserList->AddNode(_T("{x 4}{i gameicons.png 18 0}{x 4}等待开始"),0);
	pWaitForAccept = pUserList->AddNode(_T("{x 4}{i gameicons.png 18 14}{x 4}等待应答"),0,pWaitForStart);

	InitRightTalkList();

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
	else if (msg.pSender == m_pVoiceBtn)
		OnBtnVoice(msg);


	if (msg.pSender->GetName() == _T("fileSendBtn"))
	{
		OnBtnSendFile(msg);
	}

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
					ShowRightOptionFrameView(m_curSelectId);
					break;
				}
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
				CDuiString str = node->data()._text;
				unsigned long id = node->data()._uid;
				m_savedClickId = id;
				
				OnItemClickEvent(id,0);
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

			OnActiveUser(uid);
			  
		}
	}

	else if (msg.pSender->GetName() == _T("talklist"))
	{
		if (_tcscmp(msg.pSender->GetClass(), _T("ListLabelElementUI")) == 0)
		{
			UserListUI::Node* node = (UserListUI::Node*)msg.pSender->GetTag();
			m_pTalkList->ExpandNode(node, !node->data()._expand);

		}
	}
}


void CMainFrame::OnItemRbClick(TNotifyUI &msg)
{
	CDuiString xmlPath = L"";
	unsigned long uid = m_savedClickId;
	CWebUserObject *pWebUser = NULL;
	CUserObject *pUser = NULL;

	if (msg.pSender->GetName() == L"userlist")
	{
		//根据选择的对象不同 弹出相应的右键菜单
		//UserListUI::Node* node = (UserListUI::Node*)msg.pSender->GetTag();
		//pUserList->ExpandNode(node,!node->data()._expand);
		

		CheckIdForUerOrWebuser(uid,&pWebUser,&pUser);

		//轮询查找 查找当前选择的 uid是 空 还是坐席 等待中的用户 等等
		if (uid == 0) //既不是访客 也不是坐席
		{
			xmlPath = L"menu\\menu_right_2.xml";
		}
		else if (uid == m_manager->m_userInfo.UserInfo.uid) //自己的uid
		{
			xmlPath = L"menu\\menu_right_3.xml";
		}

		else
		{
			if (pWebUser != NULL)
			{
				if (pWebUser->onlineinfo.talkstatus == TALKSTATUS_REQUEST)
				{
					xmlPath = L"menu\\menu_right_1.xml";
				}
			}
			else
			{
				if (pUser == NULL)
					return;

				if (pUser->status == STATUS_OFFLINE)
				{
					xmlPath = L"menu\\menu_right_4.xml";
				}
				else if (pUser->status == STATUS_ONLINE)
				{
					xmlPath = L"menu\\menu_right_5.xml";
				}

			}
		}

		if (!xmlPath.IsEmpty())
		{
			CMenuWnd* pMenu = new CMenuWnd(m_hMainWnd);
			CPoint cpoint = msg.ptMouse;

			ClientToScreen(this->m_hWnd, &cpoint);
			pMenu->SetPath((WCHAR*)xmlPath.GetData());
			pMenu->Init(NULL, _T(""), _T("xml"), cpoint);
		}



/*
		CMenuWnd2* pMenu = new CMenuWnd2();
		if (pMenu == NULL) { return; }
		POINT pt = { msg.ptMouse.x, msg.ptMouse.y };
		//::ClientToScreen(*this, &pt);
		pMenu->SetXMLPath(L"menu\\menu_right.xml");
		pMenu->Init(msg.pSender, pt);
		*/

		/*
		CMenuWnd* pMenu = new CMenuWnd(m_hMainWnd);
		CPoint cpoint = msg.ptMouse;

		//cpoint.y -= 65;
		//ClientToScreen(m_hMenuWnd, &cpoint);
		pMenu->SetPath(L"menu\\menu_right_4.xml");
		pMenu->Init(NULL, _T(""), _T("xml"), cpoint);

*/


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
	char getInput[MAX_1024_LEN] = {0};
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
	//先添加自己的位置
	AddMyselfToList(pUserList, &m_manager->m_userInfo);

	//再添加最后一个的位置
	AddOnlineVisitor(pUserList, NULL, -1);

	//AddOnlineVisitor(pUserList,NULL,-1);
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

		//map<unsigned long, UserListUI::Node*>::iterator  iter = m_allVisitorNodeMap.find(user->UserInfo.uid);
		//if (iter != m_allVisitorNodeMap.end())
			//m_allVisitorNodeMap.erase(iter);

		//m_allVisitorNodeMap.insert(pair<unsigned long, UserListUI::Node*>(user->UserInfo.uid, pUserNameNode));

		pUserList->ExpandNode(pUserNameNode, false);
	}
	else  if (user->status == STATUS_ONLINE)     //在线
	{
		m_onlineNodeMap.insert(pair<unsigned long, UserListUI::Node*>(user->UserInfo.uid, pUserNameNode));

		//map<unsigned long, UserListUI::Node*>::iterator  iter = m_allVisitorNodeMap.find(user->UserInfo.uid);
		//if (iter != m_allVisitorNodeMap.end())
			//m_allVisitorNodeMap.erase(iter);
		//m_allVisitorNodeMap.insert(pair<unsigned long, UserListUI::Node*>(user->UserInfo.uid, pUserNameNode));

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

	if (user->status == STATUS_OFFLINE)  //离线
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
	else  if (user->status == STATUS_ONLINE)                   //在线
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


	taklString.Format(_T("{x 4}{i gameicons.png 18 10}{x 4}对话中"));
	pUserTalkNode = pUserList->AddNode(taklString, user->UserInfo.uid, pUserNameNode);

	changeString.Format(_T("{x 4}{i gameicons.png 18 10}{x 4}转接中"));
	pUserChangeNode = pUserList->AddNode(changeString, user->UserInfo.uid, pUserNameNode);


	acceptString.Format(_T("{x 4}{i gameicons.png 18 10}{x 4}邀请中"));
	pUserAcceptNode = pUserList->AddNode(acceptString, user->UserInfo.uid, pUserNameNode);


	if (  user->status == STATUS_OFFLINE )    //离线
	{
		m_offlineNodeMap.insert(pair<unsigned long, UserListUI::Node*>(user->UserInfo.uid, pUserNameNode));

		//m_allVisitorNodeMap.insert(pair<unsigned long, UserListUI::Node*>(user->UserInfo.uid, pUserNameNode));


		pUserList->ExpandNode(pUserNameNode, false);

	}
	else  if (user->status == STATUS_ONLINE) //在线
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

		if (userId != m_curSelectId)
		{
			return;
		}

		if (msgContent.length() == 0)
		{
			g_WriteLog.WriteLog(C_LOG_ERROR, "插入空的聊天记录");
			return;
		}

		CefString strCode(msgContent), strUrl("");
		m_pListMsgHandler.handler->GetBrowser()->GetMainFrame()->ExecuteJavaScript(strCode, strUrl, 0);
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
	if (msg.pSender->GetName() == _T("managerbutton_1"))
	{
		if (m_curSelectId > 0)
		{
			map<unsigned long, UserListUI::Node*>::iterator iter = m_waitVizitorMap.find(m_curSelectId);
			if (iter != m_waitVizitorMap.end())
			{
				m_manager->SendTo_AcceptChat(m_curSelectId);
			}
		}
	}

	//转接
	else if (msg.pSender->GetName() == _T("managerbutton_2"))
	{
		m_topWndType = 1;
		//获取在线坐席数
		m_manager->SendTo_GetOnlineUser();

	}
	//结束
	else if (msg.pSender->GetName() == _T("managerbutton_3"))
	{
		m_manager->SendTo_CloseChat(m_curSelectId, CHATCLOSE_USER);

	}

	//屏蔽
	else if (msg.pSender->GetName() == _T("managerbutton_4"))
	{
	}

	//邀请评价
	else if (msg.pSender->GetName() == _T("managerbutton_5"))
	{
	}


	//筛选访客
	else if (msg.pSender->GetName() == _T("managerbutton_6"))
	{
		CSelectVisitorWnd *dlg= new CSelectVisitorWnd();

		dlg->Create(m_hWnd, _T(""), UI_WNDSTYLE_DIALOG, 0, 0, 0, 0, 0, NULL);
		dlg->CenterWindow();
		dlg->ShowModal();
	}

	//释放会话
	else if (msg.pSender->GetName() == _T("managerbutton_7"))
	{
		m_manager->SendTo_ReleaseChat(m_curSelectId);

	}

	//邀请协助
	else if (msg.pSender->GetName() == _T("managerbutton_8"))
	{

		m_topWndType = 2;
		//获取在线坐席数
		m_manager->SendTo_GetOnlineUser();


	}
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

		sprintf(sURL, "%s&token=%s", m_manager->m_initConfig.webpage_SvrMsg, m_manager->m_login->m_szAuthtoken);
		ShowWebBrowser(sURL);
	}


	//访客来电
	else if (msg.pSender->GetName() == _T("managerbutton_11"))
	{

		sprintf(sURL, "%s&kefu_uin=%lu&token=%s", m_manager->m_initConfig.webpage_querywebphone, m_manager->m_userInfo.UserInfo.uid, m_manager->m_login->m_szAuthtoken);
		ShowWebBrowser(sURL);
	}
	//访客留言
	else if (msg.pSender->GetName() == _T("managerbutton_12"))
	{
		sprintf(sURL, "%s&action=query&result=0&kefu_uin=%lu&token=%s", m_manager->m_initConfig.webpage_note, m_manager->m_userInfo.UserInfo.uid, m_manager->m_login->m_szAuthtoken);
		ShowWebBrowser(sURL);
	}
	//客户管理
	else if (msg.pSender->GetName() == _T("managerbutton_13"))
	{
		sprintf(sURL, "%s&token=%s", m_manager->m_initConfig.webpage_crm, m_manager->m_login->m_szAuthtoken);
		ShowWebBrowser(sURL);
	}
	//统计分析
	else if (msg.pSender->GetName() == _T("managerbutton_14"))
	{
		sprintf(sURL, "%s&token=%s", m_manager->m_initConfig.webpage_workbillurl, m_manager->m_login->m_szAuthtoken);
		ShowWebBrowser(sURL);
	}
	//管理中心
	else if (msg.pSender->GetName() == _T("managerbutton_15"))
	{
		sprintf(sURL, "%s&token=%s", m_manager->m_initConfig.webpage_mgmt, m_manager->m_login->m_szAuthtoken);
		ShowWebBrowser(sURL);
	}


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
	StringReplace(strName, "\\", "\\\\");
	StringReplace(strName, "'", "&#039;");
	StringReplace(strName, "\r\n", "<br>");
	f_covet.Gb2312ToUTF_8(name, strName.c_str(), strName.length());

	StringReplace(strMsg, "\\", "\\\\");
	StringReplace(strMsg, "'", "&#039;");
	StringReplace(strMsg, "\r\n", "<br>");
	f_covet.Gb2312ToUTF_8(msg, strMsg.c_str(), strMsg.length());

	imagePath = FullPath("SkinRes\\mainframe\\msg_wait.gif");
	StringReplace(imagePath, "\\", "/");

	if (msgType == MSG_DATA_TYPE_IMAGE)
	{
		sprintf(contentMsg, "<img id=\"%s_image\" class=\"wait_image\" src=\"%s\"><img class=\"msg_image\" src=\"%s\">",
			msgId.c_str(), imagePath.c_str(), msg.c_str());
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
		sprintf(contentMsg, "<img id=\"%s_image\" class=\"wait_image\" src=\"%s\"><span id=\"%s_span\" style=\"color:red\">%s</span>",
			msgId.c_str(), imagePath.c_str(), msgId.c_str(), msg.c_str());
		msg = contentMsg;
	}

	//组合消息
	string msgTime = GetTimeByMDAndHMS(0);
	sprintf(strJsCode, "AppendMsgToHistory('%d', '%d', '%s', '%s', '%s', '%lu', '%s', '%s', '%s'); ",
		MSG_FROM_SELF, msgType, name.c_str(), msgTime.c_str(), msg.c_str(), userId, m_manager->m_userInfo.m_headPath.c_str(), msgId.c_str(), imagePath.c_str());

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




void CMainFrame::ShowRightOptionFrameView(unsigned long id)
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
	pWebUser = m_manager->GetWebUserObjectByUid(id);

	if (id == 0 || pWebUser == NULL || !m_pVisitorRelatedHandler.isCreated)
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
			if (m_manager->m_sysConfig->m_sStrServer == "tcp01.tq.cn" || m_manager->m_sysConfig->m_sStrServer == "211.151.52.48")//公网使用原来的链接
			{
				sprintf(showMsg, m_manager->m_initConfig.visitorpage_visitortail, pWebUser->chatid, pWebUser->webuserid, pWebUser->info.sid, 0, nTransFrom, strFrom.c_str(), strEnd.c_str(),/*pWebUser->floatadminuid*/  m_manager->m_userInfo.UserInfo.uid);
			}
			else//公司内部使用新开发修改的链接
			{	
				sprintf(showMsg, m_manager->m_initConfig.visitorpage_visitortail, pWebUser->chatid, pWebUser->info.sid, 0, nTransFrom);
				//strURL.Format(pApp->m_InitConfig.visitorpage_visitortail, pWebUser->chatid, pWebUser->info.sid, 0, nTransFrom);
			}
				strUrl = showMsg;
				break;
		case TYPESELECT_CHATID:
			sprintf(showMsg, m_manager->m_initConfig.visitorpage_visitorbill, pWebUser->chatid, pWebUser->webuserid, pWebUser->info.sid, 0, 0, strFrom.c_str(), strEnd.c_str());
			strUrl = showMsg;
			break;
		case TYPESELECT_CLIENTINFO:
			//("http://vip.tq.cn/vip/visitorinfo.do?billid=%s&cuin=%lu&rand=%s&transtype=%d&transfrom=%lu&stime=%s&etime=%s")
			//客户信息
			if (m_manager->m_sysConfig->m_sStrServer == "tcp01.tq.cn" || m_manager->m_sysConfig->m_sStrServer == "211.151.52.48")
				sprintf(showMsg, m_manager->m_initConfig.visitorpage_visitorinfo, pWebUser->chatid, pWebUser->webuserid, pWebUser->info.sid, 0, 0, strFrom.c_str(), strEnd.c_str(), pWebUser->info.sid);
			else
				sprintf(showMsg, m_manager->m_initConfig.visitorpage_visitorinfo, pWebUser->chatid, "", 0, 0, strFrom.c_str(), strEnd.c_str(), pWebUser->info.thirdid, pWebUser->info.sid);

				//strURL.Format(pApp->m_InitConfig.visitorpage_visitorinfo, pWebUser->chatid, "", 0, 0, strFrom, strEnd, pWebUser->info.thirdid, pWebUser->info.sid);
			strUrl = showMsg;
			break;
		case TYPESELECT_NOTICE:
			//改成下订单
			if (pWebUser->m_bIsFrWX)
			{
				sprintf(showMsg, m_manager->m_initConfig.visitorpage_visitororder, pWebUser->chatid, pWebUser->webuserid, pWebUser->info.sid, 0, 0, strFrom.c_str(), strEnd.c_str(), pWebUser->info.thirdid, pWebUser->m_sWxAppid, 0);
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
			sprintf(showMsg, m_manager->m_initConfig.visitorpage_visitororder, pWebUser->chatid, pWebUser->webuserid, pWebUser->info.sid, 0, 0, strFrom.c_str(), strEnd.c_str(), pWebUser->info.thirdid, pWebUser->m_sWxAppid, 1);
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


	if (m_curSelectId == id || id == 0)//切换聊天对象显示 
		return;


	CheckIdForUerOrWebuser(id, &pWebUser, &pUser);
	
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
void CMainFrame::OnMenuEvent(CDuiString controlName)
{
	if (controlName.IsEmpty())
		return;

//选择 0 enter 发送消息 还是 1 CTRL enter 发送消息  
	if (controlName == L"MenuElement_btn_send_menu_1")
	{
		if (m_manager->m_sysConfig->m_nKeySendType == 1)
		    m_manager->m_sysConfig->m_nKeySendType = 0;
	}
	else if (controlName == L"MenuElement_btn_send_menu_2")
	{
		if (m_manager->m_sysConfig->m_nKeySendType == 0)
		    m_manager->m_sysConfig->m_nKeySendType  =  1;
	}

	if (controlName == L"menu_hide_main_wnd")
	{

	}
	else if (controlName == L"menu_online")
	{

	}
	else if (controlName == L"menu_busy")
	{
	}
	else if (controlName == L"menu_leave")
	{
	}
	else if (controlName == L"menu_logout")
	{
	}
	else if (controlName == L"menu_quit")
	{
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

	if (id == 0)
		return MSG_RECV_ERROR;

	CheckIdForUerOrWebuser(id,&pWebUser,&pUser);
	//CUserObject *pUser = m_manager->GetUserObjectByUid(id);
	if (pUser == NULL)
	{
		//CWebUserObject  *pWebUser = m_manager->GetWebUserObjectByUid(id);
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


void CMainFrame::CheckIdForUerOrWebuser(unsigned long id,CWebUserObject **pWebUser,CUserObject **pUser)
{


	*pUser  = m_manager->GetUserObjectByUid(id);

	if (*pUser == NULL)
	{
		*pWebUser = m_manager->GetWebUserObjectByUid(id);
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
	char strJsCode[MAX_256_LEN];
	sprintf(strJsCode, "StartRecordAudio(\"%lu\",\"%d\");", m_curSelectId, MSG_RECV_WX);
	CefString strCode(strJsCode), strUrl("");
	m_pListMsgHandler.handler->GetBrowser()->GetMainFrame()->ExecuteJavaScript(strCode, strUrl, 0);
}


//0 max 1 retore
void CMainFrame::MoveAndRestoreRightFrameControl(int type)
{
	int width = 0;
	CHorizontalLayoutUI   *m_hLayout = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(_T("HorizontalLayout_RightFrameOption")));
	
	RECT rect = m_pRightCommonWordCombo->GetPos();
	if (type == 1)
		width = m_hLayout->GetWidth() - 10;
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
	m_pTalkList = static_cast<UserListUI*>(m_PaintManager.FindControl(_T("talklist")));

	m_pTalkList->SetListName(_T("talklist"));

	UserListUI::Node*  TalkList1 = m_pTalkList->AddNode(_T("早起签到"), 0);
	UserListUI::Node*  TalkList2 = m_pTalkList->AddNode(_T("晚安签到"), 0);
	UserListUI::Node*  TalkList3 = m_pTalkList->AddNode(_T("云咖平台回复语"), 0);
	UserListUI::Node*  TalkList4 = m_pTalkList->AddNode(_T("顺风车"), 0);
	UserListUI::Node*  TalkList5 = m_pTalkList->AddNode(_T("服务时间"), 0);
	UserListUI::Node*  TalkList6 = m_pTalkList->AddNode(_T("八妹说"), 0);
	UserListUI::Node*  TalkList7 = m_pTalkList->AddNode(_T("测试"), 0);

	//以下数据作为测试数据 暂时显示 后面再修改 lxh
	m_pTalkList->AddNode(_T("{x 12}早起签到成功回复语"),0, TalkList1);
	m_pTalkList->AddNode(_T("{x 12}早起团报名链接"), 0, TalkList1);



	m_pTalkList->AddNode(_T("{x 12}晚安签到中奖"), 0, TalkList2);
	m_pTalkList->AddNode(_T("{x 12}晚安签到中奖1"), 0, TalkList2);
	m_pTalkList->AddNode(_T("{x 12}晚安签到中奖2"), 0, TalkList2);
	m_pTalkList->AddNode(_T("{x 12}晚安签到中奖3"), 0, TalkList2);
	m_pTalkList->AddNode(_T("{x 12}晚安签到中奖4"), 0, TalkList2);
	m_pTalkList->AddNode(_T("{x 12}晚安签到中奖5"), 0, TalkList2);
	m_pTalkList->AddNode(_T("{x 12}晚安签到中奖6"), 0, TalkList2);
	m_pTalkList->AddNode(_T("{x 12}晚安签到中奖7"), 0, TalkList2);
	m_pTalkList->AddNode(_T("{x 12}晚安签到中奖8"), 0, TalkList2);
	m_pTalkList->AddNode(_T("{x 12}晚安签到中奖9"), 0, TalkList2);
	m_pTalkList->AddNode(_T("{x 12}晚安签到中奖10"), 0, TalkList2);

	m_pTalkList->AddNode(_T("{x 12}云咖平台不能支持的回复"), 0, TalkList3);

	m_pTalkList->AddNode(_T("{x 12}发送66顺风车的回复"), 0, TalkList4);
	m_pTalkList->AddNode(_T("{x 12}发送66顺风车广告语"), 0, TalkList4);
	m_pTalkList->AddNode(_T("{x 12}发送投票的回复"), 0, TalkList4);

	m_pTalkList->AddNode(_T("{x 12}9588服务时间回复语"), 0, TalkList5);
	m_pTalkList->AddNode(_T("{x 12}云咖9588服务时间回复语"), 0, TalkList5);

	m_pTalkList->AddNode(_T("{x 12}八妹说的话"), 0, TalkList6);
	m_pTalkList->AddNode(_T("{x 12}测试"), 0, TalkList7);

	m_pTalkList->ExpandNode(TalkList2, false);
	m_pTalkList->ExpandNode(TalkList3, false);	
	m_pTalkList->ExpandNode(TalkList4, false);	
	m_pTalkList->ExpandNode(TalkList5, false);	
	m_pTalkList->ExpandNode(TalkList6, false);
	m_pTalkList->ExpandNode(TalkList7, false);
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
			pMenu->SetAttrData(L"MenuElement_btn_send_select_1", path);
		}
		else
		{
			pMenu->SetAttrData(L"MenuElement_btn_send_select_2", path);
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

	CheckIdForUerOrWebuser(id, &pWebUser, &pUser);
	if (id == 0 || pUser != NULL)  //上层管理按钮 设置初始状态
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
			}
			//筛选访客按钮
			if (i == 5 || i >= 8)
			{
				m_pManagerBtn[i].m_pManagerBtn->SetNormalImage(m_pManagerBtn[i].pushedImage);
				m_pManagerBtn[i].m_pManagerBtn->SetHotImage(m_pManagerBtn[i].hotImage);
				m_pManagerBtn[i].m_pManagerBtn->SetPushedImage(m_pManagerBtn[i].hotImage);
				m_pManagerBtn[i].m_buttonState = 1;
			}
			else
			{
				m_pManagerBtn[i].m_pManagerBtn->SetNormalImage(m_pManagerBtn[i].normalImage);
				m_pManagerBtn[i].m_pManagerBtn->SetHotImage(m_pManagerBtn[i].normalImage);
				m_pManagerBtn[i].m_pManagerBtn->SetPushedImage(m_pManagerBtn[i].normalImage);
				m_pManagerBtn[i].m_buttonState = 0;
			}
		}
		return;
	}
	//如果在等待列表 显示 接受 屏蔽 邀请评价   这几个按钮
	map<unsigned long, UserListUI::Node*>::iterator iter = m_waitVizitorMap.find(id);
	if (iter != m_waitVizitorMap.end() ) 
	{

		for (int i = 0; i < MID_MANAGER_BUTTON_NUM; i++)
		{
			if (i == 1 || i == 2|| i == 6 || i == 7)
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
	}
	else
	{
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

	}


}

void CMainFrame::RecvWebUserInfo(CWebUserObject* pWebUser)
{

}

void CMainFrame::OnActiveUser(unsigned long id)
{
	CUserObject	*pUser = m_manager->GetUserObjectByUid(m_selectUserId);
	CWebUserObject *pWebUser = m_manager->GetWebUserObjectByUid(id);
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
					//在邀请列表里删除 
					m_transferUserList.erase(iterList);
					type = 1;
					break;
				}
			}
		}
	}
	if (type == 0)
	{
		if (pUser == NULL || pWebUser == NULL)
			return;

		m_manager->SendTo_InviteUserResult(pWebUser, pUser, true);
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
		if (tempNode != NULL && tempNode->data()._level >= 0)
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
		UserListUI::Node * addNode = pUserList->AddNode(text, pWebUser->webuserid, tempChildNode);
		m_allVisitorNodeMap.insert(pair<unsigned long, UserListUI::Node*>(pWebUser->webuserid, addNode));
	}

	else if (type == 1)
	{
		if (pUser == NULL || pWebUser == NULL)
			return;
		m_manager->SendTo_TransferUserResult(pWebUser, pUser, true);
		//然后加到对话中 
		map<unsigned long, UserListUI::Node*>::iterator iter = m_allVisitorNodeMap.find(pWebUser->webuserid);
		//没有找到
		if (iter == m_allVisitorNodeMap.end())
		{
			return;
		}
		UserListUI::Node *tempNode = iter->second;
		CDuiString text = tempNode->data()._text;

		//先在用户的对话列表中删除 
		if (tempNode != NULL && tempNode->data()._level >= 0)
			pUserList->RemoveNode(tempNode);

		m_allVisitorNodeMap.erase(iter);

		//先删掉 转接中的用户 然后放入对话中
		UserListUI::Node *tempChildNode = pMySelfeNode->child(0);
		UserListUI::Node * addNode = pUserList->AddNode(text, pWebUser->webuserid, tempChildNode);
		m_allVisitorNodeMap.insert(pair<unsigned long, UserListUI::Node*>(pWebUser->webuserid, addNode));
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

		if (m_topWndType == 1)  //转接 
		{
			if (m_curSelectId > 0)
			{
				if (pUser != NULL && pWebUser != NULL)
				    m_manager->SendTo_TransferRequestUser(pWebUser, pUser);
			}
		}
		else if (m_topWndType == 2)  //邀请协助
		{
			if (pUser != NULL && pWebUser != NULL)
				m_manager->SendTo_InviteUser(pWebUser, pUser);
		}
		else if (m_topWndType == 3)  //内部对话
		{
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

				currentNode = pUserList->AddNode(nameString.GetData(), m_selectUserId, addNode);

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
		int index = 0;
		if (pUser->status == STATUS_ONLINE)  //当前坐席是离线状态 同时过来的状态是上线状态
		{
			//先删除当前的离线坐席 list 再添加上线的坐席状态

			map<unsigned long, UserListUI::Node*>::iterator  iter = m_offlineNodeMap.find(pUser->UserInfo.uid);
			if (iter == m_offlineNodeMap.end())
				return;

			UserListUI::Node* tempNode = iter->second;


			//先删除
			if (tempNode != NULL && tempNode->data()._level >= 0)
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

	else if (pObj->m_nEMObType == OBJECT_WEBUSER)
	{
		CWebUserObject* pWebUser = (CWebUserObject*)pObj;
		CDuiString text;
		UserListUI::Node* tempNode;
		WCHAR name[64] = { 0 };
		ANSIToUnicode(pWebUser->info.name, name);

		if (pWebUser->onlineinfo.talkstatus == TALKSTATUS_NO)
		{
			tempNode = pOnlineNode->child(1);
		}
		else if (pWebUser->onlineinfo.talkstatus == TALKSTATUS_AUTOINVITE)
		{
			tempNode = pOnlineNode->child(0);
		}
		else
			return;
		//是否来自微信
		if (pWebUser->m_bIsFrWX)
		{
			text.Format(_T("{x 4}{i user_wx.png 1 0}{x 4}%s"), name);
		}
		else
		{
			text.Format(_T("{x 4}{i user_web.png 1 0}{x 4}%s"), name);
		}

	
		//添加等待列表
		UserListUI::Node* AddNode = pUserList->AddNode(text, pWebUser->webuserid, tempNode);
		pUserList->ExpandNode(tempNode, true);


	}

}

// 坐席下线消息
void CMainFrame::RecvOffline(IBaseObject* pObj)
{
	if (pObj->m_nEMObType == OBJECT_USER)
	{
		CUserObject* pUser = (CUserObject*)pObj;
		int index = 0;
		if (pUser->status == STATUS_OFFLINE)  //当前坐席是在线状态 同时过来的状态是离线状态
		{
			//先删除当前的在线 坐席 list   再添加离线的坐席状态

			map<unsigned long, UserListUI::Node*>::iterator  iter = m_onlineNodeMap.find(pUser->UserInfo.uid);
			if (iter == m_onlineNodeMap.end())
				return;


			UserListUI::Node* tempNode = iter->second;
#if 0
			


			for (int i = 0; i < tempNode->num_children; i++)
			{

				UserListUI::Node* child = tempNode->child[i];

				for (int j = 0; j < child->num_children; j++)
				{

					UserListUI::Node* child1 = child->child[j];

				}


			}

#endif

			//先删除
			if (tempNode != NULL && tempNode->data()._level >= 0)
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

	else if (pObj->m_nEMObType == OBJECT_WEBUSER)
	{


	}
}



void CMainFrame::RecvChatInfo(CWebUserObject* pWebUser, CUserObject* pUser)
{
	CDuiString text;
	WCHAR name[64] = { 0 };
	ANSIToUnicode(pWebUser->info.name, name);

	//首先得找一下 webuser id  如果已经在 数列表里 则先删除
	UserListUI::Node* tempNode = GetOneUserNode(pWebUser->webuserid);
	if (tempNode != NULL)
	{
		if ( tempNode->data()._level >= 0)
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
	if (pWebUser->onlineinfo.talkstatus == TALKSTATUS_REQUEST)
	{
		//添加等待列表
		UserListUI::Node* tempNode = pUserList->AddNode(text, pWebUser->webuserid, pWaitForAccept);
		pUserList->ExpandNode(pWaitForAccept, true);

		m_waitVizitorMap.insert(pair<unsigned long, UserListUI::Node*>(pWebUser->webuserid, tempNode));

		m_allVisitorUserMap.insert(pair<unsigned long, unsigned long>(pWebUser->webuserid, 0));

		//m_allVisitorNodeMap.insert(pair<unsigned long, UserListUI::Node*>(pWebUser->webuserid, tempNode));
	}
	//已接收的在会话列表
	else if (pWebUser->onlineinfo.talkstatus == TALKSTATUS_TALK)
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
			UserListUI::Node* addNode = pUserList->AddNode(text, pWebUser->webuserid, tempNode);
			pUserList->ExpandNode(tempNode, true);

			m_allVisitorNodeMap.insert(pair<unsigned long, UserListUI::Node*>(pWebUser->webuserid, addNode));
		}
		else //加到其他人列表底下
		{
			UserListUI::Node* tempNode = GetOneUserNode(pUser->UserInfo.uid);
			if (tempNode == NULL)
				return;

			UserListUI::Node* child = tempNode->child(0);

			UserListUI::Node* addNode = pUserList->AddNode(text, pWebUser->webuserid, child);
			pUserList->ExpandNode(tempNode, true);
			pUserList->ExpandNode(child, true);

			m_allVisitorNodeMap.insert(pair<unsigned long, UserListUI::Node*>(pWebUser->webuserid, addNode));

		}

		m_allVisitorUserMap.insert(pair<unsigned long, unsigned long>(pWebUser->webuserid, pUser->UserInfo.uid));
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
	}
	else
	{
		//iter = m_allVisitorNodeMap.find(pWebUser->webuserid);
		//if (iter != m_allVisitorNodeMap.end())
		//{
		//	tempNode = iter->second;
		//	text = tempNode->data()._text;
		//	uid = tempNode->data()._uid;
		//	//当前选择 的用户就是 激活的用户
		//	m_curSelectId = uid;

		//	type = 1;
		//}


		return;
	}

	m_waitVizitorMap.erase(iter);
	//需要从等待列表删除 这个用户
	if (tempNode != NULL && tempNode->data()._level >= 0)
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
	UserListUI::Node* currentNode = pUserList->AddNode(text, uid, addNode);
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
	map<unsigned long, UserListUI::Node*>::iterator iter = m_waitVizitorMap.find(pWebUser->webuserid);


	list<unsigned long>::iterator iterList = m_activeList.begin();
	for (; iterList != m_activeList.end(); iterList++)
	{
		if (*iterList == pWebUser->webuserid)
		{
			m_activeList.erase(iterList);
			break;
		}
	}

	//没有找到
	if (iter == m_waitVizitorMap.end())
	{
		iter = m_allVisitorNodeMap.find(pWebUser->webuserid);

		if (iter == m_allVisitorNodeMap.end())
			return;

		type = 1;
	}
	tempNode = iter->second;

	if (tempNode != NULL && tempNode->data()._level >= 0)
	{
		//从坐席列表底下删除 然后加入等待列表
		pUserList->RemoveNode(tempNode);
	}



	//从 会话中 删除 当前访客 信息
	map<unsigned long, unsigned long>::iterator iterLong = m_allVisitorUserMap.find(pWebUser->webuserid);

	if (type == 0)
		m_waitVizitorMap.erase(iter);
	else
	{
		m_allVisitorNodeMap.erase(iter);

		if (iterLong == m_allVisitorUserMap.end())
			return;
		m_allVisitorUserMap.erase(iterLong);

	}

}

void CMainFrame::RecvReleaseChat(CWebUserObject* pWebUser)
{
	UserListUI::Node *tempNode = NULL;

	list<unsigned long>::iterator iterList = m_activeList.begin();
	for (; iterList != m_activeList.end(); iterList++)
	{
		if (*iterList == pWebUser->webuserid)
		{
			m_activeList.erase(iterList);
			break;
		}
	}


	map<unsigned long, UserListUI::Node*>::iterator iter = m_allVisitorNodeMap.find(pWebUser->webuserid);
	//没有找到
	if (iter == m_allVisitorNodeMap.end())
	{
		return;
	}
	tempNode = iter->second;


	m_allVisitorNodeMap.erase(iter);

	if (tempNode != NULL && tempNode->data()._level >= 0)
	{
		//从坐席列表底下删除 然后加入等待列表
		pUserList->RemoveNode(tempNode);
	}


	CWebUserObject *webuser = pWebUser;
	webuser->onlineinfo.talkstatus = TALKSTATUS_REQUEST;
	RecvChatInfo(pWebUser, NULL);



	//从 会话中 删除 当前访客 信息
	map<unsigned long, unsigned long>::iterator iterLong = m_allVisitorUserMap.find(pWebUser->webuserid);
	if (iterLong == m_allVisitorUserMap.end())
		return;
	m_allVisitorUserMap.erase(iterLong);

}



//收到邀请协助的 申请  邀请协助的 回调函数
void CMainFrame::RecvInviteUser(CWebUserObject* pWebUser, CUserObject* pUser)
{
	//CUserObject	*pUser = m_manager->GetUserObjectByUid(9692111);
	//CWebUserObject *pWebUser = m_manager->GetWebUserObjectByUid(m_curSelectId);
	int type = -1;

	if (pUser == NULL || pWebUser == NULL)
		return;

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
	if (iter == m_allVisitorNodeMap.end())
	{
		return;
	}

	UserListUI::Node *tempNode = iter->second;
	CDuiString text = L"";
	unsigned long id = 0;
	if (tempNode != NULL)
	{
		text = tempNode->data()._text;
		id = tempNode->data()._uid;
	}

	//先在用户的对话列表中删除 
	if (tempNode != NULL && tempNode->data()._level >= 0)
		pUserList->RemoveNode(tempNode);
	m_allVisitorNodeMap.erase(iter);


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

	UserListUI::Node*currentNode = pUserList->AddNode(text, id, ChildNode);

	pUserList->ExpandNode(ChildNode, true);
	m_allVisitorNodeMap.insert(pair<unsigned long, UserListUI::Node*>(id, currentNode));


}

//邀请协助 做的最后一次 回调

void CMainFrame::ResultInviteUser(CWebUserObject* pWebUser, CUserObject* pUser, RESULT_STATUS status)
{

	map<unsigned long, UserListUI::Node*>::iterator iter = m_allVisitorNodeMap.find(pWebUser->webuserid);
	//没有找到
	if (iter == m_allVisitorNodeMap.end())
	{
		return;
	}


	if (status == INVITE_ACCEPT)
	{
		UserListUI::Node *tempNode = iter->second;

		//先在用户的对话列表中删除 
		if (tempNode != NULL && tempNode->data()._level >= 0)
			pUserList->RemoveNode(tempNode);

		m_allVisitorNodeMap.erase(iter);


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

		if (pUser->UserInfo.uid != m_manager->m_userInfo.UserInfo.uid)
			m_activeList.push_back(pWebUser->webuserid);

		UserListUI::Node *tempChildNode = pMySelfeNode->child(0);

		UserListUI::Node * addNode = pUserList->AddNode(text, pWebUser->webuserid, tempChildNode);
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
void CMainFrame::RecvTransferUser(CWebUserObject* pWebUser, CUserObject* pUser)
{

	int type = -1;

	if (pUser == NULL || pWebUser == NULL)
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
	if (iter == m_allVisitorNodeMap.end())
	{
		return;
	}

	UserListUI::Node *tempNode = iter->second;
	CDuiString text = L"";
	unsigned long id = 0;
	if (tempNode != NULL)
	{
		text = tempNode->data()._text;
		id = tempNode->data()._uid;
	}

	//先在用户的对话列表中删除 
	if (tempNode != NULL && tempNode->data()._level >= 0)
		pUserList->RemoveNode(tempNode);

	m_allVisitorNodeMap.erase(iter);


	//然后显示 在转接列表中
	UserListUI::Node* ChildNode = pMySelfeNode->child(1);


	UserListUI::Node*currentNode = pUserList->AddNode(text, id, ChildNode);

	pUserList->ExpandNode(ChildNode, true);
	m_allVisitorNodeMap.insert(pair<unsigned long, UserListUI::Node*>(id, currentNode));


}


//邀请转接的 最后一次 回调      
void CMainFrame::ResultTransferUser(CWebUserObject* pWebUser, CUserObject* pUser, RESULT_STATUS status)
{

	map<unsigned long, UserListUI::Node*>::iterator iter = m_allVisitorNodeMap.find(pWebUser->webuserid);
	//没有找到
	if (iter == m_allVisitorNodeMap.end())
	{
		return;
	}

	if (status == INVITE_ACCEPT)
	{
		UserListUI::Node *tempNode = iter->second;

		CDuiString text = tempNode->data()._text;

		//先在用户的对话列表中删除 
		if (tempNode != NULL && tempNode->data()._level >= 0)
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

		UserListUI::Node * addNode = pUserList->AddNode(text, pWebUser->webuserid, tempChildNode);
		m_allVisitorNodeMap.insert(pair<unsigned long, UserListUI::Node*>(pWebUser->webuserid, addNode));

		pUserList->ExpandNode(tempChildNode, true);

	}

	else  if (status == INVITE_REFUSE)
	{



	}







}




void CMainFrame::RecvWebUserInInvite(CWebUserObject* pWebUser, CUserObject* pInviteUser)
{

}

/********************  回调接口的处理   end    ***********    ********************************************************************************************************************************/



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

					list<unsigned long>::iterator iterList = m_activeList.begin();
					for (; iterList != m_activeList.end(); iterList++)
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
	if (id > 0)
	{
		if (m_curSelectId != id || type == -1)//切换聊天对象显示 
		{

			ChangeShowUserMsgWnd(id);

			ShowRightOptionFrameView(id);
		}
		m_curSelectId = id;
	}

	//更新 上层的按钮状态
	UpdateTopCenterButtonState(id);

}

void CMainFrame::ResultInviteWebUser(CWebUserObject* pWebUser, bool bAgree)
{
	
}
