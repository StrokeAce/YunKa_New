#include "../stdafx.h"
#include "chat_manager.h"
#include "../chat_common/comclt.h"
#include "../chat_common/comsrv.h"
#include "../chat_common/comfloat.h"
#include "../chat_common/comform.h"
#include "../chat_common/comfunc.h"
#include "../chat_common/comdef.h"
#include "../chat_common/comstruct.h"
#include "../chat_common/commsg.h"
#include "../ui_common/common_utility.h"
#include "../json/jsonext.h"
#include "../json/cJSON.h"
#include "ssl.h"
#include "md5.h"
#include "switch_sock.h"
#include "chat_history_msg.h"
#include "http_unit.h"
#include "code_convert.h"
#include <cctype>
#include <algorithm>
#include <process.h>
#include <shlwapi.h>
#include <functional>

using namespace std;

#define CLIENTVERSION	20000
#define  HASINSERTRIGHT   false

unsigned short g_packSeq = 0;

CChatManager::CChatManager()
{
	m_bExit = false;
	m_bLoginSuccess = false;
	m_server = "tcp01.tq.cn";
	m_port = 443;
	m_usSrvRand = 0;
	m_usCltRand = (unsigned short)(rand() & 0xFFFF);
	m_socket.SetReceiveObject(this);		
	m_msgId = 0;
	m_fileId = -1;
	m_nMyInfoIsGet = 0;
	m_nClientIndex = 1;
	m_nLoginToVisitor = -20;
	m_nSendPing = -20;
	m_handlerLogin = NULL;
	m_handlerMsgs = NULL;
	m_hScreenDll = NULL;
	m_vistor = NULL;
	m_login = new CLogin();
	m_login->m_manager = this;
	m_nOnLineStatus = STATUS_OFFLINE;
	m_nOnLineStatusEx = STATUS_OFFLINE;
	m_mapTokens.clear();
	m_mapUsers.clear();
	m_mapWebUsers.clear();	
	m_timers = new CTimerManager(CChatManager::TimerProc, this);
	m_sysConfig = new CSysConfigFile();
	LoadINIResource();
	ReadSystemConfig();
	string tempPath = FullPath("temp\\");
	CreateDirectoryA(tempPath.c_str(),NULL);
}

CChatManager::~CChatManager()
{
}

void CChatManager::ScreenCapture(HWND hWnd)
{
	string strCurrentPath = FullPath("\\ScreenCapture.dll");
	if (m_hScreenDll == NULL)
	{
		m_hScreenDll = LoadLibraryA((LPSTR)strCurrentPath.c_str());
	}

	if (m_hScreenDll)
	{
		typedef void(*PStartCapture)(HWND, int, LPSTR);
		PStartCapture StartCapture = (PStartCapture)GetProcAddress(m_hScreenDll, "StartCaptureScreen");

		if (StartCapture != NULL)
		{
			StartCapture(hWnd, CAPTURE_SAVE_TYPE_CLIPBOARD, NULL);
		}
	}
}

ListLoginedInfo CChatManager::GetPreLoginInfo()
{
	return m_sysConfig->m_cLoginInfoList;
}

void CChatManager::StartLogin(string loginName, string password, bool isAutoLogin, bool isKeepPwd)
{
	assert(m_handlerLogin); // ��¼֮ǰ��Ӧ������Ϣ�ص��ӿ����ú�

	m_sLogin = loginName;
	m_password = password;
	m_bAutoLogin = isAutoLogin;
	m_bKeepPwd = isKeepPwd;

	// ��ʼ��¼ʱ������һ���жϵ�¼��ʱ�Ķ�ʱ��
	m_timers->SetTimer(10000, TIMER_LOGIN);
	m_login->StartLogin(loginName, password, isAutoLogin, isKeepPwd);
}

bool CChatManager::ReadSystemConfig()
{
	// ����Ĭ�ϵ������ļ�
	char sFile[MAX_256_LEN];
	string strFile = GetCurrentPath();
	sprintf(sFile, "%s\\config.dat", strFile.c_str(), m_userInfo.UserInfo.uid);
	if (!m_sysConfig->LoadData(sFile))
	{
		SetSystemConfigByInitconfig();		
	}

	m_sysConfig->m_sVisitorServer = m_initConfig.sVisitorServer;
	m_sysConfig->m_nVisitorServerPort = m_initConfig.nVisitorServerPort;
	return false;
}

void CChatManager::SetSystemConfigByInitconfig()
{
	int i;

	m_sysConfig->m_sStrServer = m_initConfig.sDefaultServer;
	m_sysConfig->m_nServerPort = m_initConfig.nDefaultPort;

	for (i = 0; i<10; i++)
	{
		if (strlen(m_initConfig.sListServer[i]) > 0)
		{
			m_sysConfig->m_cServerAddressList.push_back(m_initConfig.sListServer[i]);
		}
	}

	m_sysConfig->m_nFilterType = m_initConfig.visitorfilter;
	SolveUserdefineFilter(m_sysConfig->m_cWebUserfilter, m_initConfig.infofilter);
}

void CChatManager::SolveUserdefineFilter(FILTER_USERDEFINE &filter, char *sfilter)
{
	if (sfilter == NULL)
		return;

	char sbuff[MAX_256_LEN];

	char seps[] = ",";
	char *token;
	int i;

	memset(&filter, '\0', sizeof(FILTER_USERDEFINE));
	strncpy(sbuff, sfilter, MAX_256_LEN);
	token = strtok(sbuff, seps);
	i = 0;
	while (token != NULL)
	{
		if (i == 0)
		{
			filter.include = atol(token);
		}
		else
		{
			strncpy(filter.text[i - 1], token, MAX_STRINGFILTER_LEN - 2);
			filter.num++;
		}
		if (i > MAX_STRINGFILTER_NUM)
			break;

		token = strtok(NULL, seps);
		i++;
	}

	return;
}

