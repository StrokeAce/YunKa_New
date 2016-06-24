#ifndef _MAIN_FRAME_H_
#define _MAIN_FRAME_H

#pragma once
#include <map>
#include "small_menu.h"

#include "chat_manager.h"
#include "cef_browser/client_handler.h"

#include "rich_edit_util.h"
#include "IImageOle.h"
#include "face_list.h"
#include "face_sel_dlg.h"
#include "user_list.h"



#define MID_MANAGER_BUTTON_NUM    15
#define MAX_PATH_LENGTH           512


#define MSG_TYPE_SYS  3               //系统消息
/** 聊天用户的类型区分 */
#define  User_Type_Client 1 // 坐席用户
#define  User_Type_Wx 2	// 微信用户
#define  User_Type_Web 3 // 网页用户
#define  User_Type_WxGroup 4 // 微信群用户



typedef struct CONTROL_ATTR
{
	int centerFrameWitdh;
	int showMsgWidth;

}CONTROL_ATTR;


// 将HWND显示到CControlUI上面
class CWndUI : public CControlUI
{
public:
	CWndUI() : m_hWnd(NULL){}

	virtual void SetVisible(bool bVisible = true)
	{
		__super::SetVisible(bVisible);
		::ShowWindow(m_hWnd, bVisible);
		//SetBkImage(_T("E:\\WeiBo_3\\client\\cppprj\\common\\media\\bin\\debug\\SkinRes\\videobg.bmp"));
	}

	virtual void SetInternVisible(bool bVisible = true)
	{
		__super::SetInternVisible(bVisible);
		::ShowWindow(m_hWnd, bVisible);
	}

	void SetPos(RECT rc)
	{
		__super::SetPos(rc);
		::SetWindowPos(m_hWnd, NULL, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOZORDER | SWP_NOACTIVATE);
	}

	BOOL Attach(HWND hWndNew)
	{
		if (!::IsWindow(hWndNew))
		{
			return FALSE;
		}

		m_hWnd = hWndNew;
		return TRUE;
	}

	HWND Detach()
	{
		HWND hWnd = m_hWnd;
		m_hWnd = NULL;
		return hWnd;
	}

	HWND GetHWND()
	{
		return m_hWnd;
	}

public:
	HWND m_hWnd;
};


typedef struct ManagerButtonStruct
{
	CButtonUI *m_pManagerBtn;
	int   m_buttonState;
	WCHAR hotImage[MAX_PATH_LENGTH];
	WCHAR pushedImage[MAX_PATH_LENGTH];
	WCHAR normalImage[MAX_PATH_LENGTH];

}ManagerButtonStruct;


class CMainFrame : public WindowImplBase, public IHandlerMsgs
{
public:

	CMainFrame(CChatManager * manager);
	~CMainFrame();

public:

	LPCTSTR GetWindowClassName() const;
	virtual void OnFinalMessage(HWND hWnd);
	virtual LRESULT ResponseDefaultKeyEvent(WPARAM wParam);
	virtual CDuiString GetSkinFile();
	virtual CDuiString GetSkinFolder();
	virtual UILIB_RESOURCETYPE GetResourceType() const;
	virtual CControlUI* CreateControl(LPCTSTR pstrClass);
	virtual LRESULT OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);




	LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam);

	DWORD GetBkColor();
	void SetBkColor(DWORD dwBackColor);

	//virtual LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual void OnClick(TNotifyUI& msg);
	virtual void OnSelectChanged(TNotifyUI &msg);
	virtual void OnItemClick(TNotifyUI &msg);
	virtual void OnItemRbClick(TNotifyUI &msg);
	virtual void OnTimer(TNotifyUI &msg);
	virtual void OnHeaderClick(TNotifyUI& msg);
	virtual void OnMouseEnter(TNotifyUI& msg);
	virtual void OnItemSelect(TNotifyUI &msg);


	void OnItemActive(TNotifyUI &msg);

