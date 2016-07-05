#pragma once

#include "login.h"
#include "timer.h"
#include "chat_visitor.h"
#include "../chat_common/comobject.h"
#include "../chat_common/comstruct.h"
#include "../chat_common/comfloat.h"
#include "../chat_common/comenum.h"
#include "../chat_common/markup.h"

typedef map<string/*thirdId*/, string/*���ں�token*/> MapWxTokens; // ���ںŵ�thirdid��tokenһһ��Ӧ
typedef map<unsigned long, CUserObject*> MapUsers; // ������ϯ�û�
typedef map<string, CWebUserObject*> MapWebUsers; // ����ÿ�

// ��¼��Ϣ�Ļص��ӿ�
class IHandlerLgoin
{
public:
	// ��¼�Ľ���,percent=100ʱ��ʾ��¼�ɹ�
	virtual void LoginProgress(int percent) = 0;
};

// ��¼��ͨ����Ϣ�Ļص��ӿ�
class IHandlerMsgs
{
public:
	// �յ���ϯ�б�
	virtual void RecvShareListCount(int len) = 0;
	
	// �յ�һ����ϯ�û�����Ϣ,������ʼ����ϯ�б�
	virtual void RecvUserInfo(CUserObject* pWebUser) = 0;	

	// �յ�һ���Ự��Ϣ
	virtual void RecvChatInfo(CWebUserObject* pWebUser,CUserObject* pUser=NULL) = 0;

	// �յ������û�������״̬
	virtual void RecvUserStatus(CUserObject* pUser) = 0;

	// ��ϯ������Ϣ
	virtual void RecvOnline(IBaseObject* pObj) = 0;

	// ��ϯ������Ϣ
	virtual void RecvOffline(IBaseObject* pObj) = 0;

	//************************************
	// Method:    RecvAcceptChat
	// Qualifier: ��ϯ���ܻỰ��֪ͨ��Ϣ
	// Parameter: pUser ���ܻỰ����ϯ����
	// Parameter: pWebUser �����ܵķÿͶ���
	//************************************
	virtual void RecvAcceptChat(CWebUserObject* pWebUser, CUserObject* pUser) = 0;

	//************************************
	// Method:    RecvCloseChat
	// Qualifier: �Ự�ر�֪ͨ��Ϣ
	// Parameter: pWebUser �Ự�ر���صķÿ�
	//************************************
	virtual void RecvCloseChat(CWebUserObject* pWebUser) = 0;

	//************************************
	// Method:    RecvReleaseChat
	// Qualifier: �ͷŻỰ
	// Parameter: pWebUser ���ͷŵķÿ�
	//************************************
	virtual void RecvReleaseChat(CWebUserObject* pWebUser) = 0;

	//************************************
	// Method:    RecvMsg
	// Qualifier: �յ�һ����Ϣ
	// Parameter: pObj ����Ķ���������ϯ��web�ÿ͡�΢�ŷÿ�
	// Parameter: msgFrom ��Ϣ�ķ��������ͣ�������ϯ���ÿ�(΢�Ż�web)��Э������(��һ����ϯ)
	// Parameter: msgId ��Ϣ��Ψһid
	// Parameter: msgType ��Ϣ���ͣ�Ԥ֪��Ϣ(��Ҫ���web�ÿ�)����ͨ��Ϣ
	// Parameter: msgDataType ��Ϣ���������ͣ�����(��������)��������ͼƬ�����ꡢ��Ƶ��
	// Parameter: msgContent ��Ϣ�ľ�������
	// Parameter: msgTime �յ���Ϣ��ʱ��
	// Parameter: pAssistUser Э�����󣬵���ϢΪЭ��������ʱ����Ҫ�ò���
	// Parameter: msgContentWx ΢����Ϣ���������ֵ�΢����Ϣʱ����Ҫ�ò���
	// Parameter: bSuccess �Ƿ�ɹ�������Ϣ
	//************************************
	virtual void RecvMsg(IBaseObject* pObj, MSG_FROM_TYPE msgFrom, string msgId, MSG_TYPE msgType, MSG_DATA_TYPE msgDataType, string msgContent,
		string msgTime = "", CUserObject* pAssistUser = NULL, WxMsgBase* msgContentWx = NULL) = 0;