void CChatManager::OnReceive(void* wParam, void* lParam)
{
	if (m_bExit) return;

	int nError = 0;
	TCP_PACK_HEADER TcpPackHead;

	int nPackHeadLen = COM_HEAD_PACK::GetHeadLen();
	int nTcpPackHeadLen = sizeof(TCP_PACK_HEADER);
	int nErrType = 0;

	TcpPackHead = *((TCP_PACK_HEADER *)wParam);
	assert(TcpPackHead.len <= PACKMAXLEN);

	char *RecvBuf = (char*)lParam;

	COM_HEAD_PACK Head;
	Head.head = *((PACK_HEADER*)RecvBuf);
	g_WriteLog.WriteLog(C_LOG_TRACE, "OnReceive logondlg recv Cmd:%.4x", Head.head.cmd);

	switch (Head.head.cmd)
	{
	case CMD_SRV_CONF_LOGON: // ��¼ȷ��
		nError = RecvSrvConfLogon(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		if (nError != 0)
		{
			goto FAIL;
		}
		m_handlerLogin->LoginProgress(80);
		break;
	case CMD_SRV_REP_USERINFO: // �û���Ϣ��
		nError = RecvSrvRepUserinfo(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		if (nError != 0)
		{
			goto FAIL;
		}
		break;
	case CMD_SRV_DENY_LOGON: // �ܾ���¼
		nError = RecvSrvDenyLogon(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen, nErrType);
		if (nError != 0)
		{
			goto FAIL;
		}
		else
		{
			nError = COMM_DENYLOGIN_SEG + nErrType;
			goto FAIL;
		}
		break;
	case CMD_SRV_CONF_LOGOFF:   // �ǳ�ȷ�ϰ�
		nError = RecvSrvConfLogOff(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_SRV_STATUS_FRDONLINE: // ��������
		nError = RecvSrvStatusFrdOnline(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_SRV_STATUS_FRDOFFLINE: // ��������
		nError = RecvSrvStatusFrdOffline(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_SRV_STATUS_USER_FORM: // ���Ѹ�������״̬
		nError = RecvSrvStatusUserForm(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_FLOAT_SHARELIST: // �յ����Ѷ����б�
		nError = RecvFloatShareList(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_FLOAT_CREATECHAT: // ����Ự
		nError = RecvFloatCreateChat(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_FLOAT_CHATINFO: // �Ự��ϸ��Ϣ
		nError = RecvFloatChatInfo(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_COM_SEND_MSG: // �Ự��Ϣ,�ǵȴ�Ӧ�����
		nError = RecvComSendMsg(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_FLOAT_CHATMSG: // �Ự��Ϣ,�ȴ�Ӧ�����
		nError = RecvFloatChatMsg(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_FLOAT_CHATMSG_ACK: // ������ϢӦ��
		nError = RecvFloatChatMsgAck(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_FLOAT_ACCEPTCHAT: // ���ܻỰ
		nError = RecvFloatAcceptChat(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_FLOAT_TRANSQUEST: // �ȴ�Ӧ��  �Ựת������
		nError = RecvFloatTransQuest(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_FLOAT_TRANSFAILED:   // �ȴ�Ӧ��  �Ựת��ʧ��
		nError = RecvFloatTransFailed(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_FLOAT_INVITE_REQUEST:// �ȴ�Ӧ��  ��������
		nError = RecvInviteRequest(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_FLOAT_INVITE_RESULT:// �ȴ�Ӧ��  ������
		nError = RecvInviteResult(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_FLOAT_RELEASE: // ��ϯ�����ͷŻỰ
		nError = RecvFloatRelease(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_FLOAT_CMDERROR: // �ȴ�Ӧ��  ����ʧ��
		nError = RecvFloatCMDError(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_FLOAT_CLOSECHAT: // �Ự�ر�
		nError = RecvFloatCloseChat(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_FLOAT_LISTCHAT: //�Ự�б�
		nError = RecvFloatListChat(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;	
	case CMD_EVENT_ANNOUNCEMENT: // �յ���Ϣͨ��
		nError = RecvEventAnnouncement(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_SRV_UPDATE_SUCC: //������Ϣ�ɹ�
		nError = RecvSrvUpdateSucc(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_SRV_UPDATE_FAIL: // ������Ϣʧ��
		nError = RecvSrvUpdateFail(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_SRV_SERVER_COPY: // �û�����ص�½
		nError = RecvSrvDown(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_SRV_REP_TRANSFERCLIENT: // 440 ת����ʱ�û��ɹ�ʧ��
		nError = RecvRepTransferClient(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_SRV_TRANSFERCLIENT: // ����ת�ƻỰ���û�����
		nError = RecvTransferClient(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	default:
		break;
	}

	return;

FAIL:
	m_handlerLogin->LoginProgress(-1);
	return;
}

void CChatManager::OnReceiveEvent(int wParam, int lParam)
{
	if (m_bExit) return;

	if (wParam == WM_SOCKET_CLOSE)
	{
		m_nOnLineStatus = STATUS_OFFLINE;
		g_WriteLog.WriteLog(C_LOG_TRACE, "OnReceiveEvent SetOfflineStatus");
		SetOfflineStatus();
	}
	else if (wParam == WM_SOCKET_RECVFAIL)
	{
		if (m_nOnLineStatus == STATUS_OFFLINE)
			return;

		m_nOnLineStatus = STATUS_OFFLINE;
		SetOfflineStatus();
		g_WriteLog.WriteLog(C_LOG_TRACE, "OnReceiveEvent OnSocketRecvFail");
	}
}

int CChatManager::RecvSrvConfLogon(PACK_HEADER packhead, char *pRecvBuff, int len)
{
	SRV_CONF_LOGON RecvInfo(packhead.ver);
	int nError = 0;

	nError = UnPack(&RecvInfo, pRecvBuff, len);
	if (nError != 0)
	{
		return nError;
	}

	m_sysConfig->m_sStrServer = m_server;
	m_sysConfig->m_nServerPort = m_port;

	m_sysConfig->m_sStrRealServer = m_server;
	m_sysConfig->m_nRealServerPort = m_port;

	m_userInfo.UserInfo.uid = RecvInfo.uin;
	strcpy(m_userInfo.UserInfo.sid, RecvInfo.strid);
	strcpy(m_userInfo.UserInfo.pass, m_password.c_str());

	//���������ص�RecvInfo.type_loginbyΪ0����
	m_sysConfig->m_nLastLoginBy = LOGIN_BYSTRING;
	if (strlen(RecvInfo.strid) <= 0)
		m_sysConfig->m_sLastLoginStr = m_sLogin;
	else
		m_sysConfig->m_sLastLoginStr = RecvInfo.strid;
	m_sysConfig->m_sLastLoginPass = m_password;

	m_sysConfig->AddServerAddress(m_sysConfig->m_sStrServer);

	m_usSrvRand = RecvInfo.m_Head.head.random;
	nError = 0;

	char lognmame[100] = { 0 };
	sprintf(lognmame, "_%s(%u)_emocss.log", RecvInfo.strid, RecvInfo.uin);
	g_WriteLog.InitLog(GetCurrentPath() + "\\log", lognmame, C_LOG_ALL);

	return nError;
}

int CChatManager::RecvSrvRepUserinfo(PACK_HEADER packhead, char *pRecvBuff, int len)
{
	SRV_REP_USERINFO RecvInfo(packhead.ver);
	int nError = 0;
	CUserObject *pUser = NULL;
	CWebUserObject *pWebUser = NULL;

	nError = UnPack(&RecvInfo, pRecvBuff, len);
	if (nError != 0)
	{
		goto RETURN;
	}

	if (!m_bLoginSuccess)
	{
		pUser = AddUserObject(RecvInfo.uin, RecvInfo.strid, RecvInfo.UserInfo.info.nickname, STATUS_ONLINE, -1);

		CCodeConvert convert;
		string msg = "�Լ�����Ϣ��¼";
		string sMsg;
		convert.Gb2312ToUTF_8(sMsg, msg.c_str(), msg.length());
		ONE_MSG_INFO ongMsg;
		ongMsg.msgId = GetMsgId();
		char strJsCode[MAX_512_LEN];
		sprintf(strJsCode, "AppendMsgToHistory('%d','%d','%s','%s','%s','%s','%s','%s');",
			MSG_FROM_SYS, MSG_DATA_TYPE_TEXT, "", GetTimeByMDAndHMS(0).c_str(), sMsg.c_str(),
			"0", pUser->m_headPath.c_str(), ongMsg.msgId.c_str());
		ongMsg.msg = strJsCode;
		pUser->m_strMsgs.push_back(ongMsg);

		// ����ͷ��
		pUser->DownLoadFace(m_initConfig.webpage_DownloadHeadImage);

		LoginSuccess();
	}
	else
	{
		// ��¼�ɹ����յ��û���Ϣ������
		if (packhead.langtype == LANGUAGE_UTF8)
		{
			// ת��
			ConvertMsg(RecvInfo.UserInfo.info.nickname, sizeof(RecvInfo.UserInfo.info.nickname) - 1);
		}

		g_WriteLog.WriteLog(C_LOG_TRACE, "RecvSrvRepUserinfo uid=%u,sid=%s,nickname=%s,username=%s,langtype=%u", RecvInfo.uin,
			RecvInfo.UserInfo.info.sid, RecvInfo.UserInfo.info.nickname, RecvInfo.UserInfo.info.username, RecvInfo.UserInfo.info.langtype);

		if (RecvInfo.uin > WEBUSER_UIN)
		{
			pWebUser = GetWebUserObjectByUid(RecvInfo.uin);

			if (pWebUser != NULL)
			{
				strcpy(pWebUser->info.name, RecvInfo.UserInfo.info.nickname);
				if (strlen(pWebUser->info.name) < 2)
				{
					g_WriteLog.WriteLog(C_LOG_ERROR, "RecvSrvRepUserinfo() name length��%d", strlen(pWebUser->info.name));
				}
			}
		}
		else
		{
			ONLINEFLAGUNION onlineflag;
			onlineflag.online_flag = RecvInfo.UserInfo.info.onlineflag;

			pUser = GetUserObjectByUid(RecvInfo.uin);
			if (pUser == NULL)
			{
				pUser = AddUserObject(RecvInfo.uin, RecvInfo.strid, RecvInfo.UserInfo.info.nickname, onlineflag.stStatus.nOnlineStatus, -1);

				if (pUser == NULL)
				{
					goto RETURN;
				}
				// ����ͷ��
				pUser->DownLoadFace(m_initConfig.webpage_DownloadHeadImage);
				pUser->m_nFlag = 1;
			}
			else
			{
				if (pUser->status != onlineflag.stStatus.nOnlineStatus
					|| strcmp(pUser->UserInfo.nickname, RecvInfo.UserInfo.info.nickname) != 0)
				{
					pUser->status = onlineflag.stStatus.nOnlineStatus;
					strcpy(pUser->UserInfo.nickname, RecvInfo.UserInfo.info.nickname);
					
					if (pUser->m_bFriend)
					{
						CCodeConvert convert;
						string msg = pUser->UserInfo.nickname;
						msg += "����Ϣ��¼";
						string sMsg;
						convert.Gb2312ToUTF_8(sMsg, msg.c_str(), msg.length());
						ONE_MSG_INFO ongMsg;
						ongMsg.msgId = GetMsgId();
						char strJsCode[MAX_512_LEN];
						sprintf(strJsCode, "AppendMsgToHistory('%d','%d','%s','%s','%s','%s','%s','%s');",
							MSG_FROM_SYS, MSG_DATA_TYPE_TEXT, "", GetTimeByMDAndHMS(0).c_str(), sMsg.c_str(),
							"0", pUser->m_headPath.c_str(), ongMsg.msgId.c_str());
						ongMsg.msg = strJsCode;
						pUser->m_strMsgs.push_back(ongMsg);

						m_handlerMsgs->RecvUserInfo(pUser);
					}
				}
			}
		}
	}
	
	nError = 0;
RETURN:

	return nError;
}

int CChatManager::RecvSrvDenyLogon(PACK_HEADER packhead, char *pRecvBuff, int len, int &errtype)
{
	m_socket.Close();

	SRV_DENY_LOGON RecvInfo(packhead.ver);
	int nError = 0;

	nError = UnPack(&RecvInfo, pRecvBuff, len);
	if (nError != 0)
	{
		goto RETURN;
	}

	errtype = RecvInfo.deny;

RETURN:

	return nError;
}

int CChatManager::UnPack(CPackInfo *pPackInfo, char *buff, int len)
{
	int nError = 0;

	if (pPackInfo == NULL)
	{
		nError = SYS_ERROR_MEMORY;
		goto RETURN;
	}

	pPackInfo->m_Pack.Clear();

	pPackInfo->m_Pack.CopyBuff((unsigned char *)buff, len, 0);

	if (!(pPackInfo->unpack()))
	{
		nError = SYS_ERROR_UNPACKINFO;
		goto RETURN;
	}

	nError = 0;

RETURN:
	return nError;
}

bool CChatManager::LoadINIResource()
{
	char sFile[MAX_256_LEN], sKey[MAX_256_LEN];

	int i, len = MAX_256_LEN;

	sprintf(sFile, "%s\\TQConfig.ini", GetCurrentPath().c_str());

	LoadIniString("common", "programe name mini", m_initConfig.sProgrameNameMini, len, sFile, NULL);

	LoadIniString("common", "help url", m_initConfig.sHelpUrl, len, sFile);

	LoadIniString("common", "copy id", m_initConfig.sCopyID, len, sFile);

	for (i = 0; i < 3; i++)
	{
		sprintf(sKey, "help about %d", i + 1);
		LoadIniString("common", sKey, m_initConfig.sHelpAbout[i], len, sFile);

		sprintf(sKey, "help about url %d", i + 1);
		LoadIniString("common", sKey, m_initConfig.sHelpAboutURL[i], len, sFile);
	}

	LoadIniInt("common", "display username type", m_initConfig.nDisplayUserNameType, sFile);

	LoadIniString("server", "address", m_initConfig.sDefaultServer, len, sFile);

	LoadIniInt("server", "port", m_initConfig.nDefaultPort, sFile);

	LoadIniString("visitor", "address", m_initConfig.sVisitorServer, len, sFile);

	LoadIniInt("visitor", "port", m_initConfig.nVisitorServerPort, sFile);

	int temp(0);
	LoadIniInt("visitor", "ZlibEnabled", temp, sFile);
	m_initConfig.bZlibEnabled = (1 == temp);

	for (i = 0; i < 10; i++)
	{
		sprintf(sKey, "list %d", i + 1);
		LoadIniString("server", sKey, m_initConfig.sListServer[i], len, sFile);
	}

	LoadIniInt("login", "save pass", m_initConfig.bSavePass, sFile);
	LoadIniInt("login", "auto login", m_initConfig.bAutoLogin, sFile);

	//visitor filter
	LoadIniInt("visitor filter", "filter", m_initConfig.visitorfilter, sFile, "0");
	LoadIniString("visitor filter", "info filter", m_initConfig.infofilter, len, sFile, "");
	LoadIniString("AuthClient", "AuthAddr", m_initConfig.sAuthAddr, len, sFile, "passport.tq.cn");
	LoadIniInt("AuthClient", "AuthPort", m_initConfig.nAuthPort, sFile, "80");
	LoadIniString("WebPages", "logout", m_initConfig.webpage_lgout, len, sFile, "http://vip.tq.cn/vip/logout.do");
	LoadIniString("WebPages", "webphone", m_initConfig.webpage_webphone, len, sFile, "http://vip.tq.cn/vip/screenIndex.do?a=1");

	// ��ѯ����
	LoadIniString("WebPages", "querywebphone", m_initConfig.webpage_querywebphone, len, sFile, "http://vip.tq.cn/vip/screenPhoneRecord.do?action=query&deal_state=0");

	// ����
	LoadIniString("WebPages", "note", m_initConfig.webpage_note, len, sFile, "http://vip.tq.cn/vip/viewLiuyan.do?a=1");

	// �ͻ�����
	LoadIniString("WebPages", "crm", m_initConfig.webpage_crm, len, sFile, "http://vip.tq.cn/vip/visitorInfomation.do?a=1");

	// ͳ�Ʒ���
	LoadIniString("WebPages", "analyze", m_initConfig.webpage_analyze, len, sFile, "http://vip.tq.cn/vip/serviceStutasE.do?a=1");

	// ��������
	LoadIniString("WebPages", "mgmt", m_initConfig.webpage_mgmt, len, sFile, "http://vip.tq.cn/vip/clientIndex.do?a=1");

	// ���㴰��
	LoadIniString("WebPages", "news", m_initConfig.webpage_news, len, sFile, "http://vip.tq.cn/vip/rss/tq_mini_loading.jsp?a=");

	// ע�����û�
	LoadIniString("WebPages", "regnewuser", m_initConfig.webpage_regnewuser, len, sFile, "http://www.tq.cn/vip/prenewqttclt.do");
	LoadIniInt("WebPages", "regnewuser_w", m_initConfig.webpage_regnewuser_w, sFile, "520");
	LoadIniInt("WebPages", "regnewuser_h", m_initConfig.webpage_regnewuser_h, sFile, "410");

	// ����ָ��
	LoadIniString("WebPages", "tqhelp", m_initConfig.webpage_tqhelp, len, sFile, "http://www.tq.cn/help_3.html");

	// ͳһ����
	LoadIniString("WebPages", "UniIdioms", m_initConfig.webpage_UniIdioms, len, sFile, "http://vip.tq.cn/vip/GetRequestXMLAction.do?cmd=NEWQUICKREPLY");

	// ʵʱ���
	LoadIniString("WebPages", "RtmMsg", m_initConfig.webpage_RtmMsg, len, sFile, "http://vip.tq.cn/vip/realTime.do?a=");

	// ������Ϣ
	LoadIniString("WebPages", "SvrMsg", m_initConfig.webpage_SvrMsg, len, sFile, "http://vip.tq.cn/vip/historyDialog.do?a=");

	// TQ����
	LoadIniString("WebPages", "TQadviser", m_initConfig.webpage_TQadviser, len, sFile, "http://vip.tq.cn/vip/myTqGuWen.do?a=");

	// �ϴ�ͷ��
	LoadIniString("WebPages", "UploadHeadImage", m_initConfig.webpage_UploadHeadImage, len, sFile, "http://vip.tq.cn/vip/preuploadfacelink.do?a=");

	// ����ͷ��
	LoadIniString("WebPages", "DownloadHeadImage", m_initConfig.webpage_DownloadHeadImage, len, sFile, "http://admin.tq.cn/vip/facelinkimgs/");

	// ����
	LoadIniString("WebPages", "FaceImage", m_initConfig.webpage_FaceImage, len, sFile, "http://vip.tq.cn/clientimages/face/images");

	// ϵͳ��Ϣ
	LoadIniString("WebPages", "SysMessage", m_initConfig.webpage_SysMessage, len, sFile, "http://sysmessage.tq.cn/SysMsg/getmesg.do?strid=%s&uin=%lu&departId=%d");

	// Ͷ�߽���
	LoadIniString("WebPages", "complaint", m_initConfig.webpage_complaint, len, sFile, "http://211.151.52.62:8081/zhengquan/preLeaveMsg.do?uin=%lu");

	//�����ļ�
	LoadIniString("WebPages", "SendWebFileLink", m_initConfig.webpage_SendWebFileLink, len, sFile, "http://vip.tq.cn/upload/upload2.do?version=100");


	LoadIniString("WebPages", "iconurl", m_initConfig.webpage_iconurl, len, sFile, "http://sysimages.tq.cn/clientimages/face/ywt_face.html");
	LoadIniString("WebPages", "faqInfo", m_initConfig.webpage_faqInfo, len, sFile, "http://211.151.52.39:8080/vip/DoRequestXMLAction.do?action=faqInfo");
	LoadIniString("WebPages", "faqSort", m_initConfig.webpage_faqSort, len, sFile, "http://211.151.52.39:8080/vip/DoRequestXMLAction.do?action=faqSort");
	LoadIniString("WebPages", "faqAll", m_initConfig.webpage_faqall, len, sFile, "http://211.151.52.39:8080/vip/DoRequestXMLAction.do?action=faqAll");
	LoadIniString("WebPages", "repickchat", m_initConfig.webpage_repickchaturl, len, sFile, "http://106.120.108.230:8031/wxcreatechat?uin=%lu&signature=%s&timestamp=%s&nonce=%s&admiuin=%lu&%s&msg=hello");
	//��������evaluate
	LoadIniString("WebPages", "evaluate", m_initConfig.webpage_evaluate, len, sFile, "http://106.120.108.230:8524/vip/scorebill.do?billid=%s&admin_uin=%u&action=scoreedit");

	LoadIniString("WebPages", "sendurl", m_initConfig.webpage_sendurl, len, sFile, "http://211.151.52.39:8080/vip/DoRequestXMLAction.do?action=url");

	//��ȡ��˾�û�
	LoadIniString("WebPages", "companyuser", m_initConfig.webpage_companyuser, len, sFile, "http://vip.tq.cn/vip/GetRequestXMLData?cmd=TRANSFER");

	// ��ȡ΢��token
	LoadIniString("WebPages", "accesstoken", m_initConfig.webpage_accesstoken, len, sFile, "http://106.120.108.230:8031/wxkfgetac?uin=%lu&signature=%s&timestamp=%s&nonce=%s&{$MSG_WORKBILL.UserDefineParams}");

	//����
	LoadIniString("WebPages", "workbillurl", m_initConfig.webpage_workbillurl, len, sFile, "http://106.120.108.230:8524/vip/workOrder.do?r=%s");

	// �ͻ�����
	LoadIniString("VisitorPages", "visitortail", m_initConfig.visitorpage_visitortail, len, sFile, "http://vip.tq.cn/vip/ClientDialog.do?billid=%s&cuin=%lu&rand=%s&transtype=%d&transfrom=%lu&stime=%s&etime=%s");

	//�Ự�ǵ�
	LoadIniString("VisitorPages", "visitorbill", m_initConfig.visitorpage_visitorbill, len, sFile, "http://vip.tq.cn/vip/recrodworkbill.do?action=prerecord&billid=%s&cuin=%lu&rand=%s&transtype=%d&transfrom=%lu&stime=%s&etime=%s");

	//�ͻ���Ϣ
	LoadIniString("VisitorPages", "visitorinfo", m_initConfig.visitorpage_visitorinfo, len, sFile, "http://vip.tq.cn/vip/visitorinfo.do?billid=%s&cuin=%lu&rand=%s&transtype=%d&transfrom=%lu&stime=%s&etime=%s");
	//�¶���
	LoadIniString("VisitorPages", "visitororder", m_initConfig.visitorpage_visitororder, len, sFile, "http://106.120.108.230:8524/vip/visitorinfo.do?billid=%s&cuin=%lu&rand=%s&transtype=%d&transfrom=%lu&stime=%s&etime=%s&clientid=%s&type=0");

	//ͨ����Ϣ
	LoadIniString("VisitorPages", "visitornotice", m_initConfig.visitorpage_notice, len, sFile, "http://www.tq.cn");
	LoadIniString("WebPages", "RegisterAccount", m_initConfig.registerAccount, len, sFile, "http://www.tq.cn");
	LoadIniString("WebPages", "ForgetPassword", m_initConfig.forgetPassword, len, sFile, "http://www.tq.cn");
	LoadIniInt("WebPages", "ForgetPSVisible", m_initConfig.forgetPSVisible, sFile);

	// ��ѯ��ʷ��¼
	LoadIniString("WebPages", "QueryHistoryRecords", m_initConfig.query_history_records, len, sFile, "http://106.120.108.230:8524/vip/getbilllist.do?adminuin=%lu&kefu_uin=%lu&client_id=%s&weixin_id=%s&current_id=%s&timestamp=%s&token=%s&pagesize=1");

	// ΢�ŷ�����ý���ļ��ϴ�url
	LoadIniString("WebPages", "WeChatMediaUpload", m_initConfig.wechat_media_upload, len, sFile, "http://file.api.weixin.qq.com/cgi-bin/media/upload?access_token=%s&type=%s");

	// ΢�ŷ�����ý���ļ���ȡurl
	LoadIniString("WebPages", "WeChatMediaUrl", m_initConfig.wechat_media_url, len, sFile, "http://file.api.weixin.qq.com/cgi-bin/media/get?access_token=%s&media_id=%s");

	// ��Ѷ��ͼ��̬ͼurl
	LoadIniString("WebPages", "WeChatStaticMap", m_initConfig.wechat_static_map, len, sFile, "http://apis.map.qq.com/ws/staticmap/v2/?key=JRYBZ-QIAWS-GJ3OB-6GXXF-F3WMZ-RNBGV&size=500x400&center=%s,%s&zoom=12");

	// ��Ѷ��ͼ�ص��עurl
	LoadIniString("WebPages", "WeChatMapLocation", m_initConfig.wechat_map_location, len, sFile, "http://apis.map.qq.com/uri/v1/marker?marker=coord:%s,%s;title:%s;addr:%s&referer=myapp");

	// �Լ����ļ����������ص�΢�ŷ�������ý���ļ���url
	LoadIniString("WebPages", "FileServerMediaDownload", m_initConfig.fileserver_media_download, len, sFile, "http://106.120.108.230:8531/media/get?access_token=%s&media_id=%s");

	// �Լ����ļ��������ϴ�url
	LoadIniString("WebPages", "FileServerMediaUpload", m_initConfig.fileserver_media_upload, len, sFile, "http://106.120.108.230:8531/media/upload/%s");

	// ΢���ļ�����url
	LoadIniString("WebPages", "FileServerMediaTask", m_initConfig.fileserver_media_task, len, sFile, "http://file.api.weixin.qq.com/cgi-bin/media/get?access_token=%s&media_id=%s");

	// ΢���ļ�����url
	LoadIniString("WebPages", "FileServerMediaFileId", m_initConfig.fileserver_media_fileid, len, sFile, "http://wxm.tq.cn/media/%s");

	return true;
}

int CChatManager::SendPackTo(CPackInfo *pPackInfo, unsigned long recvuid, unsigned long recvsock, HWND hWnd)
{
	int nrtn = 0;

	TCP_PACK_HEADER TcpPackHead;
	int nError;
	int nPackHeadLen = COM_HEAD_PACK::GetHeadLen();
	int nTcpPackHeadLen = sizeof(TCP_PACK_HEADER);
	int nSendLen;

	if (pPackInfo == NULL)
	{
		nrtn = SYS_ERROR_MEMORY;
		goto RETURN;
	}

	pPackInfo->m_Head.SetValue(pPackInfo->version,
		m_usCltRand,
		m_userInfo.UserInfo.uid,
		m_usSrvRand,
		pPackInfo->m_Head.head.cmd, 0,
		pPackInfo->m_Head.head.langtype,
		0,
		TERMINAL_PCEX);

	if (!(pPackInfo->packhead()))
	{
		nrtn = SYS_ERROR_PACKHEAD;
		goto RETURN;
	}
	if (!(pPackInfo->pack()))
	{
		nrtn = SYS_ERROR_PACKINFO;
		goto RETURN;
	}

	//���ܰ�  20090420
	pPackInfo->m_Pack.Encode();

	memset(&TcpPackHead, '\0', sizeof(TCP_PACK_HEADER));
	TcpPackHead.len = pPackInfo->m_Pack.GetPackLength();

	g_WriteLog.WriteLog(C_LOG_TRACE, "SendOnePack send pack cmd:%.4X", pPackInfo->m_Head.head.cmd);

	nSendLen = m_socket.SendImPack((char *)(pPackInfo->m_Pack.GetPackBuff()), TcpPackHead, nError);
	if (nSendLen != TcpPackHead.len + nTcpPackHeadLen)
	{
		nrtn = SYS_ERROR_SENDFAIL;

		g_WriteLog.WriteLog(C_LOG_ERROR, "SendOnePack failed %d,fd=%d", TcpPackHead.len, m_socket.getfd());
		goto RETURN;
	}

	nrtn = 0;

RETURN:
	return nrtn;
}

int CChatManager::RecvSrvConfLogOff(PACK_HEADER packhead, char *pRecvBuff, int len)
{
	return 0;
}

int CChatManager::RecvSrvStatusFrdOnline(PACK_HEADER packhead, char *pRecvBuff, int len)
{
	SRV_STATUS_FRDONLINE RecvInfo(packhead.ver);
	int nError = 0;
	CUserObject *pUser = NULL;

	nError = UnPack(&RecvInfo, pRecvBuff, len);
	if (nError != 0 || RecvInfo.m_Head.head.random != m_usSrvRand)
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "RecvSrvStatusFrdOnline unpack failed");
		return nError;
	}

	if (packhead.langtype == LANGUAGE_UTF8)
	{
		ConvertMsg(RecvInfo.nickname, sizeof(RecvInfo.nickname) - 1);
	}

	if (RecvInfo.uin > WEBUSER_UIN)
	{
		//�ÿͶԻ���ʼ
		g_WriteLog.WriteLog(C_LOG_TRACE, "RecvSrvStatusFrdOnline: RecvInfo.uin > WEBUSER_UIN");
	}
	else
	{
		pUser = GetUserObjectByUid(RecvInfo.uin);
		if (pUser == NULL)
		{
			//���ﲻ���Ǻܸ��ӣ�������µ��û�����Ҫ�Զ���ˢ�����£��Ժ�����ʵʱ�ش���
			pUser = AddUserObject(RecvInfo.uin, "", RecvInfo.nickname, RecvInfo.status, -1);

			pUser->status = STATUS_ONLINE;
			strcpy(pUser->UserInfo.nickname, RecvInfo.nickname);
			pUser->m_nFlag = 1;
			pUser->DownLoadFace(m_initConfig.webpage_DownloadHeadImage);
		}
		else
		{
			pUser->status = STATUS_ONLINE;
			strcpy(pUser->UserInfo.nickname, RecvInfo.nickname);
		}

		if (pUser->UserInfo.uid == m_userInfo.UserInfo.uid)
		{
			m_nOnLineStatus = RecvInfo.status;
		}

		m_handlerMsgs->RecvOnline(pUser);
	}

	nError = 0;

	return nError;
}

int CChatManager::RecvSrvStatusFrdOffline(PACK_HEADER packhead, char *pRecvBuff, int len)
{
	SRV_STATUS_FRDOFFLINE RecvInfo(packhead.ver);
	int nError = 0;
	CUserObject *pUser = NULL;
	CWebUserObject *pWebUser = NULL;

	nError = UnPack(&RecvInfo, pRecvBuff, len);
	if (nError != 0 || RecvInfo.m_Head.head.random != m_usSrvRand)
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "RecvSrvStatusFrdOffline unpack failed");
		return nError;
	}
	g_WriteLog.WriteLog(C_LOG_TRACE, "RecvSrvStatusFrdOffline uin:%u,externid:%u, bak:%u",
		RecvInfo.uin, RecvInfo.externid, RecvInfo.bak);
	if (RecvInfo.uin > WEBUSER_UIN)
	{
		//�ÿͶԻ�����
		pWebUser = GetWebUserObjectByUid(RecvInfo.uin);
		if (pWebUser != NULL)
		{
			pWebUser->m_bNewComm = false;
			pWebUser->cTalkedSatus = HASTALKED;
			pWebUser->transferuid = 0;
			if (pWebUser->m_bConnected)
			{
				if (m_vistor)
				{
					m_vistor->SendWebuserTalkEnd(pWebUser);
				}
				else
				{
					g_WriteLog.WriteLog(C_LOG_ERROR,"RecvSrvStatusFrdOffline visit������δ��¼ǰ���յ�������Ϣ");
				}

				pWebUser->m_bConnected = true;
				pWebUser->m_nWaitTimer = -20;
				pWebUser->onlineinfo.talkstatus = TALKSTATUS_NO;
			}
		}
	}
	else
	{
		pUser = GetUserObjectByUid(RecvInfo.uin);
		if (pUser != NULL)
		{
			bool bexpand = false;
			if (pUser->status != STATUS_OFFLINE)
			{
				bexpand = true;
				pUser->m_bConnected = true;
				pUser->talkstatus = TALKSTATUS_NO;
				pUser->status = STATUS_OFFLINE;
			}

			if (pUser->UserInfo.uid == m_userInfo.UserInfo.uid)
			{
				g_WriteLog.WriteLog(C_LOG_TRACE, "RecvSrvStatusFrdOffline uid:%u", m_userInfo.UserInfo.uid);
				m_nOnLineStatus = STATUS_OFFLINE;
				m_nOnLineStatusEx = STATUS_OFFLINE;
			}

			m_handlerMsgs->RecvOffline(pUser);
		}
	}

	nError = 0;

	return nError;
}

int CChatManager::RecvSrvStatusUserForm(PACK_HEADER packhead, char *pRecvBuff, int len)
{
	SRV_STATUS_USER_FORM RecvInfo(packhead.ver);
	int nError = 0;
	CUserObject *pUser = NULL;

	nError = UnPack(&RecvInfo, pRecvBuff, len);
	if (nError != 0)
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "RecvSrvStatusUserForm unpack failed");
		return nError;
	}

	pUser = GetUserObjectByUid(RecvInfo.uin);
	if (pUser != NULL)
	{
		pUser->status = RecvInfo.GetOnlineStatus();
		m_handlerMsgs->RecvUserStatus(pUser);
	}

	nError = 0;

	return nError;
}

int CChatManager::RecvFloatShareList(PACK_HEADER packhead, char *pRecvBuff, int len)
{
	COM_FLOAT_SHARELIST RecvInfo(packhead.ver);
	int nError = 0;
	CUserObject *pUser = NULL;

	std::vector<SHAREUSEROBJ>::iterator iter;

	nError = UnPack(&RecvInfo, pRecvBuff, len);
	if (nError != 0)
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "RecvFloatShareList unpack failed,Cmd:%.4x", packhead.cmd);
		return nError;
	}

	int bSelf = 0;
	ONLINEFLAGUNION onlineflag;
	for (iter = RecvInfo.sharememlist.begin(); iter != RecvInfo.sharememlist.end(); iter++)
	{
		SHAREUSEROBJ ShareUserOb = (SHAREUSEROBJ)(*iter);

		onlineflag.online_flag = ShareUserOb.onlineflag;

		pUser = GetUserObjectByUid(ShareUserOb.uin);
		if (pUser == NULL)
		{
			pUser = AddUserObject(ShareUserOb.uin, "", "", onlineflag.stStatus.nOnlineStatus, -1);
			pUser->DownLoadFace(m_initConfig.webpage_DownloadHeadImage);
		}
		else
		{
			if (pUser->UserInfo.uid == m_userInfo.UserInfo.uid)
			{
				bSelf = 1;
			}
		}

		g_WriteLog.WriteLog(C_LOG_TRACE, "RecvFloatShareList:uin=%u:OnlineStatus=%d", ShareUserOb.uin, onlineflag.stStatus.nOnlineStatus);
		pUser->m_bFriend = true;
	}

	m_handlerMsgs->RecvShareListCount(RecvInfo.sharememlist.size() - bSelf);

	nError = 0;

	return nError;
}

int CChatManager::RecvComSendMsg(PACK_HEADER packhead, char *pRecvBuff, int len)
{
	COM_SEND_MSG RecvInfo(packhead.ver);
	int nError = 0;
	MSG_TYPE msgType = MSG_TYPE_NORMAL;
	MSG_FROM_TYPE msgFrom = MSG_FROM_WEBUSER;
	CWebUserObject *pWebUser = NULL;
	CUserObject *pUser = NULL;

	nError = UnPack(&RecvInfo, pRecvBuff, len);

	if (nError != 0)
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "RecvComSendMsg unpack failed,Cmd:%.4x", packhead.cmd);
		goto RETURN;
	}

	g_WriteLog.WriteLog(C_LOG_TRACE, "RecvComSendMsg chatid:%s,Rand:%s,Thirdid:%s,recvuin:%u,senduin:%u,strfontinfo:%s,msgtype:%d,msg:%s",
		RecvInfo.strChatid, RecvInfo.strRand, RecvInfo.strThirdid, RecvInfo.msg.recvuin,
		RecvInfo.msg.senduin, RecvInfo.msg.strfontinfo, RecvInfo.msg.msgtype, RecvInfo.msg.strmsg);

	//����ʾ���ݣ� ���ƶ�item
	if (packhead.langtype == 4)
	{
		ConvertMsg(RecvInfo.msg.strmsg, sizeof(RecvInfo.msg.strmsg) - 1);

		ConvertMsg(RecvInfo.msg.strmobile, sizeof(RecvInfo.msg.strmobile) - 1);
	}

	if (RecvInfo.msg.senduin > WEBUSER_UIN)
	{
		//�ÿͷ�����Ϣ
		switch (RecvInfo.msg.msgtype)
		{
		case MSG_WORKBILL:
		case MSG_CLIENT_COME:
			RecvComSendWorkBillMsg(RecvInfo.msg.senduin, RecvInfo.msg.recvuin, RecvInfo.msg.strmsg, RecvInfo.msg.strmobile);
			break;
		case MSG_WEBUI_SEND_CHAT_HISMSG:   //�յ�webui�������ĻỰ��ʷ��Ϣ�����ܶ����,��Ҫ�ͻ��˷�������MSG_KEFU_GET_CHAT_HISMSG
			RecvComSendNormalChatidHisMsg(RecvInfo.msg.senduin, RecvInfo.msg.recvuin, RecvInfo);
			break;
		case MSG_TRANSFER_REQUEST://�Ựת������
			RecvComTransRequest(packhead.uin, RecvInfo);
			break;
		case MSG_TRANSFER_ANSWER://�Ựת������
			RecvComTransAnswer(packhead.uin, RecvInfo);
			break;
		case MSG_NORMAL:
		case MSG_WX:
			pWebUser = GetWebUserObjectByUid(RecvInfo.msg.senduin);
			if (pWebUser == NULL)
			{
				//��Ҫȥ��������ȡ�÷ÿ���Ϣ
				string l_msg = RecvInfo.msg.strmsg;
				if (l_msg != "null")
				{
					SendTo_GetWebUserInfo(RecvInfo.msg.senduin, RecvInfo.strChatid);

					int pos = l_msg.find("\"msgtype\":\"userinfo\"");
					int pos1 = l_msg.find("\"msgtype\":\"wxactoken\"");
					if (pos < 0 && pos1 < 0)
					{
						SaveEarlyMsg(&(RecvInfo.msg));
					}
					else
					{
						g_WriteLog.WriteLog(C_LOG_ERROR, "userinfo��Ϣ������Ϣ��¼bug��%s", RecvInfo.msg.strmsg);
					}
				}
			}
			else
			{
				WxMsgBase* pWxMsg = NULL;
				if (strcmp(RecvInfo.msg.strfontinfo, "JSON=WX") == 0)
				{
					if (!pWebUser->m_bIsFrWX)
					{
						pWebUser->m_bIsFrWX = true;
						m_handlerMsgs->RecvWebUserInfo(pWebUser);
					}
					pWxMsg = ParseWxMsg(pWebUser, RecvInfo.msg.strmsg, NULL, RecvInfo.msg.sendtime);

					if (pWxMsg == NULL)
					{
						goto RETURN;
					}
				}
				//��ҳ�����ĻỰ������chatid��仯����Ҫ��ʱ����
				if (strcmp(pWebUser->chatid, RecvInfo.strChatid) != 0 && strlen(RecvInfo.strChatid) > 0)
					strcpy(pWebUser->chatid, RecvInfo.strChatid);
				pWebUser->m_bNewComm = false;

				if (strstr(RecvInfo.msg.strmsg, "[ϵͳ��Ϣ]") != NULL
					&& strstr(RecvInfo.msg.strmsg, "����վ���ÿ���") != NULL)
				{
					SendTo_GetWebUserInfo(RecvInfo.msg.senduin, RecvInfo.strChatid);
				}

				if (pWebUser->info.name[0] == '\0')
				{
					strcpy(pWebUser->info.name, RecvInfo.msg.strmobile);
					if (strlen(pWebUser->info.name) < 2)
					{
						g_WriteLog.WriteLog(C_LOG_ERROR, "RecvComSendMsg1 name length��%d", strlen(pWebUser->info.name));
					}
				}
				else if (!pWebUser->info.nameflag && strcmp(RecvInfo.msg.strmobile, pWebUser->info.name) != 0)
				{
					if (RecvInfo.msg.strmobile[0] != '\0')
					{
						strcpy(pWebUser->info.name, RecvInfo.msg.strmobile);
						if (strlen(pWebUser->info.name) < 2)
						{
							g_WriteLog.WriteLog(C_LOG_ERROR, "RecvComSendMsg2 name length��%d", strlen(pWebUser->info.name));
						}
					}
					else
					{
						if (strlen(pWebUser->info.name) < 2)
						{
							g_WriteLog.WriteLog(C_LOG_ERROR, "RecvComSendMsg3 name length��%d", strlen(pWebUser->info.name));
						}
					}
				}

				if (m_sysConfig->IsWebuserSidForbid(pWebUser->info.sid))
				{
					//��Ϣ�����ˣ�ֱ������������
					goto RETURN;
				}

				if (!(strstr(RecvInfo.msg.strmsg, "[ϵͳ��Ϣ]") != NULL && strstr(RecvInfo.msg.strmsg, "�����뿪") != NULL))
				{
					pWebUser->m_resptimeoutmsgtimer = -1;
					pWebUser->m_resptimeoutclosetimer = -1;
					if (pWebUser->m_waitresptimeouttimer < 0)
						pWebUser->m_waitresptimeouttimer = 0;	// �ÿͷ�˵���� [12/29/2010 SC]

					if (RecvInfo.msg.bak != 0)
					{
						msgType = MSG_TYPE_PREV;
					}

					AddMsgToList((IBaseObject*)pWebUser, msgFrom, MSG_RECV_ERROR, GetMsgId(), msgType, MSG_DATA_TYPE_TEXT,
						RecvInfo.msg.strmsg, GetTimeByMDAndHMS(RecvInfo.msg.sendtime), NULL, pWxMsg);

					if ((RecvInfo.msg.bak == MSG_BAK_NORMAL) && !pWebUser->m_bConnected)
					{
						if (m_vistor)
						{
							m_vistor->SendWebuserTalkBegin(pWebUser);
						}
						else
						{
							g_WriteLog.WriteLog(C_LOG_ERROR,"RecvComSendMsg visit������δ��¼ǰ���յ���Ϣ");
						}
						pWebUser->cTalkedSatus = INTALKING;
						pWebUser->talkuid = m_userInfo.UserInfo.uid;

						pWebUser->onlineinfo.talkstatus = TALKSTATUS_TALK;
						pWebUser->transferuid = 0;
						pWebUser->m_nWaitTimer = 0;
						pWebUser->m_bConnected = true;
						//�ÿ����ˣ��϶���������
						if (pWebUser->info.status == STATUS_OFFLINE)
							pWebUser->info.status = STATUS_ONLINE;

						// ��ʾ
					}
					else if (RecvInfo.msg.bak == MSG_BAK_NORMAL)
					{
						pWebUser->cTalkedSatus = INTALKING;

						// ��ʾ 
					}

					if (m_sysConfig->m_bAutoRespUnnormalStatus)
					{
						string strResp;

						switch (m_nOnLineStatus)
						{
						case STATUS_BUSY:
						case STATUS_WORK:
						case STATUS_REFUSE_NEWWEBMSG:
							strResp = m_sysConfig->m_sUnnormalStatusMsg;
							break;
						case STATUS_LEAVE:
						case STATUS_EATING:
						case STATUS_OFFLINE:
							strResp = m_sysConfig->m_sUnnormalStatusMsg;
							break;
						}

						if (!strResp.empty())
						{
							SendAutoRespMsg(pWebUser, strResp.c_str());
						}
					}
				}
				else
				{
					pWebUser->cTalkedSatus = HASTALKED;
					if (m_vistor)
					{
						m_vistor->SendWebuserTalkEnd(pWebUser);
					}
					else
					{
						g_WriteLog.WriteLog(C_LOG_ERROR, "RecvComSendMsg visit������δ��¼ǰ���յ�������Ϣ");
					}
					
					if (pWebUser->m_bConnected)
					{
						pWebUser->m_bConnected = false;
						pWebUser->m_nWaitTimer = -20;
						pWebUser->onlineinfo.talkstatus = TALKSTATUS_NO;
					}
				}

				pWebUser->m_nWaitTimer = 0;
			}
			break;
		default:
			break;
		}
	}
	else
	{
		//�ͷ�������Ϣ
		msgFrom = MSG_FROM_CLIENT;
		pUser = GetUserObjectByUid(RecvInfo.msg.senduin);
		if (pUser == NULL)
		{
			pUser = AddUserObject(RecvInfo.msg.senduin, "", RecvInfo.msg.strmobile, STATUS_ONLINE, -1);
			// ���ظ���ϯ��ͷ��
			pUser->DownLoadFace(m_initConfig.webpage_DownloadHeadImage);
			if (pUser == NULL)
			{
				goto RETURN;
			}
			pUser->m_nWaitTimer = 0;

			AddMsgToList((IBaseObject*)pUser, msgFrom, MSG_RECV_ERROR, GetMsgId(), msgType, MSG_DATA_TYPE_TEXT,
				RecvInfo.msg.strmsg, GetTimeByMDAndHMS(RecvInfo.msg.sendtime), NULL, NULL);
			if ((RecvInfo.msg.bak == MSG_BAK_NORMAL || RecvInfo.msg.bak == MSG_BAK_AUTOANSER) && !(pUser->m_bInnerTalk))
			{
				pUser->m_bInnerTalk = true;
			}
		}
		else
		{
			strncpy(pUser->UserInfo.nickname, RecvInfo.msg.strmobile, MAX_USERNAME_LEN);//�˴��᷵�ء�ϵͳ������
			pUser->m_nWaitTimer = 0;

			AddMsgToList((IBaseObject*)pUser, msgFrom, MSG_RECV_ERROR, GetMsgId(), msgType, MSG_DATA_TYPE_TEXT,
				RecvInfo.msg.strmsg, GetTimeByMDAndHMS(RecvInfo.msg.sendtime), NULL, NULL);

			if ((RecvInfo.msg.bak == MSG_BAK_NORMAL || RecvInfo.msg.bak == MSG_BAK_AUTOANSER) && !(pUser->m_bInnerTalk))
			{
				pUser->m_bInnerTalk = true;
			}
		}
		
		// ��ʾ
	}

RETURN:
	SendAckEx(RecvInfo.msg.seq, RecvInfo.msg.senduin, RecvInfo.sendip, RecvInfo.sendport);
	nError = 0;
	return 0;
}

int CChatManager::RecvFloatCreateChat(PACK_HEADER packhead, char *pRecvBuff, int len)
{
	COM_FLOAT_CREATECHAT RecvInfo(packhead.ver, packhead.random);
	int nError = 0;
	CUserObject *pUser = NULL;
	CWebUserObject *pWebUser = NULL;

	nError = UnPack(&RecvInfo, pRecvBuff, len);
	if (nError != 0)
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "RecvFloatCreateChat unpack failed,Cmd:%.4x", packhead.cmd);
		goto RETURN;
	}
	else
	{
		g_WriteLog.WriteLog(C_LOG_TRACE, "RecvFloatCreateChat uAdminId:%u,usort:%hu,uWebuin:%u,chatid:%s,clienttid:%s,webname:%s,uKefu:%u,uFromAdmin:%u,uFromSort:%hu",
			RecvInfo.uAdminId, RecvInfo.usort, RecvInfo.uWebuin, RecvInfo.chatid, RecvInfo.clienttid, RecvInfo.webname, RecvInfo.uKefu, RecvInfo.uFromAdmin, RecvInfo.uFromSort);
	}
	if (0 == RecvInfo.clienttid[0])
	{
		//����cookie��û��clientid�Ի�����
		sprintf(RecvInfo.clienttid, "%lu", RecvInfo.uWebuin);
		g_WriteLog.WriteLog(C_LOG_TRACE, "RecvFloatCreateChat clientid is empty,set as webuin:%u", RecvInfo.uWebuin);
	}
	pWebUser = GetWebUserObjectBySid(RecvInfo.clienttid);
	if (pWebUser == NULL)
	{
		pWebUser = AddWebUserObject(RecvInfo.clienttid, RecvInfo.thirdid, RecvInfo.webname, "", "", STATUS_ONLINE, 0);
		pWebUser->m_onlinetime = 0;
		pWebUser->m_bIsShow = false;
	}

	if (RecvInfo.webname[0] != '\0' && strcmp(RecvInfo.webname, pWebUser->info.name) != 0)
	{
		strcpy(pWebUser->info.name, RecvInfo.webname);
		if (strlen(pWebUser->info.name) < 2)
		{
			g_WriteLog.WriteLog(C_LOG_ERROR, "RecvFloatCreateChat name length��%d", strlen(pWebUser->info.name));
		}
	}

	SendTo_GetWebUserInfo(RecvInfo.uWebuin, RecvInfo.chatid);

	pWebUser->cTalkedSatus = INTALKING;
	if (RecvInfo.uFromAdmin == 0)
	{
		pWebUser->nVisitFrom = WEBUSERICON_DIRECTURL;
	}
	else if (RecvInfo.uFromAdmin != m_login->m_authAdminid)
	{
		pWebUser->nVisitFrom = WEBUSERICON_UNIONURL;
	}

	pWebUser->webuserid = RecvInfo.uWebuin;
	if (strcmp(pWebUser->chatid, RecvInfo.chatid) != 0 && strlen(RecvInfo.chatid) > 0)
		strcpy(pWebUser->chatid, RecvInfo.chatid);
	pWebUser->floatadminuid = RecvInfo.uAdminId;
	pWebUser->floatfromadminuid = RecvInfo.uFromAdmin;
	pWebUser->floatfromsort = RecvInfo.uFromSort;
	pWebUser->gpid = packhead.random;
	pWebUser->m_bNewComm = true;
	pWebUser->m_sNewSeq = packhead.sendrandom; //������Ϣid
	pWebUser->info.status = STATUS_ONLINE;
	pWebUser->talkuid = 0;//�ȴ�Ӧ������ĻỰ����û�н���ͷ�
	if (RecvInfo.uKefu == 0)
	{
		if (pWebUser->onlineinfo.talkstatus != TALKSTATUS_REQUEST)
		{
			pWebUser->onlineinfo.talkstatus = TALKSTATUS_REQUEST;
			pWebUser->m_nWaitTimer = 0;

			char msg[MAX_256_LEN];
			sprintf(msg, "�û� %s ����Ի�!", pWebUser->info.name);

			AddMsgToList((IBaseObject*)pWebUser, MSG_FROM_SYS, MSG_RECV_ERROR, GetMsgId(), MSG_TYPE_NORMAL, MSG_DATA_TYPE_TEXT,
				msg, GetTimeByMDAndHMS(0), NULL, NULL);
		}
	}
	else
	{
		pWebUser->onlineinfo.talkstatus = TALKSTATUS_TALK;
		pUser = GetUserObjectByUid(RecvInfo.uKefu);

		if (pUser != NULL &&(pUser->UserInfo.uid == m_userInfo.UserInfo.uid || pUser->m_bFriend))
		{
			pWebUser->transferuid = 0;
			if (RecvInfo.uKefu == m_userInfo.UserInfo.uid)
			{
				//���Լ�����ķÿ�
				pWebUser->m_nWaitTimer = 0;
				pWebUser->m_bConnected = true;
			}
			else
			{
				//����������ķÿ�
				pWebUser->m_nWaitTimer = -20;
				pWebUser->m_bConnected = HASINSERTRIGHT;
			}

			pWebUser->talkuid = RecvInfo.uKefu;

			//��ȡ�Ự��ʷ��Ϣ���������µ���Ϣ
			//SendStartRecvFloatMsg(packhead.random, RecvInfo.uAdminId, RecvInfo.chatid, pWebUser->m_sNewSeq);

			char msg[MAX_256_LEN];
			sprintf(msg, "%s�����˷ÿ�%s�ĻỰ", pUser->UserInfo.nickname, pWebUser->info.name);
			
			AddMsgToList((IBaseObject*)pWebUser, MSG_FROM_SYS, MSG_RECV_ERROR, GetMsgId(), MSG_TYPE_NORMAL, MSG_DATA_TYPE_TEXT,
				msg, GetTimeByMDAndHMS(0), NULL, NULL);
		}
	}

	if (RecvInfo.memlist.find(m_userInfo.UserInfo.uid) != RecvInfo.memlist.end())
		pWebUser->m_bNotResponseUser = 0;
	else
		pWebUser->m_bNotResponseUser = 1;

	if (m_bLoginSuccess)
	{
		m_handlerMsgs->RecvChatInfo(pWebUser, pUser);
	}

	nError = 0;
RETURN:

	return nError;
}

int CChatManager::RecvFloatChatInfo(PACK_HEADER packhead, char *pRecvBuff, int len)
{
	COM_FLOAT_CHATINFO RecvInfo(packhead.ver, packhead.random);
	int nError = 0;
	CUserObject *pUser = NULL;
	CWebUserObject *pWebUser = NULL;
	std::vector<unsigned int>::iterator iter;
	char strMsg[MAX_256_LEN];

	strcpy(strMsg, "waiter");

	nError = UnPack(&RecvInfo, pRecvBuff, len);
	if (nError != 0)
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "RecvFloatChatInfo unpack failed,Cmd:%.4x", packhead.cmd);

		goto RETURN;
	}
	else
	{
		g_WriteLog.WriteLog(C_LOG_TRACE, "RecvFloatChatInfo uAdminId:%u,usort:%hu,uWebuin:%u,chatid:%s,clienttid:%s,webname:%s,uKefu:%u,uFromAdmin:%u,uFromSort:%hu",
			RecvInfo.uAdminId, RecvInfo.sSort, RecvInfo.uWebUin, RecvInfo.chatid, RecvInfo.strClientId, RecvInfo.webnickname, RecvInfo.uKefuUin, RecvInfo.uFromAmdind, RecvInfo.sFromSort);

	}

	pWebUser = GetWebUserObjectByUid(RecvInfo.uWebUin);
	if (pWebUser == NULL)
	{
		pWebUser = AddWebUserObject(RecvInfo.strClientId, RecvInfo.strThirdId, RecvInfo.webnickname, "", "", STATUS_UNDEFINE, 0);
		pWebUser->webuserid = RecvInfo.uWebUin;
	}
	else
	{
		ChangeWebUserSid(pWebUser, RecvInfo.strClientId, RecvInfo.strThirdId);
	}

	// ֻҪ�յ��Ự��Ϣ��һ��ȥ��������ȡwork_bill��
	SendTo_GetWebUserInfo(pWebUser->webuserid, RecvInfo.chatid, strMsg);
	if (pWebUser->m_nEMObType != OBJECT_WEBUSER)
	{
		pWebUser->m_nEMObType = OBJECT_WEBUSER;
	}
	pWebUser->m_bNewComm = true;
	pWebUser->cTalkedSatus = INTALKING;

	pWebUser->floatadminuid = RecvInfo.uAdminId;
	if (strcmp(pWebUser->chatid, RecvInfo.chatid) != 0 && strlen(RecvInfo.chatid) > 0)
		strcpy(pWebUser->chatid, RecvInfo.chatid);

	pWebUser->floatfromadminuid = RecvInfo.uFromAmdind;
	pWebUser->floatfromsort = RecvInfo.sFromSort;
	pWebUser->gpid = packhead.random;

	if (RecvInfo.uKefuUin == 0)
	{
		//�ȴ�Ӧ��
		pWebUser->onlineinfo.talkstatus = TALKSTATUS_REQUEST;
		pWebUser->info.status = STATUS_ONLINE;
		pWebUser->m_nWaitTimer = 0;

		SendStartRecvFloatMsg(packhead.random, RecvInfo.uAdminId, RecvInfo.chatid, pWebUser->m_sNewSeq);
		m_handlerMsgs->RecvChatInfo(pWebUser);
	}
	else
	{
		if (m_nNextInviteWebuserUid == pWebUser->webuserid)
		{
			pWebUser->onlineinfo.talkstatus = TALKSTATUS_INVITE;
			pWebUser->inviteuid = m_userInfo.UserInfo.uid;
			pWebUser->info.status = STATUS_ONLINE;
			pWebUser->frominviteuid = m_nNextInviteUid;
			pWebUser->m_nWaitTimer = 0;
			CUserObject *pInviteUser = GetUserObjectByUid(m_nNextInviteUid);
			m_nNextInviteWebuserUid = 0;
			m_nNextInviteUid = 0;
			m_handlerMsgs->RecvInviteUser(pWebUser, &m_userInfo);
		}
		else
		{
			pUser = NULL;
			if (m_userInfo.UserInfo.uid != RecvInfo.uKefuUin)
			{
				for (int i = 0; i < RecvInfo.InviteUin.size(); ++i)
				{
					if (RecvInfo.InviteUin[i] == m_userInfo.UserInfo.uid)
					{
						pUser = GetUserObjectByUid(m_userInfo.UserInfo.uid);
						pWebUser->onlineinfo.bInvited = true;
						break;
					}
				}
			}

			if (pUser == NULL)
			{
				pUser = GetUserObjectByUid(RecvInfo.uKefuUin);
				pWebUser->onlineinfo.bInvited = false;
			}

			if (RecvInfo.uTansferingToKefu == 0)
			{
				if (pUser != NULL)
				{
					if (pUser->UserInfo.uid == m_userInfo.UserInfo.uid ||
						pWebUser->onlineinfo.bInvited)
					{
						//���Լ�ת�Ƶķÿ�
						pWebUser->m_nWaitTimer = 0;
						pWebUser->m_bConnected = true;
					}
					else
					{
						//������ת�Ƶķÿ�
						pWebUser->m_bConnected = false;
						pWebUser->m_nWaitTimer = -20;
					}

					//���ڶԻ�
					pWebUser->onlineinfo.talkstatus = TALKSTATUS_TALK;
					pWebUser->info.status = STATUS_ONLINE;
					pWebUser->transferuid = 0;
					pWebUser->talkuid = pUser->UserInfo.uid;//�����û���Ҫ�ı�talkidΪ�Լ���

					char msg[MAX_256_LEN];
					sprintf(msg, "%s�����˷ÿ�%s�ĻỰ", pUser->UserInfo.nickname, pWebUser->info.name);

					AddMsgToList((IBaseObject*)pWebUser, MSG_FROM_SYS, MSG_RECV_ERROR, GetMsgId(), MSG_TYPE_NORMAL, MSG_DATA_TYPE_TEXT,
						msg, GetTimeByMDAndHMS(0), NULL, NULL);

					if (RecvInfo.uKefuUin&&pUser->m_bFriend
						|| !RecvInfo.uKefuUin&&!pWebUser->m_bNotResponseUser)
					{
						//��ȡ�Ự��ʷ��Ϣ���������µ���Ϣ
						SendStartRecvFloatMsg(packhead.random, RecvInfo.uAdminId, RecvInfo.chatid, pWebUser->m_sNewSeq);
					}

					m_handlerMsgs->RecvChatInfo(pWebUser, pUser);
				}
			}
			else
			{
				//����ת��
				pWebUser->onlineinfo.talkstatus = TALKSTATUS_TRANSFER;
				pWebUser->info.status = STATUS_ONLINE;
				pWebUser->transferuid = RecvInfo.uTansferingToKefu;
				CUserObject *pAcceptUser = GetUserObjectByUid(RecvInfo.uTansferingToKefu);

				//�����������htmleditor����ʾ��Ȼ�����ƶ�λ�ã���Ϊ�ƶ�λ�ÿ��ܻᵼ��������л��������ط�ͬ������
				if (strcmp(pWebUser->chatid, RecvInfo.chatid) != 0 && strlen(RecvInfo.chatid) > 0)
					strcpy(pWebUser->chatid, RecvInfo.chatid);
				pWebUser->floatadminuid = RecvInfo.uAdminId;

				char msg[MAX_256_LEN];
				if (pUser != NULL)
					GetInviteChatSysMsg(msg, pUser, pWebUser, INVITE_TRANSFER, pAcceptUser);
				else if (pAcceptUser != NULL)
					sprintf(msg, "�ÿ� %s ת�Ƶ� %s", pWebUser->info.name, pAcceptUser->UserInfo.nickname);

				AddMsgToList((IBaseObject*)pWebUser, MSG_FROM_SYS, MSG_RECV_ERROR, GetMsgId(), MSG_TYPE_NORMAL, MSG_DATA_TYPE_TEXT,
					msg, GetTimeByMDAndHMS(0), NULL, NULL);

				m_handlerMsgs->RecvTransferUser(pWebUser,pAcceptUser);
			}
		}
	}

	pWebUser->m_bNotResponseUser = 1;
	for (iter = RecvInfo.webRecvUin.begin(); iter != RecvInfo.webRecvUin.end(); iter++)
	{
		unsigned long uid = (unsigned long)(*iter);

		if (uid == m_userInfo.UserInfo.uid)
		{
			pWebUser->m_bNotResponseUser = 0;
			break;
		}
	}

	for (iter = RecvInfo.InviteUin.begin(); iter != RecvInfo.InviteUin.end(); iter++)
	{
		unsigned long uid = (unsigned long)(*iter);

		pWebUser->AddCommonTalkId(uid);
	}	

	nError = 0;
RETURN:

	return nError;
}

int CChatManager::RecvFloatChatMsg(PACK_HEADER packhead, char *pRecvBuff, int len)
{
	if (packhead.uin == m_userInfo.UserInfo.uid)
		return 0;

	int nError = 0;
	MSG_TYPE msgType = MSG_TYPE_NORMAL;
	MSG_FROM_TYPE msgFrom = MSG_FROM_WEBUSER;
	CUserObject *pAssistUser = NULL;
	CWebUserObject *pWebUser = NULL;
	WxMsgBase* pWxMsg = NULL;

	COM_FLOAT_CHATMSG RecvInfo(packhead.ver, packhead.random);
	nError = UnPack(&RecvInfo, pRecvBuff, len);
	if (nError != 0)
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "RecvFloatChatMsg unpack failed,Cmd:%.4x", packhead.cmd);
		goto RETURN;
	}

	pWebUser = GetWebUserObjectByUid(RecvInfo.webuin);
	if (pWebUser == NULL)
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "RecvFloatChatMsg getwebuserobjectbyid(%u) failed", RecvInfo.webuin);
		goto RETURN;
	}

	if (pWebUser->m_sNewSeq < packhead.sendrandom)
	{
		pWebUser->m_sNewSeq = packhead.sendrandom;
	}

	g_WriteLog.WriteLog(C_LOG_TRACE, "RecvFloatChatMsg chatid:%s,uAdminId:%u,uWebuin:%u,msgtype:%d,sendname:%s,strfontinfo:%s,msg:%s",
		RecvInfo.chatid, RecvInfo.uAdminId, RecvInfo.webuin, RecvInfo.nMsgType, RecvInfo.nickname, RecvInfo.strfontinfo, RecvInfo.strmsg);

	if (pWebUser->m_sWxAppid.empty()) //WxAppidΪ�գ�ȥ����˻�ȡ
	{
		SendTo_GetWebUserInfo(pWebUser->webuserid, RecvInfo.chatid);
	}	

	if (RecvInfo.strRand[0] != 0)
	{
		strcpy(pWebUser->info.sid, RecvInfo.strRand);
	}

	if (RecvInfo.strThirdid[0] != 0)
	{
		strcpy(pWebUser->info.thirdid, RecvInfo.strThirdid);
	}

	if (packhead.uin != RecvInfo.webuin)
	{
		// ����΢���û���������Ϣ������Э������������Ϣ
		pAssistUser = GetUserObjectByUid(packhead.uin);
		if (pAssistUser == NULL)
		{
			// ����uin���ǳ��ȳ�ʼ����ϯ����Ϣ
			pAssistUser = AddUserObject(packhead.uin, "", RecvInfo.nickname, STATUS_ONLINE, -1);
			pAssistUser->DownLoadFace(m_initConfig.webpage_DownloadHeadImage);

			// Ȼ����¸���ϯ��������Ϣ
			SendTo_GetUserInfo(packhead.uin);
		}
	}

	if (packhead.langtype == LANGUAGE_UTF8)
	{
		ConvertMsg(RecvInfo.strmsg, sizeof(RecvInfo.strmsg) - 1);

		ConvertMsg(RecvInfo.nickname, sizeof(RecvInfo.nickname) - 1);
	}

	if (strcmp(RecvInfo.strfontinfo, "JSON=WX") == 0)
	{
		if (!pWebUser->m_bIsFrWX)
		{
			pWebUser->m_bIsFrWX = true;
			m_handlerMsgs->RecvWebUserInfo(pWebUser);
		}
		pWxMsg = ParseWxMsg(pWebUser, RecvInfo.strmsg, pAssistUser,RecvInfo.tMsgTime);

		if (pWxMsg == NULL)
		{
			goto RETURN;
		}
	}
	else
	{
		if (ParseTextMsg(pWebUser, RecvInfo.strmsg, pAssistUser, RecvInfo.tMsgTime))
		{
			goto RETURN;
		}

		std::string content = RecvInfo.strmsg;
		bool isUrl = PathIsURLA(content.c_str());
		if (isUrl && pWebUser->m_bIsFrWX)
		{
			content = "<a href=\"" + content + "\" target=\"blank\">" + content + "</a>";
		}

		TransferStrToFace(content);
		ReplaceFaceId(content);
		strncpy(RecvInfo.strmsg, content.c_str(), MAX_MSG_RECVLEN);
	}

	//�ÿͷ�����Ϣ
	switch (RecvInfo.nMsgType)
	{
	case MSG_FLOAT_NORMAL:
	case MSG_FLOAT_PRV:
	case MSG_FLOAT_INPUTING:
	case MSG_WX://΢����Ϣ 
	{
		if (pWebUser->m_nWaitTimer<0)
		{
			pWebUser->m_nWaitTimer = 0;
		}

		// �Ŷӵȴ����ܺ���Ϣ�������� [12/9/2010 SC]
		if (m_sysConfig->IsWebuserSidForbid(pWebUser->info.sid))
		{
			//��Ϣ�����ˣ�ֱ������������
			goto RETURN;
		}

		if (RecvInfo.nMsgType == MSG_FLOAT_NORMAL)
		{
			if (pAssistUser == NULL)
			{
				// �ÿͷ�����Ϣʱ����δӦ��ȴ�ʱ������
				pWebUser->m_resptimeoutmsgtimer = -1;
				pWebUser->m_resptimeoutclosetimer = -1;

				if (pWebUser->m_waitresptimeouttimer < 0)
					pWebUser->m_waitresptimeouttimer = 0;
			}
			else
			{
				msgFrom = MSG_FROM_ASSIST;
			}

			// ����Ӧ������Ϣ��ʾ
		}
		else
		{
			msgType = MSG_TYPE_PREV;
		}

		if (RecvInfo.tMsgTime == 0)
		{
			// ��ȡϵͳ��ǰʱ��
		}

		string msgId = GetMsgId();
		
		AddMsgToList((IBaseObject*)pWebUser, msgFrom, MSG_RECV_ERROR, msgId, msgType, (MSG_DATA_TYPE)RecvInfo.nMsgDataType,
			RecvInfo.strmsg, GetTimeByMDAndHMS(RecvInfo.tMsgTime), pAssistUser, pWxMsg);

		// ͬ�����¹�����
		//if (m_sysConfig->m_bAutoSearchKeyword)
		{
			//CUserObject *pUser = m_pFormKeyWord->GetCurSelUserOb();
			//m_pFormKeyWord->ResetKeyWord((m_nOnLineStatus != STATUS_OFFLINE), pUser, RecvInfo.strmsg);
		}

		if (m_sysConfig->m_bAutoRespUnnormalStatus)
		{
			string strResp;

			switch (this->m_nOnLineStatus)
			{
			case STATUS_BUSY:
			case STATUS_WORK:
			case STATUS_REFUSE_NEWWEBMSG:
				strResp = m_sysConfig->m_sUnnormalStatusMsg;
				break;
			case STATUS_LEAVE:
			case STATUS_EATING:
			case STATUS_OFFLINE:
				strResp = m_sysConfig->m_sUnnormalStatusMsg;
				break;
			}

			if (!strResp.empty())
			{
				SendAutoRespMsg(pWebUser, strResp.c_str());
			}
		}
	}
		break;
	case MSG_WORKBILL:
	case MSG_CLIENT_COME:
		g_WriteLog.WriteLog(C_LOG_TRACE, "RecvFloatChatMsg MSG_WORKBILL msgtype--");
		RecvComSendWorkBillMsg(RecvInfo.webuin, -1, RecvInfo.strmsg, RecvInfo.nickname);
		break;
	default:
		g_WriteLog.WriteLog(C_LOG_ERROR, "RecvFloatChatMsg unkown msgtype:%d", RecvInfo.nMsgType);
		break;
	}

	nError = 0;
RETURN:
	if (pWxMsg)
	{
		delete pWxMsg;
	}
	return nError;
}

int CChatManager::RecvFloatChatMsgAck(PACK_HEADER packhead, char *pRecvBuff, int len)
{
	COM_FLOAT_CHATMSG_ACK RecvInfo(packhead.ver, packhead.random);
	int nError = 0;
	CUserObject *pUser = NULL;
	CWebUserObject *pWebUser = NULL;

	nError = UnPack(&RecvInfo, pRecvBuff, len);
	if (nError != 0)
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "RecvFloatChatMsgAck unpack failed,Cmd:%.4x", packhead.cmd);

		goto RETURN;
	}

	g_WriteLog.WriteLog(C_LOG_TRACE, "RecvFloatChatMsgAck uAdminId:%u,uWebuin:%u,chatid:%s,msgseq:%hu",
		RecvInfo.uAdminId, RecvInfo.webuin, RecvInfo.chatid, RecvInfo.chatseq);

	pWebUser = GetWebUserObjectByUid(RecvInfo.webuin);

	if (pWebUser != NULL)
		pWebUser->m_sNewSeq = RecvInfo.chatseq;

	nError = 0;
RETURN:

	return nError;
}

int CChatManager::RecvFloatAcceptChat(PACK_HEADER packhead, char *pRecvBuff, int len)
{
	COM_FLOAT_ACCEPTCHAT RecvInfo(packhead.ver, packhead.random);
	int nError = 0;
	CUserObject *pUser = NULL;
	CWebUserObject *pWebUser = NULL;
	int nRet = 0;
	nError = UnPack(&RecvInfo, pRecvBuff, len);
	if (nError != 0)
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "RecvFloatAcceptChat unpack failed,Cmd:%.4x", packhead.cmd);
		goto RETURN;
	}

	g_WriteLog.WriteLog(C_LOG_TRACE, "RecvFloatAcceptChat sender:%u,uAdminId:%u,uWebuin:%u,chatid:%s,nickname:%s",
		packhead.uin, RecvInfo.uAdminId, RecvInfo.uWebuin, RecvInfo.chatid, RecvInfo.nickname);


	pUser = GetUserObjectByUid(packhead.uin);
	pWebUser = GetWebUserObjectByUid(RecvInfo.uWebuin);
	if (pWebUser == NULL)
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "RecvFloatAcceptChat GetWebUserObjectByID(%u) failed", RecvInfo.uWebuin);
		goto RETURN;
	}
	if (m_userInfo.UserInfo.uid != packhead.uin)//�����߲��ǵ�ǰ��ϯ
	{
		pWebUser->m_nWaitTimer = -20;
		pWebUser->m_bConnected = HASINSERTRIGHT;
	}
	pWebUser->onlineinfo.talkstatus = TALKSTATUS_TALK;
	pWebUser->transferuid = 0;
	pWebUser->talkuid = packhead.uin;
	if (pUser == NULL || (m_userInfo.UserInfo.uid != pUser->UserInfo.uid&&!pUser->m_bFriend))
	{
		pWebUser->cTalkedSatus = HASTALKED;
		g_WriteLog.WriteLog(C_LOG_ERROR, "RecvFloatAcceptChat delete user name is :%s, chatid: %s", pWebUser->info.name, pWebUser->chatid);
	}
	else
	{
		pWebUser->cTalkedSatus = INTALKING;
		pWebUser->onlineinfo.talkstatus = TALKSTATUS_TALK;
		if (packhead.uin == m_userInfo.UserInfo.uid)
		{
			//���Լ���������ķÿ�
			pWebUser->m_nWaitTimer = 0;
			pWebUser->m_bConnected = true;
		}

		//����������ķÿ�
		pWebUser->RemoveMutiUser(pWebUser->talkuid);
		pWebUser->inviteuid = 0;

		char msg[MAX_256_LEN];
		sprintf(msg, "%s�����˷ÿ�%s�ĻỰ", pUser->UserInfo.nickname, pWebUser->info.name);

		AddMsgToList((IBaseObject*)pWebUser, MSG_FROM_SYS, MSG_RECV_ERROR, GetMsgId(), MSG_TYPE_NORMAL, MSG_DATA_TYPE_TEXT,
			msg, GetTimeByMDAndHMS(0), NULL, NULL);

		//��ȡ�Ự��ʷ��Ϣ���������µ���Ϣ
		SendStartRecvFloatMsg(packhead.random, RecvInfo.uAdminId, RecvInfo.chatid, pWebUser->m_sNewSeq);
	}

	m_handlerMsgs->RecvAcceptChat(pWebUser, pUser);

	nError = 0;
RETURN:

	return nError;
}

int CChatManager::RecvFloatTransQuest(PACK_HEADER packhead, char *pRecvBuff, int len)
{
	COM_FLOAT_TRANSREQUEST RecvInfo(packhead.ver, packhead.random);
	int nError = 0;
	CUserObject *pInviteUser = NULL;
	CUserObject *pAcceptUser = NULL;
	CWebUserObject *pWebUser = NULL;
	nError = UnPack(&RecvInfo, pRecvBuff, len);
	if (nError != 0)
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "RecvFloatTransQuest unpack failed,Cmd:%.4x", packhead.cmd);
		goto RETURN;
	}

	g_WriteLog.WriteLog(C_LOG_TRACE, "RecvFloatTransQuest sender:%u,uAdminId:%u,uWebuin:%u,chatid:%s",
		packhead.uin, RecvInfo.uAdminId, RecvInfo.uWebuin, RecvInfo.chatid);

	pAcceptUser = GetUserObjectByUid(RecvInfo.uToKefu);
	pInviteUser = GetUserObjectByUid(packhead.uin);
	pWebUser = GetWebUserObjectByUid(RecvInfo.uWebuin);
	if (pAcceptUser == NULL)//��ȡ�������Ͳ����Լ���Э������
	{
		goto RETURN;
	}

	char msg[MAX_256_LEN];
	if (pWebUser == NULL)
	{
		SendTo_GetWebUserChatInfo(packhead.random, RecvInfo.uAdminId, RecvInfo.chatid);
	}
	else if (RecvInfo.uToKefu == m_userInfo.UserInfo.uid)//��ǰ��ϯ�ǽ����ߣ��ƶ���ת���С�����
	{
		if (strcmp(pWebUser->chatid, RecvInfo.chatid) != 0 && strlen(RecvInfo.chatid) > 0)
			strcpy(pWebUser->chatid, RecvInfo.chatid);
		pWebUser->floatadminuid = RecvInfo.uAdminId;
		pWebUser->m_bNewComm = true;
		pWebUser->gpid = packhead.random;
		pWebUser->info.status = STATUS_ONLINE;
		pWebUser->onlineinfo.bInvited = false;//ת�ӹ������Ͳ�������Э����
		pWebUser->onlineinfo.talkstatus = TALKSTATUS_TRANSFER;
		pWebUser->transferuid = RecvInfo.uToKefu;

		if (pInviteUser != NULL)
			GetInviteChatSysMsg(msg, pInviteUser, pWebUser, INVITE_TRANSFER, pAcceptUser);
		else
			sprintf(msg, "�ÿ� %s ת�Ƶ� %s", pWebUser->info.name, pAcceptUser->UserInfo.nickname);

		AddMsgToList((IBaseObject*)pWebUser, MSG_FROM_SYS, MSG_RECV_ERROR, GetMsgId(), MSG_TYPE_NORMAL, MSG_DATA_TYPE_TEXT,
			msg, GetTimeByMDAndHMS(0), NULL, NULL);
		
		m_handlerMsgs->RecvTransferUser(pWebUser, pAcceptUser);
	}
	if (packhead.uin == m_userInfo.UserInfo.uid)//��ǰ��ϯ�Ƿ���ת����
	{
		sprintf(msg, "�ѷ���ת������%s", pAcceptUser->UserInfo.nickname);

		AddMsgToList((IBaseObject*)pWebUser, MSG_FROM_SYS, MSG_RECV_ERROR, GetMsgId(), MSG_TYPE_NORMAL, MSG_DATA_TYPE_TEXT,
			msg, GetTimeByMDAndHMS(0), NULL, NULL);
	}
	nError = 0;
RETURN:
	return nError;
}

int CChatManager::RecvFloatTransFailed(PACK_HEADER packhead, char *pRecvBuff, int len)
{
	COM_FLOAT_TRANSFAILED RecvInfo(packhead.ver, packhead.random);
	int nError = 0;

	CUserObject *pInviteUser = NULL;
	CUserObject *pAcceptUser = NULL;
	CWebUserObject *pWebUser = NULL;
	HTREEITEM hItem = NULL;
	nError = UnPack(&RecvInfo, pRecvBuff, len);
	if (nError != 0)
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "RecvFloatTransFailed unpack failed,Cmd:%.4x", packhead.cmd);
		goto RETURN;
	}

	g_WriteLog.WriteLog(C_LOG_TRACE, "RecvFloatTransFailed sender:%u,uAdminId:%u,uWebuin:%u,chatid:%s,TransFromKefu:%u,to:%u",
		packhead.uin, RecvInfo.uAdminId, RecvInfo.uWebuin, RecvInfo.chatid, RecvInfo.uKefu, RecvInfo.TransToKefu);

	pInviteUser = GetUserObjectByUid(RecvInfo.uKefu);
	pWebUser = GetWebUserObjectByUid(RecvInfo.uWebuin);
	pAcceptUser = GetUserObjectByUid(RecvInfo.TransToKefu);
	if (pWebUser == NULL)
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "RecvFloatTransFailed GetWebUserObjectByID(%u) failed", RecvInfo.uWebuin);
		goto RETURN;
	}
	pWebUser->onlineinfo.talkstatus = TALKSTATUS_TALK;//TALKSTATUS_REQUEST --> TALKSTATUS_TALK

	if (pInviteUser != NULL)
	{
		//hItem = m_pFormUser->m_TreeListUser.GetChildItemByData((DWORD)pInviteUser, TREELPARAM_USER_TALK);
	}


	//������Ҫ�ж��ǲ���Ҫ������ʾ���û�
	if (pWebUser->m_bNotResponseUser || hItem == NULL&&pInviteUser != NULL || pInviteUser == NULL)
	{
		pWebUser->cTalkedSatus = HASTALKED;
		//m_pFormUser->m_TreeListUser.DeleteItemByLParam((LPARAM)pWebUser);
	}
	else
	{
		char msg[MAX_256_LEN];
		sprintf(msg, "�ÿ� %s ת��ʧ��", pWebUser->info.name);
		pWebUser->cTalkedSatus = INTALKING;
		pWebUser->transferuid = 0;

		AddMsgToList((IBaseObject*)pWebUser, MSG_FROM_SYS, MSG_RECV_ERROR, GetMsgId(), MSG_TYPE_NORMAL, MSG_DATA_TYPE_TEXT,
			msg, GetTimeByMDAndHMS(0), NULL, NULL);

		// ת�ӳ�ʱ�Ự�ص�����ת����ϯ�����ǵȴ�Ӧ��
		if (m_userInfo.UserInfo.uid != pInviteUser->UserInfo.uid) 
		{
			sprintf(msg, "�û� %s ����Ի�!", pWebUser->info.name);
		}
	}

	nError = 0;
