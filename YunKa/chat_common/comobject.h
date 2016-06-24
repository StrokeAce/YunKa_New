#ifndef _COMOBJECT_EMC
#define _COMOBJECT_EMC

#include "comdef.h"
#include "string.h"
#include "comstruct.h"
#include "wx_obj.h"

#include <list>     
using namespace std;

typedef list<LOGIN_INFO*> ListLoginedInfo;
typedef list<ALERT_INFO*> ListAlertInfo;
typedef map<string, WEBUSER_URL_INFO*> MapWebUserFLag;

class IBaseReceive
{
public:
	virtual void OnReceive(void* wParam, void* lParam) = 0;
};

//��ʼ����
class CSysConfigFile
{
public:
	CSysConfigFile();
	~CSysConfigFile();

	void ResetValue();
	bool LoadData(char *sFilename);
	bool SaveData(char *sFilename=NULL);
	bool ReadFile(ifstream& fout);
	bool WriteFile(ofstream& fin);
	void SetWndInitPos(bool bAlways);
	void DeleteAll();
	void DeleteAllLoginInfo();
	void DeleteAllAlertInfo();

	ALERT_INFO *SetAllDefaultAlertInfo(int type = -1);
	ALERT_INFO *CSysConfigFile::GetAlertInfo(int type);

public:
	char m_sConfigname[MAX_FILENAME_LEN];

public:
	//ͨ����¼�Ż�õ�¼�ַ���
	string GestringFromUID(unsigned long uid);
	unsigned long GetUIDFromString(int type, string strID);

	//����һ���µĶ���
	LOGIN_INFO *AddLatestLoginInfo(unsigned long uid, string sid, string pass, bool bAutoLogin, bool bKeepPwd);

	//����һ����¼����������
	void AddServerAddress(string strServer, bool bMoveToTop = true);
	void CSysConfigFile::AddKeyWordSerchString(string strSearch, bool bMoveToTop);

	void CSysConfigFile::AddOrDeleteForbidWebuserSid(string sid, bool bAdd);
	bool CSysConfigFile::IsWebuserSidForbid(string sid);

	string CSysConfigFile::CombineFilterString();
	void CSysConfigFile::ParseFilterString(string strString);

	void Write(ofstream& fin, bool bVal);
	void Write(ofstream& fin, int iVal);
	void Write(ofstream& fin, unsigned long lVal);
	void Write(ofstream& fin, unsigned int Val);
	void Write(ofstream& fin, const char* sVal);
	void Write(ofstream& fin, byte byVal);

	void Read(ifstream& fout, bool& bVal);
	void Read(ifstream& fout, int& iVal);
	void Read(ifstream& fout, unsigned long& lVal);
	void Read(ifstream& fout, unsigned int& Val);
	void Read(ifstream& fout, string& sVal);
	void Read(ifstream& fout, byte& byVal);
	void Read(ifstream& fout, char* chVal);