	//************************************
	// Method:    ResultRecvMsg
	// Qualifier: ����һ��������ͼƬ��ý���ļ���Ϣ�Ľ��
	// Parameter: msgId ��Ϣid
	// Parameter: bSuccess �Ƿ���ճɹ�
	//************************************
	virtual void ResultRecvMsg(string msgId, bool bSuccess, string url, unsigned long msgFromUserId,
		unsigned long assistUserId, string filePath, MSG_FROM_TYPE msgFromType, MSG_DATA_TYPE msgDataType) = 0;

	//************************************
	// Method:    ResultSendMsg
	// Qualifier: ����һ����Ϣ�Ľ��
	// Parameter: msgId ��Ϣid
	// Parameter: bSuccess �Ƿ��ͳɹ�
	//************************************
	virtual void ResultSendMsg(string msgId, bool bSuccess = true,unsigned long userId=0,MSG_RECV_TYPE userType= MSG_RECV_WX,
								MSG_DATA_TYPE msgDataType= MSG_DATA_TYPE_IMAGE, string msg="") = 0;

	//************************************
	// Method:    ResultScreenCapture
	// Qualifier: ��ͼ�Ľ��
	// Parameter: imagePath ��ͼ�ı���λ�ã�����Ϊ��
	//************************************
	virtual void ResultScreenCapture(string imagePath) = 0;

	//************************************
	// Method:    RecvInviteUser
	// Qualifier: �յ�����Э��������
	// Parameter: pWebUser ����Э���Ự������ÿ�
	// Parameter: pUser �������Э����
	//************************************
	virtual void RecvInviteUser(CWebUserObject* pWebUser, CUserObject* pUser) = 0;

	//************************************
	// Method:    ResultInviteUser
	// Qualifier: ����Э���Ľ��
	// Parameter: pWebUser ����Э���Ự������ÿ�
	// Parameter: pUser �������Э����
	// Parameter: bSuccess true ��������,false �ܾ�����
	//************************************
	virtual void ResultInviteUser(CWebUserObject* pWebUser, CUserObject* pUser, RESULT_STATUS status) = 0;

	//************************************
	// Method:    RecvTransferUser
	// Qualifier: �յ�����ת�ӵ�����
	// Parameter: pWebUser ת�ӻỰ�е�����ÿ�
	// Parameter: pUser �������ת����
	//************************************
	virtual void RecvTransferUser(CWebUserObject* pWebUser, CUserObject* pUser) = 0;

	// �յ�������ϯ��Ϣ
	virtual void RecvOnlineUsers(CGroupObject* pGroup) = 0;

	// �յ��ÿ���Ϣ
	virtual void RecvWebUserInfo(CWebUserObject* pWebUser) = 0;

	// �յ���ϯ�������е���Ϣ
	virtual void RecvWebUserInInvite(CWebUserObject* pWebUser, CUserObject* pInviteUser) = 0;

	virtual void ResultInviteWebUser(CWebUserObject* pWebUser, bool bAgree) = 0;
};

class CChatManager : public IBaseReceive
{
public:

	~CChatManager();

	// ��ȡͨ�Ź�����ʵ��������ģʽ
	static CChatManager* GetInstance();

	// ���ý��յ�¼��Ϣ�Ľӿ�
	void SetHandlerLogin(IHandlerLgoin* handlerLogin);

	// ���ý��յ�¼��ͨ����Ϣ�Ľӿ�
	void SetHandlerMsgs(IHandlerMsgs* handlerMsgs);

public:

	// ��ȡ��һ�ε�¼��Ϣ,�����߲��ù�������
	ListLoginedInfo GetPreLoginInfo();

	// ��ʼ��¼
	void StartLogin(string loginName, string password, bool isAutoLogin, bool isKeepPwd);

	// ��������״̬����Ϣ
	int SendTo_UpdateOnlineStatus(unsigned short status);

	// ���ͻ�ȡ���Ѷ����б����Ϣ
	int SendTo_GetShareList();

	// ���ͻ�ȡ�Ự�б����Ϣ
	int SendTo_GetListChatInfo();

	// ���ͻ�ȡ�����û�����Ϣ����Ϣ
	int SendTo_GetAllUserInfo();

	int StartLoginVisitor();

	// ���ͻ�ȡĳ����ϯ��Ϣ����Ϣ
	int SendTo_GetUserInfo(unsigned long uid);

	// ���ͻ�ȡĳ���ÿ���Ϣ����Ϣ
	int SendTo_GetWebUserInfo(unsigned long webuserid, const char *chatid, char *szMsg = "", unsigned int chatkefuid = 0);

	// ���ͻ�ȡĳ���Ự��Ϣ����Ϣ
	int SendTo_GetWebUserChatInfo(unsigned short gpid, unsigned long adminid, char *chatid);

	//************************************
	// Method:    SendTo_Msg
	// Qualifier: ����һ����Ϣ
	// Parameter: userId ������Ϣ�Ķ����id
	// Parameter: userType ���ն�������ͣ���ϯ��ÿ�
	// Parameter: msgId	��Ϣid
	// Parameter: string msgDataType ��������
	// Parameter: string msg ��������
	//************************************
	int SendTo_Msg(unsigned long userId, MSG_RECV_TYPE userType, string msgId, MSG_DATA_TYPE msgDataType, string msg);

	// ���½���һ����Ϣ
	int ReRecv_Msg(string url, MSG_FROM_TYPE msgFromUserType, string msgId, MSG_DATA_TYPE nMsgDataType,
		unsigned long msgFromUserId, unsigned long assistUserId, unsigned long time);

	// ������ܷÿͻỰ
	int SendTo_AcceptChat(unsigned long webuserid);

	// �����ͷŷÿͻỰ
	int SendTo_ReleaseChat(unsigned long webuserid);

	//************************************
	// Method:    SendTo_CloseChat
	// Qualifier: ����رջỰ
	// Parameter: webuserid �Ự�еķÿ͵�id
	// Parameter: ntype �Ự�رյ�ԭ�����磺CHATCLOSE_USER
	//************************************
	void SendTo_CloseChat(unsigned long webuserid, int ntype);

	//************************************
	// Method:    SendTo_InviteWebUser
	// Qualifier: ��������ÿͲ���Ự
	// Parameter: pWebUser �Ự�еķÿ�
	// Parameter: type �Ự�еķÿ�
	// Parameter: strText �Ự�еķÿ�
	//************************************
	int SendTo_InviteWebUser(CWebUserObject *pWebUser, int type, string strText);

	//************************************
	// Method:    SendTo_InviteUser
	// Qualifier: ��������������ϯ�ỰЭ��
	// Parameter: pWebUser �Ự�еķÿ�
	// Parameter: pUser �������ϯ
	//************************************
	int SendTo_InviteUser(CWebUserObject* pWebUser, CUserObject* pAcceptUser);

	//************************************
	// Method:    SendTo_InviteUserResult
	// Qualifier: �����Ƿ���ܸ���ϯ������Э��
	// Parameter: pWebUser �Ự�зÿ�
	// Parameter: pUser �������ϯ
	// Parameter: bAccept �Ƿ�ͬ��
	//************************************
	int SendTo_InviteUserResult(CWebUserObject* pWebUser, CUserObject* pUser, bool bAccept);

	// ����Ựת�ӵ�������ϯ������
	int SendTo_TransferRequestUser(CWebUserObject* pWebUser, CUserObject* pAcceptUser);