RETURN:

	return nError;
}

int CChatManager::RecvInviteRequest(PACK_HEADER packhead, char *pRecvBuff, int len)
{
	COM_FLOAT_INVITEREQUEST RecvInfo(packhead.ver, packhead.random);
	int nError = 0;
	CUserObject *pInviteUser = NULL;
	CUserObject *pAcceptUser = NULL;
	CWebUserObject *pWebUser = NULL;
	char msg[MAX_256_LEN];

	nError = UnPack(&RecvInfo, pRecvBuff, len);
	if (nError != 0)
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "RecvInviteRequest unpack failed,Cmd:%.4x", packhead.cmd);
		goto RETURN;
	}

	g_WriteLog.WriteLog(C_LOG_TRACE, "RecvInviteRequest sender:%u,uAdminId:%u,uWebuin:%u,chatid:%s,uInviteUser:%u",
		packhead.uin, RecvInfo.uAdminId, RecvInfo.uWebuin, RecvInfo.chatid, RecvInfo.uInviteUser);


	pWebUser = GetWebUserObjectByUid(RecvInfo.uWebuin);
	pInviteUser = GetUserObjectByUid(packhead.uin);
	pAcceptUser = GetUserObjectByUid(RecvInfo.uInviteUser);
	if (pWebUser == NULL)
	{
		if (RecvInfo.uInviteUser == m_userInfo.UserInfo.uid)
		{
			m_nNextInviteUid = packhead.uin;
			m_nNextInviteWebuserUid = RecvInfo.uWebuin;

			SendTo_GetWebUserChatInfo(packhead.random, RecvInfo.uAdminId, RecvInfo.chatid);
		}

		g_WriteLog.WriteLog(C_LOG_ERROR, "RecvInviteRequest GetWebUserObjectByID(%u) failed", RecvInfo.uWebuin);
		goto RETURN;
	}
	else
	{
		pWebUser->frominviteuid = packhead.uin;

	}

	if (RecvInfo.uInviteUser == m_userInfo.UserInfo.uid)
	{
		pWebUser->onlineinfo.talkstatus = TALKSTATUS_INVITE;
		pWebUser->inviteuid = m_userInfo.UserInfo.uid;
		if (!pWebUser->m_bNewComm)
		{
			pWebUser->m_bNewComm = true;
		}
		pWebUser->m_nWaitTimer = 0;

		//�����������htmleditor����ʾ��Ȼ�����ƶ�λ�ã���Ϊ�ƶ�λ�ÿ��ܻᵼ��������л��������ط�ͬ������
		GetInviteChatSysMsg(msg, pInviteUser, pWebUser, INVITE_HELP, pAcceptUser);

		m_handlerMsgs->RecvInviteUser(pWebUser, pAcceptUser);
	}

	nError = 0;
RETURN:

	return nError;
}

int CChatManager::RecvInviteResult(PACK_HEADER packhead, char *pRecvBuff, int len)
{
	COM_FLOAT_INVITERESULT RecvInfo(packhead.ver, packhead.random);
	int nError = 0;
	CUserObject *pInviteUser = NULL;
	CUserObject *pAcceptUser = NULL;
	CWebUserObject *pWebUser = NULL;

	nError = UnPack(&RecvInfo, pRecvBuff, len);
	if (nError != 0)
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "RecvInviteResult unpack failed,Cmd:%.4x", packhead.cmd);
		goto RETURN;
	}

	g_WriteLog.WriteLog(C_LOG_TRACE, "RecvInviteResult sResult:%hu,sender:%u,uAdminId:%u,uWebuin:%u,chatid:%s,uInviteFrom:%u",
		RecvInfo.sResult, packhead.uin, RecvInfo.uAdminId, RecvInfo.uWebuin, RecvInfo.chatid, RecvInfo.uInviteFrom);

	pWebUser = GetWebUserObjectByUid(RecvInfo.uWebuin);
	if (pWebUser == NULL)
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "RecvInviteResult uWebuin:%u", RecvInfo.uWebuin);
		return nError;
	}
	pWebUser->cTalkedSatus = INTALKING;
	pInviteUser = GetUserObjectByUid(RecvInfo.uInviteFrom);
	pAcceptUser = GetUserObjectByUid(packhead.uin);
	if (!pWebUser->m_bNewComm)
	{
		pWebUser->m_bNewComm = true;
	}

	char msg[MAX_256_LEN];
	if (!RecvInfo.sResult)//�ܾ�
	{
		if (RecvInfo.uInviteFrom == m_userInfo.UserInfo.uid)
		{
			//���ͷ��յ��Է�Ӧ��
			pWebUser->inviteuid = m_userInfo.UserInfo.uid;

			pWebUser->m_nWaitTimer = 0;

			GetInviteChatSysMsg(msg, pInviteUser, pWebUser, INVITE_HELP_REFUSE, pAcceptUser);
		}

		if (packhead.uin == m_userInfo.UserInfo.uid)
		{
			//���Լ������ľܾ���
			pWebUser->m_bConnected = false;
			pWebUser->onlineinfo.talkstatus = TALKSTATUS_NO;

			pWebUser->inviteuid = 0;

			if (pWebUser->m_bNotResponseUser || pInviteUser == NULL)
			{
				pWebUser->cTalkedSatus = HASTALKED;
			}
		}
		m_handlerMsgs->ResultInviteUser(pWebUser, pAcceptUser, INVITE_REFUSE);
		goto RETURN;
	}
	else//����
	{
		pWebUser->onlineinfo.talkstatus = TALKSTATUS_TALK;

		GetInviteChatSysMsg(msg, pInviteUser, pWebUser, INVITE_HELP_ACCEPT, pAcceptUser);
		if (!pWebUser->IsMutiUser())
		{
			pWebUser->AddCommonTalkId(RecvInfo.uInviteFrom);
		}

		pWebUser->AddCommonTalkId(packhead.uin);
		m_handlerMsgs->ResultInviteUser(pWebUser, pAcceptUser, INVITE_ACCEPT);
	}

	//����Ҫ���������ʾ
	if (packhead.uin == m_userInfo.UserInfo.uid)
	{
		//���Լ�����Ϣ
		pWebUser->onlineinfo.bInvited = true;
		pWebUser->m_nWaitTimer = 0;
		pWebUser->m_bConnected = true;
		pWebUser->talkuid = RecvInfo.uInviteFrom;
	}
	nError = 0;
RETURN:

	return nError;
}

int CChatManager::RecvFloatRelease(PACK_HEADER packhead, char *pRecvBuff, int len)
{
	CFloatChatRelease RecvInfo(packhead.ver, packhead.random);
	int nError = 0;
	CUserObject *pUser = NULL;
	CWebUserObject *pWebUser = NULL;
	char msg[MAX_256_LEN];

	nError = UnPack(&RecvInfo, pRecvBuff, len);
	if (nError != 0)
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "RecvFloatRelease unpack failed,Cmd:%.4x", packhead.cmd);
		goto RETURN;
	}

	g_WriteLog.WriteLog(C_LOG_TRACE, "RecvFloatRelease sender:%lu,uAdminId:%lu,uWebuin:%lu,chatid:%s,kefuname:%s",
		packhead.uin, RecvInfo.uAdminId, RecvInfo.webuin, RecvInfo.chatid, RecvInfo.szKefuName);


	pWebUser = GetWebUserObjectByUid(RecvInfo.webuin);
	if (pWebUser == NULL)
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "RecvFloatRelease GetWebUserObjectByID(%u) failed", RecvInfo.webuin);
		SendTo_GetWebUserChatInfo(packhead.random, RecvInfo.uAdminId, RecvInfo.chatid);
		goto RETURN;
	}
	pUser = GetUserObjectByUid(RecvInfo.uKefu);

	pWebUser->cTalkedSatus = HASTALKED;
	pWebUser->onlineinfo.talkstatus = TALKSTATUS_REQUEST;
	pWebUser->m_bConnected = false;
	pWebUser->RemoveAllMutiUser();
	if (!pWebUser->m_bNewComm)
	{
		pWebUser->m_bNewComm = true;
	}

	switch (RecvInfo.usReason)
	{
	case REASON_CLIENT_RELEASE:    //�ͷ������ͷ��˿ͻ�
		sprintf(msg, "�ͷ� %s(%u) �����ͷ��˻Ự", RecvInfo.szKefuName, RecvInfo.uKefu);
		break;
	default:
		sprintf(msg, "�ͷ� %s(%u) �ͷ��˻Ự", RecvInfo.szKefuName, RecvInfo.uKefu);
		break;

	}

	//������Ҫ�ж��ǲ���Ҫ������ʾ���û�
	if (pWebUser->m_bNotResponseUser)
	{
		pWebUser->cTalkedSatus = HASTALKED;
	}
	else
	{
		pWebUser->cTalkedSatus = INTALKING;
		sprintf(msg, "�ͷ� %s(%lu) �Էÿ� %s �ĽӴ���ֹ", !pUser ? RecvInfo.szKefuName : pUser->UserInfo.nickname, RecvInfo.uKefu, pWebUser->info.name);
	}
	AddMsgToList((IBaseObject*)pWebUser, MSG_FROM_SYS, MSG_RECV_ERROR, GetMsgId(), MSG_TYPE_NORMAL, MSG_DATA_TYPE_TEXT,
		msg, GetTimeByMDAndHMS(0), NULL, NULL);
	m_handlerMsgs->RecvReleaseChat(pWebUser);
	nError = 0;
RETURN:

	return nError;
}

int CChatManager::RecvFloatCMDError(PACK_HEADER packhead, char *pRecvBuff, int len)
{
	COM_FLOAT_ERROR RecvInfo(packhead.ver, packhead.random);
	int nError = 0;
	CUserObject *pUser = NULL;
	CWebUserObject *pWebUser = NULL;

	nError = UnPack(&RecvInfo, pRecvBuff, len);
	if (nError != 0)
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "RecvFloatCMDError unpack failed,Cmd:%.4x", packhead.cmd);
		goto RETURN;
	}
	nError = 0;

RETURN:
	return nError;
}

