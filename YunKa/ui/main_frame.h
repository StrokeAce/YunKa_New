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
#include "show_big_image_dlg.h"


#define MID_MANAGER_BUTTON_NUM    15
#define MAX_PATH_LENGTH           512


#define MSG_TYPE_SYS  3               //ϵͳ��Ϣ
/** �����û����������� */
#define  User_Type_Client 1 // ��ϯ�û�
#define  User_Type_Wx 2	// ΢���û�
#define  User_Type_Web 3 // ��ҳ�û�
#define  User_Type_WxGroup 4 // ΢��Ⱥ�û�



typedef struct CONTROL_ATTR
{
	int centerFrameWitdh;
	int showMsgWidth;

}CONTROL_ATTR;


// ��HWND��ʾ��CControlUI����
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

public:    //��������Ϣ�ص�
	// �յ���ϯ�б�
	virtual void RecvShareListCount(int len);

	// �յ�һ����ϯ�û�����Ϣ,������ʼ����ϯ�б�
	virtual void RecvUserInfo(CUserObject* pWebUser);

	// �յ�һ���Ự��Ϣ
	virtual void RecvChatInfo(CWebUserObject* pWebUser, CUserObject* pUser);

	// �յ������û�������״̬
	virtual void RecvUserStatus(CUserObject* pUser);

	// ��ϯ������Ϣ
	virtual void RecvOnline(IBaseObject* pObj);

	// ��ϯ������Ϣ
	virtual void RecvOffline(IBaseObject* pObj);

	virtual void RecvAcceptChat(CWebUserObject* pWebUser, CUserObject* pUser);

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

	virtual void ResultInviteUser(CWebUserObject* pWebUser, CUserObject* pUser, RESULT_STATUS status);

	virtual void RecvTransferUser(CWebUserObject* pWebUser, CUserObject* pUser);

	virtual void ResultTransferUser(CWebUserObject* pWebUser, CUserObject* pUser, RESULT_STATUS status);

	virtual void RecvOnlineUsers(CGroupObject* pGroup);

	virtual void RecvWebUserInfo(CWebUserObject* pWebUser, int updateNum=0);

	virtual void RecvWebUserInInvite(CWebUserObject* pWebUser, CUserObject* pInviteUser);

	virtual void ResultInviteWebUser(CWebUserObject* pWebUser, bool bAgree);

public:
	//�Լ�����Ĳ�������
	void OnBtnFont(TNotifyUI& msg);
	void OnBtnFace(TNotifyUI& msg);
	void OnBtnScreen(TNotifyUI& msg);
	void OnBtnVoice(TNotifyUI& msg);
	void OnManagerButtonEvent(TNotifyUI& msg);
	void OnBtnSendFile(TNotifyUI& msg);
	void OnFaceCtrlSel(UINT uMsg, WPARAM wParam, LPARAM lParam);
	void OnBtnSendMessage(TNotifyUI& msg);

	BOOL _RichEdit_InsertFace(CRichEditUI * pRichEdit, LPCTSTR lpszFileName, int nFaceId, int nFaceIndex);

//�������б�Ĳ���
	void SendMsgToGetList();
	void AddHostUserList(UserListUI * ptr, CUserObject *user);
	void AddHostUserList(UserListUI * ptr, CUserObject *user, int pos);
	void AddOnlineVisitor(UserListUI * ptr, CUserObject *user, int index);
	void AddMyselfToList(UserListUI * ptr, CUserObject *user);
	UserListUI::Node* CMainFrame::GetOneUserNode(unsigned long id);
	void CMainFrame::DeleteOneUserNode(unsigned long id);
	void CMainFrame::OnItemClickEvent(unsigned long id,int type);

	void CMainFrame::HostUserOnlineAndOffline(CUserObject* pUser, bool type);
	void CMainFrame::VisitorUserOnlineAndOffline(CWebUserObject* pWebUser, bool type);
	void CMainFrame::FindVisitorFromOnlineNode(CWebUserObject* pWebUser);
	BOOL CMainFrame::CheckItemForOnlineVisitor(UserListUI::Node *curNode);
	void CMainFrame::ShowBigImage(string url, MSG_DATA_TYPE msgDataType);

	void CMainFrame::AcceptChat();
	void CMainFrame::RefuseChat();

	//�ж���ǰ���û�id ��������״̬����
	TREENODEENUM  CMainFrame::CheckIdForNodeType(unsigned long id);
	VISITOR_TYPE  CMainFrame::CheckIdForTalkType(unsigned long id);