public:    //主界面消息回调
	// 收到坐席列表
	virtual void RecvShareListCount(int len);

	// 收到一个坐席用户的信息,用来初始化坐席列表
	virtual void RecvUserInfo(CUserObject* pWebUser);

	// 收到一个会话消息
	virtual void RecvChatInfo(CWebUserObject* pWebUser, CUserObject* pUser);

	// 收到更新用户的在线状态
	virtual void RecvUserStatus(CUserObject* pUser);

	// 坐席上线消息
	virtual void RecvOnline(IBaseObject* pObj);

	// 坐席下线消息
	virtual void RecvOffline(IBaseObject* pObj);

	virtual void RecvAcceptChat(CUserObject* pUser, CWebUserObject* pWebUser);

	virtual void RecvCloseChat(CWebUserObject* pWebUser);

	virtual void RecvReleaseChat(CWebUserObject* pWebUser);

	virtual void RecvMsg(IBaseObject* pObj, MSG_FROM_TYPE msgFrom, string msgId, MSG_TYPE msgType, MSG_DATA_TYPE msgDataType, string msgContent,
		string msgTime, CUserObject* pAssistUser, WxMsgBase* msgContentWx);

	virtual void ResultRecvMsg(string msgId, bool bSuccess, string url, unsigned long msgFromUserId,
		unsigned long assistUserId, string filePath, MSG_FROM_TYPE msgFromType, MSG_DATA_TYPE msgDataType);

	virtual void ResultSendMsg(string msgId, bool bSuccess = true, unsigned long userId = 0, MSG_RECV_TYPE recvUserType = MSG_RECV_WX,
		MSG_DATA_TYPE msgDataType = MSG_DATA_TYPE_IMAGE, string msg = "");

	virtual void ResultScreenCapture(string imagePath);

	virtual void RecvInviteUser(CWebUserObject* pWebUser, CUserObject* pUser);

	virtual void ResultInviteUser(CWebUserObject* pWebUser, CUserObject* pUser, bool bSuccess);

	virtual void RecvTransferUser(CWebUserObject* pWebUser, CUserObject* pUser);

	virtual void ResultTransferUser(CWebUserObject* pWebUser, CUserObject* pUser, bool bSuccess);

	virtual void RecvOnlineUsers(CGroupObject* pGroup);

public:
	//自己定义的操作函数
	void OnBtnFont(TNotifyUI& msg);
	void OnBtnFace(TNotifyUI& msg);
	void OnBtnScreen(TNotifyUI& msg);
	void OnBtnVoice(TNotifyUI& msg);
	void OnManagerButtonEvent(TNotifyUI& msg);
	void OnBtnSendFile(TNotifyUI& msg);
	void OnFaceCtrlSel(UINT uMsg, WPARAM wParam, LPARAM lParam);
	void OnBtnSendMessage(TNotifyUI& msg);

	BOOL _RichEdit_InsertFace(CRichEditUI * pRichEdit, LPCTSTR lpszFileName, int nFaceId, int nFaceIndex);

//操作树列表的操作
	void SendMsgToGetList();
	void AddHostUserList(UserListUI * ptr, CUserObject *user);
	void AddHostUserList(UserListUI * ptr, CUserObject *user, int pos);
	void AddOnlineVisitor(UserListUI * ptr, CUserObject *user, int index);
	void AddMyselfToList(UserListUI * ptr, CUserObject *user);



//接入聊天 相关处理
	void OnSendToAcceptChat(unsigned long webUserid);
	void OnSendToReleaseChat(unsigned long webUserid);
	void OnSendToCloseChat(unsigned long webUserid);
	void CMainFrame::OnMenuEvent(CDuiString controlName);
	void CMainFrame::OnCtrlVEvent();
	bool CMainFrame::SaveBitmapToFile(HBITMAP hbitmap, BITMAP bitmap, string lpFileName);

	void CMainFrame::UpdateTopCenterButtonState(unsigned long id);