int CChatManager::RecvFloatCloseChat(PACK_HEADER packhead, char *pRecvBuff, int len)
{
	COM_FLOAT_CLOSECHAT RecvInfo(packhead.ver, packhead.random);
	int nError = 0;
	CUserObject *pUser = NULL;
	CWebUserObject *pWebUser = NULL;

	nError = UnPack(&RecvInfo, pRecvBuff, len);
	if (nError != 0)
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "RecvFloatCloseChat unpack failed,Cmd:%.4x", packhead.cmd);
		return nError;
	}

	g_WriteLog.WriteLog(C_LOG_TRACE, "RecvFloatCloseChat sender:%s(%u),uAdminId:%u,uWebuin:%u,chatid:%s,usType:%hu",
		RecvInfo.nickname, packhead.uin, RecvInfo.uAdminId, RecvInfo.webuin, RecvInfo.chatid, RecvInfo.usType);

	pUser = GetUserObjectByUid(packhead.uin);
	pWebUser = GetWebUserObjectByUid(RecvInfo.webuin);
	if (pWebUser == NULL || strcmp(pWebUser->chatid, RecvInfo.chatid) != 0)
	{
		if (pWebUser == NULL)
		{
			g_WriteLog.WriteLog(C_LOG_ERROR, "RecvFloatCloseChat GetUserObjectByUid(%u) failed", RecvInfo.webuin);
		}
		else
		{
			g_WriteLog.WriteLog(C_LOG_ERROR, "RecvFloatCloseChat chatid(local:%s,pack:%s) not same", pWebUser->chatid, RecvInfo.chatid);
		}
		return nError;
	}

	if (CHATCLOSE_INVISTEXIT == RecvInfo.usType)
	{
		g_WriteLog.WriteLog(C_LOG_TRACE, "RecvFloatCloseChat ��������ϯ����(%u)", packhead.uin);

		//����������˳�����
		pWebUser->RemoveMutiUser(packhead.uin);

		//�����ʾ
		char msg[MAX_256_LEN];
		if (pUser != NULL)
		{
			sprintf(msg, "%s(%u)�˳��Ự", pUser->UserInfo.nickname, packhead.uin);
		}
		else
		{
			sprintf(msg, "�û�(%u)�˳��Ự", packhead.uin);
		}

		AddMsgToList((IBaseObject*)pWebUser, MSG_FROM_SYS, MSG_RECV_ERROR, GetMsgId(), MSG_TYPE_NORMAL, MSG_DATA_TYPE_TEXT,
			msg, GetTimeByMDAndHMS(0), NULL, NULL);
		pWebUser->RemoveMutiUser(packhead.uin);
	}
	else
	{
		//�����Ự�����Э����ϯ
		if (pWebUser->IsMutiUser())
			pWebUser->RemoveAllMutiUser();
		pWebUser->onlineinfo.bInvited = false;
		pWebUser->m_bNewComm = false;
		pWebUser->m_bConnected = true;
		pWebUser->onlineinfo.talkstatus = TALKSTATUS_NO;
		pWebUser->cTalkedSatus = HASTALKED;
		string strMsg = "";
		if (CHATCLOSE_UNSUBSCRIBE == RecvInfo.usType)
		{
			strMsg = "΢���û�ȡ����ע�����Ự";
		}
		else
		{
			strMsg = "�ÿͻỰ�ѽ���";
		}

		AddMsgToList((IBaseObject*)pWebUser, MSG_FROM_SYS, MSG_RECV_ERROR, GetMsgId(), MSG_TYPE_NORMAL, MSG_DATA_TYPE_TEXT,
			strMsg.c_str(), GetTimeByMDAndHMS(0), NULL, NULL);

		if (!pWebUser->IsOnline())//�����ǲ��Ǹ��û�����������
		{
			g_WriteLog.WriteLog(C_LOG_TRACE, "RecvFloatCloseChat ��ϯ����(%u)�ÿ�����", packhead.uin);

			m_vistor->SetVisitorOffline(pWebUser);
		}
		else
		{
			g_WriteLog.WriteLog(C_LOG_TRACE, "RecvFloatCloseChat ��ϯ����(%u)�ÿͷ�����", packhead.uin);
			//�ÿ��˻ص������б�
			pWebUser->m_nWaitTimer = -20;
			pWebUser->onlineinfo.talkstatus = TALKSTATUS_NO;
			m_handlerMsgs->RecvCloseChat(pWebUser);
		}
	}

	nError = 0;

	return nError;
}

int CChatManager::RecvFloatListChat(PACK_HEADER packhead, char *pRecvBuff, int len)
{
	COM_FLOAT_RECVLIST RecvInfo;
	int nError = 0;
	std::vector<LISTCHATINFO>::iterator iter;

	nError = UnPack(&RecvInfo, pRecvBuff, len);
	if (nError != 0)
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "RecvFloatListChat unpack failed,Cmd:%.4x", packhead.cmd);

		return nError;
	}

	string chaidlist;
	for (iter = RecvInfo.recvchatlist.begin(); iter != RecvInfo.recvchatlist.end(); iter++)
	{
		LISTCHATINFO ListChatInfo = (LISTCHATINFO)(*iter);

		SendTo_GetWebUserChatInfo(packhead.random, RecvInfo.uAdminId, ListChatInfo.chatid);

		chaidlist += ListChatInfo.chatid;
		chaidlist += ",";

	}

	g_WriteLog.WriteLog(C_LOG_TRACE, "RecvFloatListChat %s", chaidlist.c_str());

	nError = 0;
	return nError;
}

int CChatManager::RecvEventAnnouncement(PACK_HEADER packhead, char *pRecvBuff, int len)
{
	g_WriteLog.WriteLog(C_LOG_ERROR,"RecvEventAnnouncement");
	return 0;
}

int CChatManager::RecvSrvUpdateSucc(PACK_HEADER packhead, char *pRecvBuff, int len)
{
	SRV_UPDATE_SUCC RecvInfo(packhead.ver);
	int nError = 0;

	nError = UnPack(&RecvInfo, pRecvBuff, len);
	if (nError != 0)
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "RecvSrvUpdateSucc unpack failed,Cmd:%.4x", packhead.cmd);
		goto RETURN;
	}

	switch (RecvInfo.type)
	{
	case UPDATE_STATUS:
		m_nOnLineStatus = GetMutiByte(RecvInfo.onlinestatus, ONLINE_INFO_STATUS, ONLINE_INFO_STATUS_LEN);
		m_userInfo.status = m_nOnLineStatus;
		break;
	case UPDATE_PASS:
		
		break;
	default:
		break;
	}

RETURN:
	return nError;
}

int CChatManager::RecvSrvUpdateFail(PACK_HEADER packhead, char *pRecvBuff, int len)
{
	SRV_UPDATE_FAIL RecvInfo(packhead.ver);
	int nError = 0;

	nError = UnPack(&RecvInfo, pRecvBuff, len);
	if (nError != 0)
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "RecvSrvUpdateFail unpack failed,Cmd:%.4x", packhead.cmd);
		return nError;
	}

	return nError;
}

int CChatManager::RecvSrvDown(PACK_HEADER packhead, char *pRecvBuff, int len)
{
	int nError = 0;
	SRV_SERVER_COPY RecvInfo(packhead.ver);

	nError = UnPack(&RecvInfo, pRecvBuff, len);
	if (nError != 0)
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "RecvSrvDown unpack failed,Cmd:%.4x", packhead.cmd);
		return nError;
	}
	g_WriteLog.WriteLog(C_LOG_TRACE, "RecvSrvDown uin:%u, type:%u, seq", RecvInfo.uin, RecvInfo.type, RecvInfo.seq);
	if (RecvInfo.uin == m_userInfo.UserInfo.uid)
	{
		if (m_nOnLineStatus == STATUS_OFFLINE) 
			return 0;

		g_WriteLog.WriteLog(C_LOG_TRACE, "RecvSrvDown uid%u", m_userInfo.UserInfo.uid);
		m_handlerMsgs->RecvOffline(&m_userInfo);
		if (RecvInfo.type >= SRVNORMAL_IPERROR)
		{
			m_lastError = "���û����������ط����е�½";
		}
	}

	return 0;
}

int CChatManager::RecvRepTransferClient(PACK_HEADER packhead, char *pRecvBuff, int len)
{
	int nError = 0;
	SRV_REP_TRANSFERCLIENT RecvInfo(packhead.ver);

	CUserObject *pInviteUser = NULL;
	CUserObject *pAcceptUser = NULL;
	CWebUserObject *pWebUser = NULL;
	char msg[MAX_256_LEN];

	nError = UnPack(&RecvInfo, pRecvBuff, len);
	if (nError != 0)
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "RecvRepTransferClient unpack failed,Cmd:%.4x", packhead.cmd);
		return nError;
	}

	g_WriteLog.WriteLog(C_LOG_TRACE, "RecvRepTransferClient chatid:%s,rand:%u,szThirdid:%s,recvuid:%u,recvName:%s, webuid:%u,webName:%s,senduid:%u,sendName:%s,result:%u",
		RecvInfo.szChatId, RecvInfo.szRand, RecvInfo.szThirdid, RecvInfo.recvinfo.id, RecvInfo.recvinfo.name, RecvInfo.clientinfo.id, RecvInfo.clientinfo.name, RecvInfo.sendinfo.id, RecvInfo.sendinfo.name, RecvInfo.result);
	//ת��ʧ��
	pWebUser = GetWebUserObjectByUid(RecvInfo.clientinfo.id);
	if (RecvInfo.result != 0)
	{
		sprintf(msg, "ת��ʧ��%s", RecvInfo.reason);
		
		AddMsgToList((IBaseObject*)pWebUser, MSG_FROM_SYS, MSG_RECV_ERROR, GetMsgId(), MSG_TYPE_NORMAL, MSG_DATA_TYPE_TEXT,
			msg, GetTimeByMDAndHMS(0), NULL, NULL);

		g_WriteLog.WriteLog(C_LOG_TRACE, "RecvRepTransferClient chatid:%s,rand:%u,szThirdid:%s,recvuid:%u, webuid:%u,senduid:%u,result:%u,reason:%s",
			RecvInfo.szChatId, RecvInfo.szRand, RecvInfo.szThirdid, RecvInfo.recvinfo.id, RecvInfo.clientinfo.id, RecvInfo.sendinfo.id, RecvInfo.result, RecvInfo.reason);
		goto RETURN;
	}

	pAcceptUser = GetUserObjectByUid(RecvInfo.recvinfo.id);
	pInviteUser = GetUserObjectByUid(RecvInfo.sendinfo.id);

	if (pAcceptUser == NULL)
	{
		pWebUser->cTalkedSatus = HASTALKED;
		goto RETURN;
	}

	//ת�Ƴɹ��ĻỰ��Ҫɾ����
	if (pWebUser == NULL)
	{
		
	}
	else if (RecvInfo.sendinfo.id == m_userInfo.UserInfo.uid)
	{
		//��ǰ��ϯ�ɹ�ת�Ƴ�ȥ�ĻỰ
		pWebUser->cTalkedSatus = HASTALKED;//ͨ����
		pWebUser->onlineinfo.talkstatus = TALKSTATUS_TALK;
		pWebUser->m_nWaitTimer = 0;
		pWebUser->m_bConnected = false;
		pWebUser->transferuid = 0;
		pWebUser->talkuid = RecvInfo.recvinfo.id;
		if (pInviteUser != NULL)
			GetInviteChatSysMsg(msg, pInviteUser, pWebUser, INVITE_TRANSFER, pAcceptUser);
		else
			sprintf(msg, "�ÿ� %s ת�Ƶ� %s", pWebUser->info.name, pAcceptUser->UserInfo.nickname);
		
		AddMsgToList((IBaseObject*)pWebUser, MSG_FROM_SYS, MSG_RECV_ERROR, GetMsgId(), MSG_TYPE_NORMAL, MSG_DATA_TYPE_TEXT,
			msg, GetTimeByMDAndHMS(0), NULL, NULL);
	}

	nError = 0;
RETURN:

	return nError;
}

int CChatManager::RecvTransferClient(PACK_HEADER packhead, char *pRecvBuff, int len)
{
	int nError = 0;

	//������
	SRV_TRANSFERCLIENT RecvInfo(packhead.ver);
	nError = UnPack(&RecvInfo, pRecvBuff, len);
	if (nError != 0)
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "RecvTransferClient unpack failed,Cmd:%.4x", packhead.cmd);
		return nError;
	}
	g_WriteLog.WriteLog(C_LOG_TRACE, "RecvTransferClient chatid:%s,rand:%u,szThirdid:%s,recvuid:%u, webuid:%u,senduid:%u",
		RecvInfo.szChatId, RecvInfo.szRand, RecvInfo.szThirdid, RecvInfo.recvinfo.id, RecvInfo.clientinfo.id, RecvInfo.sendinfo.id);

	CWebUserObject *pWebUser = NULL;
	pWebUser = GetWebUserObjectByUid(RecvInfo.clientinfo.id);

	if (pWebUser == NULL)//��ȡWorkBill
	{
		g_WriteLog.WriteLog(C_LOG_TRACE, "RecvTransferClient pWebUser==NULL");
		SendTo_GetWebUserInfo(RecvInfo.clientinfo.id, RecvInfo.szChatId);
	}
	else
	{
		//���û��Ƶ��Լ��Ի���

		char msg[MAX_256_LEN];
		if (strcmp(pWebUser->chatid, RecvInfo.szChatId) != 0 && strlen(RecvInfo.szChatId) > 0)
		{
			strcpy(pWebUser->chatid, RecvInfo.szChatId);
		}
		if (RecvInfo.recvinfo.id == m_userInfo.UserInfo.uid)
		{
			g_WriteLog.WriteLog(C_LOG_TRACE, "RecvTransferClient CurrentKefu is the accept:%u", RecvInfo.recvinfo.id);
			pWebUser->cTalkedSatus = INTALKING;
			pWebUser->onlineinfo.talkstatus = TALKSTATUS_TALK;
			pWebUser->m_nWaitTimer = 0;
			pWebUser->m_bConnected = true;
			pWebUser->inviteuid = 0;
			pWebUser->talkuid = m_userInfo.UserInfo.uid;

			//ת�ƵĻỰ�϶������ߵ�
			if (pWebUser->info.status == STATUS_OFFLINE)
				pWebUser->info.status = STATUS_ONLINE;

			//�ÿ�����
			sprintf(msg, "%s�����˷ÿ�%s�ĻỰ", m_userInfo.UserInfo.nickname, pWebUser->info.name);
			
			AddMsgToList((IBaseObject*)pWebUser, MSG_FROM_SYS, MSG_RECV_ERROR, GetMsgId(), MSG_TYPE_NORMAL, MSG_DATA_TYPE_TEXT,
				msg, GetTimeByMDAndHMS(0), NULL, NULL);
		}
		else
		{
			g_WriteLog.WriteLog(C_LOG_ERROR, "RecvTransferClient recvuid error=����ת�ƻỰ��ϯ���ǵ�ǰ��¼��ϯ,recvuid:%u, localuid:%u",
				RecvInfo.recvinfo.id, m_userInfo.UserInfo.uid);
			nError = -2;
		}
	}

	return nError;
}

int CChatManager::SendTo_GetShareList()
{
	assert(m_handlerMsgs);

	if (!m_bLoginSuccess)
		return SYS_ERROR_BEFORE_LOGIN;

	COM_FLOAT_SHARELIST SendInfo(VERSION);
	return SendPackTo(&SendInfo);
}

int CChatManager::SendTo_GetListChatInfo()
{
	if (!m_bLoginSuccess)
		return SYS_ERROR_BEFORE_LOGIN;

	COM_FLOAT_GETLIST SendInfo(VERSION);
	SendInfo.uAdminId = m_login->m_authAdminid;
	return SendPackTo(&SendInfo);	
}

CUserObject * CChatManager::AddUserObject(unsigned long id, char *sid, char *name, unsigned char status, unsigned long fd)
{
	CUserObject *pUser = GetUserObjectByUid(id);

	if (pUser == NULL)
	{
		pUser = new CUserObject();

		pUser->UserInfo.uid = id;
		m_mapUsers.insert(map<unsigned long, CUserObject*>::value_type(id, pUser));
		pUser->m_nFlag = 2;
	}
	else
	{
		pUser->m_nFlag = 1;
	}

	strcpy(pUser->UserInfo.sid, sid);
	strcpy(pUser->UserInfo.nickname, name);
	pUser->status = status;
	pUser->fd = fd;

	if (m_nMyInfoIsGet >= 0 && pUser->UserInfo.uid != 0 && pUser->UserInfo.uid == m_userInfo.UserInfo.uid)
	{
		m_nMyInfoIsGet = -20;
	}
	return pUser;
}

CUserObject * CChatManager::GetUserObjectByUid(unsigned long id)
{
	if (id == 0)
	{
		return &m_commUserInfo;
	}

	if (m_userInfo.UserInfo.uid != 0 && m_userInfo.UserInfo.uid == id)
		return &m_userInfo;

	CUserObject *pUser = NULL;

	map <unsigned long, CUserObject*>::iterator iter_user;
	iter_user = m_mapUsers.find(id);
	if (iter_user != m_mapUsers.end())
	{
		pUser = iter_user->second;
		return pUser;
	}
	else
	{
		return NULL;
	}
}

void CChatManager::TimerProc(int timeId, LPVOID pThis)
{
	CChatManager* chat_manager = (CChatManager*)pThis;

	if (chat_manager->m_bExit) return;

	if (timeId == TIMER_NORMAL)
	{
		if (chat_manager->m_nOnLineStatus == STATUS_OFFLINE)
			return;
		if (chat_manager->m_nMyInfoIsGet >= 0)
		{
			chat_manager->m_nMyInfoIsGet++;

			if (chat_manager->m_nMyInfoIsGet >= 3)
			{
				//����Լ�����Ϣû�л�ȡ������ô��Ҫ�ظ���ȥ��ȡ
				chat_manager->m_nMyInfoIsGet = 0;
				chat_manager->SendGetSelfInfo(chat_manager->m_userInfo.UserInfo.uid, chat_manager->m_userInfo.UserInfo.sid, CMD_SRV_REP_USERINFO);
			}
		}

		chat_manager->TimerSolveAck();
		chat_manager->TimerSolveLoginToVisitorServer();
		chat_manager->m_login->TimerSolveAuthToken();
	}
	else if (timeId == TIMER_LOGIN)
	{
		if (!chat_manager->m_bLoginSuccess)
		{
			chat_manager->m_lastError = "��¼��ʱ";
			chat_manager->m_handlerLogin->LoginProgress(-1);
		}
		chat_manager->m_timers->KillTimer(TIMER_LOGIN);
	}
}

int CChatManager::SendTo_GetAllUserInfo()
{
	if (!m_bLoginSuccess)
		return SYS_ERROR_BEFORE_LOGIN;

	int nError = 0;
	CUserObject *pUser;
	map <unsigned long, CUserObject*>::iterator iter_user;
	for (iter_user = m_mapUsers.begin(); iter_user != m_mapUsers.end(); iter_user++)
	{
		pUser = iter_user->second;
		if (pUser != NULL && strlen(pUser->UserInfo.nickname) == NULL)
		{
			nError = SendTo_GetUserInfo(pUser->UserInfo.uid);
		}
	}
	return nError;
}

int CChatManager::SendTo_GetUserInfo(unsigned long uid)
{
	if (!m_bLoginSuccess) 
		return SYS_ERROR_BEFORE_LOGIN;
	
	if (uid == 0) 
		return SYS_ERROR_PARAMETER;

	CLT_GET_CLT_INFO SendInfo(VERSION);

	SendInfo.seq = GetPackSeq();
	SendInfo.uin = uid;
	SendInfo.type = UPDATE_ALLINFO;

	return SendPackTo(&SendInfo);
}

unsigned short CChatManager::GetPackSeq()
{
	++g_packSeq;
	if (g_packSeq == 0)
	{
		++g_packSeq;
	}
	return g_packSeq;
}

int CChatManager::SendGetSelfInfo(unsigned long id, char *strid, unsigned short cmd, unsigned short cmdtype, unsigned short type)
{
	int nError = 0;
	CLT_GET_INFO SendInfo(VERSION, cmd);

	SendInfo.cmdtype = cmdtype;
	SendInfo.type = type;

	SendInfo.id = id;
	strcpy(SendInfo.strid, strid);

	nError = SendPackTo(&SendInfo);

	return nError;
}

void CChatManager::TimerSolveAck()
{
	if (m_nSendPing < 0)
		return;

	m_nSendPing++;

	//30�뷢��һ�δ̼���
	if (m_nSendPing > 30)
	{
		if (m_nOnLineStatus != STATUS_OFFLINE)
		{
			// ����ping�������������˽�����Լ����ping��
			if (SendPing() != 0)
			{
				m_nSendPingFail++;
			}
			else
			{
				m_nSendPingFail = 0;
			}

			if (m_nSendPingFail > 3)
			{
				// �ۼ�3�����Ӳ��ϣ������ߴ���
				g_WriteLog.WriteLog(C_LOG_TRACE, "SendPingFailed3times--SetOfflineStatus ");
				SetOfflineStatus();

				CloseAllSocket();
			}
		}

		if (m_nOnLineStatusEx != STATUS_OFFLINE)
		{
			// ��������㼶��ping����ȷ�Ϸ������Ƿ�������
			if (m_vistor && m_vistor->SendPingToVisitorServer() != 0)
			{
				// ���Ӳ��ϴ���
				m_nOnLineStatusEx = STATUS_OFFLINE;
				m_nLoginToVisitor = 0;
			}
		}

		// ʱ������
		m_nSendPing = 0;
	}
}

int CChatManager::SendPing()
{
	int nError = 0;

	COM_SEND_PING SendInfo(VERSION);
	SendInfo.o.online_flag = m_userInfo.UserInfo.onlineflag;

	nError = SendPackTo(&SendInfo, 0, 0);

	return nError;
}

void CChatManager::SetOfflineStatus()
{
	m_nOnLineStatus = STATUS_OFFLINE;
	m_nOnLineStatusEx = STATUS_OFFLINE;
	SetAllUserOffline();
	DeleteAllSrvInfo();
	CloseAllSocket();
	if (m_login)
	{
		m_login->SetOffline();
	}
}

void CChatManager::CloseAllSocket()
{
	m_socket.Close();
	if (m_vistor)
	{
		m_vistor->m_socketEx.Close();
	}	
}

int CChatManager::SendTo_UpdateOnlineStatus(unsigned short status)
{
	if (!m_bLoginSuccess)
		return SYS_ERROR_BEFORE_LOGIN;

	unsigned int dwstatus = 0;
	CLT_UPDATE_STATUS SendInfo(VERSION);
	SendInfo.seq = GetPackSeq();
	SendInfo.bSendToServer = true;
	SendInfo.type = UPDATE_STATUS;
	SendInfo.onlinestatus = SetMutiByte(dwstatus, ONLINE_INFO_STATUS, ONLINE_INFO_STATUS_LEN, (DWORD)status);
	return SendPackTo(&SendInfo);
}

CWebUserObject * CChatManager::GetWebUserObjectBySid(char *sid)
{
	MapWebUsers::iterator iter = m_mapWebUsers.find(sid);
	if (iter != m_mapWebUsers.end())
	{
		return iter->second;
	}
	return NULL;
}

CWebUserObject * CChatManager::AddWebUserObject(char *sid, char *thirdid, char *name,
	char *scriptflag, char *url, unsigned char status, unsigned char floatauth)
{
	CWebUserObject* pWebUser = new CWebUserObject();

	pWebUser->m_nFlag = 2;
	pWebUser->m_nIndex = m_nClientIndex;
	m_nClientIndex++;
	strcpy(pWebUser->info.sid, sid);
	pWebUser->AddScriptFlag(scriptflag, url);
	strcpy(pWebUser->info.thirdid, thirdid);
	strcpy(pWebUser->info.name, name);
	if (strlen(pWebUser->info.ipfromname) <= 0)
		strcpy(pWebUser->info.ipfromname, name);
	pWebUser->info.status = status;
	pWebUser->SetForbid(m_sysConfig->IsWebuserSidForbid(sid));
	pWebUser->RemoveAllMutiUser();

	m_mapWebUsers.insert(MapWebUsers::value_type(sid, pWebUser));

	if (strlen(pWebUser->info.name) < 2)
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "AddWebUserObject name length��%d", strlen(pWebUser->info.name));
	}
	return pWebUser;
}

CWebUserObject * CChatManager::GetWebUserObjectByUid(unsigned long uid)
{
	MapWebUsers::iterator iter = m_mapWebUsers.begin();
	for (iter; iter != m_mapWebUsers.end(); iter++)
	{
		if (iter->second->webuserid == uid)
		{
			return iter->second;
		}
	}
	return NULL;
}

int CChatManager::SendTo_GetWebUserInfo(unsigned long webuserid, const char *chatid, char *szMsg, unsigned int chatkefuid)
{
	int nError = 0;
	COM_SEND_MSG SendInfo(VERSION);
	Json::Value jv;
	if (szMsg != NULL && chatkefuid > 0)
	{
		jv["ReturnParameters"] = szMsg;
		jv["ChatKefu"] = chatkefuid;
	}
	else if (chatkefuid > 0)
	{
		jv["ChatKefu"] = chatkefuid;
	}
	else if (szMsg != NULL){
		jv["ReturnParameters"] = szMsg;
	}

	SendInfo.msg.msgtype = MSG_GET_WORKBILL;
	SendInfo.msg.recvuin = webuserid;
	SendInfo.msg.sendtime = 0;
	strncpy(SendInfo.strChatid, chatid, MAX_CHATID_LEN);
	SendInfo.msg.bak = 0;
	SendInfo.msg.seq = GetPackSeq();
	SendInfo.version = VERSION;
	SendInfo.msg.senduin = m_userInfo.UserInfo.uid;
	strncpy(SendInfo.msg.strmsg, jv.toStyledString().c_str(), MAX_MSG_RECVLEN);
	g_WriteLog.WriteLog(C_LOG_TRACE, "SendTo_GetWebUserInfo recvuin:%u,senduin:%u��szMsg=%s", SendInfo.msg.recvuin, SendInfo.msg.senduin, szMsg);

	nError = SendPackTo(&SendInfo);

	return nError;
}

WxMsgBase* CChatManager::ParseWxMsg(CWebUserObject* pWebUser, char* msg, CUserObject* pAssistUser, unsigned long time)
{
	//΢����Ϣ���ͣ�utf8�������� 
	WxMsgBase* msgBase = NULL;
	WxObj *pwxobj = ParseWxJsonMsg(msg);
	MSG_FROM_TYPE msgFormType = MSG_FROM_WEBUSER;
	if (pAssistUser)
	{
		msgFormType = MSG_FROM_ASSIST;
	}

	if (pwxobj != NULL)
	{
		if ("userinfo" == pwxobj->MsgType)
		{
			// �˴�ע�⣬����ڶ����յ�userinfo��Ӧ����ǰ���յ��Ǵ���Ϣ������
			if (pWebUser->m_pWxUserInfo == NULL)
			{
				pWebUser->m_pWxUserInfo = (WxUserInfo*)pwxobj;
			}
			else
			{
				delete pWebUser->m_pWxUserInfo;
				pWebUser->m_pWxUserInfo = (WxUserInfo*)pwxobj;
			}
			int oldLen = strlen(pWebUser->info.name);
			int newLen = ((WxUserInfo*)pwxobj)->nickname.length();
			if (oldLen == 0 && newLen > 0)
			{
				strcpy(pWebUser->info.name, ((WxUserInfo*)pwxobj)->nickname.c_str());
				m_handlerMsgs->RecvWebUserInfo(pWebUser);
			}
			
			pWebUser->m_bIsGetInfo = true;
			return NULL;
		}
		else if ("wxactoken" == pwxobj->MsgType)
		{
			//todo:�յ���������������΢�ŵ�access_token��Ϣ
			AddToken(pWebUser->m_pWxUserInfo, ((WxAccessTokenInfo*)pwxobj)->szAcToken);
			delete pwxobj;
			return NULL;
		}
		else if ("image" == pwxobj->MsgType)
		{
			//GetWxUserInfoAndToken(pWebUser);
			string msgId = GetMsgId();
			string imagePath = FullPath("SkinRes\\mainframe\\");
			StringReplace(imagePath, "\\", "/");

			char contentMsg[MAX_1024_LEN];
			sprintf(contentMsg, "<img id=\"%s_msg\" class=\"msg_image\" src=\"%srecv_image_fail.jpg\"><img id=\"%s_image\" class=\"wait_image\" src=\"%smsg_wait.gif\">",
				msgId.c_str(),imagePath.c_str(), msgId.c_str(), imagePath.c_str());

			AddMsgToList((IBaseObject*)pWebUser, msgFormType, MSG_RECV_ERROR, msgId, MSG_TYPE_NORMAL, MSG_DATA_TYPE_IMAGE, contentMsg, GetTimeByMDAndHMS(0), pAssistUser, NULL, false, true, false);
			DownLoadFile(pWebUser, MSG_DATA_TYPE_IMAGE, ((WxMsgImage*)pwxobj)->MediaUrl, pAssistUser, time, msgId);
			delete pwxobj;
			return NULL;
		}
		else if ("voice" == pwxobj->MsgType)
		{
			string msgId = GetMsgId();
			string imagePath = FullPath("SkinRes\\mainframe\\");
			StringReplace(imagePath, "\\", "/");

			char contentMsg[MAX_2048_LEN];
			sprintf(contentMsg, "<audio id=\"%s_msg\" class=\"msg_voice\" controls=\"controls\" src=\"\" type=\"audio/mpeg\"></audio><img id=\"%s_image\" class=\"wait_image\" src=\"%smsg_wait.gif\">",
				msgId.c_str(), msgId.c_str(), imagePath.c_str());

			AddMsgToList((IBaseObject*)pWebUser, msgFormType, MSG_RECV_ERROR, msgId, MSG_TYPE_NORMAL, MSG_DATA_TYPE_VOICE, contentMsg, GetTimeByMDAndHMS(0), pAssistUser, NULL, false, true, false);

			DownLoadFile(pWebUser, MSG_DATA_TYPE_VOICE, ((WxMsgVoice*)pwxobj)->MediaUrl, pAssistUser, time, msgId);
			delete pwxobj;
			return NULL;
		}
		else if ("video" == pwxobj->MsgType || "shortvideo" == pwxobj->MsgType)
		{
			string msgId = GetMsgId();
			string imagePath = FullPath("SkinRes\\mainframe\\");
			StringReplace(imagePath, "\\", "/");

			char contentMsg[MAX_2048_LEN];
			sprintf(contentMsg, "<video id=\"%s_msg\" class=\"msg_voice\" controls=\"controls\" src=\"\" type=\"video/mp4\"></video><img id=\"%s_image\" class=\"wait_image\" src=\"%smsg_wait.gif\">",
				msgId.c_str(), msgId.c_str(), imagePath.c_str());

			AddMsgToList((IBaseObject*)pWebUser, msgFormType, MSG_RECV_ERROR, msgId, MSG_TYPE_NORMAL, MSG_DATA_TYPE_VIDEO, contentMsg, GetTimeByMDAndHMS(0), pAssistUser, NULL, false, true, false);

			DownLoadFile(pWebUser, MSG_DATA_TYPE_VIDEO, ((WxMsgVideo*)pwxobj)->MediaUrl, pAssistUser, time, msgId);
			delete pwxobj;
			return NULL;
		}
		else if ("location" == pwxobj->MsgType)
		{
			msgBase = (WxMsgBase*)pwxobj;
			
		}
		else if ("link" == pwxobj->MsgType)
		{
			msgBase = (WxMsgBase*)pwxobj;			
		}
		else
		{
			g_WriteLog.WriteLog(C_LOG_ERROR, "ParseWxMsg unknow weixin Json:%s", msg);
			delete pwxobj;
			return NULL;
		}
	}
	else
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "ParseWxMsg NOT the weixin Json:%s", msg);
		return NULL;
	}
	return msgBase;
}

string CChatManager::GetMsgId()
{
	char msgid[256];
	sprintf(msgid, "id_%d", m_msgId);
	m_idLock.Lock();
	m_msgId++;
	m_idLock.Unlock();
	return msgid;
}