//�������� ��ش���
	void CMainFrame::OnMenuEvent(CDuiString controlName);
	void CMainFrame::OnCtrlVEvent();
	bool CMainFrame::SaveBitmapToFile(HBITMAP hbitmap, BITMAP bitmap, string lpFileName);

	void CMainFrame::UpdateTopCenterButtonState(unsigned long id);
	void CMainFrame::OnSelectUser(unsigned long id);
	void CMainFrame::OnActiveUser(unsigned long id,string sid);

//	void CMainFrame::ReplaceFaceId(string &msg);
	void CMainFrame::ShowMySelfSendMsg(string strMsg, MSG_DATA_TYPE msgType, string msgId);
	void CMainFrame::MoveAndRestoreMsgWnd(int type);
	void CMainFrame::InitLibcef(void);
	void CMainFrame::LoadBrowser(char* url);
	void CMainFrame::ShowRightOptionFrameView(unsigned long id,string sid);
	void CMainFrame::ShowClearMsg();
	void CMainFrame::ChangeShowUserMsgWnd(unsigned long id);
	void CMainFrame::SetHandler();
	MSG_RECV_TYPE  CMainFrame::GetSendUserType(unsigned long id);
	void CMainFrame::CheckIdForUerOrWebuser(unsigned long id,string sid, CWebUserObject **pWebUser, CUserObject **pUser);
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
	map<unsigned long, UserListUI::Node*> m_onlineNodeMap; //������ϯ
	map<unsigned long, UserListUI::Node*> m_offlineNodeMap; //������ϯ
	UserListUI::Node* pOnlineNode ;   //���߷ÿ� 
	UserListUI::Node* pWaitForStart;  //�ȴ���ʼ
	UserListUI::Node* pWaitForAccept;  //�ȴ�Ӧ��
	UserListUI::Node* pMySelfeNode;    //�Լ���node
	map<unsigned long, UserListUI::Node*> m_waitVizitorMap;  //�ȴ��б�
	map<unsigned long, UserListUI::Node*> m_allVisitorNodeMap;  //���зÿ��б�
	list<unsigned long>m_activeList;     //���Լ���������Э���� id

	map<unsigned long, unsigned long > m_allVisitorUserMap;   //���зÿ͵Ĺ����б�
	list<unsigned long >m_acceptingsUserList;                 //�����б�
	list<unsigned long >m_transferUserList;                 //�����б�


	UserListUI::Node* m_pLastOfflineNode;
	UserListUI::Node* m_pLastOnlineNode;

	map<string, UserListUI::Node*> m_visitorOnlineNode; //���߷ÿ� 

	list<CUserObject* > m_upUser;
	unsigned int m_recordWaitNumber;

	CONTROL_ATTR m_centerChatInfo;

	RECT m_rightRectWnd;
	RECT m_rightRectMax;

	string m_defaultUrlInfo;
	string		m_audioPath;			// ����¼���ļ���·��
	bool		m_bRecording;			// ����¼��


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


	HandlerInfo m_pListMsgHandler; // ��Ϣ�б�
	HandlerInfo m_pWebURLHandler; // �ÿ���ʷ �ÿ����� �ÿ����� �ͻ����� ͳ�Ʒ��� 
	HandlerInfo m_pVisitorRelatedHandler;;


	//HandlerInfo m_pShowImageHandler; // ��Ϣ�б�

	CDuiString m_sendMsgString;
	HWND m_hMainWnd;

	UserListUI* pUserList;
	UserListUI* m_pTalkList;

	unsigned long m_curSelectId;


	string m_facePathUrl;
	int  m_curSelectOptionBtn;
	unsigned long m_savedImageIndex;

	int m_userListCount;
	int m_recordListCount;

	unsigned long m_selectUserId;


	int  m_topWndType; //��ǰѡ������ַ�ʽ  ���������б�ѡ�� ��������




	UserListUI::Node*  m_curClickItemNode;
	string m_curSavedSid;
	unsigned long m_savedClickId;
	CUserObject *m_recvUserObj;

	CShowBigImageDlg *pShowImgDlg;
	//CShowBigImageDlg m_pShowImgDlg;
};















#endif