	int SendToTransferUser(CUserObject *pAcceptUser, CWebUserObject *pWebUser, unsigned long acceptuin = 0);

	int SendToRefuseChat(CWebUserObject *pWebUser, string strReason = "NO");

	//************************************
	// Method:    SendTo_InviteUserResult
	// Qualifier: �����Ƿ���ܸ���ϯ�ĻỰת��
	// Parameter: pWebUser �Ự�зÿ�
	// Parameter: pUser �������ϯ
	// Parameter: bAccept �Ƿ�ͬ��
	//************************************
	int SendTo_TransferUserResult(CWebUserObject* pWebUser, CUserObject* pUser, bool bAccept);

	int SendTo_GetOnlineUser();

	// ��ȡ��һ�δ�����Ϣ
	string GetLastError();

	// ��ͼ
	void ScreenCapture(HWND hWnd);

	// ���������Ự
	void RestartSession(LPARAM lParam);

	void FormatRequestUrl(string &strUrl,string strMsg);

	bool CChatManager::RepickChatCon(string url, string& strRet, unsigned long &uin, string &strErrMsg);
	
	// �˳��������˳�ʱ����
	void Exit();

private:
	CChatManager();
	friend class CLogin;
	friend class CMySocket;

	/***************     �̳нӿڵĺ���ʵ��    *****************/

	virtual void OnReceive(void* wParam, void* lParam);

	virtual void OnReceiveEvent(int wParam, int lParam);


	/***************     �����ļ���������     *****************/

	bool ReadSystemConfig();

	void SetSystemConfigByInitconfig();

	void SolveUserdefineFilter(FILTER_USERDEFINE &filter, char *sfilter);

	bool LoadINIResource();


	/***************     ��Ϣ��������      *****************/

	int RecvSrvConfLogon(PACK_HEADER packhead, char *pRecvBuff, int len);

	int RecvSrvRepUserinfo(PACK_HEADER packhead, char *pRecvBuff, int len);

	int RecvSrvDenyLogon(PACK_HEADER packhead, char *pRecvBuff, int len, int &errtype);

	int RecvSrvConfLogOff(PACK_HEADER packhead, char *pRecvBuff, int len);

	int RecvSrvStatusFrdOnline(PACK_HEADER packhead, char *pRecvBuff, int len);

	int RecvSrvStatusFrdOffline(PACK_HEADER packhead, char *pRecvBuff, int len);

	int RecvSrvStatusUserForm(PACK_HEADER packhead, char *pRecvBuff, int len);

	int RecvFloatShareList(PACK_HEADER packhead, char *pRecvBuff, int len);

	int RecvComSendMsg(PACK_HEADER packhead, char *pRecvBuff, int len);

	int RecvFloatCreateChat(PACK_HEADER packhead, char *pRecvBuff, int len);

	int RecvFloatChatMsg(PACK_HEADER packhead, char *pRecvBuff, int len);

	int RecvFloatChatMsgAck(PACK_HEADER packhead, char *pRecvBuff, int len);

	int RecvFloatAcceptChat(PACK_HEADER packhead, char *pRecvBuff, int len);

	int RecvFloatTransQuest(PACK_HEADER packhead, char *pRecvBuff, int len);

	int RecvFloatTransFailed(PACK_HEADER packhead, char *pRecvBuff, int len);

	int RecvInviteRequest(PACK_HEADER packhead, char *pRecvBuff, int len);

	int RecvInviteResult(PACK_HEADER packhead, char *pRecvBuff, int len);

	int RecvFloatRelease(PACK_HEADER packhead, char *pRecvBuff, int len);

	int RecvFloatCMDError(PACK_HEADER packhead, char *pRecvBuff, int len);

	int RecvFloatCloseChat(PACK_HEADER packhead, char *pRecvBuff, int len);