void CChatManager::SaveEarlyMsg(MSG_INFO *pMsgInfo)
{
	MSG_INFO *pInfo = new MSG_INFO;

	memcpy(pInfo, pMsgInfo, sizeof(MSG_INFO));

	m_listEarlyMsg.push_back(pInfo);
}

int CChatManager::SendAckEx(unsigned short seq, unsigned long uid, unsigned long ip, unsigned short port)
{
	int nError = 0;
	COM_ACKEX SendInfo(VERSION);

	SendInfo.seq = seq;
	SendInfo.uin = uid;
	SendInfo.ip = ip;
	SendInfo.port = port;

	nError = SendPackTo(&SendInfo);

	return nError;
}

int CChatManager::SendAutoRespMsg(CWebUserObject *pWebUser, const char *msg, BOOL bClearTimer)
{
	int rtn = SYS_ERROR_SENDFAIL;
	if (pWebUser == NULL || msg == NULL || strlen(msg) <= 0)
		return rtn;

	if (!pWebUser->m_bConnected)
	{
		return rtn;
	}

	if (SendMsg(pWebUser, msg) == SYS_SUCCESS)
	{
		AddMsgToList((IBaseObject*)pWebUser, MSG_FROM_SELF, MSG_RECV_ERROR, GetMsgId(), MSG_TYPE_NORMAL, MSG_DATA_TYPE_TEXT,
			msg, GetTimeByMDAndHMS(0), NULL, NULL);

		if (bClearTimer && pWebUser != NULL)
		{
			pWebUser->m_resptimeoutmsgtimer = -1;
			pWebUser->m_resptimeoutclosetimer = -1;
			pWebUser->m_waitresptimeouttimer = -1;
		}
	}
	else
	{
		AddMsgToList((IBaseObject*)pWebUser, MSG_FROM_SELF, MSG_RECV_ERROR, GetMsgId(), MSG_TYPE_NORMAL, MSG_DATA_TYPE_TEXT,
			"�Զ��ظ���Ϣʧ�ܣ�", GetTimeByMDAndHMS(0), NULL, NULL);
	}

	return rtn;
}

void CChatManager::RecvComSendWorkBillMsg(unsigned long senduid, unsigned long recvuid, char *msg, char* mobile)
{
	char sid[MAX_WEBCLIENID_LEN + 1] = { 0 };
	char billid[MAX_CHATID_LEN + 1] = { 0 };
	char szReturnParameters[51] = { 0 };
	CWebUserObject *pWebUser;
	string strMsg = "", strReturnParameters = "";
	strMsg = msg;
	int nPos = strMsg.find("ReturnParameters:");
	strReturnParameters = strMsg.substr(nPos + 17, strMsg.size() - nPos - 17);
	if (!GetContentBetweenString(msg, "WorkBillID:", "\n", billid))
	{
		billid[0] = 0;
	}
	if (GetContentBetweenString(msg, "rand=", "&", sid))
	{
		char thirdid[MAX_THIRDID_LEN + 1] = { 0 };
		GetContentBetweenString(msg, "rand=", "&", sid);
		GetContentBetweenString(msg, "clientid=", "&", thirdid);

		char chatfrom[MAX_CHATID_LEN + 1] = { 0 };
		GetContentBetweenString(msg, "chatfrom=", "&", chatfrom);

		char wxappid[100] = { 0 };
		GetContentBetweenString(msg, "wxappid=", "&", wxappid);
		GetContentBetweenString(msg, "ReturnParameters:", "\0", szReturnParameters);
		pWebUser = GetWebUserObjectByUid(senduid);
		if (pWebUser != NULL)
		{
			//��webuiʱ���get��û�в���rand��webui��rand��Ϊclientuin
			if (strlen(sid) > 0 || ((unsigned long)atol(sid)) != senduid)
			{
				ChangeWebUserSid(pWebUser, sid, thirdid);
			}
			//���˷ÿͣ��϶������ߵ�
			if (pWebUser->info.status == STATUS_OFFLINE)
				pWebUser->info.status = STATUS_ONLINE;
			if (senduid > WEBUSER_UIN && pWebUser->webuserid != senduid)
			{
				pWebUser->webuserid = senduid;
			}
		}
		else
		{
			pWebUser = GetWebUserObjectBySid(sid);

			if (pWebUser == NULL)
			{
				pWebUser = AddWebUserObject(sid, "", mobile, "", "", STATUS_ONLINE, 0);
				g_WriteLog.WriteLog(C_LOG_ERROR,"RecvComSendWorkBillMsg ����˿����ַÿ�");
			}
			else
			{
				strcpy(pWebUser->info.thirdid, thirdid);
			}

			pWebUser->webuserid = senduid;
			SolveWebUserEarlyMsg(pWebUser);
		}

		if (pWebUser != NULL&&billid[0] != 0)
		{
			strncpy(pWebUser->info.chatfrom, chatfrom, MAX_CHATID_LEN);
			strncpy(pWebUser->chatid, billid, MAX_CHATID_LEN);
			pWebUser->m_sWxAppid = wxappid;

			// ΢���û���������ȡ΢�ŵ�userinfo
			if (!pWebUser->m_sWxAppid.empty())
			{
				if (!pWebUser->m_bIsFrWX)
				{
					pWebUser->m_bIsFrWX = true;
					m_handlerMsgs->RecvWebUserInfo(pWebUser);
				}
				SendGetWxUserInfoAndToken(pWebUser);
			}

			time_t tnow = time(NULL);

			if (!pWebUser->GetNormalChatHisMsgSuccess
				&&tnow - pWebUser->tGetNormalChatHismsgTime > 3) //�жϳ�ʱ���������ж�ʱ���ж��£���ֹ�յ�����������Ӧ��
			{
				SendGetChatHisMsg(senduid, billid);// ����Ƿǵȴ�Ӧ��Ĳ���Ҫȥ��ȡ��Ϣ��¼
				pWebUser->tGetNormalChatHismsgTime = tnow;
			}
		}

		nPos = strReturnParameters.find("transfer");
		if (nPos >= 0)//ת����
		{
			g_WriteLog.WriteLog(C_LOG_ERROR, "RecvComSendWorkBillMsg strReturnParameters: %s", strReturnParameters.c_str());
			//����ת��
			string::iterator new_end1 = remove_if(strReturnParameters.begin(), strReturnParameters.end(), bind2nd(equal_to<char>(), '\\'));
			strReturnParameters.erase(new_end1, strReturnParameters.end());
			string::iterator new_end2 = remove_if(strReturnParameters.begin(), strReturnParameters.end(), bind2nd(equal_to<char>(), '\n'));
			strReturnParameters.erase(new_end2, strReturnParameters.end());
			Json::Value jv;
			if (!ParseJson(strReturnParameters, jv)) return;

			//�жϷ��سɹ�ʧ�ܽ��
			unsigned long uTransuid = GetIntFromJson(jv, "transfer");
			if (pWebUser->transferuid != m_userInfo.UserInfo.uid)
			{
				pWebUser->onlineinfo.talkstatus = TALKSTATUS_TRANSFER;
				pWebUser->info.status = STATUS_ONLINE;
				pWebUser->transferuid = m_userInfo.UserInfo.uid;
				pWebUser->talkuid = uTransuid;
				char msg[MAX_256_LEN];
				sprintf(msg, "�ÿ� %s ת�Ƶ� %s", pWebUser->info.name, m_userInfo.UserInfo.nickname);
				m_handlerMsgs->RecvMsg((IBaseObject*)pWebUser, MSG_FROM_SYS,GetMsgId(),MSG_TYPE_NORMAL,
					MSG_DATA_TYPE_TEXT,msg,GetTimeByMDAndHMS(0),NULL,NULL);
				m_handlerMsgs->RecvTransferUser(pWebUser, &m_userInfo);
				//m_tranferList[pWebUser->webuserid] = 0;
				//KillTimer(TIMER_TRANS_TIMEOUT);
				//SetTimer(TIMER_TRANS_TIMEOUT, 1000, NULL);
			}
		}
		else if ((pWebUser->cTalkedSatus != INTALKING || !pWebUser->m_bConnected)
			&& m_userInfo.UserInfo.uid == recvuid && !pWebUser->m_bNewComm)//�ǵȴ�Ӧ��ĻỰ
		{
			if (m_vistor)
			{
				m_vistor->SendWebuserTalkBegin(pWebUser);
			}
			else
			{
				g_WriteLog.WriteLog(C_LOG_ERROR, "RecvComSendWorkBillMsg visit������δ��¼ǰ���յ��»Ự");
			}

			pWebUser->cTalkedSatus = INTALKING;

			pWebUser->talkuid = m_userInfo.UserInfo.uid;

			pWebUser->onlineinfo.talkstatus = TALKSTATUS_TALK;
			pWebUser->transferuid = 0;
			pWebUser->m_nWaitTimer = 0;
			pWebUser->m_bConnected = true;

			if (m_bLoginSuccess)
			{
				m_handlerMsgs->RecvChatInfo(pWebUser, &m_userInfo);
			}
			
			//if (nRet < 0)
			//	m_pFormUser->m_TreeListUser.DeleteItemByLParam((LPARAM)pWebUser);
			//else
			//	AddMultiWebUserToList(pWebUser);//�ѽ���ĻỰ�ƶ����Ự�б���
			
			// ��ʾ
		}
	}
}

void CChatManager::SolveWebUserEarlyMsg(CWebUserObject *pWebUser)
{
	list<MSG_INFO*>::iterator iter = m_listEarlyMsg.begin();

	for (iter; iter != m_listEarlyMsg.end(); iter++)
	{
		if (pWebUser->webuserid == (*iter)->senduin)
		{
			pWebUser->m_nWaitTimer = 0;
			if (strlen(pWebUser->info.name) <= 0)
			{
				strcpy(pWebUser->info.name, (*iter)->strmobile);
				if (strlen(pWebUser->info.ipfromname) <= 0)
					strcpy(pWebUser->info.ipfromname, pWebUser->info.name);
				if (strlen(pWebUser->info.name) < 2)
				{
					g_WriteLog.WriteLog(C_LOG_ERROR, "SolveWebUserEarlyMsg name length��%d", strlen(pWebUser->info.name));
				}
			}
		}
	}

	
	for (iter = m_listEarlyMsg.begin(); iter != m_listEarlyMsg.end(); iter++)
	{
		if (pWebUser->webuserid == (*iter)->senduin)
		{
			delete (*iter);
			m_listEarlyMsg.erase(iter);
			return;
		}
	}
}

void CChatManager::RecvComSendNormalChatidHisMsg(unsigned long senduid, unsigned long recvuid, COM_SEND_MSG& RecvInfo)
{
	if (!g_NomalChatHistoryMsgManage.ParseJsonHisMsg(RecvInfo.strChatid, RecvInfo.msg.strmsg))
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "RecvComSendNormalChatidHisMsg failed,chatid:%s,msg:%s", RecvInfo.strChatid, RecvInfo.msg.strmsg);
		return;
	}
	g_WriteLog.WriteLog(C_LOG_TRACE, "RecvComSendNormalChatidHisMsg msg:%s", RecvInfo.msg.strmsg);
	
	
	CNChatHisMsg nchatmsg;
	if (CNomalChatHistoryMsgManage::NHISMSGTYPE::SUCCESS != g_NomalChatHistoryMsgManage.GetCompleteMsgs(RecvInfo.strChatid, nchatmsg))
	{
		return;
	}
	
	//��ʾ��ʷ��Ϣ���������ѻ�ȡ��ʷ��Ϣ״̬
	
	CWebUserObject *pWebUser = GetWebUserObjectBySid(RecvInfo.strChatid);
	if (pWebUser != NULL)
	{
		pWebUser->GetNormalChatHisMsgSuccess = true; // ���λỰ��Ҫ���ñ���
	}
}

int CChatManager::RecvComTransRequest(unsigned long senduid, COM_SEND_MSG& RecvInfo)
{
	int nError = 0;
	CUserObject *pAcceptUser = NULL;
	CWebUserObject *pWebUser = NULL;

	g_WriteLog.WriteLog(C_LOG_TRACE, "RecvComTransRequest ��%s", RecvInfo.msg.strmsg);

	pAcceptUser = GetUserObjectByUid(RecvInfo.msg.recvuin);
	pWebUser = GetWebUserObjectByUid(RecvInfo.msg.senduin);
	if (pAcceptUser == NULL)
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "RecvComTransRequest GetUserObject failed");
		goto RETURN;
	}

	if (pWebUser == NULL)
	{
		pWebUser = GetWebUserObjectBySid(RecvInfo.strRand);
	}

	if (pWebUser == NULL)
	{
		g_WriteLog.WriteLog(C_LOG_TRACE, "RecvComTransRequest pWebUser==NULL");
		char strMsg[MAX_256_LEN];
		sprintf(strMsg, "{\"transfer\":%u}", senduid);
		SendTo_GetWebUserInfo(RecvInfo.msg.senduin, RecvInfo.strChatid, strMsg, senduid);
	}
	else if (RecvInfo.msg.recvuin == m_userInfo.UserInfo.uid && pWebUser->transferuid != m_userInfo.UserInfo.uid)//��ǰ��ϯ�ǽ����ߣ��ƶ���ת���С�����
	{
		if (strcmp(pWebUser->chatid, RecvInfo.strChatid) != 0 && strlen(RecvInfo.strChatid) > 0)
			strcpy(pWebUser->chatid, RecvInfo.strChatid);
		pWebUser->talkuid = senduid;
		pWebUser->m_bNewComm = false;
		pWebUser->info.status = STATUS_ONLINE;
		if (RecvInfo.msg.senduin > WEBUSER_UIN && pWebUser->webuserid != RecvInfo.msg.senduin)
		{
			pWebUser->webuserid = RecvInfo.msg.senduin;
		}

		pWebUser->onlineinfo.talkstatus = TALKSTATUS_TRANSFER;
		pWebUser->transferuid = m_userInfo.UserInfo.uid;

		//m_pFormMain->RecvTransferWebUser(pWebUser, pAcceptUser->UserInfo.nickname);

		m_handlerMsgs->RecvTransferUser(pWebUser, pAcceptUser);
		
		//��ȡ��ʷ��Ϣ

		//���ó�ʱ��ʱ��
		//m_TranfserList[pWebUser->webuserid] = 0;
		//KillTimer(TIMER_TRANS_TIMEOUT);
		//SetTimer(TIMER_TRANS_TIMEOUT, 1000, NULL);
	}

	nError = 0;
RETURN:
	return nError;
}

int CChatManager::RecvComTransAnswer(unsigned long senduid, COM_SEND_MSG& RecvInfo)
{
	int nError = 0;
	CUserObject *pAcceptUser = NULL;//��ת����ϯ
	CWebUserObject *pWebUser = NULL;
	string strMsg = "";
	g_WriteLog.WriteLog(C_LOG_TRACE, "RecvComTransAnswer ��%s,recvuin:%u", RecvInfo.msg.strmsg, RecvInfo.msg.recvuin);

	pAcceptUser = GetUserObjectByUid(senduid);
	pWebUser = GetWebUserObjectByUid(RecvInfo.msg.senduin);
	if (senduid == 0)
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "RecvComTransAnswer GetUserObject��ȡ��ת����ʧ�ܣ�%u", senduid);
		nError = -1;
		goto RETURN;
	}
	if (pWebUser == NULL)
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "RecvComTransAnswer GetWebUserObject��ȡ�û�ʧ�ܣ�chatid%s", RecvInfo.strChatid);
		nError = -2;
		goto RETURN;
	}

	strMsg = RecvInfo.msg.strmsg;
	if ((int)strMsg.find("OK") >= 0)//ͬ��ת��
	{
		//��ϯͬ�����CLT_TRANSFERCLIENT��
		strMsg = pWebUser->info.name;
		strMsg = "�Է�ͬ�����Էÿ�[%s]��ת��" + strMsg;
		SendToTransferUser(pAcceptUser, pWebUser, senduid);
	}
	else if ((int)strMsg.find("NO") >= 0)//�ܾ�ת��
	{
		strMsg = pWebUser->info.name;
		strMsg = "�Է��ܾ����Էÿ�[%s]��ת��" + strMsg;
	}
	else if ((int)strMsg.find("TIMEOUT") >= 0)//��ʱ
	{
		strMsg = pWebUser->info.name;
		strMsg = "���Էÿ�[%s]��ת�ӳ�ʱ" + strMsg;
	}

RETURN:
	return nError;
}

int CChatManager::SendTo_GetWebUserChatInfo(unsigned short gpid, unsigned long adminid, char *chatid)
{
	int nError = 0;
	COM_FLOAT_CHATINFO SendInfo(VERSION, gpid);

	SendInfo.uAdminId = adminid;
	strcpy(SendInfo.chatid, chatid);

	nError = SendPackTo(&SendInfo);
	g_WriteLog.WriteLog(C_LOG_TRACE, "SendTo_GetWebUserChatInfo chatid:%s,gpid:%u", chatid, gpid);
	return nError;
}

CWebUserObject * CChatManager::ChangeWebUserSid(CWebUserObject *pWebUser, char *sid, char *thirdid)
{
	if (pWebUser == NULL || sid == NULL)
		return NULL;

	if (strlen(pWebUser->info.sid) <= 0)
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "ChangeWebUserSid ");
	}
	else
	{
		if (strcmp(pWebUser->info.sid, sid) != 0)
		{
			MapWebUsers::iterator iter = m_mapWebUsers.find(pWebUser->info.sid);
			if (iter != m_mapWebUsers.end())
			{
				m_mapWebUsers.erase(iter);
			}
			sprintf(pWebUser->info.sid, sid);
			m_mapWebUsers.insert(MapWebUsers::value_type(sid, pWebUser));

			pWebUser->SetForbid(m_sysConfig->IsWebuserSidForbid(sid));
		}
	}

	return pWebUser;
}

int CChatManager::SendStartRecvFloatMsg(unsigned short gpid, unsigned long adminid, char *chatid, unsigned short sLastMsgid)
{
	int nError = 0;
	COM_FLOAT_STARTORSTOPRECV StartInfo(VERSION, gpid);

	StartInfo.uAdminId = adminid;
	StartInfo.sAcitonType = 0;
	strcpy(StartInfo.chatid, chatid);
	StartInfo.sLastMsgid = sLastMsgid;


	nError = SendPackTo(&StartInfo);
	g_WriteLog.WriteLog(C_LOG_TRACE, "SendStartRecvFloatMsg chatid:%s,gpid:%u,sLastMsgid:%u", chatid, gpid, sLastMsgid);
	return nError;
}

void CChatManager::SendGetWxUserInfoAndToken(CWebUserObject* pWebUser)
{
	if (pWebUser == NULL || !pWebUser->m_bIsFrWX)
		return;

	if (pWebUser->m_pWxUserInfo == NULL)
	{
		SendGetWxUserInfo(pWebUser->webuserid, pWebUser->chatid);
	}

	SendGetWxToken(pWebUser->webuserid, pWebUser->chatid);
}

int CChatManager::SendGetWxUserInfo(unsigned long webuserid, const char *chatid)
{
	int nError = 0;
	COM_SEND_MSG SendInfo(VERSION);

	SendInfo.msg.msgtype = MSG_KEFU_GET_WX_USERINFO;
	SendInfo.msg.recvuin = webuserid;
	SendInfo.msg.sendtime = GetTimeLong();
	strncpy(SendInfo.strChatid, chatid, MAX_CHATID_LEN);
	SendInfo.msg.bak = 0;
	SendInfo.msg.seq = GetPackSeq();
	SendInfo.version = VERSION;
	SendInfo.msg.senduin = m_userInfo.UserInfo.uid;

	g_WriteLog.WriteLog(C_LOG_TRACE, "SendGetWxUserInfo: recvuin:%u,senduin:%u", SendInfo.msg.recvuin, SendInfo.msg.senduin);
	nError = SendPackTo(&SendInfo);
	return nError;
}

int CChatManager::SendGetWxToken(unsigned long webuserid, const char *chatid)
{
	int nError = 0;
	COM_SEND_MSG SendInfo(VERSION);

	SendInfo.msg.msgtype = MSG_KEFU_GET_WX_ACCESSTOKEN;
	SendInfo.msg.recvuin = webuserid;
	SendInfo.msg.sendtime = GetTimeLong();
	strncpy(SendInfo.strChatid, chatid, MAX_CHATID_LEN);
	SendInfo.msg.bak = 0;
	SendInfo.msg.seq = GetPackSeq();
	SendInfo.version = VERSION;
	SendInfo.msg.senduin = m_userInfo.UserInfo.uid;

	g_WriteLog.WriteLog(C_LOG_TRACE, "SendGetWxToken: recvuin:%u,senduin:%u", SendInfo.msg.recvuin, SendInfo.msg.senduin);
	nError = SendPackTo(&SendInfo);
	return nError;
}

void CChatManager::GetInviteChatSysMsg(char* msg, CUserObject *pInviteUser, CWebUserObject *pWebUser, int type, CUserObject *pAcceptUser)
{
	if (pWebUser == NULL)
		return;

	switch (type)
	{
	case APPLY_ASK:
		if (pInviteUser == NULL)
			return;
		sprintf(msg, "%s������һ����������", pInviteUser->UserInfo.nickname);
		break;
	case APPLY_OPEN:
		if (pInviteUser == NULL)
			return;
		sprintf(msg, "%s����ֱ�ӶԻ�", pInviteUser->UserInfo.nickname);
		break;
	case MYGETNOTE:
		if (pInviteUser == NULL)
			return;
		sprintf(msg, "%s��������", pInviteUser->UserInfo.nickname);
		break;
	case INVITE_TRANSFER:
		if (pAcceptUser == NULL)
			return;

		if (pInviteUser == NULL)
			sprintf(msg, "%sת�ӵ��û� %s", pWebUser->info.name, pAcceptUser->UserInfo.nickname);
		else
			sprintf(msg, "%sת���û� %s �� %s", pInviteUser->UserInfo.nickname, pWebUser->info.name, pAcceptUser->UserInfo.nickname);
		break;
	case INVITE_HELP:
		if (pAcceptUser == NULL)
			return;

		if (pInviteUser == NULL)
			sprintf(msg, "%s������Ӵ� %s", pAcceptUser->UserInfo.nickname, pWebUser->info.name);
		else
			sprintf(msg, "%s�����û� %s �Ӵ� %s", pInviteUser->UserInfo.nickname, pAcceptUser->UserInfo.nickname, pWebUser->info.name);
		break;
	case INVITE_HELP_REFUSE:
		if (pAcceptUser == NULL)
			return;

		if (pInviteUser == NULL)
			sprintf(msg, "%s�ܾ�����Ӵ� %s", pAcceptUser->UserInfo.nickname, pWebUser->info.name);
		else
			sprintf(msg, "%s�ܾ��û� %s ������ �Ӵ� %s", pAcceptUser->UserInfo.nickname, pInviteUser->UserInfo.nickname, pWebUser->info.name);
		break;
	case INVITE_HELP_ACCEPT:
		if (pAcceptUser == NULL)
			return;

		if (pInviteUser == NULL)
			sprintf(msg, "%sͬ������Ӵ� %s", pAcceptUser->UserInfo.nickname, pWebUser->info.name);
		else
			sprintf(msg, "%sͬ���û� %s ������ �Ӵ� %s", pAcceptUser->UserInfo.nickname, pInviteUser->UserInfo.nickname, pWebUser->info.name);
		break;
	default:
		sprintf(msg, "����Э�������쳣");
		break;
	}
}

void CChatManager::SetHandlerLogin(IHandlerLgoin* handlerLogin)
{
	m_handlerLogin = handlerLogin;
}

void CChatManager::SetHandlerMsgs(IHandlerMsgs* handlerMsgs)
{
	m_handlerMsgs = handlerMsgs;
}

void CChatManager::Exit()
{
	SendLoginOff();

	g_WriteLog.WriteLog(C_LOG_TRACE, "Exit");

	m_bExit = true;
	m_socket.m_bRecvThread = false;
	m_sysConfig->SaveData();

	if (m_timers)
	{
		m_timers->TimerClear();
		delete m_timers;
	}

	if (m_sysConfig)
	{
		delete m_sysConfig;
	}

	if (m_login)
	{
		delete m_login;
	}

	if (m_vistor)
	{
		m_vistor->m_socketEx.m_bRecvThread = false;
		delete m_vistor;
	}

	string tempPath = FullPath("temp\\");
	ClearDirectory(tempPath);

	Sleep(500);
}

CChatManager* CChatManager::GetInstance()
{
	static CChatManager instance;
	return &instance;
}

void CChatManager::LoginSuccess()
{
	m_bLoginSuccess = true;

	m_commUserInfo.UserInfo.compid = m_userInfo.UserInfo.compid;

	// �յ���¼�û�����Ϣ������¼�ɹ�
	m_handlerLogin->LoginProgress(100);
	m_nOnLineStatus = STATUS_ONLINE;	
	m_timers->SetTimer(1000, TIMER_NORMAL);

	if (m_bKeepPwd)
	{
		m_sysConfig->AddLatestLoginInfo(m_userInfo.UserInfo.uid, m_userInfo.UserInfo.sid,
			m_userInfo.UserInfo.pass, m_bAutoLogin, m_bKeepPwd);
	}
	else
	{
		m_sysConfig->AddLatestLoginInfo(m_userInfo.UserInfo.uid, m_userInfo.UserInfo.sid,
			"", m_bAutoLogin, m_bKeepPwd);
	}

	m_sysConfig->m_nLastLoginBy = m_login->m_nLoginBy;
	switch (m_login->m_nLoginBy)
	{
	case LOGIN_BYUID:
		m_sysConfig->m_sLastLoginUid = StringToDWORD(m_sLogin);
		break;
	case LOGIN_BYSTRING:
		m_sysConfig->m_sLastLoginStr = m_sLogin;
		break;
	default:
		break;
	}

	m_commUserInfo.UserInfo.compid = m_userInfo.UserInfo.compid;
	m_nOnLineStatus = STATUS_ONLINE;
	m_nSendPing = 0;
	m_nSendPingFail = 0;

	// ����Ĭ�������ļ�
	m_sysConfig->SaveData();

	char sFile[MAX_256_LEN];
	string strFile = GetCurrentPath();
	sprintf(sFile, "%s\\config_%u.dat", strFile.c_str(), m_userInfo.UserInfo.uid);
	m_sysConfig->LoadData(sFile);

	SendTo_UpdateOnlineStatus(m_nOnLineStatus);

	m_login->CheckLoginFlag(m_sysConfig->m_sLastLoginUid, m_sysConfig->m_sLastLoginStr, true);
}

CWebUserObject * CChatManager::GetWebUserObjectByScriptFlag(char *scriptflag)
{
	return NULL;
}

CUserObject* CChatManager::GetMySelfUserInfo()
{
	return &m_userInfo;
}

void CChatManager::TimerSolveLoginToVisitorServer()
{
	if (m_nLoginToVisitor < 0)
		return;
	if (m_nOnLineStatusEx != STATUS_OFFLINE)
	{
		m_nLoginToVisitor = -20;
		return;
	}

	m_nLoginToVisitor++;

	if (m_nLoginToVisitor > 5)
	{
		//��������
		if (m_vistor && m_vistor->ConnectAndLoginToVisitorServer())
		{
			m_nLoginToVisitor = -20;
		}
		else
			m_nLoginToVisitor = 0;
	}
}

void CChatManager::TimerSolveRequestTimerOut()
{
	MapWebUsers::iterator iter = m_mapWebUsers.begin();
	for (iter; iter != m_mapWebUsers.end(); iter++)
	{
		if (!iter->second->m_bIsShow)
			continue;

		switch (iter->second->onlineinfo.talkstatus)
		{
		case TALKSTATUS_TALK:
			if (iter->second->m_bConnected && iter->second->talkuid == m_userInfo.UserInfo.uid)
			{
				if (m_sysConfig->m_bAutoResp && iter->second->m_resptimeoutmsgtimer >= 0)
				{
					iter->second->m_resptimeoutmsgtimer++;
					int resptimeout =m_sysConfig->m_nVisitorTimeoutTime * 60;
					if (iter->second->m_resptimeoutmsgtimer > resptimeout)
					{
						//3	���˴𸴳�ʱ--����û�з�����Ϣ��ʱ��
						SendAutoRespMsg(iter->second, m_sysConfig->m_sVisitorTimeoutMsg.c_str(), false);
						iter->second->m_resptimeoutmsgtimer = -20;
					}
				}

				if (m_sysConfig->m_bAutoResp && iter->second->m_waitresptimeouttimer >= 0)
				{
					iter->second->m_waitresptimeouttimer++;
					int waitresptimeout = m_sysConfig->m_nUserTimeoutTime * 60;
					if (iter->second->m_waitresptimeouttimer > waitresptimeout)
					{
						//2���˵ȴ��𸴳�ʱ--���˷�����Ϣ��ȴ���ʱ���͵���Ϣ
						SendAutoRespMsg(iter->second, m_sysConfig->m_sUserTimeoutMsg.c_str(), false);
						iter->second->m_waitresptimeouttimer = -20;
					}
				}

				if (m_sysConfig->m_bVisotorTimeoutClose && iter->second->m_resptimeoutclosetimer >= 0)
				{
					iter->second->m_resptimeoutclosetimer++;
					int m_resptimeoutclose = m_sysConfig->m_nVisitorTimeoutCloseTime * 60;
					if (iter->second->m_resptimeoutclosetimer > m_resptimeoutclose)
					{
						//4	���˴𸴳�ʱ�رնԻ�--����û�з�����Ϣ��ʱ�䳬������ֹ�Ի�
						SendAutoRespMsg(iter->second, m_sysConfig->m_sVisitorTimeoutCloseMsg.c_str(), false);
						iter->second->m_resptimeoutclosetimer = -20;

						SendTo_CloseChat(iter->second->webuserid, CHATCLOSE_TIMEOUT);
					}
				}
			}
		}
	}
}

void CChatManager::SendTo_CloseChat(unsigned long webuserid, int ntype)
{
	CWebUserObject* pWebUser = GetWebUserObjectByUid(webuserid);
	if (pWebUser == NULL || pWebUser->onlineinfo.talkstatus == TALKSTATUS_NO)
		return;

	pWebUser->onlineinfo.talkstatus = TALKSTATUS_NO;
	pWebUser->m_bConnected = false;
	pWebUser->cTalkedSatus = HASTALKED;
	pWebUser->m_nWaitTimer = -20;
	pWebUser->m_resptimeoutmsgtimer = -20;
	pWebUser->m_resptimeoutclosetimer = -20;
	pWebUser->m_waitresptimeouttimer = -20;

	char msg[MAX_256_LEN];
	GetStopChatSysMsg(msg, pWebUser, ntype, &m_userInfo);
	if (ntype != CHATCLOSE_INVISTEXIT)
	{

		pWebUser->talkuid = 0;
		pWebUser->RemoveAllMutiUser();

		//����Ҫ��ֹͣ�Ի��ı�ʾ
		if (pWebUser->m_bNewComm)
		{
			int res = SendCloseChat(pWebUser, ntype);
			g_WriteLog.WriteLog(C_LOG_TRACE, "SendTo_CloseChat %d", res);
		}
		else
		{
			if (SendMsg(pWebUser, "|ForceClose()|", MSG_BAK_INPUTING, "") == SYS_SUCCESS)
			{
				AddMsgToList((IBaseObject*)pWebUser, MSG_FROM_SELF, MSG_RECV_ERROR, GetMsgId(), MSG_TYPE_NORMAL, MSG_DATA_TYPE_TEXT,
					msg, GetTimeByMDAndHMS(0), NULL, NULL);
			}
			if (m_vistor)
			{
				m_vistor->SendWebuserTalkEnd(pWebUser);
			}
		}
	}
	else
	{
		//���Ǳ���������
		if (pWebUser->m_bNewComm)
		{
			SendCloseChat(pWebUser, ntype);
		}
	}
}