	int					m_nKeySendType;				// �������ͣ�enter��ctrl+enter
	int					m_nX;						// �ͻ�����ʼx����
	int					m_nY;						// �ͻ�����ʼy����
	int					m_nWidth;					// �ͻ��˵Ŀ�
	int					m_nHeight;					// �ͻ��˵ĸ�
	int					m_nSendMsgHeight;			// ���Ϳ�ĸ߶�
	int					m_nPreSendMsgHeight;		// Ԥ����Ϣ��ĸ߶�
	int					m_nLastLoginBy;				// ����½������
	unsigned long		m_sLastLoginUid;			// ����½���û�uin
	string				m_sLastLoginStr;			// ����½���û���
	string				m_sLastLoginPass;			// ����½������
	string				m_sStrServer;				// ��½�ķ�������ַ
	int					m_nServerPort;				// ��½�ķ������˿�
	string				m_sVisitorServer;			// web�ÿͷ�������ַ
	int					m_nVisitorServerPort;		// web�ÿͷ������˿�
	string				m_sStrRealServer;			// ��ʵ��½�ķ�������ַ
	int					m_nRealServerPort;			// ��ʵ��½�ķ������˿�
	bool				m_bAutoSearchKeyword;
	ListLoginedInfo		m_cLoginInfoList;			// ��½���û����б�
	ListAlertInfo		m_cAlertInfoList;			// ��ʾ���б�
	list<string>		m_cServerAddressList;		// ��������ַ�б�
	list<string>		m_cKeyWordSearchList;		// �������Ĺؼ����б�
	list<string>		m_cForbidWebUserList;		// �����εķÿ͵ı���б�
	FILTER_USERDEFINE	m_cWebUserfilter;			// ���������б�
	int					m_nFilterType;				// Ĭ�Ϸÿ͹����������У�ֻ�Եȴ��Ӵ��������к����뿪��Ч
	bool				m_bAutoResp;				// �Զ�Ӧ�𿪹�
	string				m_sWellcomeMsg;				// ��ӭ��
	int					m_nUserTimeoutTime;			// �ͷ���ʱ��δӦ���Զ��ظ�ʱ��
	string				m_sUserTimeoutMsg;			// �ظ���
	int					m_nVisitorTimeoutTime;		// �ÿͳ�ʱ��δ��Ӧ�Զ��ظ�ʱ��
	string				m_sVisitorTimeoutMsg;		// �ظ���
	bool				m_bVisotorTimeoutClose;		// �Զ��رջỰ����
	int					m_nVisitorTimeoutCloseTime;	// �ÿͳ�ʱ��δ�ظ��Զ��رջỰʱ��
	string				m_sVisitorTimeoutCloseMsg;	// �ظ���
	bool				m_bAutoRespUnnormalStatus;	// �����������Զ�Ӧ�𿪹�
	string				m_sUnnormalStatusMsg;		// �ظ���
	string				m_sInviteWords;				// ������
	list<string>		m_cInviteWordsList;			// �������б�
	int					m_nInviteType;				// ��������
	int					m_nKeywordsSort;			// ����Ӧ�����������
	int					m_nKeywordsUser;			// ����Ӧ������������
	string				m_sKeywordsFind;			// ����Ӧ��������ؼ���
};

class IBaseObject
{
public:
	IBaseObject(){};
	~IBaseObject(){};

	int m_nEMObType;
};

class CUserObject : public IBaseObject
{
public:
	CUserObject();
	~CUserObject();

	void DownLoadFace(char* loadUrl);

public:
	USER_INFO UserInfo;
	unsigned char talkstatus;

	int	m_nWaitTimer;		//���룬 ��Ϣ���͵ȴ�ʱ�Ķ�ʱ����

	unsigned char status;
	char sstatus[MAX_STATUS_LEN];

	unsigned long talkuid;
	unsigned long fd;	//����socket���ӣ�������Ϣת��

	list<ONE_MSG_INFO> m_strMsgs;// ���������¼
	string m_strPreSendMsg;
	bool m_bConnected;

	bool m_bKeywordsChange;
	bool m_bKeywordsGet;
	string m_loadHeadUrl;
	string m_headPath;

public:
	int	nTimer;  //��ʱ��

	int nVisitNum;
	int nTalkNum;
	int nLastVisitTime;

	int nFlashFirstOnline;			//��һ��������˸��־ 0 ������1 -> ��ʼ��

	unsigned long nbak;
	string sbak;

	bool bVisitMeAfterLogon;		//���û���������û�н�����
	bool bTalkMeAfterLogon;			//���û���������û�н�����

	int m_nFlag;					//������ʱʹ�ñ���
	bool m_bOpen;
	bool m_bReset;

	bool m_bFriend;					//�Ƿ��кö���
	bool m_bInnerTalk;				//�Ƿ��ڲ��Ի�
};

class CWebUserObject : public IBaseObject
{
public:
	CWebUserObject();
	~CWebUserObject();

	void AddCommonTalkId(unsigned long uid);

	bool IsExistCommonTalkId(unsigned long uid);

	bool IsOnline();

	int IsForbid();

	bool IsDisplay(CSysConfigFile *pConfig, unsigned long uid = 0);

	void SetForbid(bool bForbid);

	bool IsIDIsMutiUser(unsigned long uid);
	void AddMutiUser(unsigned long uid);
	void RemoveMutiUser(unsigned long uid);
	bool IsMutiUser();
	void RemoveAllMutiUser();