//	void CMainFrame::ReplaceFaceId(string &msg);
	void CMainFrame::ShowMySelfSendMsg(string strMsg, MSG_DATA_TYPE msgType, string msgId);
	void CMainFrame::MoveAndRestoreMsgWnd(int type);
	void CMainFrame::InitLibcef(void);
	void CMainFrame::LoadBrowser(char* url);
	void CMainFrame::ShowRightOptionFrameView(unsigned long id);
	void CMainFrame::ShowOptionTab1FrameView();
	void CMainFrame::ShowClearMsg();
	void CMainFrame::ChangeShowUserMsgWnd(unsigned long id);
	void CMainFrame::SetHandler();
	MSG_RECV_TYPE  CMainFrame::GetSendUserType(unsigned long id);
	void CMainFrame::CheckIdForUerOrWebuser(unsigned long id, CWebUserObject **pWebUser, CUserObject **pUser);
	string CMainFrame::CreateClientInfoHtml(WxUserInfo* pWxUser);
	void CMainFrame::MoveAndRestoreRightFrameControl(int type); //0 max 1 retore
	void CMainFrame::InitRightTalkList();
	void CMainFrame::OnBtnSelectSendType(TNotifyUI& msg);
	void JsCallMFC(WPARAM wParam, LPARAM lParam);
	void CMainFrame::ShowWebBrowser(char *url);
	void CMainFrame::HideWebBrowser();

	CODE_RECORD_AUDIO StartRecordAudio();

	void CancelRecordAudio();

protected:

	void Notify(TNotifyUI& msg);
	void OnPrepare(TNotifyUI& msg);
	void OnCloseBtn(TNotifyUI& msg);
	void OnMaxBtn(TNotifyUI& msg);
	void OnRestoreBtn(TNotifyUI& msg);
	void OnMinBtn(TNotifyUI& msg);





public:
	CChatManager* m_manager;
    int	m_currentNumber;
	map<unsigned long, UserListUI::Node*> m_onlineNodeMap;
	map<unsigned long, UserListUI::Node*> m_offlineNodeMap;
	UserListUI::Node* pOnlineNode ;
	UserListUI::Node* pWaitForStart;
	UserListUI::Node* pWaitForAccept;
	UserListUI::Node* pMySelfeNode;
	map<unsigned long, UserListUI::Node*> m_waitVizitorMap;
	map<unsigned long, UserListUI::Node*> m_allVisitorNodeMap;


	map<unsigned long, unsigned long > m_allVisitorUserMap;


	UserListUI::Node* m_pLastOfflineNode;
	UserListUI::Node* m_pLastOnlineNode;

	list<CUserObject* > m_upUser;
	unsigned int m_recordWaitNumber;

	CONTROL_ATTR m_centerChatInfo;

	RECT m_rightRectWnd;
	RECT m_rightRectMax;

	string m_defaultUrlInfo;
	string		m_audioPath;			// 正在录音文件的路径
	bool		m_bRecording;			// 正在录音


private:

	CSmallMenu m_frameSmallMenu;

	CEditUI *m_pRightCommonWordEdit, *m_pRightCommonTypeEdit , *m_pRightCommonFindEdit;
	CComboUI *m_pRightCommonWordCombo, *m_pRightCommonTypeCombo , *m_pRightCommonFindCombo;


	RECT m_mainCenterAndRightRect;
	CControlUI *m_MainCenterRightWND;
	CButtonUI * m_pFontBtn, *m_pFaceBtn, *m_pScreenBtn, *pSendMsgBtn,*m_pVoiceBtn;
	CFaceSelDlg m_faceSelDlg;
	CFaceList  m_faceList;


	ManagerButtonStruct m_pManagerBtn[MID_MANAGER_BUTTON_NUM];

	//CRichEditUI2    *m_pSendEdit;
	CRichEditUI* m_pSendEdit;


	HandlerInfo m_pListMsgHandler; // 消息列表
	HandlerInfo m_pWebURLHandler; // 访客历史 访客来电 访客留言 客户管理 统计分析 
	HandlerInfo m_pVisitorRelatedHandler;;

	CDuiString m_sendMsgString;
	HWND m_hMainWnd;

	UserListUI* pUserList;
	UserListUI* m_pTalkList;
	CUserObject* m_mySelfInfo;

	unsigned long m_curSelectId;

	unsigned long m_savedClickId;

	string m_facePathUrl;
	int  m_curSelectOptionBtn;
	unsigned long m_savedImageIndex;



};















#endif