void CChatManager::GetStopChatSysMsg(char* msg, CWebUserObject *pWebUser, int type, CUserObject *pSendUser)
{
	if (pWebUser == NULL || pWebUser == NULL)
		return;

	string clientName;
	if (pSendUser != NULL)
	{
		clientName = "�ͷ� ";
		clientName += pSendUser->UserInfo.nickname;
	}
	else
	{
		clientName = "�ͷ�";
	}

	switch (type)
	{
	case CHATCLOSE_TIMEOUT:
		sprintf(msg, "��ÿ� %s �ظ���Ϣ��ʱ���Ի�����", pWebUser->info.name);
		break;
	case CHATCLOSE_USER:
		sprintf(msg, "%s ��ֹ�� %s �ĽӴ�", clientName.c_str(), pWebUser->info.name);
		break;
	case CHATCLOSE_INVISTEXIT:
		sprintf(msg, "������ %s �˳��ÿ� %s �ĻỰ", clientName.c_str(), pWebUser->info.name);
	default:
		break;
	}
}

int CChatManager::SendMsg(IBaseObject* pUser, const char *msg, int bak, char *sfont)
{
	int nError = SYS_FAIL;
	if (pUser->m_nEMObType == OBJECT_WEBUSER)
	{
		CWebUserObject* pWebUser = (CWebUserObject*)pUser;
		pWebUser->m_nWaitTimer = -10;

		if (pWebUser->m_bNewComm)
		{
			nError = SendFloatMsg(pWebUser, msg, sfont);
		}
		else
		{
			nError = SendComMsg(pWebUser->webuserid, pWebUser->info.sid, msg, pWebUser->chatid, pWebUser->info.thirdid, bak, sfont);
		}
	}
	else
	{
		CUserObject* pObjUser = (CUserObject*)pUser;
		nError = SendComMsg(pObjUser->UserInfo.uid, "", msg, "", "", bak);
	}
	return nError;
}

int CChatManager::SendTo_Msg(unsigned long userId, MSG_RECV_TYPE userType, string msgId, MSG_DATA_TYPE msgDataType, string msg)
{
	int nError = SYS_FAIL;	
	if (userId <= 0 || msgId.empty() || msg.empty())
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "SendTo_Msg params error");
		return nError;
	}

	string sMsg = msg;

	if (userType == MSG_RECV_CLIENT)
	{
		CUserObject* pUser = GetUserObjectByUid(userId);
		if (pUser == NULL) return nError;
		switch (msgDataType)
		{
		case MSG_DATA_TYPE_TEXT:
			TransferFaceToStr(sMsg, userType);
			if (SendMsg(pUser, sMsg.c_str()) == SYS_SUCCESS)
			{
				StringReplace(msg, "\\", "/");
				ReplaceFaceId(msg);
				AddMsgToList((IBaseObject*)pUser, MSG_FROM_SELF, MSG_RECV_ERROR, msgId, MSG_TYPE_NORMAL, MSG_DATA_TYPE_TEXT,
					msg, GetTimeByMDAndHMS(0), NULL, NULL);
			}			
			break;
		case MSG_DATA_TYPE_IMAGE:
			StringReplace(msg, "\\", "/");
			UpLoadFile(userId, userType, msgId, msg, msgDataType);
			break;
		case MSG_DATA_TYPE_FILE:
			SendFileToUser(pUser, msg, msgId, userType);
			break;
		default:
			break;
		}
	}
	else if (userType == MSG_RECV_WEB || userType == MSG_RECV_WX)
	{
		CWebUserObject* pWebUser = GetWebUserObjectByUid(userId);
		if (pWebUser == NULL) return nError;

		switch (msgDataType)
		{
		case MSG_DATA_TYPE_TEXT:
			TransferFaceToStr(sMsg, userType);
			if (SendMsg(pWebUser, sMsg.c_str()) == SYS_SUCCESS)
			{
				StringReplace(msg, "\\", "/");
				ReplaceFaceId(msg);
				AddMsgToList((IBaseObject*)pWebUser, MSG_FROM_SELF, MSG_RECV_ERROR, GetMsgId(), MSG_TYPE_NORMAL, MSG_DATA_TYPE_TEXT,
					msg, GetTimeByMDAndHMS(0), NULL, NULL);
			}
			break;
		case MSG_DATA_TYPE_IMAGE:
		case MSG_DATA_TYPE_VOICE:
			StringReplace(msg, "\\", "/");
			UpLoadFile(userId, userType, msgId, msg, msgDataType);
			break;
		case MSG_DATA_TYPE_FILE:
			SendFileToUser(pWebUser, msg, msgId, userType);
			break;
		default:
			break;
		}
	}
	else
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "SendTo_Msg userType:error");
	}
	return nError;
}

int CChatManager::SendGetChatHisMsg(unsigned long webuserid, const char *chatid)
{
	int nError = 0;
	COM_SEND_MSG SendInfo(VERSION);

	SendInfo.msg.msgtype = MSG_KEFU_GET_CHAT_HISMSG;
	SendInfo.msg.recvuin = webuserid;
	SendInfo.msg.sendtime = GetTimeLong();

	SendInfo.msg.bak = 0;
	SendInfo.msg.seq = GetPackSeq();
	SendInfo.version = VERSION;
	SendInfo.msg.senduin = m_userInfo.UserInfo.uid;//���˺���
	strncpy(SendInfo.strChatid, chatid, MAX_CHATID_LEN);

	g_WriteLog.WriteLog(C_LOG_TRACE, "SendGetChatHisMsg: chatid:%s,webuserid:%u,senduin:%u",
		SendInfo.strChatid, SendInfo.msg.recvuin, SendInfo.msg.senduin);
	nError = SendPackTo(&SendInfo);

	return nError;
}

std::string CChatManager::GetLastError()
{
	return "";
}

int CChatManager::ReRecv_Msg(string url, MSG_FROM_TYPE msgFromUserType, string msgId, MSG_DATA_TYPE nMsgDataType,
	unsigned long msgFromUserId, unsigned long assistUserId, unsigned long time)
{
	CWebUserObject* pWebUser = NULL;
	CUserObject* pUser = NULL;
	CUserObject* pAssistUser = NULL;
	if (msgFromUserType == MSG_FROM_WEBUSER)
	{
		pWebUser = GetWebUserObjectByUid(msgFromUserId);
	}
	else if (msgFromUserType == MSG_FROM_ASSIST)
	{
		pWebUser = GetWebUserObjectByUid(msgFromUserId);
		pAssistUser = GetUserObjectByUid(assistUserId);
	}
	else if (msgFromUserType == MSG_FROM_CLIENT)
	{
		pUser = GetUserObjectByUid(msgFromUserId);
	}
	DownLoadFile(pWebUser, nMsgDataType, url, pAssistUser, time, msgId);
	return 1;
}

int CChatManager::SendTo_AcceptChat(unsigned long webuserid)
{
	int nError = SYS_FAIL;
	CWebUserObject* pWebUser = GetWebUserObjectByUid(webuserid);
	if (pWebUser)
	{
		if (m_sysConfig->IsWebuserSidForbid(pWebUser->info.sid))
		{
			AddMsgToList((IBaseObject*)pWebUser, MSG_FROM_SYS, MSG_RECV_ERROR, GetMsgId(), MSG_TYPE_NORMAL, MSG_DATA_TYPE_TEXT,
				"�÷ÿ��ѱ������Σ����Ƚ�����κ��ٽ��ܶԻ�����!", GetTimeByMDAndHMS(0), NULL, NULL);
		}
		else
		{
			if (pWebUser->m_bNewComm)
			{
				COM_FLOAT_ACCEPTCHAT SendInfo(VERSION, pWebUser->gpid);

				SendInfo.uFromAdminid = pWebUser->floatfromadminuid;
				SendInfo.uAdminId = pWebUser->floatadminuid;
				SendInfo.uWebuin = pWebUser->webuserid;
				strcpy(SendInfo.chatid, pWebUser->chatid);
				strcpy(SendInfo.nickname, m_userInfo.UserInfo.nickname);

				g_WriteLog.WriteLog(C_LOG_TRACE, "SendTo_AcceptChat [chatid:%s, uAdminId:%u,uWebuin:%u,Acceptname:%s,senduid:%u]",
					SendInfo.chatid, SendInfo.uAdminId, SendInfo.uWebuin, m_userInfo.UserInfo.nickname, m_userInfo.UserInfo.uid);

				nError = SendPackTo(&SendInfo);
			}
		}
	}

	return nError;
}

int CChatManager::SendTo_ReleaseChat(unsigned long webuserid)
{
	int nError = SYS_FAIL;
	CWebUserObject* pWebUser = GetWebUserObjectByUid(webuserid);
	if (pWebUser)
	{
		CFloatChatRelease RelInfo(VERSION, pWebUser->gpid);
		strcpy(RelInfo.chatid, pWebUser->chatid);
		RelInfo.uKefu = m_userInfo.UserInfo.uid;
		RelInfo.uAdminId = m_login->m_authAdminid;
		strcpy(RelInfo.szKefuName, m_userInfo.UserInfo.nickname);
		RelInfo.webuin = pWebUser->webuserid;
		RelInfo.usReason = REASON_CLIENT_RELEASE;

		nError = SendPackTo(&RelInfo);
	}
	return nError;
}

int CChatManager::SendCloseChat(CWebUserObject* pWebUser, int ntype)
{
	int nError = SYS_FAIL;
	if (pWebUser)
	{
		COM_FLOAT_CLOSECHAT SendInfo(VERSION, pWebUser->gpid);

		SendInfo.uAdminId = pWebUser->floatadminuid;
		SendInfo.webuin = pWebUser->webuserid;
		SendInfo.usType = ntype;
		strcpy(SendInfo.chatid, pWebUser->chatid);
		strcpy(SendInfo.nickname, m_userInfo.UserInfo.nickname);

		g_WriteLog.WriteLog(C_LOG_TRACE, "SendCloseChat[chatid:%s, uAdminId:%u,uWebuin:%u,type:%d,senduid:%u]",
			SendInfo.chatid, SendInfo.uAdminId, SendInfo.webuin, ntype, m_userInfo.UserInfo.uid);
		nError = SendPackTo(&SendInfo);
	}
	return nError;
}

int CChatManager::SendTo_InviteWebUser(CWebUserObject *pWebUser, int type, string strText)
{
	int nError = SYS_FAIL;
	if (pWebUser == NULL)
	{
		if (m_sysConfig->IsWebuserSidForbid(pWebUser->info.sid))
		{
			AddMsgToList((IBaseObject*)pWebUser, MSG_FROM_SYS, MSG_RECV_ERROR, GetMsgId(), MSG_TYPE_NORMAL, MSG_DATA_TYPE_TEXT,
				"�÷ÿ��ѱ������Σ����Ƚ�����κ�������!", GetTimeByMDAndHMS(0), NULL, NULL);
			return nError;
		}

		char sbuff[MAX_1024_LEN];
		WEBUSER_URL_INFO *pOb = pWebUser->GetLastScriptFlagOb();
		if (pOb == NULL)
			return nError;

		string strScriptFlag = pOb->url;
		if (strText.empty())
			strText = m_sysConfig->m_sInviteWords;

		sprintf(sbuff, "<MSG><COMMAND>%s</COMMAND><NEEDNOTE>NO</NEEDNOTE><TEXT>%s</TEXT><SERVICEUIN>%d</SERVICEUIN><SCRIPTFLAG>%s</SCRIPTFLAG><IP>%s</IP><PORT>%d</PORT><AUTO>0</AUTO></MSG>\r\n",
			GetApplyTypeString(type).c_str(), strText.c_str(), m_userInfo.UserInfo.uid, strScriptFlag.c_str(), pWebUser->info.sip, 2450);

		if (m_vistor)
		{
			nError = m_vistor->SendBuffToVisitorServer(sbuff, strlen(sbuff));
		}
	}
	
	return nError;
}

int CChatManager::SendTo_InviteUser(CWebUserObject* pWebUser, CUserObject* pAcceptUser)
{
	int nError = 0;
	COM_FLOAT_INVITEREQUEST SendInfo(VERSION, pWebUser->gpid);

	SendInfo.uInviteUser = pAcceptUser->UserInfo.uid;
	SendInfo.sSecond = 0;

	SendInfo.uAdminId = pWebUser->floatadminuid;
	strcpy(SendInfo.chatid, pWebUser->chatid);
	SendInfo.uWebuin = pWebUser->webuserid;

	g_WriteLog.WriteLog(C_LOG_TRACE, "��������Э������[chatid:%s, InviteUser:%u,uAdminId:%u,uWebuin:%u]",
		SendInfo.chatid, SendInfo.uInviteUser, SendInfo.uAdminId, SendInfo.uWebuin);

	nError = SendPackTo(&SendInfo);

	return nError;
}

int CChatManager::SendTo_TransferRequestUser(CWebUserObject* pWebUser, CUserObject* pAcceptUser)
{
	int nError = 0;
	if (pWebUser->m_bNewComm)
	{
		//�ȴ�Ӧ��Ựת��
		COM_FLOAT_TRANSREQUEST SendInfo(VERSION, pWebUser->gpid);
		SendInfo.uAdminId = pWebUser->floatadminuid;
		strcpy(SendInfo.chatid, pWebUser->chatid);
		SendInfo.uWebuin = pWebUser->webuserid;
		SendInfo.nTimeOutSecond = 0;
		SendInfo.uToAdminId = 0;
		SendInfo.uToKefu = pAcceptUser->UserInfo.uid;

		g_WriteLog.WriteLog(C_LOG_TRACE, "���͵ȴ�Ӧ��ÿ�ת������[chatid:%s,acceptuid:%u,acceptname:%s,sendname:%s,senduid:%u]",
			SendInfo.chatid, SendInfo.uToKefu, pAcceptUser->UserInfo.nickname, m_userInfo.UserInfo.nickname, m_userInfo.UserInfo.uid);

		nError = SendPackTo(&SendInfo);
	}
	else
	{
		//һ��һ�Ựת��
		//ת��Ҫ�ĳ��ȷ������룬������ֱ��ת��ȥ
		COM_SEND_MSG SendInfo(VERSION);
		char strMsg[MAX_256_LEN];
		sprintf(strMsg, "%u|%s\n", m_userInfo.UserInfo.uid, m_userInfo.UserInfo.nickname);
		SendInfo.msg.msgtype = MSG_TRANSFER_REQUEST;
		SendInfo.msg.recvuin = pAcceptUser->UserInfo.uid;
		SendInfo.msg.sendtime = GetTimeLong();
		strncpy(SendInfo.msg.strmsg, strMsg, MAX_MSG_RECVLEN);
		SendInfo.msg.bak = 0;
		SendInfo.msg.seq = GetPackSeq();
		SendInfo.version = VERSION;
		SendInfo.msg.senduin = pWebUser->webuserid;//�ͷ�����
		strncpy(SendInfo.strChatid, pWebUser->chatid, MAX_CHATID_LEN);
		strncpy(SendInfo.strRand, pWebUser->info.sid, MAX_WEBCLIENID_LEN);
		strncpy(SendInfo.strThirdid, pWebUser->info.thirdid, MAX_THIRDID_LEN);
		g_WriteLog.WriteLog(C_LOG_TRACE, "����ת������: recvuin:%u,senduin:%u", SendInfo.msg.recvuin, SendInfo.msg.senduin);

		nError = SendPackTo(&SendInfo);
	}

	return nError;
}

int CChatManager::SendTo_InviteUserResult(CWebUserObject* pWebUser, CUserObject* pUser, bool result)
{
	int nError = 0;
	unsigned long recvuin = 0;

	if (pUser)
	{
		recvuin = pUser->UserInfo.uid;
	}
	else
	{
		recvuin = pWebUser->frominviteuid;
	}

	COM_FLOAT_INVITERESULT SendInfo(VERSION, pWebUser->gpid);
	SendInfo.sResult = result;
	SendInfo.uInviteFrom = recvuin;
	SendInfo.uAdminId = pWebUser->floatadminuid;
	strcpy(SendInfo.chatid, pWebUser->chatid);
	SendInfo.uWebuin = pWebUser->webuserid;

	g_WriteLog.WriteLog(C_LOG_TRACE, "��������Э�����ܰ�[chatid:%s, sResult:%d,uInviteFrom:%u,uAdminId:%u,uWebuin:%u]",
		SendInfo.chatid, SendInfo.sResult, SendInfo.uInviteFrom, SendInfo.uAdminId, SendInfo.uWebuin);
	return SendPackTo(&SendInfo);
}

int CChatManager::SendComMsg(unsigned long recvuid, char *visitorid, const char *msg, char * chatid, char* thirdid, int bak, char *sfontinfo)
{
	int nError = 0;

	COM_SEND_MSG SendInfo(VERSION);

	SendInfo.msg.msgtype = MSG_NORMAL;
	SendInfo.msg.recvuin = recvuid;
	SendInfo.msg.sendtime = GetTimeLong();
	strncpy(SendInfo.msg.strmsg, msg, MAX_MSG_RECVLEN);
	strncpy(SendInfo.msg.strmobile, m_userInfo.UserInfo.nickname, MAX_MSGNICKNAME_LEN);
	strncpy(SendInfo.msg.strfontinfo, sfontinfo, 8);

	SendInfo.msg.bak = bak;
	SendInfo.msg.seq = GetPackSeq();
	SendInfo.version = VERSION;
	SendInfo.msg.senduin = m_userInfo.UserInfo.uid;//���˺���

	if (chatid != NULL)
	{
		strncpy(SendInfo.strChatid, chatid, MAX_CHATID_LEN);
	}
	if (visitorid != NULL)
	{
		strncpy(SendInfo.strRand, visitorid, MAX_WEBCLIENID_LEN);
	}
	if (thirdid != NULL)
	{
		strncpy(SendInfo.strThirdid, thirdid, MAX_THIRDID_LEN);
	}

	g_WriteLog.WriteLog(C_LOG_TRACE, "SendComMsg [ chatid:%s,recvuin:%u,senduin:%u,nickname:%s,strmsg:%s]",
		SendInfo.strChatid, SendInfo.msg.recvuin, SendInfo.msg.senduin, SendInfo.msg.strmobile, SendInfo.msg.strmsg);

	nError = SendPackTo(&SendInfo);

	return nError;
}

int CChatManager::SendFloatMsg(CWebUserObject *pWebUser, const char *msg, char *sfont)
{
	if (pWebUser == NULL)
		return -1;
	int nError = 0;

	COM_FLOAT_CHATMSG SendInfo(VERSION, pWebUser->gpid);

	SendInfo.uAdminId = pWebUser->floatadminuid;
	SendInfo.webuin = pWebUser->webuserid;
	SendInfo.nMsgType = 2;
	SendInfo.tMsgTime = GetTimeLong();
	strcpy(SendInfo.strfontinfo, sfont);

	strncpy(SendInfo.chatid, pWebUser->chatid, MAX_CHATID_LEN);
	strncpy(SendInfo.nickname, m_userInfo.UserInfo.nickname, MAX_USERNAME_LEN);
	strncpy(SendInfo.strmsg, msg, MAX_MSG_RECVLEN);

	strncpy(SendInfo.strRand, pWebUser->info.sid, MAX_WEBCLIENID_LEN);
	strncpy(SendInfo.strThirdid, pWebUser->info.thirdid, MAX_THIRDID_LEN);

	g_WriteLog.WriteLog(C_LOG_TRACE, "SendFloatMsg [chatid:%s, uAdminId:%u,webuin:%u,tMsgTime:%u,nickname:%s,strmsg:%s,strRand:%s,strThirdid:%s]",
		SendInfo.chatid, SendInfo.uAdminId, SendInfo.webuin, SendInfo.tMsgTime, SendInfo.nickname, SendInfo.strmsg, SendInfo.strRand, SendInfo.strThirdid);

	nError = SendPackTo(&SendInfo);

	return nError;
}

void CChatManager::UpLoadFile(unsigned long userId, MSG_RECV_TYPE userType, string msgId, string filePath, MSG_DATA_TYPE msgDataType)
{
	UPLOAD_INFO* upLoadInfo = new UPLOAD_INFO();
	upLoadInfo->filePath = filePath;
	upLoadInfo->pThis = this;
	upLoadInfo->msgDataType = msgDataType;
	upLoadInfo->userType = userType;
	upLoadInfo->userId = userId;
	upLoadInfo->msgId = msgId;

	if (userType != MSG_RECV_CLIENT)
	{
		CWebUserObject* pWebUser = GetWebUserObjectByUid(userId);
		if (pWebUser == NULL) return;
		if (pWebUser->m_bIsFrWX)
		{
			userType = MSG_RECV_WX;
		}	
	}	

	if (userType == MSG_RECV_WX)
	{
		_beginthreadex(NULL, 0, UpLoadFileToWxServerThread, (void*)upLoadInfo, 0, NULL);
	}
	else if (userType == MSG_RECV_WEB || userType == MSG_RECV_CLIENT)
	{
		_beginthreadex(NULL, 0, UpLoadFileToServerThread, (void*)upLoadInfo, 0, NULL);
	}
}

void CChatManager::AfterUpload(unsigned long userId, MSG_RECV_TYPE userType, string msgId, string mediaID, MSG_DATA_TYPE msgDataType, string fileId, string filePath, string wxToken)
{
	CUserObject* pUser = NULL;
	CWebUserObject* pWebUser = NULL;	
	if (userType == MSG_RECV_WX || userType == MSG_RECV_WEB)
	{
		pWebUser = GetWebUserObjectByUid(userId);
		if (pWebUser == NULL)
		{
			g_WriteLog.WriteLog(C_LOG_ERROR, "AfterUpload Ϊ��ȡ���ÿ���Ϣ");
			return;
		}
	}
	else if (userType == MSG_RECV_CLIENT)
	{
		pUser = GetUserObjectByUid(userId);
		if (pUser == NULL)
		{
			g_WriteLog.WriteLog(C_LOG_ERROR, "AfterUpload Ϊ��ȡ����ϯ��Ϣ");
			return;
		}
	}

	char contentMsg[MAX_1024_LEN];
	if (mediaID.empty())
	{
		// �ϴ�ʧ�ܴ���,֪ͨ���棬�ļ�����ʧ��
		m_handlerMsgs->ResultSendMsg(msgId, false, userId, userType, msgDataType, filePath);

		string imagePath = FullPath("SkinRes\\mainframe\\");
		StringReplace(imagePath, "\\", "/");

		if (msgDataType == MSG_DATA_TYPE_IMAGE)
		{
			sprintf(contentMsg, "<img id = \"%s_image\" onclick=ReSendFile(\"%s\",\"%d\",\"%s\",\"%d\",\"%lu\",\"%s\") class=\"wait_image\" src=\"%smsg_fail.png\"><img class=\"msg_image\" src=\"%s\">",
				msgId.c_str(), filePath.c_str(), userType, msgId.c_str(), MSG_DATA_TYPE_IMAGE, userId, imagePath.c_str(), imagePath.c_str(), filePath.c_str());
			filePath = contentMsg;
		}
		else if (msgDataType == MSG_DATA_TYPE_VOICE)
		{
			string fullPath = filePath + ".wav";
			sprintf(contentMsg, "<img id = \"%s_image\" onclick=ReSendFile(\"%s\",\"%d\",\"%s\",\"%d\",\"%lu\",\"%s\") class=\"wait_image\" src=\"%smsg_fail.png\"><audio class=\"msg_voice\" controls=\"controls\" src=\"%s\" type = \"audio/mpeg\"></audio>",
				msgId.c_str(), filePath.c_str(), userType, msgId.c_str(), MSG_DATA_TYPE_VOICE, userId, imagePath.c_str(), imagePath.c_str(), fullPath.c_str());
			filePath = contentMsg;
		}
		else if (msgDataType == MSG_DATA_TYPE_FILE)
		{
			return;
		}

		AddMsgToList((IBaseObject*)pWebUser, MSG_FROM_SELF, userType, msgId, MSG_TYPE_NORMAL,
			msgDataType, filePath, GetTimeByMDAndHMS(0), NULL, NULL, true, false ,false);

		g_WriteLog.WriteLog(C_LOG_TRACE, "ý���ļ���Ϣ����ʧ��");
	}
	else
	{
		// �ϴ��ɹ�����	
		Json::Value json;
		string msgSendTo;
		char fileServerMediaUrl[MAX_512_LEN];
		if (msgDataType == MSG_DATA_TYPE_IMAGE)
		{
			if (userType == MSG_RECV_WX)
			{
				if (!fileId.empty())
				{
					// ƴ����url��
					sprintf(fileServerMediaUrl, m_initConfig.fileserver_media_fileid, fileId.c_str());

					// ���ɷ��͸�΢�ŵ���Ϣ
					WxMsgImage* imageObj = new WxMsgImage("image");
					imageObj->MediaId = mediaID;
					imageObj->PicUrl = fileServerMediaUrl;
					imageObj->MediaUrl = fileServerMediaUrl;
					imageObj->ToUserName = pWebUser->info.sid;
					imageObj->FromUserName = pWebUser->info.thirdid;
					imageObj->ToSendJson(json);
					msgSendTo = json.toStyledString();
					delete imageObj;
					if (SendMsg(pWebUser, msgSendTo.c_str(), 0, "JSON=WX") == SYS_SUCCESS)
					{
						m_handlerMsgs->ResultSendMsg(msgId);

						AddMsgToList((IBaseObject*)pWebUser, MSG_FROM_SELF, MSG_RECV_ERROR, msgId, MSG_TYPE_NORMAL,
							msgDataType, filePath, GetTimeByMDAndHMS(0), NULL, NULL, true, false);
					}
				}
				else
				{
					// ���ɷ��͸�΢�ŵ���Ϣ
					sprintf(fileServerMediaUrl, m_initConfig.wechat_media_url, wxToken.c_str(), mediaID.c_str());
					WxMsgImage* imageObj = new WxMsgImage("image");
					imageObj->MediaId = mediaID;
					imageObj->PicUrl = fileServerMediaUrl;
					imageObj->MediaUrl = fileServerMediaUrl;
					imageObj->ToUserName = pWebUser->info.sid;
					imageObj->FromUserName = pWebUser->info.thirdid;
					imageObj->ToSendJson(json);
					msgSendTo = json.toStyledString();
					delete imageObj;
					if (SendMsg(pWebUser, msgSendTo.c_str(), 0, "JSON=WX") == SYS_SUCCESS)
					{
						m_handlerMsgs->ResultSendMsg(msgId);

						AddMsgToList((IBaseObject*)pWebUser, MSG_FROM_SELF, MSG_RECV_ERROR, msgId, MSG_TYPE_NORMAL,
							msgDataType, filePath, GetTimeByMDAndHMS(0), NULL, NULL, true, false);
					}
				}

			}
			else if (userType == MSG_RECV_WEB)
			{
				// ��ȡ�ļ��������ļ�·��format��
				// ͼƬ���ļ���������url
				sprintf(fileServerMediaUrl, m_initConfig.fileserver_media_fileid, mediaID.c_str());

				// ���ɷ��͸�web����Ϣ
				char sSendTo[MAX_1024_LEN];
				sprintf(sSendTo, "<span style=\"font-size:10pt; color:#000000; font-family:΢���ź�\">\
						�յ�һ��ͼƬ������鿴ԭͼ��<br></span><a href = \"%s\" target=\"blank\"><img \
						src=\"%s\" width=30%%></a>", fileServerMediaUrl, fileServerMediaUrl);
				if (SendMsg(pWebUser, sSendTo, 0) == SYS_SUCCESS)
				{
					m_handlerMsgs->ResultSendMsg(msgId);
				}
			}
			else if (userType == MSG_RECV_CLIENT)
			{
				// ƴ����url��
				sprintf(fileServerMediaUrl, m_initConfig.fileserver_media_fileid, mediaID.c_str());
				char sSendTo[MAX_1024_LEN];
				sprintf(sSendTo, "<img class=\"msg_image\" src=\"%s\">", fileServerMediaUrl);
				if (SendMsg(pUser, sSendTo, 0) == SYS_SUCCESS)
				{
					m_handlerMsgs->ResultSendMsg(msgId);
				}
			}
		}
		else if (msgDataType == MSG_DATA_TYPE_VOICE)
		{
			CCodeConvert t_convert;
			string wavFilePath = t_convert.URLDecodeALL((string)filePath);
			if (userType == MSG_RECV_WX)
			{
				if (!fileId.empty())
				{
					// ƴ����url��
					sprintf(fileServerMediaUrl, m_initConfig.fileserver_media_fileid, fileId.c_str());

					WxMsgVoice* voiceObj = new WxMsgVoice("voice");
					voiceObj->MediaId = mediaID;
					voiceObj->MediaUrl = fileServerMediaUrl;
					voiceObj->ToUserName = pWebUser->info.sid;
					voiceObj->FromUserName = pWebUser->info.thirdid;
					voiceObj->ToSendJson(json);
					delete voiceObj;
					msgSendTo = json.toStyledString();

					if (SendMsg(pWebUser, msgSendTo.c_str(), 0, "JSON=WX") == SYS_SUCCESS)
					{
						m_handlerMsgs->ResultSendMsg(msgId);

						AddMsgToList((IBaseObject*)pWebUser, MSG_FROM_SELF, MSG_RECV_ERROR, msgId, MSG_TYPE_NORMAL,
							msgDataType, filePath + ".wav", GetTimeByMDAndHMS(0), NULL, NULL, true, false);
					}
				}
				else
				{
					// ƴ����url��
					sprintf(fileServerMediaUrl, m_initConfig.wechat_media_url, wxToken.c_str(), mediaID.c_str());

					WxMsgVoice* voiceObj = new WxMsgVoice("voice");
					voiceObj->MediaId = mediaID;
					voiceObj->MediaUrl = fileServerMediaUrl;
					voiceObj->ToUserName = pWebUser->info.sid;
					voiceObj->FromUserName = pWebUser->info.thirdid;
					voiceObj->ToSendJson(json);
					delete voiceObj;
					msgSendTo = json.toStyledString();
					if (SendMsg(pWebUser, msgSendTo.c_str(), 0, "JSON=WX") == SYS_SUCCESS)
					{
						m_handlerMsgs->ResultSendMsg(msgId);

						AddMsgToList((IBaseObject*)pWebUser, MSG_FROM_SELF, MSG_RECV_ERROR, msgId, MSG_TYPE_NORMAL,
							msgDataType, filePath + ".wav", GetTimeByMDAndHMS(0), NULL, NULL, true, false);
					}
				}
			}
		}
		else if (msgDataType == MSG_DATA_TYPE_VIDEO)
		{
			if (userType == MSG_RECV_WX)
			{
				// �ϴ��ɹ�
				WxMsgVideo* videoObj = new WxMsgVideo("video");
				videoObj->MediaId = mediaID;
				videoObj->MediaUrl = fileServerMediaUrl;
				videoObj->ToUserName = pWebUser->info.sid;
				videoObj->FromUserName = pWebUser->info.thirdid;
				videoObj->ToSendJson(json);
				delete videoObj;
				msgSendTo = json.toStyledString();
				if (SendMsg(pWebUser, msgSendTo.c_str(), 0, "JSON=WX") == SYS_SUCCESS)
				{
					m_handlerMsgs->ResultSendMsg(msgId);

					AddMsgToList((IBaseObject*)pWebUser, MSG_FROM_SELF, MSG_RECV_ERROR, msgId, MSG_TYPE_NORMAL,
						msgDataType, filePath + ".mp4", GetTimeByMDAndHMS(0), NULL, NULL, true, false);
				}
			}
		}
		else if (msgDataType == MSG_DATA_TYPE_FILE)
		{
			int pos = mediaID.find_last_of("/");
			string fileName = mediaID.substr(pos + 1, mediaID.length() - pos - 1);
			if (userType == MSG_RECV_WX)
			{
				// �ϴ��ɹ�
				char sSendTo[MAX_1024_LEN];
				sprintf(sSendTo, "�յ��ļ� <a href=\"%s\" >%s</a> (����ʹ�õ��Խ��д򿪣����ӵ�ַ��Чʱ��30����)", mediaID.c_str(), mediaID.c_str());
				if (SendMsg(pWebUser, sSendTo, 0) == SYS_SUCCESS)
				{
					m_handlerMsgs->ResultSendMsg(msgId,true,pWebUser->webuserid,userType,msgDataType,mediaID);
					sprintf(sSendTo, "<span style=\"color:red\">�����ļ�</span> <a style=\"color: blue;cursor:pointer\" href=\"%s\" target=\"_blank\">%s</a>",
						mediaID.c_str(), fileName.c_str());
					AddMsgToList((IBaseObject*)pWebUser, MSG_FROM_SELF, MSG_RECV_ERROR, msgId, MSG_TYPE_NORMAL,
						msgDataType, sSendTo, GetTimeByMDAndHMS(0), NULL, NULL);
				}
			}
			else if (userType == MSG_RECV_WEB)
			{
				if (SendMsg(pWebUser, mediaID.c_str(), 0,"") == SYS_SUCCESS)
				{
					char sSendTo[MAX_1024_LEN];
					m_handlerMsgs->ResultSendMsg(msgId, true, pWebUser->webuserid, userType, msgDataType, mediaID);
					sprintf(sSendTo, "<span style=\"color:red\">�����ļ�</span> <a style=\"color: blue;cursor:pointer\" href=\"%s\" target=\"_blank\">%s</a>",
						mediaID.c_str(), fileName.c_str());
					AddMsgToList((IBaseObject*)pWebUser, MSG_FROM_SELF, MSG_RECV_ERROR, msgId, MSG_TYPE_NORMAL,
						msgDataType, sSendTo, GetTimeByMDAndHMS(0), NULL, NULL);
				}
			}
			else if (userType == MSG_RECV_CLIENT)
			{
				// ƴ����url��
				char sSendTo[MAX_1024_LEN];
				sprintf(sSendTo, "�յ��ļ� <a href=\"%s\">%s</a>", mediaID.c_str(), fileName.c_str());
				if (SendMsg(pUser, sSendTo, 0) == SYS_SUCCESS)
				{
					char sSendTo[MAX_1024_LEN];
					m_handlerMsgs->ResultSendMsg(msgId, true, pWebUser->webuserid, userType, msgDataType, mediaID);
					sprintf(sSendTo, "<span style=\"color:red\">�����ļ�</span> <a style=\"color: blue;cursor:pointer\" href=\"%s\" target=\"_blank\">%s</a>",
						mediaID.c_str(), fileName.c_str());
					AddMsgToList((IBaseObject*)pUser, MSG_FROM_SELF, MSG_RECV_ERROR, msgId, MSG_TYPE_NORMAL,
						msgDataType, sSendTo, GetTimeByMDAndHMS(0), NULL, NULL);
				}
			}
		}
	}
}