	bool ScriptFlagIsExist(char *scriptflag);
	void AddScriptFlag(char *scriptflag, char *url);
	void DeleteScriptFlag(char *scriptflag);
	void DeleteAllScriptFlag();
	WEBUSER_URL_INFO* GetScriptFlagOb(char *scriptflag);
	WEBUSER_URL_INFO* GetLastScriptFlagOb();

public:
	unsigned char			m_bNewComm;					//��Э��
	unsigned short			m_sNewSeq;					//��Э��,��ǰ���ܵ���Ϣid
	int						m_nIndex;					//���ڱ�ʾ�û�����������
	WEBUSER_INFO			info;
	WEBONLINE_INFO			onlineinfo;
	WEBUSEREX_INFO			exinfo;
	unsigned long			webuserid;
	unsigned long			floatadminuid;				//Ư�������Ա����
	int						nlangtype;					//�ַ���
	unsigned long			floatfromadminuid;			//�Ự��Դ��վ����Ա
	unsigned short			floatfromsort;				//�Ự����Ư����id
	char					chatid[MAX_CHATID_LEN + 1];	//����id��Ψһ����һ�λỰ
	unsigned short			gpid;

	//������Ϣ�����û���һ��������ʹ��
	unsigned long 			talkuid;					//�Ի����û� ��һ�ζԻ��û�
	unsigned long 			transferuid;				//Ҫת�ӵ��û�
	unsigned long 			inviteuid;					//������û���Ĭ��ΪSYS_WEBUSER
	unsigned long 			frominviteuid;				//����������û���

	unsigned char			cTalkedSatus;				//0û��ͨ������HASTALKED 1  ͨ����,INTALKING	2 ����ͨ����
	MapWebUserFLag			m_mapUrlAndScriptFlagOb;	// �ж�web�û��Ƿ�����
	list<unsigned long>		m_listCommonTalkId;			// ��ͬ�Ի��������ͷ��û��б�
	unsigned char			m_bNotResponseUser;			//1 �Լ�����Ӧ��ͷ���0�Լ���Ӧ��ͷ�
	char					prevurl[MAX_URL_LEN];
	char					prevurlhost[MAX_URL_LEN];
	char					prevurlvar[MAX_URL_LEN];
	unsigned char			m_refuseinvite;				//�û��ܾ������룬 ���������Զ�����
	int						m_nWaitTimer;				//���룬 ��Ϣ���͵ȴ�ʱ�Ķ�ʱ����
	int						m_resptimeoutmsgtimer;		//�ÿ�δӦ���Զ��ظ�ʱ��
	int						m_resptimeoutclosetimer;	//�ÿͳ�ʱ��δ�ظ�����ʼ�Զ��رնԻ�
	int						m_waitresptimeouttimer;		//�ͷ�δӦ���Զ��ظ�ʱ��

	//���������Զ�����
	unsigned long			m_onlinetime;				// ���ߵ�ʱ��
	time_t					m_leaveTime;				// �������� [12/14/2010 SC]
	string					m_strInfoHtml;
	string					m_strTail;
	string					m_strInfo;
	string					m_strPreSendMsg;
	list<ONE_MSG_INFO>		m_strMsgs;
	bool					m_bConnected;				// ���û��Ѿ����㽨�����ӣ�����ͨѶ��
	bool					m_bIsGetInfo;				// �Ƿ��ѻ�ȡ����Ϣ
public:
	string					m_strHistory;				//�ܵ���ʷ��¼����ʾ
	string					m_strTotal;
	string					m_strListMsg;
	string					m_strListInfo;
	string					m_strListTail;
	int						nTimer;						//��ʱ��
	int						nVisitFrom;					//�ÿ���Դ
	int						nVisitNum;
	int						nTalkNum;
	int						nLastVisitTime;
	unsigned long			bak;
	int						m_nFlag;					//������ʱʹ�ñ���
	bool					m_bIsShow;					//�Ƿ���ʾ
	bool					m_bIsFrWX;					//�Ƿ�����΢��
	string					m_sWxAppid;					//�Ự��Դ��΢��appid
	bool					GetNormalChatHisMsgSuccess;	//������ʷ��Ϣ
	time_t					tGetNormalChatHismsgTime;

	WxUserInfo*				m_pWxUserInfo;
};

// �����
class CGroupObject : public IBaseObject
{
public:
	CGroupObject();
	CGroupObject(unsigned long id, string name);
	~CGroupObject();

	CGroupObject *AddGroupObject(unsigned long id, string name);
	void AddGroupObject(CGroupObject * pGroup);
	CUserObject *AddUserObject(unsigned long id, string name, char sex);

	void DeleteAll();

	void DeleteAllUser();
	void DeleteAllGroup();

public:
	unsigned long id;
	string strName;

	list<CGroupObject *>m_ListGroupInfo;
	list<CUserObject *>m_ListUserInfo;
};

#endif