	int RecvFloatListChat(PACK_HEADER packhead, char *pRecvBuff, int len);

	int RecvFloatChatInfo(PACK_HEADER packhead, char *pRecvBuff, int len);

	int RecvEventAnnouncement(PACK_HEADER packhead, char *pRecvBuff, int len);

	int RecvSrvUpdateSucc(PACK_HEADER packhead, char *pRecvBuff, int len);

	int RecvSrvUpdateFail(PACK_HEADER packhead, char *pRecvBuff, int len);

	//�û�����ص�½
	int RecvSrvDown(PACK_HEADER packhead, char *pRecvBuff, int len);

	//ת����ʱ�û��ɹ�ʧ��
	int RecvRepTransferClient(PACK_HEADER packhead, char *pRecvBuff, int len);

	//����ת�ƻỰ���û�����
	int RecvTransferClient(PACK_HEADER packhead, char *pRecvBuff, int len);

	void RecvComSendNormalChatidHisMsg(unsigned long senduid, unsigned long recvuid, COM_SEND_MSG& RecvInfo);

	int RecvComTransRequest(unsigned long senduid, COM_SEND_MSG& RecvInfo);

	int RecvComTransAnswer(unsigned long senduid, COM_SEND_MSG& RecvInfo);

	void RecvComSendWorkBillMsg(unsigned long senduid, unsigned long recvuid, char *msg, char* mobile);


	/***************     ��ϯ�ͷÿ���Ϣ������      *****************/

	int UnPack(CPackInfo *pPackInfo, char *buff, int len);

	int SendPackTo(CPackInfo *pPackInfo, unsigned long recvuid = 0, unsigned long recvsock = 0, HWND hWnd = NULL);

public:
	CUserObject* GetMySelfUserInfo();

	CUserObject *AddUserObject(unsigned long id, char *sid, char *name, unsigned char status, unsigned long fd);

	CUserObject *GetUserObjectByUid(unsigned long id);

	CWebUserObject *GetWebUserObjectBySid(char *sid);

	CWebUserObject *GetWebUserObjectByUid(unsigned long uid);

	CWebUserObject *AddWebUserObject(char *sid, char *thirdid, char *name, char *scriptflag, char *url,
		unsigned char status, unsigned char floatauth);

	unsigned short GetPackSeq();	

	// ��ʱ��������������
	void TimerSolveAck();

	// ��ʱ���ӵ�¼visit������
	void TimerSolveLoginToVisitorServer();

	// ��ʱ��������ĳ�ʱ���
	void TimerSolveRequestTimerOut();

	int SendPing();

	void SetOfflineStatus();

	void CloseAllSocket();

	void SetAllUserOffline();

	void DeleteAllSrvInfo();

	void DeleteAllUserInfo();

	void DeleteAllWebUserInfo();

	/***************     ����ʹ�õķ��͵�����˵���Ϣ      *****************/

	int SendGetSelfInfo(unsigned long id, char *strid, unsigned short cmd, unsigned short cmdtype = 0, unsigned short type = 0);

	int SendAckEx(unsigned short seq, unsigned long uid = 0, unsigned long ip = 0, unsigned short port = 0);

	int SendAutoRespMsg(CWebUserObject *pWebUser, const char *msg, BOOL bClearTimer = true);

	//��ʼ���ܻỰ��Ϣ
	int SendStartRecvFloatMsg(unsigned short gpid, unsigned long adminid, char *chatid, unsigned short sLastMsgid);

	// ��ȡuserinfo��token
	void SendGetWxUserInfoAndToken(CWebUserObject* pWebUser);

	// ��ȡ΢���û���Ϣ
	int SendGetWxUserInfo(unsigned long webuserid, const char *chatid);

	// ��ȡ΢�Ź��ں�token
	int SendGetWxToken(unsigned long webuserid, const char *chatid);