int CChatManager::SendLoginOff()
{
	int nError = 0;

	CLT_LOGOFF SendInfo(VERSION);

	SendInfo.seq = 0;
	SendInfo.uin = m_userInfo.UserInfo.uid;
	SendInfo.online_flag = OFFLINE_STATUS_MSG;

	return SendPackTo(&SendInfo);
}

void CChatManager::TransferFaceToStr(string& msg, MSG_RECV_TYPE recvType)
{
	int n;
	std::string::size_type len, begin, pos, end;
	const wx_face_t *face = wx_faces;
	for (int i = 0; i < _countof(wx_faces); ++i, ++face)
	{
		for (len = face->raw.length(), pos = 0; begin = msg.find("[", pos), begin != std::string::npos;)
		{
			end = msg.find(']', begin);
			if (end == std::string::npos)
				break;
			pos = msg.find(".gif", begin);
			if (pos == std::string::npos)
				break;
			pos = begin;
			if ((n = GetFaceIndex(msg.c_str() + begin + 1)) < 0)
				continue;
			string face = GetFaceStr(n, recvType);
			msg.replace(begin, ++end - begin, face);
			pos += face.length();
		}
	}
}

int CChatManager::GetFaceIndex(const char * faceStr)
{
	char *p;
	int n = strtol(faceStr, &p, 10);
	if (n > _countof(wx_faces)) return -1;
	if (_strnicmp(p, ".gif", sizeof(".gif") - 1)) return -1;
	return n;
}

string CChatManager::GetFaceStr(int id,MSG_RECV_TYPE recvType)
{
	if (recvType == MSG_RECV_WEB)
	{
		char str[MAX_256_LEN];
		sprintf(str, "<img src=\"%s%d.gif\" >", m_initConfig.webpage_FaceImage, id);
		return str;
	}
	else
	{
		if (id < 0 || _countof(wx_faces) - 1 < id) return "";
		return wx_faces[id].enc.empty() ? wx_faces[id].raw : wx_faces[id].enc;
	}
}

void CChatManager::TransferStrToFace(string& msg)
{
	char buf[8];
	std::string::size_type pos, n;
	const std::string *fs;
	const wx_face_t *face = wx_faces;
	for (int i = 0; i < _countof(wx_faces); ++i, ++face)
	{
		for (pos = 0;;)
		{
			fs = &face->raw;
			if (std::string::npos == (n = msg.find(*fs, pos)))
			{
				fs = &face->enc;
				if (face->enc.empty()
					|| std::string::npos == (n = msg.find(*fs, pos)))
					break;
			}
			pos = n;
			_itoa(i, buf, 10);
			msg.insert(pos, FACE_PREFIX);
			pos += sizeof(FACE_PREFIX)-1;
			msg.insert(pos, buf);
			pos += strlen(buf);
			msg.replace(pos, fs->length(), FACE_SUFFIX);
			pos += sizeof(FACE_SUFFIX) - 1;
		}
	}

	// deal with android/ios emoji face
	int len = msg.length() - 3;
	for (int i = 0; i < len; ++i)
	{
		if (msg[i] == -16 && msg[i + 1] == -97)
		{
			msg[i] = -18;
			msg[i + 1] = -112;
			msg[i + 2] = -116;
			msg.erase(i + 3, 1);
		}
	}
}

extern "C" _declspec(dllimport) void AMRToWAV(char* amrID);
void CChatManager::Amr2Wav(string filePath)
{
	if (filePath.empty())
		return;
	if (!PathFileExistsA((LPSTR)filePath.c_str()))
	{
		AMRToWAV((char*)filePath.c_str());
	}
}

int CChatManager::SendTo_TransferUserResult(CWebUserObject* pWebUser, CUserObject* pUser, bool bAccept)
{
	if (bAccept)
	{
		SendTo_AcceptChat(pWebUser->webuserid);
	}
	else
	{
		SendToRefuseChat(pWebUser);
	}
	
	return 0;
}

int CChatManager::SendToTransferUser(CUserObject *pAcceptUser, CWebUserObject *pWebUser, unsigned long acceptuin)
{
	int nError = 0;
	CLT_TRANSFERCLIENT Info(VERSION);
	if (pAcceptUser != NULL)
	{
		Info.recvinfo.id = pAcceptUser->UserInfo.uid;
		strcpy(Info.recvinfo.name, pAcceptUser->UserInfo.nickname);
	}
	else
	{
		Info.recvinfo.id = acceptuin;
	}

	Info.clientinfo.id = pWebUser->webuserid;
	strcpy(Info.clientinfo.name, pWebUser->info.name);
	Info.sendinfo.id = m_userInfo.UserInfo.uid;
	strcpy(Info.sendinfo.name, m_userInfo.UserInfo.nickname);
	strcpy(Info.szChatId, pWebUser->chatid);
	strcpy(Info.szRand, pWebUser->info.sid);
	strcpy(Info.szThirdid, pWebUser->info.thirdid);

	g_WriteLog.WriteLog(C_LOG_TRACE, "SendToTransferUser[chatid:%s, acceptuid:%u,acceptname:%s,Webname:%s,sendname:%s,senduid:%u]",
		Info.szChatId, Info.recvinfo.id, Info.recvinfo.name, Info.clientinfo.name, Info.sendinfo.name, m_userInfo.UserInfo.uid);

	Info.seq = GetPackSeq();
	nError = SendPackTo(&Info);

	return nError;
}

void CChatManager::DownLoadFile(CWebUserObject *pWebUser, MSG_DATA_TYPE nMsgDataType, string url, CUserObject *pAssistUser, unsigned long time, string msgId)
{
	unsigned long assistUid = 0;
	MSG_FROM_TYPE msgFromType = MSG_FROM_WEBUSER;
	if (pAssistUser)
	{
		assistUid = pAssistUser->UserInfo.uid;
		msgFromType = MSG_FROM_ASSIST;

	}
	if (nMsgDataType == MSG_DATA_TYPE_IMAGE)
	{
		if (pWebUser->m_bIsFrWX)
		{
			string path = FullPath("temp\\") + GetFileId();
			string fullPath = path + ".jpg";
			if (!PathFileExistsA(fullPath.c_str()) && pWebUser && pWebUser->m_pWxUserInfo)
			{
				CCodeConvert convert;
				string loadUrl = url;
				DOWNLOAD_INFO* param = new DOWNLOAD_INFO();
				param->pUser = NULL;
				param->pThis = this;
				param->filePath = convert.URLEncode(path.c_str());
				MapWxTokens::iterator iter = m_mapTokens.find(pWebUser->m_pWxUserInfo->fromwxname);
				if (iter != m_mapTokens.end())
				{
					//if (TokenIsDifferent(iter->second, url))
					//{
					//	// ����ƴurl�ǽ�����token�滻�ϣ���ֹ����ͼƬ��Ч
					//	loadUrl = ReplaceToken(url, iter->second);
					//}
				}
				else
				{
					SendGetWxUserInfoAndToken(pWebUser);
				}
				param->downLoadUrl = loadUrl;
				param->msgDataType = MSG_DATA_TYPE_IMAGE;
				if (pAssistUser)
				{
					param->pUser = pAssistUser;
				}
				param->time = GetTimeByMDAndHMS(time);
				param->pWebUser = pWebUser;
				param->msgId = msgId;
				param->msgFromType = msgFromType;
				_beginthreadex(NULL, 0, DownLoadFileFromServerThread, (void*)param, 0, NULL);
			}
			else
			{
				g_WriteLog.WriteLog(C_LOG_TRACE, "DownLoadFile û���õ�token֮ǰ���ܽ����ļ�����");

				string imagePath = FullPath("SkinRes\\mainframe\\");
				StringReplace(imagePath, "\\", "/");

				char contentMsg[MAX_1024_LEN];
				sprintf(contentMsg, "<img id=\"%s_msg\" class=\"msg_image\" src=\"%srecv_image_fail.jpg\"><img id = \"%s_image\" onclick=ReRecvFile(\"%s\",\"%d\",\"%s\",\"%d\",\"%lu\",\"%lu\",\"%s\") class=\"wait_image\" src=\"%smsg_fail.png\">",
					msgId.c_str(), imagePath.c_str(), msgId.c_str(), url.c_str(), msgFromType, msgId.c_str(),
					MSG_DATA_TYPE_IMAGE, pWebUser->webuserid, assistUid, imagePath.c_str(), imagePath.c_str());

				// �������δ���ؼ�¼
				AddMsgToList((IBaseObject*)pWebUser, msgFromType, MSG_RECV_ERROR, msgId, MSG_TYPE_NORMAL,
					nMsgDataType, contentMsg, GetTimeByMDAndHMS(time), pAssistUser, NULL, true, false, false);
			}
		}
		else
		{
			string path = FullPath("temp\\") + GetFileId();
			string fullPath = path + ".jpg";
			if (!PathFileExistsA(fullPath.c_str()) && pWebUser)
			{
				CCodeConvert convert;
				DOWNLOAD_INFO* param = new DOWNLOAD_INFO();
				param->pUser = NULL;
				param->pThis = this;
				param->filePath = convert.URLEncode(path.c_str());
				param->downLoadUrl = url;
				param->msgDataType = MSG_DATA_TYPE_IMAGE;
				if (pAssistUser)
				{
					param->pUser = pAssistUser;
				}
				param->time = GetTimeByMDAndHMS(time);
				param->pWebUser = pWebUser;
				param->msgId = msgId;
				param->msgFromType = msgFromType;
				_beginthreadex(NULL, 0, DownLoadFileFromServerThread, (void*)param, 0, NULL);
			}
			else
			{
				g_WriteLog.WriteLog(C_LOG_ERROR, "DownLoadFile  ���ز����쳣");

				string imagePath = FullPath("SkinRes\\mainframe\\");
				StringReplace(imagePath, "\\", "/");

				char contentMsg[MAX_1024_LEN];
				sprintf(contentMsg, "<img id=\"%s_msg\" class=\"msg_image\" src=\"%srecv_image_fail.jpg\"><img id = \"%s_image\" onclick=ReRecvFile(\"%s\",\"%d\",\"%s\",\"%d\",\"%lu\",\"%lu\",\"%s\") class=\"wait_image\" src=\"%smsg_fail.png\">",
					msgId.c_str(), imagePath.c_str(), msgId.c_str(), url.c_str(), msgFromType, msgId.c_str(),
					MSG_DATA_TYPE_IMAGE, pWebUser->webuserid, assistUid, imagePath.c_str(), imagePath.c_str());

				// �������δ���ؼ�¼
				AddMsgToList((IBaseObject*)pWebUser, msgFromType, MSG_RECV_ERROR, msgId, MSG_TYPE_NORMAL,
					nMsgDataType, contentMsg, GetTimeByMDAndHMS(time), pAssistUser, NULL, true, false, false);
			}
		}
	}
	else if (nMsgDataType == MSG_DATA_TYPE_VOICE)
	{
		string path = FullPath("temp\\") + GetFileId();
		string fullPath = path + ".amr";
		if (!PathFileExistsA(fullPath.c_str()) && pWebUser && pWebUser->m_pWxUserInfo)
		{
			CCodeConvert convert;
			string loadUrl = url;
			DOWNLOAD_INFO* param = new DOWNLOAD_INFO();
			param->pUser = NULL;
			param->pThis = this;
			param->filePath = convert.URLEncode(path.c_str());
			MapWxTokens::iterator iter = m_mapTokens.find(pWebUser->m_pWxUserInfo->fromwxname);
			if (iter != m_mapTokens.end())
			{
				// ����ƴurl�ǽ�����token�滻�ϣ���ֹ����ͼƬ��Ч
				//loadUrl = ReplaceToken(url, iter->second);
			}
			else
			{
				SendGetWxUserInfoAndToken(pWebUser);
			}
			param->downLoadUrl = loadUrl;
			param->msgDataType = MSG_DATA_TYPE_VOICE;
			if (pAssistUser)
			{
				param->pUser = pAssistUser;
			}
			param->time = GetTimeByMDAndHMS(time);
			param->pWebUser = pWebUser;
			param->msgId = msgId;
			param->msgFromType = msgFromType;
			_beginthreadex(NULL, 0, DownLoadFileFromServerThread, (void*)param, 0, NULL);
		}
		else
		{
			g_WriteLog.WriteLog(C_LOG_TRACE, "DownLoadFile û���õ�token֮ǰ���ܽ����ļ�����");

			string imagePath = FullPath("SkinRes\\mainframe\\");
			StringReplace(imagePath, "\\", "/");

			char contentMsg[MAX_1024_LEN];
			sprintf(contentMsg, "<audio class=\"msg_voice\" controls=\"controls\" id=\"%s_msg\" src=\"\" type=\"audio/mpeg\"></audio><img id=\"%s_image\" onclick=ReRecvFile(\"%s\",\"%d\",\"%s\",\"%d\",\"%lu\",\"%lu\",\"%s\") class=\"wait_image\" src=\"%smsg_fail.png\">",
				msgId.c_str(), msgId.c_str(), url.c_str(), MSG_FROM_WEBUSER, msgId.c_str(), MSG_DATA_TYPE_VOICE,
				pWebUser->webuserid, assistUid, imagePath.c_str(), imagePath.c_str());
		
			// �������δ���ؼ�¼
			AddMsgToList((IBaseObject*)pWebUser, msgFromType, MSG_RECV_ERROR, msgId, MSG_TYPE_NORMAL,
				nMsgDataType, contentMsg, GetTimeByMDAndHMS(time), pAssistUser, NULL, true, false, false);
		}
	}
	else if (nMsgDataType == MSG_DATA_TYPE_VIDEO)
	{
		string path = FullPath("temp\\") + GetFileId();
		string fullPath = path + ".mp4";
		if (!PathFileExistsA(fullPath.c_str()) && pWebUser && pWebUser->m_pWxUserInfo)
		{
			CCodeConvert convert;
			string loadUrl = url;
			DOWNLOAD_INFO* param = new DOWNLOAD_INFO();
			param->pUser = NULL;
			param->pThis = this;
			param->filePath = convert.URLEncode(path.c_str());
			MapWxTokens::iterator iter = m_mapTokens.find(pWebUser->m_pWxUserInfo->fromwxname);
			if (iter != m_mapTokens.end())
			{
				// ����ƴurl�ǽ�����token�滻�ϣ���ֹ����ͼƬ��Ч
				loadUrl = ReplaceToken(url, iter->second);
			}
			else
			{
				SendGetWxUserInfoAndToken(pWebUser);
			}
			param->downLoadUrl = loadUrl;
			param->msgDataType = MSG_DATA_TYPE_VIDEO;
			if (pAssistUser)
			{
				param->pUser = pAssistUser;
			}
			param->time = GetTimeByMDAndHMS(time);
			param->pWebUser = pWebUser;
			param->msgId = msgId;
			param->msgFromType = msgFromType;
			_beginthreadex(NULL, 0, DownLoadFileFromServerThread, (void*)param, 0, NULL);
		}
		else
		{
			g_WriteLog.WriteLog(C_LOG_TRACE, "DownLoadFile û���õ�token֮ǰ���ܽ����ļ�����");

			string imagePath = FullPath("SkinRes\\mainframe\\");
			StringReplace(imagePath, "\\", "/");

			char contentMsg[MAX_1024_LEN];
			sprintf(contentMsg, "<video class=\"msg_voice\" controls=\"controls\" id=\"%s_msg\" src=\"\" type=\"video/mp4\"></video><img id=\"%s_image\" onclick=ReRecvFile(\"%s\",\"%d\",\"%s\",\"%d\",\"%lu\",\"%lu\",\"%s\") class=\"wait_image\" src=\"%smsg_fail.png\">",
				msgId.c_str(), msgId.c_str(), url.c_str(), MSG_FROM_WEBUSER, msgId.c_str(), MSG_DATA_TYPE_VIDEO, 
				pWebUser->webuserid, assistUid, imagePath.c_str(), imagePath.c_str());
		
			// �������δ���ؼ�¼
			AddMsgToList((IBaseObject*)pWebUser, msgFromType, MSG_RECV_ERROR, msgId, MSG_TYPE_NORMAL,
				nMsgDataType, contentMsg, GetTimeByMDAndHMS(time), pAssistUser, NULL, true, false, false);
		}
	}
}

void CChatManager::AddMsgToList(IBaseObject* pObj, MSG_FROM_TYPE msgFrom, MSG_RECV_TYPE recvType,string msgId, 
	MSG_TYPE msgType, MSG_DATA_TYPE msgDataType,string msgContent, string msgTime, CUserObject* pAssistUser,
	WxMsgBase* msgContentWx, bool bSave, bool bNotify, bool bSuccess)
{
	ONE_MSG_INFO ongMsg;
	ongMsg.msgId = msgId;
	string head = "unKnown";
	string name = "unKnown";
	string sName;
	string sMsg;
	char callJsMsg[MAX_2048_LEN];
	CCodeConvert convert;
	
	if (msgType == MSG_TYPE_NORMAL)
	{
		// ������Ϣ��¼�������л��������ʱ����Ҫ��ȡ��Щ��Ϣ
		if (pObj->m_nEMObType == OBJECT_WEBUSER)
		{
			CWebUserObject* pWebUser = (CWebUserObject*)pObj;

			if (msgFrom == MSG_FROM_WEBUSER)
			{
				if (pWebUser->m_pWxUserInfo)
				{
					head = pWebUser->m_pWxUserInfo->headimgurl;
				}
				else
				{
					string strPath = FullPath("res\\headimage\\default.png");
					StringReplace(strPath, "\\", "/");
					convert.Gb2312ToUTF_8(head, strPath.c_str(), strPath.length());
				}
				name = pWebUser->info.name;
			}
			else if (msgFrom == MSG_FROM_ASSIST)
			{
				head = pAssistUser->m_headPath;
				name = pAssistUser->UserInfo.nickname;
			}
			else if (msgFrom == MSG_FROM_SELF)
			{
				head = m_userInfo.m_headPath;
				name = m_userInfo.UserInfo.nickname;
			}
			else if (msgFrom == MSG_FROM_SYS)
			{
				head = "sys";
				name = "sys";
			}
			else
			{
				g_WriteLog.WriteLog(C_LOG_ERROR, "AddMsgToList ��Ϣ��Դ����");
				return;
			}

			StringReplace(name, "\\", "\\\\");
			StringReplace(name, "'", "&#039;");
			StringReplace(name, "\r\n", "<br>");
			convert.Gb2312ToUTF_8(sName, name.c_str(), name.length());

			if (bSuccess)
			{
				char formatMsg[MAX_1024_LEN];
				if (msgDataType == MSG_DATA_TYPE_TEXT)
				{
					if (msgFrom != MSG_FROM_SELF)
					{
						string strMsg = msgContent;
						transform(strMsg.begin(), strMsg.end(), strMsg.begin(), ::tolower);
						if ((int)strMsg.find("userfile") > -1 && (int)strMsg.find("�յ��ļ�") > -1)
						{
							// Э�������΢�ŷÿͷ���
							msgDataType = MSG_DATA_TYPE_FILE;

							int pos = strMsg.find("href=");							
							if (pos > -1)
							{
								int pos1 = strMsg.find(">", pos);
								if (pos1 > -1)
								{
									string fileUrl;
									fileUrl = strMsg.substr(pos + 6, pos1 - pos - 8);

									int pos2 = fileUrl.find_last_of("/");
									string fileName = fileUrl.substr(pos2 + 1, fileUrl.length() - pos2 - 1);

									sprintf(formatMsg, "�յ��ļ� <a style=\"color: blue;cursor:pointer\" href=\"%s\" target=\"_blank\">%s</a>",
										fileUrl.c_str(), fileName.c_str());
									msgContent = formatMsg;
								}
							}
						}
						else if ((int)strMsg.find("userfile") > -1)
						{
							// web�û�����
							msgDataType = MSG_DATA_TYPE_FILE;
							string vFileName;
							int pos1 = strMsg.find("userfile/");
							vFileName = strMsg.substr(pos1 + 9, strMsg.length() - pos1 - 9);

							sprintf(formatMsg, "�յ��ļ� <a style=\"color: blue;cursor:pointer\" href=\"%s\" target=\"_blank\">%s</a>",
								strMsg.c_str(), vFileName.c_str());
							msgContent = formatMsg;
						}
					}
					
					TransferStrToOldFace(msgContent);

					StringReplace(msgContent, "'", "&#039;");
					StringReplace(msgContent, "\r\n", "<br>");

					convert.Gb2312ToUTF_8(sMsg, msgContent.c_str(), msgContent.length());
				}
				else if (msgDataType == MSG_DATA_TYPE_IMAGE)
				{
					sprintf(formatMsg, "<img class=\"msg_image\" src=\"%s\">", msgContent.c_str());
					sMsg = formatMsg;
				}
				else if (msgDataType == MSG_DATA_TYPE_VOICE)
				{
					sprintf(formatMsg, "<audio controls=\"controls\" class=\"msg_voice\" src=\"%s\" type = \"audio/mpeg\"></audio>", msgContent.c_str());
					sMsg = formatMsg;
				}
				else if (msgDataType == MSG_DATA_TYPE_VIDEO)
				{
					sprintf(formatMsg, "<video controls=\"controls\" class=\"msg_voice\" src=\"%s\" type = \"video/mp4\"></video>", msgContent.c_str());
					sMsg = formatMsg;
				}
				else if (msgDataType == MSG_DATA_TYPE_LOCATION)
				{

				}
				else
				{
					convert.Gb2312ToUTF_8(sMsg, msgContent.c_str(), msgContent.length());
				}
			}
			else
			{
				sMsg = msgContent;
			}

			sprintf(callJsMsg, "AppendMsgToHistory('%d','%d','%s','%s','%s','%s','%s','%s','%s');",
				msgFrom, msgDataType, sName.c_str(), msgTime.c_str(), sMsg.c_str(), "0",
				head.c_str(), msgId.c_str(), "");
			ongMsg.msg = callJsMsg;

			if (bSave)
			{
				bool isNew = true;
				list<ONE_MSG_INFO>::reverse_iterator iter = pWebUser->m_strMsgs.rbegin();
				for (iter; iter != pWebUser->m_strMsgs.rend(); iter++)
				{
					if (iter->msgId == msgId)
					{
						iter->msg = callJsMsg;
						isNew = false;
						break;
					}
				}
				if (isNew)
				{
					pWebUser->m_strMsgs.push_back(ongMsg);
				}
			}
		}
		else if (pObj->m_nEMObType == OBJECT_USER)
		{
			CUserObject* pUser = (CUserObject*)pObj;
			if (msgFrom == MSG_FROM_CLIENT)
			{
				head = pUser->m_headPath;
				name = pUser->UserInfo.nickname;
			}
			else if (msgFrom == MSG_FROM_SELF)
			{
				head = m_userInfo.m_headPath;
				name = m_userInfo.UserInfo.nickname;
			}
			else if (msgFrom == MSG_FROM_SYS)
			{
				head = "sys";
				name = "sys";
			}
			else
			{
				g_WriteLog.WriteLog(C_LOG_ERROR, "AddMsgToList ��Ϣ��Դ����");
				return;
			}

			StringReplace(name, "\\", "\\\\");
			StringReplace(name, "'", "&#039;");
			StringReplace(name, "\r\n", "<br>");
			convert.Gb2312ToUTF_8(sName, name.c_str(), name.length());

			StringReplace(msgContent, "\\", "\\\\");
			StringReplace(msgContent, "'", "&#039;");
			StringReplace(msgContent, "\r\n", "<br>");
			convert.Gb2312ToUTF_8(sMsg, msgContent.c_str(), msgContent.length());

			ReplaceFaceId(sMsg);

			sprintf(callJsMsg, "AppendMsgToHistory('%d','%d','%s','%s','%s','%s','%s','%s');",
				msgFrom, msgDataType, sName.c_str(), msgTime.c_str(), sMsg.c_str(), "0", head.c_str(), msgId.c_str());
			ongMsg.msg = callJsMsg;

			if (bSave && msgType == MSG_TYPE_NORMAL)
			{
				pUser->m_strMsgs.push_back(ongMsg);
			}
		}
	}
	else
	{
		strcpy(callJsMsg, msgContent.c_str());
	}
	
	// ����ʵʱ��ʾ��Ϣ
	// 1. �Լ�������Ϣ�������ƣ��������������
	// 2. �����ļ������ϢҲ�����Ƹ����棬��ͨ��ResultRecvMsg��֮����
	if (msgFrom != MSG_FROM_SELF && bNotify)
	{
		m_handlerMsgs->RecvMsg(pObj, msgFrom, msgId, msgType, msgDataType,
			callJsMsg, msgTime, pAssistUser, msgContentWx);
	}
}

string CChatManager::GetFileId()
{
	if (m_fileId == -1)
	{
		m_fileId = GetTimeLongByDHMS();
	}
	char fileId[MAX_256_LEN];
	sprintf(fileId, "%lu", m_fileId);
	m_idLock.Lock();
	m_fileId++;
	m_idLock.Unlock();
	return fileId;
}

void CChatManager::ClearDirectory(string dir)
{
	WIN32_FIND_DATAA finder;
	HANDLE hFileFind;
	char search[MAX_PATH];
	strcpy(search, dir.c_str());
	strcat(search, "*.*");

	hFileFind = FindFirstFileA(search, &finder);

	if (hFileFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			char path[MAX_PATH];
			strcpy(path, dir.c_str());
			strcat(path, finder.cFileName);

			if ((finder.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				&& strcmp(finder.cFileName, ".") && strcmp(finder.cFileName, ".."))
			{
				char subdir[MAX_PATH];
				strcpy(subdir, path);
				strcat(subdir, "\\");

				ClearDirectory(subdir);
				RemoveDirectoryA(path);
			}
			else if (strcmp(finder.cFileName, ".") && strcmp(finder.cFileName, ".."))
			{
				char subdir[MAX_PATH];
				strcpy(subdir, path);
				strcat(subdir, "\\");
				DeleteFileA(path);
			}

		} while (FindNextFileA(hFileFind, &finder) != 0);

		FindClose(hFileFind);
	}
}

string CChatManager::ReplaceToken(string srcStr, string replaceStr)
{
	int tokenPos = srcStr.find("access_token=");
	int mediaIdPos = srcStr.find("&media_id=");
	int len = strlen("access_token=");
	srcStr.replace(tokenPos + len, mediaIdPos - tokenPos - len, replaceStr.c_str(), replaceStr.length());
	return srcStr;
}

void CChatManager::AddToken(WxUserInfo* userInfo, string token)
{
	if (userInfo != NULL)
	{
		MapWxTokens::iterator iter = m_mapTokens.find(userInfo->fromwxname);
		if (iter == m_mapTokens.end())
		{
			m_mapTokens.insert(map<string, string>::value_type(userInfo->fromwxname, token));
		}
		else
		{
			iter->second = token;
		}
	}
}

void CChatManager::RestartSession(LPARAM lParam)
{
	if (lParam == NULL)
		return;

	char* url = (char*)lParam;
	string strNavigateIconURL = url;

	string strUrl = "";
	string strRet,strErrMsg;
	unsigned long nUin;

	//���ɶ��η���΢�ŻỰ��url
	FormatRequestUrl(strUrl, strNavigateIconURL);

	CCodeConvert convert;
	string strUUrl;
	convert.Gb2312ToUTF_8(strUUrl, strUrl.c_str(), strUrl.length());

	//������λỰ 
	RepickChatCon(strUUrl, strRet, nUin, strErrMsg);
}

void CChatManager::FormatRequestUrl(string &strUrl,string strMsg)
{
	string strPass, strPass1;//��ǰ��ϯ����
	strPass1 = m_userInfo.UserInfo.pass;

	time_t t = time(NULL);
	char strTime[MAX_256_LEN];
	sprintf(strTime,"%ld", t);

	int wr = rand();
	char strRand[MAX_256_LEN];
	sprintf(strRand,"%d", wr);

	strPass = GetMd5Str(strPass1);
	vector<string> v;
	v.push_back(strTime);
	v.push_back(strRand);
	v.push_back(strPass);
	sort(v.begin(), v.end());
	string szall = v[0] + v[1] + v[2];
	SHA_CTX c1;
	SHA1_Init(&c1);
	SHA1_Update(&c1, szall.c_str(), szall.size());
	unsigned char sha1[SHA_DIGEST_LENGTH + 1] = { 0 };
	SHA1_Final(&(sha1[0]), &c1);
	char  szSign[SHA_DIGEST_LENGTH * 2 + 1];
	dec2hexs(sha1, SHA_DIGEST_LENGTH, szSign);
	szSign[SHA_DIGEST_LENGTH * 2] = 0;
	unsigned long adminid = m_login->m_authAdminid;
	char sUrl[MAX_1024_LEN];
	sprintf(sUrl, m_initConfig.webpage_repickchaturl, m_userInfo.UserInfo.uid, szSign, strTime, strRand, adminid, strMsg.c_str());
	strUrl = sUrl;
	g_WriteLog.WriteLog(C_LOG_TRACE, "FormatRequestUrl.%s = [%s]\n", Request_Url_RestartSession, strUrl);
}


#define  MAX_BLOCK 1048576