	//************************************
	// Method:    SendMsg
	// Qualifier: ������Ϣ��
	// Parameter: pUser ������Ϣ�Ķ���
	// Parameter: msg ��Ϣ��������
	// Parameter: bak ��������
	// Parameter: sfont ��΢���û���ý����Ϣ��"JSON=WX"�������Ķ���"HTML"
	//************************************
	int SendMsg(IBaseObject* pUser, const char *msg, int bak = 0, char *sfont = "HTML");

	// �����Ϲ������Ϣ
	int SendComMsg(unsigned long recvuid, char *visitorid, const char *msg, char * chatid, char* thirdid, int bak = 0, char *sfontinfo = "");

	// �����¹������Ϣ����Ҫ���΢���û�
	int SendFloatMsg(CWebUserObject *pWebUser, const char *msg, char *sfont);

	int SendGetChatHisMsg(unsigned long webuserid, const char *chatid);//��ȡ�ǵȴ�Ӧ��Ự�ĻỰ��ʷ��Ϣ

	//************************************
	// Method:    SendTo_CloseChat
	// Qualifier: ����رջỰ
	// Parameter: pWebUser �Ự�еķÿ�
	// Parameter: ntype �Ự�رյ�ԭ�����磺CHATCLOSE_USER
	//************************************
	int SendCloseChat(CWebUserObject *pWebUser, int ntype);

	int SendLoginOff();

	void ClearDirectory(string dir);

	// ΢����Ϣ�Ľ���
	WxMsgBase* ParseWxMsg(CWebUserObject* pWebUser, char* msg, CUserObject* pAssistUser, unsigned long time);

	bool ParseTextMsg(CWebUserObject* pWebUser, string content, CUserObject* pAssistUser, unsigned long time);

	string GetMsgId();

	string GetFileId();

	void SaveEarlyMsg(MSG_INFO *pMsgInfo);

	void SolveWebUserEarlyMsg(CWebUserObject *pWebUser);

	CWebUserObject *ChangeWebUserSid(CWebUserObject *pWebUser, char *sid, char *thirdid);

	void GetInviteChatSysMsg(char* msg, CUserObject *pInviteUser, CWebUserObject *pWebUser, int type, CUserObject *pAcceptUser = NULL);

	void GetStopChatSysMsg(char* msg, CWebUserObject *pWebUser, int type = 0, CUserObject *pSendUser = NULL);

	// ��ʱ���¼�������
	static void CALLBACK TimerProc(int timeId, LPVOID pThis); 

	void LoginSuccess();

	CWebUserObject *GetWebUserObjectByScriptFlag(char *scriptflag);	

	void UpLoadFile(unsigned long userId, MSG_RECV_TYPE userType, string msgId, string filePath,
					MSG_DATA_TYPE = MSG_DATA_TYPE_IMAGE);

	void DownLoadFile(CWebUserObject *pWebUser, MSG_DATA_TYPE nMsgDataType, string url, CUserObject *pAssistUser, unsigned long time,string msgId);

	void AfterUpload(unsigned long userId, MSG_RECV_TYPE userType, string msgId, string mediaID = "",
					MSG_DATA_TYPE = MSG_DATA_TYPE_IMAGE, string fileId = "", string filePath = "",
					string wxToken = "");

	void TransferFaceToStr(string& msg, MSG_RECV_TYPE recvType);

	void TransferStrToFace(string& msg);

	void TransferStrToOldFace(string& msg);

	void TransferFaceToServeStr(string& msg);

	int GetFaceIndex(const char * faceStr);

	string GetFaceStr(int id, MSG_RECV_TYPE recvType);

	void Amr2Wav(string filePath);

	void AddMsgToList(IBaseObject* pObj, MSG_FROM_TYPE msgFrom, MSG_RECV_TYPE recvType,string msgId, MSG_TYPE msgType, MSG_DATA_TYPE msgDataType,
					string msgContent="", string msgTime = "", CUserObject* pAssistUser = NULL, WxMsgBase* msgContentWx = NULL,
					bool bSave = true,bool bNotify = true,bool bSuccess = true);