bool CChatManager::RepickChatCon(string url, string& strRet, unsigned long &uin, string &strErrMsg)
{
	bool bRet = false;
	string recvData;
	int pos = url.find('/', 8);
	if (pos == -1)
		return false;
	string host = url.substr(7, pos - 7);
	string hostbak = host;
	string remotepath = url.substr(pos, url.length());
	pos = host.find(':');
	int port = 80;
	string dsthost;
	if (pos != -1)
	{
		port = atoi(host.substr(pos + 1, host.length() - pos).c_str());
		if (port == 0)
			port = 80;

		dsthost = host.substr(0, pos);
	}
	else
	{
		dsthost = host;
	}

	SwitchSock m_sock;
	if (!m_sock.Create())
		return false;

	if (!m_sock.Connect(dsthost, port))
		return false;

	string cmd;
	cmd = "GET " + remotepath + " HTTP/1.1\r\nHost: " + hostbak + "\r\nUser-Agent:Tracq\r\n" + "\r\n";
	m_sock.Send((char*)cmd.c_str(), cmd.length());

	char* buf = new char[MAX_BLOCK];
	if (buf == NULL)
	{
		bRet = false;
	}

	bool head = true;
	try
	{
		int len = 0;
		int nTotalLen = 0;
		int nIncreaseLen = 0;
		do
		{
			len = 0;
			len = m_sock.Recv(buf, MAX_BLOCK - 1);

			if (len > 0)
			{
				buf[len] = '\0';
				if (head)
				{
					char* pStatue = strchr(buf, ' ');
					char* headend = strstr(buf, "\r\n\r\n");
					char* pConlength = NULL;
					pConlength = strstr(pStatue, "Content-Length:");
					if (pConlength != NULL)
					{
						pConlength += 15;
						nTotalLen = atol(pConlength);
					}

					if ((pStatue == NULL) || (pStatue[1] != '2'))
					{
						bRet = false;
					}

					if (headend != NULL)
						headend += 4;
					else
					{
						headend = strstr(buf, "\n\n");
						if (headend != NULL)
							headend += 2;
					}
					if (headend != NULL)
					{
						head = false;
						recvData += string(headend, buf + len - headend);
						char* pEnd = buf + len;
						nIncreaseLen = pEnd - headend;
					}
				}
				else
				{
					recvData += string(buf, len);
					nIncreaseLen += len;
				}

				if (nIncreaseLen >= nTotalLen)
				{
					break;
				}
			}
			else
			{
				break;
			}
		} while (len > 0);
	}
	catch (...)
	{
		bRet = false;
	}
	if (strlen(recvData.c_str()) <= 0)
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "RepickChat RecvData err!");
	}
	g_WriteLog.WriteLog(C_LOG_DEBUG, "RepickChat %s", recvData.c_str());
	cJSON* json = cJSON_Parse(recvData.c_str());
	if (json != NULL)
	{

		cJSON* jsonParam = cJSON_GetObjectItem(json, "errcode");
		if (jsonParam != NULL)
		{
			int nCode = jsonParam->valueint;
			char sRet[MAX_256_LEN];
			sprintf(sRet, "%d", nCode);
			strRet = sRet;
			jsonParam = cJSON_GetObjectItem(json, "errmsg");
			if (jsonParam != NULL)
			{
				strErrMsg = jsonParam->valuestring;
			}
			char strLog[MAX_256_LEN];
			string sLog;
			sprintf(strLog, "errCode��%s, errMsg: %s", strRet.c_str(), strErrMsg.c_str());
			g_WriteLog.WriteLog(C_LOG_ERROR, "RepickChat err=[%s]\n", strLog);
			sLog = strLog;
			int pos = sLog.find("10001");
			if (pos >= 0)
			{
				MessageBoxA(NULL,"���û������������ͷ�ͨ�����޷������Ự!",0,0);
			}
			else
			{
				pos = sLog.find("45015");
				if (pos >= 0)//����48Сʱ��java�Ѿ����ˡ��˴���ʾȡ����ע
				{
					MessageBoxA(NULL, "���û���ȡ����ע���޷������Ự!",0,0);
				}
				else
				{
					MessageBoxA(NULL,"�޷������Ự!",0,0);
				}
			}
		}
		else
		{
			jsonParam = cJSON_GetObjectItem(json, "cuin");
			uin = jsonParam->valueint;
			bRet = true;
		}

		cJSON_Delete(json);
	}
	else
	{
		MessageBoxA(NULL,"�����Ựʧ�ܣ�û�н��յ���̨��������", 0,0);
		g_WriteLog.WriteLog(C_LOG_ERROR, "RepickChat cJSON_Parse err!");
	}

	m_sock.Close();
	delete[] buf;
	buf = NULL;

	return bRet;
}

int CChatManager::SendTo_GetOnlineUser()
{
	m_hGetOnlineUserThread = CreateThread(NULL, 0, GetOnlineUserThread, this, CREATE_SUSPENDED, NULL);
	ResumeThread(m_hGetOnlineUserThread);
	return 0;
}

DWORD WINAPI CChatManager::GetOnlineUserThread(void *arg)
{
	CChatManager *pManager = (CChatManager *)arg;

	pManager->GetOnlineUser();

	return 0;
}

void CChatManager::GetOnlineUser()
{
	char strURL[MAX_1024_LEN];
	string strHtml = "";

	sprintf(strURL,"%s&uin=%d&strid=%s&pwd=%s&cuin=0&rtt=%lu",m_initConfig.webpage_companyuser,
		m_userInfo.UserInfo.uid, m_userInfo.UserInfo.sid, m_userInfo.UserInfo.pass,time(NULL));

	CHttpLoad httpLoad;
	if (httpLoad.HttpLoad(string(strURL), "", REQUEST_TYPE_GET, "", strHtml))
	{
		CMarkupXml xml(strHtml.c_str());
		CGroupObject *pGroupOb = &m_groupUser;
		pGroupOb->DeleteAll();
		if (ParseGroupUser(xml, pGroupOb, "Group", "Person"))
		{
			m_handlerMsgs->RecvOnlineUsers(pGroupOb);
			return;
		}
	}
	m_handlerMsgs->RecvOnlineUsers(NULL);
}

bool CChatManager::ParseGroupUser(CMarkupXml &xml, CGroupObject *pGroupOb, char *sGroupKey, char *sUserKey)
{
	if (pGroupOb == NULL)
		return false;

	string sex;
	string name;
	unsigned long groupid, pid, id;
	char nc;

	int nPersonNum(0);

	while (xml.FindChildElem(sGroupKey))
	{
		groupid = (unsigned long)atol(xml.GetChildAttrib("id").c_str());
		pid = (unsigned long)atol(xml.GetChildAttrib("pid").c_str());
		name = xml.GetChildAttrib("name");

		CGroupObject *pGroupOb1 = new CGroupObject(pid, name);
		xml.IntoElem();
		if (ParseGroupUser(xml, pGroupOb1, sGroupKey, sUserKey))
		{
			++nPersonNum;
			pGroupOb->AddGroupObject(pGroupOb1);
		}
		else
		{
			delete pGroupOb1;
		}
		xml.OutOfElem();
	}

	while (xml.FindChildElem(sUserKey))
	{
		name = xml.GetChildAttrib("name");
		sex = xml.GetChildAttrib("sex");
		id = (unsigned long)atol(xml.GetChildData().c_str());
		if (sex.empty())
			nc = '0';
		else
			nc = sex[0];

		++nPersonNum;
		pGroupOb->AddUserObject(id, name, nc);
	}

	return nPersonNum > 0;
}

bool CChatManager::TokenIsDifferent(string oldToken, string newToken)
{
	return true;
}

int CChatManager::StartLoginVisitor()
{
	if (m_vistor == NULL)
	{
		m_vistor = new CChatVisitor();
		m_vistor->m_manager = this;
	}
	if (m_vistor->ConnectToVisitorServer())
	{
		return m_vistor->LoginToVisitorServer();
	}
	return SYS_FAIL;
}

int CChatManager::SendToRefuseChat(CWebUserObject *pWebUser, string strReason)
{
	int nError = 0;
	if (pWebUser->m_bNewComm)//�ȴ�Ӧ��ܾ�����Ŀǰ��δ����
	{
	}
	else
	{
		//Ŀǰ��֪���ǲ����������ͽ���ת�Ӱ���ͬ�⣨�ܾ�����ֵ��ô����OK��NO
		COM_SEND_MSG SendInfo(VERSION);
		char strMsg[MAX_1024_LEN];
		sprintf(strMsg,"%s,%u|%s", strReason.c_str(), m_userInfo.UserInfo.uid, m_userInfo.UserInfo.nickname);
		SendInfo.msg.msgtype = MSG_TRANSFER_ANSWER;
		SendInfo.msg.recvuin = pWebUser->talkuid;//�ظ�����ת�ӿ��˸�
		SendInfo.msg.sendtime = GetTimeLong();
		SendInfo.msg.senduin = pWebUser->webuserid;//���˺���
		SendInfo.msg.bak = 0;
		SendInfo.msg.seq = GetPackSeq();
		SendInfo.version = VERSION;
		strncpy(SendInfo.msg.strmsg, strMsg, MAX_MSG_RECVLEN);
		if (pWebUser->chatid != NULL)
			strncpy(SendInfo.strChatid, pWebUser->chatid, MAX_CHATID_LEN);
		if (pWebUser->info.sid != NULL)
			strncpy(SendInfo.strRand, pWebUser->info.sid, MAX_WEBCLIENID_LEN);
		if (pWebUser->info.thirdid != NULL)
			strncpy(SendInfo.strThirdid, pWebUser->info.thirdid, MAX_THIRDID_LEN);
		strncpy(SendInfo.msg.strmobile, m_userInfo.UserInfo.nickname, MAX_MSGNICKNAME_LEN);
		g_WriteLog.WriteLog(C_LOG_TRACE, "����commת�Ӿܾ�: recvuin:%u,senduin:%u,strMsg:%s",
			SendInfo.msg.recvuin, SendInfo.msg.senduin, SendInfo.msg.strmsg);

		nError = SendPackTo(&SendInfo);
		if (nError == 0)
		{
			//ɾ��ת���еĻỰ, Ŀǰ����û�з��͵ľܾ����ذ����޷��ж϶Է��Ƿ��յ��ܾ�����ֻ���ڷ��ͳɹ���ɾ���Ự
			pWebUser->m_bConnected = false;
			pWebUser->onlineinfo.talkstatus = TALKSTATUS_NO;
			pWebUser->info.status = STATUS_OFFLINE;
			pWebUser->transferuid = 0;
		}
	}

	return nError;
}

void CChatManager::SetAllUserOffline()
{
	CUserObject *pUserOb;
	CWebUserObject *pWebUserOb;
	string strKey;

	MapUsers::iterator iTer_user;
	for (iTer_user = m_mapUsers.begin(); iTer_user != m_mapUsers.end(); iTer_user++)
	{
		pUserOb = iTer_user->second;
		if (pUserOb != NULL)
		{
			pUserOb->status = STATUS_OFFLINE;
		}
	}

	MapWebUsers::iterator iTer_webUser;
	for (iTer_webUser = m_mapWebUsers.begin(); iTer_webUser != m_mapWebUsers.end(); iTer_webUser++)
	{
		pWebUserOb = iTer_webUser->second;
		if (pWebUserOb != NULL)
		{
			pWebUserOb->onlineinfo.talkstatus = TALKSTATUS_NO;
			pWebUserOb->info.status = STATUS_OFFLINE;
			pWebUserOb->m_bIsGetInfo = false;
		}
	}

	m_userInfo.status = STATUS_OFFLINE;
}

void CChatManager::DeleteAllSrvInfo()
{
	//DeleteAllKeyWordInfo();
	//DeleteAllKeyWordGroupInfo();
	DeleteAllUserInfo();
	DeleteAllWebUserInfo();
}

void CChatManager::DeleteAllUserInfo()
{
	CUserObject *pUserOb;
	MapUsers::iterator iTer_user;
	for (iTer_user = m_mapUsers.begin(); iTer_user != m_mapUsers.end(); iTer_user++)
	{
		pUserOb = iTer_user->second;
		if (pUserOb != NULL)
		{
			delete pUserOb;
			pUserOb = NULL;
		}
	}
	m_mapUsers.clear();
}

void CChatManager::DeleteAllWebUserInfo()
{
	CWebUserObject *pWebUserOb;
	MapWebUsers::iterator iTer_user;
	for (iTer_user = m_mapWebUsers.begin(); iTer_user != m_mapWebUsers.end(); iTer_user++)
	{
		pWebUserOb = iTer_user->second;
		if (pWebUserOb != NULL)
		{
			delete pWebUserOb;
			pWebUserOb = NULL;
		}
	}
	m_mapWebUsers.clear();
}

void CChatManager::TransferStrToOldFace(string& msg)
{
	int n;
	string strPath = FullPath("res/oldfaces/");
	StringReplace(strPath, "\\", "/");
	std::string::size_type begin, pos, end;
	char face[MAX_128_LEN];
	for ( pos = 0; begin = msg.find("\\<", pos), begin != std::string::npos;)
	{
		end = msg.find('>', begin);
		if (end == std::string::npos)
			break;
		pos = begin;
		string num = msg.substr(begin + 2, end - begin - 2);
		n = atoi(num.c_str());
		sprintf(face, "<img src=\"%s%d.gif\" width=24 height=24>", strPath.c_str(),n);
		msg.replace(begin, ++end - begin, face);
		pos += strlen(face);
	}
}

void CChatManager::TransferFaceToServeStr(string& msg)
{

}

bool CChatManager::ParseTextMsg(CWebUserObject* pWebUser, string content, CUserObject* pAssistUser, unsigned long time)
{
	if ((int)content.find("[ϵͳ��Ϣ]") > -1)
	{
		return false;
	}
	string msgId = GetMsgId();
	string imagePath = FullPath("SkinRes\\mainframe\\");
	StringReplace(imagePath, "\\", "/");

	if ((int)content.find("http:") > -1 && (int)content.find("�յ�һ��ͼƬ") > -1 && pAssistUser)
	{
		// Э�������web�ÿͷ���
		int pos = content.find("href =");
		int pos1 = content.find("target=");
		if (pos > -1 && pos1 > -1)
		{
			string imageUrl;
			imageUrl = content.substr(pos + 7, pos1 - pos - 8);

			char contentMsg[MAX_1024_LEN];
			sprintf(contentMsg, "<img id=\"%s_msg\" class=\"msg_image\" src=\"%srecv_image_fail.jpg\"><img id=\"%s_image\" class=\"wait_image\" src=\"%smsg_wait.gif\">",
				msgId.c_str(), imagePath.c_str(), msgId.c_str(), imagePath.c_str());

			AddMsgToList((IBaseObject*)pWebUser, MSG_FROM_ASSIST, MSG_RECV_ERROR, msgId, MSG_TYPE_NORMAL, MSG_DATA_TYPE_IMAGE, contentMsg, GetTimeByMDAndHMS(0), pAssistUser, NULL, false, true, false);
			DownLoadFile(pWebUser, MSG_DATA_TYPE_IMAGE, imageUrl, pAssistUser, time, msgId);
		}
		return true;
	}
	else if ((int)content.find("http:") > -1 && ((int)content.find(".jpg") > -1 ||
		(int)content.find(".jpeg") > -1 || (int)content.find(".bmp") > -1 ||
		(int)content.find(".png") > -1))
	{
		// web�û�����
		char contentMsg[MAX_1024_LEN];
		sprintf(contentMsg, "<img id=\"%s_msg\" class=\"msg_image\" src=\"%srecv_image_fail.jpg\"><img id=\"%s_image\" class=\"wait_image\" src=\"%smsg_wait.gif\">",
			msgId.c_str(), imagePath.c_str(), msgId.c_str(), imagePath.c_str());

		AddMsgToList((IBaseObject*)pWebUser, MSG_FROM_WEBUSER, MSG_RECV_ERROR, msgId, MSG_TYPE_NORMAL, MSG_DATA_TYPE_IMAGE, contentMsg, GetTimeByMDAndHMS(0), pAssistUser, NULL, false, true, false);
		DownLoadFile(pWebUser, MSG_DATA_TYPE_IMAGE, content, pAssistUser, time, msgId);

		return true;
	}
	return false;
}


// �ϴ��ļ���΢�ŷ�����
UINT WINAPI CChatManager::UpLoadFileToWxServerThread(void * pUpLoadInfo)
{
	UPLOAD_INFO* upLoadInfo = (UPLOAD_INFO*)pUpLoadInfo;
	string filePath = upLoadInfo->filePath;
	CChatManager* pThis = (CChatManager*)upLoadInfo->pThis;
	unsigned long userId = upLoadInfo->userId;
	MSG_DATA_TYPE msgDataType = upLoadInfo->msgDataType;
	MSG_RECV_TYPE userType = upLoadInfo->userType;
	string msgId = upLoadInfo->msgId;
	delete upLoadInfo;

	string wxToken;
	CHttpLoad load;
	CCodeConvert convert;
	CWebUserObject* pWebUser = pThis->GetWebUserObjectByUid(userId);
	if (pWebUser == NULL && pWebUser->m_pWxUserInfo == NULL)
	{
		pThis->SendGetWxUserInfoAndToken(pWebUser);
		pThis->AfterUpload(userId, userType, msgId, "", msgDataType, "", filePath);
		return false;
	}

	MapWxTokens::iterator iter = pThis->m_mapTokens.find(pWebUser->m_pWxUserInfo->fromwxname);
	if (iter != pThis->m_mapTokens.end())
	{
		string weChatMediaUploadFormat = pThis->m_initConfig.wechat_media_upload;
		char weChatMediaUpload[MAX_256_LEN];
		wxToken = iter->second;
		string fullFilePath = filePath;
		if (msgDataType == MSG_DATA_TYPE_IMAGE)
		{
			sprintf(weChatMediaUpload, weChatMediaUploadFormat.c_str(), wxToken.c_str(), "image");
		}
		else if (msgDataType == MSG_DATA_TYPE_VOICE)
		{
			sprintf(weChatMediaUpload, weChatMediaUploadFormat.c_str(), wxToken.c_str(), "voice");
			fullFilePath += ".amr";
		}
		// �ϴ����ļ���΢�ŷ�������΢�ŷ������᷵��һ��json��������һ��media_idֵ
		string returnCode;
		string decodeFilePath = convert.URLDecodeALL(fullFilePath);
		if (load.HttpLoad(string(weChatMediaUpload), "", REQUEST_TYPE_POST, decodeFilePath, returnCode))
		{
			Json::Value json;
			if (ParseJson(returnCode, json))
			{
				// ��ȡ��media_idֵ
				string mediaID = GetStrFromJson(json, "media_id");
				if (!mediaID.empty())
				{
					string fileServerMediaTaskFormat = pThis->m_initConfig.fileserver_media_task;
					char fileServerMediaTask[MAX_256_LEN];
					returnCode.clear();
					CHttpLoad loadTask;
					sprintf(fileServerMediaTask, fileServerMediaTaskFormat.c_str(), wxToken.c_str(), mediaID.c_str());
					// ���ݸ�media_idֵ��tokenֵ��ȥ�Լ����ļ���������һ��task���ʣ�Ҳ����һ��mediaֵ					
					if (loadTask.HttpLoad(string(fileServerMediaTask), "", REQUEST_TYPE_GET, "", returnCode))
					{
						if (msgDataType == MSG_DATA_TYPE_IMAGE || msgDataType == MSG_DATA_TYPE_VOICE)
						{
							pThis->AfterUpload(userId, userType, msgId, mediaID, msgDataType, returnCode, filePath);
						}
						else
						{
							g_WriteLog.WriteLog(C_LOG_ERROR, "δ֪��ʽ���ļ��ϴ���%s", returnCode);
						}
					}
					else
					{
						// ��task���ʲ��ɹ�ʱ����Ȼ��Ϊ�ļ��ϴ��ɹ�������ý����Ϣ���͸�΢�ŷ�
						if (msgDataType == MSG_DATA_TYPE_IMAGE || msgDataType == MSG_DATA_TYPE_VOICE)
						{
							pThis->AfterUpload(userId, userType, msgId, mediaID, msgDataType, "", filePath);
						}
						else
						{
							g_WriteLog.WriteLog(C_LOG_ERROR, "δ֪��ʽ���ļ��ϴ���%s", returnCode);
						}
						g_WriteLog.WriteLog(C_LOG_ERROR, "û�л�ȡ�ϴ��ļ���΢�ŷ�������task��%s", returnCode.c_str());
					}
					return true;
				}
				else
				{
					g_WriteLog.WriteLog(C_LOG_ERROR, "û�л�ȡ�ϴ��ļ���΢�ŷ�������media_id��%s", returnCode.c_str());
				}
			}
			else
			{
				g_WriteLog.WriteLog(C_LOG_ERROR, "�����ϴ��ļ���΢�ŷ�����������ʧ�ܣ�%s", returnCode.c_str());
			}
		}
		else
		{
			g_WriteLog.WriteLog(C_LOG_ERROR, "�ϴ��ļ���΢�ŷ�����ʧ�ܣ�%s,��ǰtokenֵ��: %s", returnCode.c_str(), wxToken.c_str());
		}
	}
	else
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "�ϴ��ļ���΢�ŷ�����ʧ�ܣ���Ϊû��tokenֵ");
	}

	// ֻҪʧ�ܣ�һ�ɻ�ȡtokenֵ
	pThis->SendGetWxToken(userId, pWebUser->chatid);

	// �������ʧ�ܣ����ļ�·�����룬�������·����õ�
	pThis->AfterUpload(userId, userType, msgId, "", msgDataType, "", filePath);
	return false;
}

// �ϴ��ļ���������
UINT WINAPI CChatManager::UpLoadFileToServerThread(void * pUpLoadInfo)
{
	UPLOAD_INFO* upLoadInfo = (UPLOAD_INFO*)pUpLoadInfo;
	string filePath = upLoadInfo->filePath;
	string msgId = upLoadInfo->msgId;
	CChatManager* pThis = (CChatManager*)upLoadInfo->pThis;
	unsigned long userId = upLoadInfo->userId;
	MSG_DATA_TYPE msgDataType = upLoadInfo->msgDataType;
	MSG_RECV_TYPE userType = upLoadInfo->userType;
	delete upLoadInfo;

	CHttpLoad httpLoad;
	CCodeConvert convert;

	string fileServerMediaUploadFormat = pThis->m_initConfig.fileserver_media_upload;
	string resultCode;
	string decodeFilePath = convert.URLDecodeALL((string)filePath);
	if (httpLoad.HttpLoad(string(fileServerMediaUploadFormat), "", REQUEST_TYPE_POST, decodeFilePath, resultCode))
	{
		Json::Value json;
		resultCode = resultCode.substr(1, resultCode.length() - 1);
		resultCode = resultCode.substr(0, resultCode.length() - 1);
		if (ParseJson(resultCode, json))
		{
			string mediaID = GetStrFromJson(json, "media_id");
			if (!mediaID.empty())
			{
				// Ŀǰֻ�ж���һ����ϯ��ͼƬ��������߼�
				pThis->AfterUpload(userId, userType, msgId, mediaID, msgDataType, resultCode, filePath);
				return true;
			}
			else
			{
				g_WriteLog.WriteLog(C_LOG_ERROR, "��ȡ�ϴ��ļ����ļ�������media_idʧ�ܣ�%s", resultCode.c_str());
			}
		}
		else
		{
			g_WriteLog.WriteLog(C_LOG_ERROR, "�����ϴ��ļ����ļ�������������ʧ�ܣ�%s", resultCode.c_str());
		}
	}
	else
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "�ϴ��ļ����ļ�������ʧ�ܣ�%s", resultCode.c_str());
	}
	pThis->AfterUpload(userId, userType, "", "", msgDataType, "", "");
	return false;
}

// �ӷ����������ļ�
UINT WINAPI CChatManager::DownLoadFileFromServerThread(void * para)
{
	DOWNLOAD_INFO* param = (DOWNLOAD_INFO*)para;
	string filePath = param->filePath;
	CChatManager* manager = (CChatManager*)param->pThis;
	string url = param->downLoadUrl;
	string time = param->time;
	CWebUserObject* pWebUser = param->pWebUser;
	CUserObject* pUser = param->pUser;
	MSG_DATA_TYPE msgDataType = param->msgDataType;
	string msgId = param->msgId;
	MSG_FROM_TYPE msgFrom = param->msgFromType;
	delete param;

	CHttpLoad httpLoad;
	CCodeConvert convert;
	string sendName;
	unsigned long userId = -1;
	unsigned long webUserId = -1;
	string rs;

	if (pWebUser == NULL)
		return false;

	// pUser��Ϊ�ձ�ʾЭ����������ý���ļ�
	if (pUser)
	{
		sendName = pUser->UserInfo.nickname;
		userId = pUser->UserInfo.uid;
	}
	else
	{
		sendName = pWebUser->info.name;
	}
	webUserId = pWebUser->webuserid;

	string downPath;
	string addPath;
	string decodePath = convert.URLDecodeALL(filePath);
	if (msgDataType == MSG_DATA_TYPE_IMAGE)
	{
		downPath = decodePath + ".jpg";
		addPath = decodePath + ".jpg";
		StringReplace(addPath, "\\", "/");
	}
	else if (msgDataType == MSG_DATA_TYPE_VOICE)
	{
		downPath = decodePath + ".amr";
		addPath = decodePath + ".wav";
		StringReplace(addPath, "\\", "/");
	}
	else if (msgDataType == MSG_DATA_TYPE_VIDEO)
	{
		downPath = decodePath + ".mp4";
		addPath = decodePath + ".mp4";
		StringReplace(addPath, "\\", "/");
	}

	if (httpLoad.HttpLoad(url, "", REQUEST_TYPE_GET, downPath, rs))
	{
		if (msgDataType == MSG_DATA_TYPE_VOICE)
		{
			// ת����ʽ
			manager->Amr2Wav(decodePath.c_str());
		}

		manager->m_handlerMsgs->ResultRecvMsg(msgId, true, url, webUserId, userId, addPath, msgFrom, msgDataType);

		// ���������¼
		manager->AddMsgToList((IBaseObject*)pWebUser, msgFrom, MSG_RECV_ERROR, msgId, MSG_TYPE_NORMAL, msgDataType, addPath, time, pUser, NULL, true, false, true);

		return true;
	}
	else
	{
		manager->m_handlerMsgs->ResultRecvMsg(msgId, false, url, webUserId, userId, "", msgFrom, msgDataType);

		string imagePath = FullPath("SkinRes\\mainframe\\");
		StringReplace(imagePath, "\\", "/");

		char contentMsg[MAX_1024_LEN];

		if (msgDataType == MSG_DATA_TYPE_IMAGE)
		{
			sprintf(contentMsg, "<img id=\"%s_msg\" class=\"msg_image\" src=\"%srecv_image_fail.jpg\"><img id = \"%s_image\" onclick=ReRecvFile(\"%s\",\"%d\",\"%s\",\"%d\",\"%lu\",\"%lu\",\"%s\") class=\"wait_image\" src=\"%smsg_fail.png\">",
				msgId.c_str(), imagePath.c_str(), msgId.c_str(), url.c_str(), msgFrom, msgId.c_str(), MSG_DATA_TYPE_IMAGE, webUserId, userId, imagePath.c_str(), imagePath.c_str());
		}
		else if (msgDataType == MSG_DATA_TYPE_VOICE)
		{
			sprintf(contentMsg, "<audio class=\"msg_voice\" controls=\"controls\" id=\"%s_msg\" src=\"\" type=\"audio/mpeg\"></audio><img id=\"%s_image\" onclick=ReRecvFile(\"%s\",\"%d\",\"%s\",\"%d\",\"%lu\",\"%lu\",\"%s\") class=\"wait_image\" src=\"%smsg_fail.png\">",
				msgId.c_str(), msgId.c_str(), url.c_str(), msgFrom, msgId.c_str(), MSG_DATA_TYPE_VOICE, webUserId, userId, imagePath.c_str(), imagePath.c_str());
		}
		else if (msgDataType == MSG_DATA_TYPE_VIDEO)
		{
			sprintf(contentMsg, "<video class=\"msg_voice\" controls=\"controls\" id=\"%s_msg\" src=\"\" type=\"video/mp4\"></video><img id=\"%s_image\" onclick=ReRecvFile(\"%s\",\"%d\",\"%s\",\"%d\",\"%lu\",\"%lu\",\"%s\") class=\"wait_image\" src=\"%smsg_fail.png\">",
				msgId.c_str(), msgId.c_str(), url.c_str(), msgFrom, msgId.c_str(), MSG_DATA_TYPE_VIDEO, webUserId, userId, imagePath.c_str(), imagePath.c_str());
		}

		manager->AddMsgToList((IBaseObject*)pWebUser, msgFrom, MSG_RECV_ERROR, msgId, MSG_TYPE_NORMAL, msgDataType,
			contentMsg, time, pUser, NULL, true, false, false);
		return true;
	}
}

DWORD WINAPI CChatManager::SendFileThread(void *arg)
{
	UPFILETHREAD_STRUCT *pUpFile = (UPFILETHREAD_STRUCT *)arg;

	CChatManager* pThis = (CChatManager*)pUpFile->pThis;
	string strURLFile;
	string errInfo;

	strURLFile = pThis->m_initConfig.webpage_SendWebFileLink;
	int nRet = ::SendFileToWebUserFunc(strURLFile, pUpFile->filePath, pThis->m_userInfo.UserInfo.uid, pUpFile->recvuid);
	if (nRet == -1)
	{
		strURLFile.clear();
		errInfo = "��֧�ֵ��ļ���ʽ�У�gif|jpg|jpeg|png|bmp|zip|rar|doc|docx|txt|xls|xlsx|swf|ppt|pptx|wps|pdf";
	}
	else if (nRet == -2)
	{
		strURLFile.clear();
		errInfo = "�����ļ�ʧ�ܣ�";
	}
	pThis->AfterUpload(pUpFile->recvuid, pUpFile->recvType, pUpFile->msgId, strURLFile, MSG_DATA_TYPE_FILE);
	return 0;
}

bool CChatManager::SendFileToUser(IBaseObject* pUser, string strPathFile, string msgId, MSG_RECV_TYPE userType)
{
	if (pUser == NULL) return false;

	if (pUser->m_nEMObType == OBJECT_WEBUSER)
	{
		CWebUserObject* pWebUser = (CWebUserObject*)pUser;
		
		if (m_sendFileThreadHandle != NULL)
		{
			TerminateThread(m_sendFileThreadHandle, 0);
		}
		m_sendFileThreadHandle = NULL;

		UPFILETHREAD_STRUCT *pUpFileStruct = new UPFILETHREAD_STRUCT();

		memset(pUpFileStruct, '\0', sizeof(UPFILETHREAD_STRUCT));

		pUpFileStruct->pThis = this;
		pUpFileStruct->recvuid = pWebUser->webuserid;
		pUpFileStruct->filePath = strPathFile;
		pUpFileStruct->recvType = userType;
		pUpFileStruct->msgId = msgId;
		m_sendFileThreadHandle = CreateThread(NULL, 0, SendFileThread, (VOID *)pUpFileStruct, CREATE_SUSPENDED, NULL);
		ResumeThread(m_sendFileThreadHandle);

		return true;
	}
	else if (pUser->m_nEMObType == OBJECT_USER)
	{
		CUserObject* pUserObj = (CUserObject*)pUser;

		if (m_sendFileThreadHandle != NULL)
		{
			TerminateThread(m_sendFileThreadHandle, 0);
		}
		m_sendFileThreadHandle = NULL;

		UPFILETHREAD_STRUCT *pUpFileStruct = new UPFILETHREAD_STRUCT();

		memset(pUpFileStruct, '\0', sizeof(UPFILETHREAD_STRUCT));
		pUpFileStruct->pThis = this;
		pUpFileStruct->recvuid = pUserObj->UserInfo.uid;
		pUpFileStruct->filePath = strPathFile;
		pUpFileStruct->recvType = userType;
		pUpFileStruct->msgId = msgId;
		m_sendFileThreadHandle = CreateThread(NULL, 0, SendFileThread, (VOID *)pUpFileStruct, CREATE_SUSPENDED, NULL);
		ResumeThread(m_sendFileThreadHandle);

		return true;
	}
}