	string ReplaceToken(string srcStr, string replaceStr);

	void AddToken(WxUserInfo* userInfo,string token);

	static DWORD WINAPI GetOnlineUserThread(void *arg);

	void GetOnlineUser();

	bool ParseGroupUser(CMarkupXml &xml, CGroupObject *pGroupOb, char *sGroupKey, char *sUserKey);

	bool TokenIsDifferent(string oldToken, string newToken);

	static UINT WINAPI UpLoadFileToWxServerThread(void * pUpLoadInfo);

	static UINT WINAPI UpLoadFileToServerThread(void * pUpLoadInfo);

	static UINT WINAPI DownLoadFileFromServerThread(void * para);

	static DWORD WINAPI SendFileThread(void *arg);

	bool SendFileToUser(IBaseObject* pUser, string strPathFile, string msgId, MSG_RECV_TYPE userType);

public:
	int						m_nOnLineStatus;		// �Ƿ�����,����im����������
	int						m_nOnLineStatusEx;		// �Ƿ�����,����visit����������
	bool					m_bExit;				// �����Ƿ��˳�
	CUserObject				m_userInfo;				// ��¼�û�����Ϣ
	CSysConfigFile*			m_sysConfig;			// �û������ļ���
	INIT_CONF				m_initConfig;			// ��������ļ���
	CLogin*					m_login;				// ��¼������	
	IHandlerLgoin*			m_handlerLogin;			// ��¼��Ϣ���սӿ�
	IHandlerMsgs*			m_handlerMsgs;			// ͨ����Ϣ���սӿ�	
	CMySocket				m_socket;				// ��Ϣ����
	CChatVisitor*			m_vistor;				// �ÿͽӴ��������
	string					m_sLogin;				// ��¼��
	string					m_password;				// ��¼����
	string					m_server;				// ��������ַ
	string					m_lastError;			// ��һ�δ�����Ϣ
	bool					m_bAutoLogin;			// �Ƿ��Զ���¼
	bool					m_bKeepPwd;				// �Ƿ��ס����
	bool					m_bLoginSuccess;		// �Ƿ��ѵ�¼
	int						m_port;					// �������˿�		
	unsigned short			m_usSrvRand;			// �������������
	unsigned short			m_usCltRand;			// �������е������
	MapUsers				m_mapUsers;				// Э������Ĵ洢����
	MapWebUsers				m_mapWebUsers;			// �ÿ͵Ĵ洢����
	CUserObject				m_commUserInfo;			// �����û�
	CTimerManager*			m_timers;				// ��ʱ��������
	int						m_nMyInfoIsGet;			// �Ƿ��ҵ���Ϣ�Ѿ���ȡ����
	int						m_nSendPing;			// ���������͵Ĵ���
	int						m_nSendPingFail;		// ����������ʧ�ܴ��� 
	int						m_nLoginToVisitor;		// ���Ե�¼visit�������Ĵ���	 
	unsigned long			m_nNextInviteWebuserUid;// ����ķÿ���Ϣ
	unsigned long			m_nNextInviteUid;		// ����������û�
	MapWxTokens				m_mapTokens;			// ���ں�token�洢����
	MMutex					m_idLock;				// ������Ϣid����
	int						m_msgId;				// ��Ϣid������
	unsigned long			m_fileId;				// �ļ�id������
	list<MSG_INFO*>			m_listEarlyMsg;			// ���滹δ��ʼ���ÿͶ���֮ǰ�յ�����Ϣ
	int						m_nClientIndex;			// �ÿ͵����кţ�����
	HMODULE					m_hScreenDll;			// ��ͼ���
	HANDLE					m_hGetOnlineUserThread;	// ��ȡ������ϯ���߳�
	CGroupObject			m_groupUser;			// 
	HANDLE					m_sendFileThreadHandle;	// 
};

