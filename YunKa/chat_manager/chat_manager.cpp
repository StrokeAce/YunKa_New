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


CChatManager::CChatManager()
{
	m_bExit = false;
	m_bLoginSuccess = false;
	m_server = "tcp01.tq.cn";
	m_port = 443;
	m_usSrvRand = 0;
	m_packSeq = 0;
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
	assert(m_handlerLogin); // 登录之前就应当将消息回调接口设置好

	m_sLogin = loginName;
	m_password = password;
	m_bAutoLogin = isAutoLogin;
	m_bKeepPwd = isKeepPwd;

	// 开始登录时，启动一个判断登录超时的定时器
	m_timers->SetTimer(10000, TIMER_LOGIN);
	m_login->StartLogin(loginName, password, isAutoLogin, isKeepPwd);
}

bool CChatManager::ReadSystemConfig()
{
	// 加载默认的设置文件
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
	case CMD_SRV_CONF_LOGON: // 登录确认
		nError = RecvSrvConfLogon(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		if (nError != 0)
		{
			goto FAIL;
		}
		m_handlerLogin->LoginProgress(80);
		break;
	case CMD_SRV_REP_USERINFO: // 用户信息包
		nError = RecvSrvRepUserinfo(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		if (nError != 0)
		{
			goto FAIL;
		}
		break;
	case CMD_SRV_DENY_LOGON: // 拒绝登录
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
	case CMD_SRV_CONF_LOGOFF:   // 登出确认包
		nError = RecvSrvConfLogOff(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_SRV_STATUS_FRDONLINE: // 好友上线
		nError = RecvSrvStatusFrdOnline(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_SRV_STATUS_FRDOFFLINE: // 好友下线
		nError = RecvSrvStatusFrdOffline(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_SRV_STATUS_USER_FORM: // 好友更改在线状态
		nError = RecvSrvStatusUserForm(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_FLOAT_SHARELIST: // 收到好友对象列表
		nError = RecvFloatShareList(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_FLOAT_CREATECHAT: // 接入会话
		nError = RecvFloatCreateChat(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_FLOAT_CHATINFO: // 会话详细信息
		nError = RecvFloatChatInfo(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_COM_SEND_MSG: // 会话消息,非等待应答策略
		nError = RecvComSendMsg(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_FLOAT_CHATMSG: // 会话消息,等待应答策略
		nError = RecvFloatChatMsg(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_FLOAT_CHATMSG_ACK: // 发送消息应答
		nError = RecvFloatChatMsgAck(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_FLOAT_ACCEPTCHAT: // 接受会话
		nError = RecvFloatAcceptChat(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_FLOAT_TRANSQUEST: // 等待应答  会话转移请求
		nError = RecvFloatTransQuest(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_FLOAT_TRANSFAILED:   // 等待应答  会话转移失败
		nError = RecvFloatTransFailed(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_FLOAT_INVITE_REQUEST:// 等待应答  邀请请求
		nError = RecvInviteRequest(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_FLOAT_INVITE_RESULT:// 等待应答  邀请结果
		nError = RecvInviteResult(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_FLOAT_RELEASE: // 坐席主动释放会话
		nError = RecvFloatRelease(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_FLOAT_CMDERROR: // 等待应答  命令失败
		nError = RecvFloatCMDError(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_FLOAT_CLOSECHAT: // 会话关闭
		nError = RecvFloatCloseChat(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_FLOAT_LISTCHAT: //会话列表
		nError = RecvFloatListChat(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;	
	case CMD_EVENT_ANNOUNCEMENT: // 收到消息通告
		nError = RecvEventAnnouncement(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_SRV_UPDATE_SUCC: //更新信息成功
		nError = RecvSrvUpdateSucc(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_SRV_UPDATE_FAIL: // 更新信息失败
		nError = RecvSrvUpdateFail(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_SRV_SERVER_COPY: // 用户在异地登陆
		nError = RecvSrvDown(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_SRV_REP_TRANSFERCLIENT: // 440 转移临时用户成功失败
		nError = RecvRepTransferClient(Head.head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_SRV_TRANSFERCLIENT: // 接收转移会话的用户处理
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

	//服务器返回的RecvInfo.type_loginby为0有误
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
		string msg = "自己的消息记录";
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

		// 下载头像
		pUser->DownLoadFace(m_initConfig.webpage_DownloadHeadImage);

		LoginSuccess();
	}
	else
	{
		// 登录成功后收到用户信息包处理
		if (packhead.langtype == LANGUAGE_UTF8)
		{
			// 转码
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
					g_WriteLog.WriteLog(C_LOG_ERROR, "RecvSrvRepUserinfo() name length：%d", strlen(pWebUser->info.name));
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
				// 下载头像
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
						msg += "的消息记录";
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

	// 查询来电
	LoadIniString("WebPages", "querywebphone", m_initConfig.webpage_querywebphone, len, sFile, "http://vip.tq.cn/vip/screenPhoneRecord.do?action=query&deal_state=0");

	// 留言
	LoadIniString("WebPages", "note", m_initConfig.webpage_note, len, sFile, "http://vip.tq.cn/vip/viewLiuyan.do?a=1");

	// 客户管理
	LoadIniString("WebPages", "crm", m_initConfig.webpage_crm, len, sFile, "http://vip.tq.cn/vip/visitorInfomation.do?a=1");

	// 统计分析
	LoadIniString("WebPages", "analyze", m_initConfig.webpage_analyze, len, sFile, "http://vip.tq.cn/vip/serviceStutasE.do?a=1");

	// 管理中心
	LoadIniString("WebPages", "mgmt", m_initConfig.webpage_mgmt, len, sFile, "http://vip.tq.cn/vip/clientIndex.do?a=1");

	// 焦点窗口
	LoadIniString("WebPages", "news", m_initConfig.webpage_news, len, sFile, "http://vip.tq.cn/vip/rss/tq_mini_loading.jsp?a=");

	// 注册新用户
	LoadIniString("WebPages", "regnewuser", m_initConfig.webpage_regnewuser, len, sFile, "http://www.tq.cn/vip/prenewqttclt.do");
	LoadIniInt("WebPages", "regnewuser_w", m_initConfig.webpage_regnewuser_w, sFile, "520");
	LoadIniInt("WebPages", "regnewuser_h", m_initConfig.webpage_regnewuser_h, sFile, "410");

	// 新手指南
	LoadIniString("WebPages", "tqhelp", m_initConfig.webpage_tqhelp, len, sFile, "http://www.tq.cn/help_3.html");

	// 统一用语
	LoadIniString("WebPages", "UniIdioms", m_initConfig.webpage_UniIdioms, len, sFile, "http://vip.tq.cn/vip/GetRequestXMLAction.do?cmd=NEWQUICKREPLY");

	// 实时监控
	LoadIniString("WebPages", "RtmMsg", m_initConfig.webpage_RtmMsg, len, sFile, "http://vip.tq.cn/vip/realTime.do?a=");

	// 漫游消息
	LoadIniString("WebPages", "SvrMsg", m_initConfig.webpage_SvrMsg, len, sFile, "http://vip.tq.cn/vip/historyDialog.do?a=");

	// TQ顾问
	LoadIniString("WebPages", "TQadviser", m_initConfig.webpage_TQadviser, len, sFile, "http://vip.tq.cn/vip/myTqGuWen.do?a=");

	// 上传头像
	LoadIniString("WebPages", "UploadHeadImage", m_initConfig.webpage_UploadHeadImage, len, sFile, "http://vip.tq.cn/vip/preuploadfacelink.do?a=");

	// 下载头像
	LoadIniString("WebPages", "DownloadHeadImage", m_initConfig.webpage_DownloadHeadImage, len, sFile, "http://admin.tq.cn/vip/facelinkimgs/");

	// 表情
	LoadIniString("WebPages", "FaceImage", m_initConfig.webpage_FaceImage, len, sFile, "http://vip.tq.cn/clientimages/face/images");

	// 系统消息
	LoadIniString("WebPages", "SysMessage", m_initConfig.webpage_SysMessage, len, sFile, "http://sysmessage.tq.cn/SysMsg/getmesg.do?strid=%s&uin=%lu&departId=%d");

	// 投诉建议
	LoadIniString("WebPages", "complaint", m_initConfig.webpage_complaint, len, sFile, "http://211.151.52.62:8081/zhengquan/preLeaveMsg.do?uin=%lu");

	//发送文件
	LoadIniString("WebPages", "SendWebFileLink", m_initConfig.webpage_SendWebFileLink, len, sFile, "http://vip.tq.cn/upload/upload2.do?version=100");


	LoadIniString("WebPages", "iconurl", m_initConfig.webpage_iconurl, len, sFile, "http://sysimages.tq.cn/clientimages/face/ywt_face.html");
	LoadIniString("WebPages", "faqInfo", m_initConfig.webpage_faqInfo, len, sFile, "http://211.151.52.39:8080/vip/DoRequestXMLAction.do?action=faqInfo");
	LoadIniString("WebPages", "faqSort", m_initConfig.webpage_faqSort, len, sFile, "http://211.151.52.39:8080/vip/DoRequestXMLAction.do?action=faqSort");
	LoadIniString("WebPages", "faqAll", m_initConfig.webpage_faqall, len, sFile, "http://211.151.52.39:8080/vip/DoRequestXMLAction.do?action=faqAll");
	LoadIniString("WebPages", "repickchat", m_initConfig.webpage_repickchaturl, len, sFile, "http://106.120.108.230:8031/wxcreatechat?uin=%lu&signature=%s&timestamp=%s&nonce=%s&admiuin=%lu&%s&msg=hello");
	//邀请评价evaluate
	LoadIniString("WebPages", "evaluate", m_initConfig.webpage_evaluate, len, sFile, "http://106.120.108.230:8524/vip/scorebill.do?billid=%s&admin_uin=%u&action=scoreedit");

	LoadIniString("WebPages", "sendurl", m_initConfig.webpage_sendurl, len, sFile, "http://211.151.52.39:8080/vip/DoRequestXMLAction.do?action=url");

	//获取公司用户
	LoadIniString("WebPages", "companyuser", m_initConfig.webpage_companyuser, len, sFile, "http://vip.tq.cn/vip/GetRequestXMLData?cmd=TRANSFER");

	// 获取微信token
	LoadIniString("WebPages", "accesstoken", m_initConfig.webpage_accesstoken, len, sFile, "http://106.120.108.230:8031/wxkfgetac?uin=%lu&signature=%s&timestamp=%s&nonce=%s&{$MSG_WORKBILL.UserDefineParams}");

	//工单
	LoadIniString("WebPages", "workbillurl", m_initConfig.webpage_workbillurl, len, sFile, "http://106.120.108.230:8524/vip/workOrder.do?r=%s");

	// 客户履历
	LoadIniString("VisitorPages", "visitortail", m_initConfig.visitorpage_visitortail, len, sFile, "http://vip.tq.cn/vip/ClientDialog.do?billid=%s&cuin=%lu&rand=%s&transtype=%d&transfrom=%lu&stime=%s&etime=%s");

	//会话登单
	LoadIniString("VisitorPages", "visitorbill", m_initConfig.visitorpage_visitorbill, len, sFile, "http://vip.tq.cn/vip/recrodworkbill.do?action=prerecord&billid=%s&cuin=%lu&rand=%s&transtype=%d&transfrom=%lu&stime=%s&etime=%s");

	//客户信息
	LoadIniString("VisitorPages", "visitorinfo", m_initConfig.visitorpage_visitorinfo, len, sFile, "http://vip.tq.cn/vip/visitorinfo.do?billid=%s&cuin=%lu&rand=%s&transtype=%d&transfrom=%lu&stime=%s&etime=%s");
	//下订单
	LoadIniString("VisitorPages", "visitororder", m_initConfig.visitorpage_visitororder, len, sFile, "http://106.120.108.230:8524/vip/visitorinfo.do?billid=%s&cuin=%lu&rand=%s&transtype=%d&transfrom=%lu&stime=%s&etime=%s&clientid=%s&type=0");

	//通告信息
	LoadIniString("VisitorPages", "visitornotice", m_initConfig.visitorpage_notice, len, sFile, "http://www.tq.cn");
	LoadIniString("WebPages", "RegisterAccount", m_initConfig.registerAccount, len, sFile, "http://www.tq.cn");
	LoadIniString("WebPages", "ForgetPassword", m_initConfig.forgetPassword, len, sFile, "http://www.tq.cn");
	LoadIniInt("WebPages", "ForgetPSVisible", m_initConfig.forgetPSVisible, sFile);

	// 查询历史记录
	LoadIniString("WebPages", "QueryHistoryRecords", m_initConfig.query_history_records, len, sFile, "http://106.120.108.230:8524/vip/getbilllist.do?adminuin=%lu&kefu_uin=%lu&client_id=%s&weixin_id=%s&current_id=%s&timestamp=%s&token=%s&pagesize=1");

	// 微信服务器媒体文件上传url
	LoadIniString("WebPages", "WeChatMediaUpload", m_initConfig.wechat_media_upload, len, sFile, "http://file.api.weixin.qq.com/cgi-bin/media/upload?access_token=%s&type=%s");

	// 微信服务器媒体文件获取url
	LoadIniString("WebPages", "WeChatMediaUrl", m_initConfig.wechat_media_url, len, sFile, "http://file.api.weixin.qq.com/cgi-bin/media/get?access_token=%s&media_id=%s");

	// 腾讯地图静态图url
	LoadIniString("WebPages", "WeChatStaticMap", m_initConfig.wechat_static_map, len, sFile, "http://apis.map.qq.com/ws/staticmap/v2/?key=JRYBZ-QIAWS-GJ3OB-6GXXF-F3WMZ-RNBGV&size=500x400&center=%s,%s&zoom=12");

	// 腾讯地图地点标注url
	LoadIniString("WebPages", "WeChatMapLocation", m_initConfig.wechat_map_location, len, sFile, "http://apis.map.qq.com/uri/v1/marker?marker=coord:%s,%s;title:%s;addr:%s&referer=myapp");

	// 自己的文件服务器下载的微信服务器的媒体文件的url
	LoadIniString("WebPages", "FileServerMediaDownload", m_initConfig.fileserver_media_download, len, sFile, "http://106.120.108.230:8531/media/get?access_token=%s&media_id=%s");

	// 自己的文件服务器上传url
	LoadIniString("WebPages", "FileServerMediaUpload", m_initConfig.fileserver_media_upload, len, sFile, "http://106.120.108.230:8531/media/upload/%s");

	// 微信文件下载url
	LoadIniString("WebPages", "FileServerMediaTask", m_initConfig.fileserver_media_task, len, sFile, "http://file.api.weixin.qq.com/cgi-bin/media/get?access_token=%s&media_id=%s");

	// 微信文件下载url
	LoadIniString("WebPages", "FileServerMediaFileId", m_initConfig.fileserver_media_fileid, len, sFile, "http://wxm.tq.cn/media/");

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

	//加密包  20090420
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
		//访客对话开始
		g_WriteLog.WriteLog(C_LOG_TRACE, "RecvSrvStatusFrdOnline: RecvInfo.uin > WEBUSER_UIN");
	}
	else
	{
		pUser = GetUserObjectByUid(RecvInfo.uin);
		if (pUser == NULL)
		{
			//这里不考虑很复杂，如果有新的用户，需要自动的刷新以下，以后再做实时地处理
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
		//访客对话结束
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
					g_WriteLog.WriteLog(C_LOG_ERROR,"RecvSrvStatusFrdOffline visit服务器未登录前，收到下线消息");
				}

				pWebUser->m_bConnected = false;
				pWebUser->m_nWaitTimer = -20;
				pWebUser->onlineinfo.talkstatus = TALKSTATUS_NO;
				m_handlerMsgs->RecvOffline(pWebUser);
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
	string content;

	nError = UnPack(&RecvInfo, pRecvBuff, len);

	if (nError != 0)
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "RecvComSendMsg unpack failed,Cmd:%.4x", packhead.cmd);
		goto RETURN;
	}

	g_WriteLog.WriteLog(C_LOG_TRACE, "RecvComSendMsg chatid:%s,Rand:%s,Thirdid:%s,recvuin:%u,senduin:%u,strfontinfo:%s,msgtype:%d,msg:%s",
		RecvInfo.strChatid, RecvInfo.strRand, RecvInfo.strThirdid, RecvInfo.msg.recvuin,
		RecvInfo.msg.senduin, RecvInfo.msg.strfontinfo, RecvInfo.msg.msgtype, RecvInfo.msg.strmsg);

	//先显示内容， 再移动item
	if (packhead.langtype == 4)
	{
		ConvertMsg(RecvInfo.msg.strmsg, sizeof(RecvInfo.msg.strmsg) - 1);

		ConvertMsg(RecvInfo.msg.strmobile, sizeof(RecvInfo.msg.strmobile) - 1);
	}

	content = RecvInfo.msg.strmsg;
	TransferStrToFace(content);
	ReplaceFaceId(content);
	strncpy(RecvInfo.msg.strmsg, content.c_str(), MAX_MSG_RECVLEN);

	if (RecvInfo.msg.senduin > WEBUSER_UIN)
	{
		//访客发来消息
		switch (RecvInfo.msg.msgtype)
		{
		case MSG_WORKBILL:
		case MSG_CLIENT_COME:
			RecvComSendWorkBillMsg(RecvInfo.msg.senduin, RecvInfo.msg.recvuin, RecvInfo.msg.strmsg, RecvInfo.msg.strmobile);
			break;
		case MSG_WEBUI_SEND_CHAT_HISMSG:   //收到webui发过来的会话历史消息，可能多个包,需要客户端发送请求MSG_KEFU_GET_CHAT_HISMSG
			RecvComSendNormalChatidHisMsg(RecvInfo.msg.senduin, RecvInfo.msg.recvuin, RecvInfo);
			break;
		case MSG_TRANSFER_REQUEST://会话转移请求
			RecvComTransRequest(packhead.uin, RecvInfo);
			break;
		case MSG_TRANSFER_ANSWER://会话转移请求
			RecvComTransAnswer(packhead.uin, RecvInfo);
			break;
		case MSG_NORMAL:
		case MSG_WX:
			pWebUser = GetWebUserObjectByUid(RecvInfo.msg.senduin);
			if (pWebUser == NULL)
			{
				//需要去服务器获取该访客信息
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
						g_WriteLog.WriteLog(C_LOG_ERROR, "userinfo信息进入消息记录bug：%s", RecvInfo.msg.strmsg);
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
						m_handlerMsgs->RecvWebUserInfo(pWebUser,0);
					}
					pWxMsg = ParseWxMsg(pWebUser, RecvInfo.msg.strmsg, NULL, RecvInfo.msg.sendtime);

					if (pWxMsg == NULL)
					{
						goto RETURN;
					}
				}
				//网页过来的会话，可能chatid会变化，需要及时更新
				if (strcmp(pWebUser->chatid, RecvInfo.strChatid) != 0 && strlen(RecvInfo.strChatid) > 0)
					strcpy(pWebUser->chatid, RecvInfo.strChatid);
				pWebUser->m_bNewComm = false;

				if (strstr(RecvInfo.msg.strmsg, "[系统消息]") != NULL
					&& strstr(RecvInfo.msg.strmsg, "您网站来访客了") != NULL)
				{
					SendTo_GetWebUserInfo(RecvInfo.msg.senduin, RecvInfo.strChatid);
				}

				if (pWebUser->info.name[0] == '\0')
				{
					strcpy(pWebUser->info.name, RecvInfo.msg.strmobile);
					if (strlen(pWebUser->info.name) < 2)
					{
						g_WriteLog.WriteLog(C_LOG_ERROR, "RecvComSendMsg1 name length：%d", strlen(pWebUser->info.name));
					}
				}
				else if (!pWebUser->info.nameflag && strcmp(RecvInfo.msg.strmobile, pWebUser->info.name) != 0)
				{
					if (RecvInfo.msg.strmobile[0] != '\0')
					{
						strcpy(pWebUser->info.name, RecvInfo.msg.strmobile);
						if (strlen(pWebUser->info.name) < 2)
						{
							g_WriteLog.WriteLog(C_LOG_ERROR, "RecvComSendMsg2 name length：%d", strlen(pWebUser->info.name));
						}
					}
					else
					{
						if (strlen(pWebUser->info.name) < 2)
						{
							g_WriteLog.WriteLog(C_LOG_ERROR, "RecvComSendMsg3 name length：%d", strlen(pWebUser->info.name));
						}
					}
				}

				if (m_sysConfig->IsWebuserSidForbid(pWebUser->info.sid))
				{
					//消息屏蔽了，直接跳出不处理
					goto RETURN;
				}

				if (!(strstr(RecvInfo.msg.strmsg, "[系统消息]") != NULL && strstr(RecvInfo.msg.strmsg, "客人离开") != NULL))
				{
					pWebUser->m_resptimeoutmsgtimer = -1;
					pWebUser->m_resptimeoutclosetimer = -1;
					if (pWebUser->m_waitresptimeouttimer < 0)
						pWebUser->m_waitresptimeouttimer = 0;	// 该客服说话了 [12/29/2010 SC]

					if (RecvInfo.msg.bak != 0)
					{
						msgType = MSG_TYPE_PREV;
					}

					AddMsgToList((IBaseObject*)pWebUser, msgFrom, MSG_RECV_ERROR, GetMsgId(), msgType, MSG_DATA_TYPE_TEXT,
						RecvInfo.msg.strmsg, RecvInfo.msg.sendtime, NULL, pWxMsg);

					if ((RecvInfo.msg.bak == MSG_BAK_NORMAL) && !pWebUser->m_bConnected)
					{
						if (m_vistor)
						{
							m_vistor->SendWebuserTalkBegin(pWebUser);
						}
						else
						{
							g_WriteLog.WriteLog(C_LOG_ERROR,"RecvComSendMsg visit服务器未登录前，收到消息");
						}
						pWebUser->cTalkedSatus = INTALKING;
						pWebUser->talkuid = m_userInfo.UserInfo.uid;
						pWebUser->onlineinfo.talkstatus = TALKSTATUS_TALK;
						pWebUser->transferuid = 0;
						pWebUser->m_nWaitTimer = 0;
						pWebUser->m_bConnected = true;
						//访客来了，肯定不是离线
						if (pWebUser->info.status == STATUS_OFFLINE)
							pWebUser->info.status = STATUS_ONLINE;

						// 提示
					}
					else if (RecvInfo.msg.bak == MSG_BAK_NORMAL)
					{
						pWebUser->cTalkedSatus = INTALKING;

						// 提示 
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
						g_WriteLog.WriteLog(C_LOG_ERROR, "RecvComSendMsg visit服务器未登录前，收到下线消息");
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
		//客服发来消息
		msgFrom = MSG_FROM_CLIENT;
		pUser = GetUserObjectByUid(RecvInfo.msg.senduin);
		if (pUser == NULL)
		{
			pUser = AddUserObject(RecvInfo.msg.senduin, "", RecvInfo.msg.strmobile, STATUS_ONLINE, -1);
			if (pUser == NULL)
			{
				goto RETURN;
			}

			// 下载该坐席的头像
			pUser->DownLoadFace(m_initConfig.webpage_DownloadHeadImage);			
			pUser->m_nWaitTimer = 0;

			string sMsg = RecvInfo.msg.strmsg;
			if ((int)sMsg.find(m_initConfig.fileserver_media_fileid) > -1)
			{
				string msgId = GetMsgId();
				string imagePath = FullPath("SkinRes\\mainframe\\");
				StringReplace(imagePath, "\\", "/");

				char contentMsg[MAX_1024_LEN];
				sprintf(contentMsg, "<img id=\"%s_msg\" class=\"msg_image\" src=\"%srecv_image_fail.jpg\"><img id=\"%s_image\" class=\"wait_image\" src=\"%smsg_wait.gif\">",
					msgId.c_str(), imagePath.c_str(), msgId.c_str(), imagePath.c_str());
				AddMsgToList(pUser, msgFrom, MSG_RECV_ERROR, msgId, MSG_TYPE_NORMAL, MSG_DATA_TYPE_IMAGE, contentMsg, RecvInfo.msg.sendtime, NULL, NULL, false, true, false);
				DownLoadFile(pUser, MSG_DATA_TYPE_IMAGE, sMsg, NULL, RecvInfo.msg.sendtime, msgId);
				goto RETURN;
			}

			AddMsgToList((IBaseObject*)pUser, msgFrom, MSG_RECV_ERROR, GetMsgId(), msgType, MSG_DATA_TYPE_TEXT,
				RecvInfo.msg.strmsg, RecvInfo.msg.sendtime, NULL, NULL);
			if ((RecvInfo.msg.bak == MSG_BAK_NORMAL || RecvInfo.msg.bak == MSG_BAK_AUTOANSER) && !(pUser->m_bInnerTalk))
			{
				pUser->m_bInnerTalk = true;
			}
		}
		else
		{
			strncpy(pUser->UserInfo.nickname, RecvInfo.msg.strmobile, MAX_USERNAME_LEN);//此处会返回“系统”两字
			pUser->m_nWaitTimer = 0;

			string sMsg = RecvInfo.msg.strmsg;
			if ((int)sMsg.find(m_initConfig.fileserver_media_fileid) > -1)
			{
				string msgId = GetMsgId();
				string imagePath = FullPath("SkinRes\\mainframe\\");
				StringReplace(imagePath, "\\", "/");

				char contentMsg[MAX_1024_LEN];
				sprintf(contentMsg, "<img id=\"%s_msg\" class=\"msg_image\" src=\"%srecv_image_fail.jpg\"><img id=\"%s_image\" class=\"wait_image\" src=\"%smsg_wait.gif\">",
					msgId.c_str(), imagePath.c_str(), msgId.c_str(), imagePath.c_str());
				AddMsgToList(pUser, msgFrom, MSG_RECV_ERROR, msgId, MSG_TYPE_NORMAL, MSG_DATA_TYPE_IMAGE, contentMsg, RecvInfo.msg.sendtime, NULL, NULL, false, true, false);
				DownLoadFile(pUser, MSG_DATA_TYPE_IMAGE, sMsg, NULL, RecvInfo.msg.sendtime, msgId);
				goto RETURN;
			}

			AddMsgToList((IBaseObject*)pUser, msgFrom, MSG_RECV_ERROR, GetMsgId(), msgType, MSG_DATA_TYPE_TEXT,
				RecvInfo.msg.strmsg, RecvInfo.msg.sendtime, NULL, NULL);

			if ((RecvInfo.msg.bak == MSG_BAK_NORMAL || RecvInfo.msg.bak == MSG_BAK_AUTOANSER) && !(pUser->m_bInnerTalk))
			{
				pUser->m_bInnerTalk = true;
			}
		}
		// 提示
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
		//处理cookie中没有clientid对话类型
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
			g_WriteLog.WriteLog(C_LOG_ERROR, "RecvFloatCreateChat name length：%d", strlen(pWebUser->info.name));
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
	pWebUser->m_sNewSeq = packhead.sendrandom; //这是消息id
	pWebUser->info.status = STATUS_ONLINE;
	pWebUser->talkuid = 0;//等待应答进来的会话，还没有接入客服
	if (RecvInfo.uKefu == 0)
	{
		if (pWebUser->onlineinfo.talkstatus != TALKSTATUS_REQUEST)
		{
			pWebUser->onlineinfo.talkstatus = TALKSTATUS_REQUEST;
			pWebUser->m_nWaitTimer = 0;

			char msg[MAX_256_LEN];
			sprintf(msg, "用户 %s 申请对话!", pWebUser->info.name);

			AddMsgToList((IBaseObject*)pWebUser, MSG_FROM_SYS, MSG_RECV_ERROR, GetMsgId(), MSG_TYPE_NORMAL, MSG_DATA_TYPE_TEXT,
				msg, 0, NULL, NULL);
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
				//是自己邀请的访客
				pWebUser->m_nWaitTimer = 0;
				pWebUser->m_bConnected = true;
			}
			else
			{
				//其他人邀请的访客
				pWebUser->m_nWaitTimer = -20;
				pWebUser->m_bConnected = false;
			}

			pWebUser->talkuid = RecvInfo.uKefu;

			//读取会话历史消息，并接受新的消息
			//SendStartRecvFloatMsg(packhead.random, RecvInfo.uAdminId, RecvInfo.chatid, pWebUser->m_sNewSeq);

			char msg[MAX_256_LEN];
			sprintf(msg, "%s接受了访客%s的会话", pUser->UserInfo.nickname, pWebUser->info.name);
			
			AddMsgToList((IBaseObject*)pWebUser, MSG_FROM_SYS, MSG_RECV_ERROR, GetMsgId(), MSG_TYPE_NORMAL, MSG_DATA_TYPE_TEXT,
				msg, 0, NULL, NULL);
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

	// 只要收到会话信息，一律去服务器获取work_bill包
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
		//等待应答
		pWebUser->onlineinfo.talkstatus = TALKSTATUS_REQUEST;
		pWebUser->info.status = STATUS_ONLINE;
		pWebUser->m_nWaitTimer = 0;

		char msg[MAX_256_LEN];
		sprintf(msg, "%s请求对话", pWebUser->info.name);
		AddMsgToList(pWebUser, MSG_FROM_SYS, MSG_RECV_ERROR, GetMsgId(), MSG_TYPE_NORMAL, MSG_DATA_TYPE_TEXT,
			msg, 0, NULL, NULL);

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
			pUser = GetUserObjectByUid(m_nNextInviteUid);
			m_nNextInviteWebuserUid = 0;
			m_nNextInviteUid = 0;
			m_handlerMsgs->RecvInviteUser(pWebUser, pUser);

			char msg[MAX_256_LEN];
			GetInviteChatSysMsg(msg, pUser, pWebUser, INVITE_HELP, &m_userInfo);
			AddMsgToList(pWebUser, MSG_FROM_SYS, MSG_RECV_ERROR, GetMsgId(), MSG_TYPE_NORMAL, MSG_DATA_TYPE_TEXT,
				msg, 0, NULL, NULL);
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
						//是自己转移的访客
						pWebUser->m_nWaitTimer = 0;
						pWebUser->m_bConnected = true;
					}
					else
					{
						//其他人转移的访客
						pWebUser->m_bConnected = false;
						pWebUser->m_nWaitTimer = -20;
					}

					//正在对话
					pWebUser->onlineinfo.talkstatus = TALKSTATUS_TALK;
					pWebUser->info.status = STATUS_ONLINE;
					pWebUser->transferuid = 0;
					pWebUser->talkuid = pUser->UserInfo.uid;//邀请用户的要改变talkid为自己的

					char msg[MAX_256_LEN];
					sprintf(msg, "%s接受了访客%s的会话", pUser->UserInfo.nickname, pWebUser->info.name);

					AddMsgToList((IBaseObject*)pWebUser, MSG_FROM_SYS, MSG_RECV_ERROR, GetMsgId(), MSG_TYPE_NORMAL, MSG_DATA_TYPE_TEXT,
						msg, 0, NULL, NULL);

					if (RecvInfo.uKefuUin&&pUser->m_bFriend
						|| !RecvInfo.uKefuUin&&!pWebUser->m_bNotResponseUser)
					{
						//读取会话历史消息，并接受新的消息
						SendStartRecvFloatMsg(packhead.random, RecvInfo.uAdminId, RecvInfo.chatid, pWebUser->m_sNewSeq);
					}

					m_handlerMsgs->RecvChatInfo(pWebUser, pUser);
				}
			}
			else
			{
				//正在转移
				pWebUser->onlineinfo.talkstatus = TALKSTATUS_TRANSFER;
				pWebUser->info.status = STATUS_ONLINE;
				pWebUser->transferuid = RecvInfo.uTansferingToKefu;
				CUserObject *pAcceptUser = GetUserObjectByUid(RecvInfo.uTansferingToKefu);

				//这里必须先在htmleditor中显示，然后再移动位置，因为移动位置可能会导致区域的切换，其他地方同样考虑
				if (strcmp(pWebUser->chatid, RecvInfo.chatid) != 0 && strlen(RecvInfo.chatid) > 0)
					strcpy(pWebUser->chatid, RecvInfo.chatid);
				pWebUser->floatadminuid = RecvInfo.uAdminId;

				char msg[MAX_256_LEN];
				if (pUser != NULL)
					GetInviteChatSysMsg(msg, pUser, pWebUser, INVITE_TRANSFER, pAcceptUser);
				else if (pAcceptUser != NULL)
					sprintf(msg, "访客 %s 转移到 %s", pWebUser->info.name, pAcceptUser->UserInfo.nickname);

				AddMsgToList((IBaseObject*)pWebUser, MSG_FROM_SYS, MSG_RECV_ERROR, GetMsgId(), MSG_TYPE_NORMAL, MSG_DATA_TYPE_TEXT,
					msg, 0, NULL, NULL);

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

	if (pWebUser->m_sWxAppid.empty()) //WxAppid为空，去服务端获取
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
		// 不是微信用户发来的消息，就是协助对象发来的消息
		pAssistUser = GetUserObjectByUid(packhead.uin);
		if (pAssistUser == NULL)
		{
			// 根据uin和昵称先初始化坐席简单信息
			pAssistUser = AddUserObject(packhead.uin, "", RecvInfo.nickname, STATUS_ONLINE, -1);
			pAssistUser->DownLoadFace(m_initConfig.webpage_DownloadHeadImage);

			// 然后更新该坐席的最新信息
			SendTo_GetUserInfo(packhead.uin);
		}
	}

	if (strcmp(RecvInfo.strfontinfo, "JSON=WX") == 0)
	{
		if (!pWebUser->m_bIsFrWX)
		{
			pWebUser->m_bIsFrWX = true;
			m_handlerMsgs->RecvWebUserInfo(pWebUser, 0);
		}
		pWxMsg = ParseWxMsg(pWebUser, RecvInfo.strmsg, pAssistUser,RecvInfo.tMsgTime);

		if (pWxMsg == NULL)
		{
			goto RETURN;
		}
		else
		{
			if (pWxMsg->MsgType == "location")
			{
				RecvInfo.nMsgDataType = MSG_DATA_TYPE_LOCATION;
			}
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

	if (packhead.langtype == LANGUAGE_UTF8)
	{
		ConvertMsg(RecvInfo.strmsg, sizeof(RecvInfo.strmsg) - 1);
		ConvertMsg(RecvInfo.nickname, sizeof(RecvInfo.nickname) - 1);
	}

	//访客发来消息
	switch (RecvInfo.nMsgType)
	{
	case MSG_FLOAT_NORMAL:
	case MSG_FLOAT_PRV:
	case MSG_FLOAT_INPUTING:
	case MSG_WX://微信消息 
	{
		if (pWebUser->m_nWaitTimer<0)
		{
			pWebUser->m_nWaitTimer = 0;
		}

		// 排队等待接受后发消息不能屏蔽 [12/9/2010 SC]
		if (m_sysConfig->IsWebuserSidForbid(pWebUser->info.sid))
		{
			//消息屏蔽了，直接跳出不处理
			goto RETURN;
		}

		if (RecvInfo.nMsgType == MSG_FLOAT_NORMAL)
		{
			if (pAssistUser == NULL)
			{
				// 访客发来消息时，其未应答等待时间清零
				pWebUser->m_resptimeoutmsgtimer = -1;
				pWebUser->m_resptimeoutclosetimer = -1;

				if (pWebUser->m_waitresptimeouttimer < 0)
					pWebUser->m_waitresptimeouttimer = 0;
			}
			else
			{
				msgFrom = MSG_FROM_ASSIST;
			}

			// 界面应做来消息提示
		}
		else
		{
			msgType = MSG_TYPE_PREV;
		}

		if (RecvInfo.tMsgTime == 0)
		{
			// 获取系统当前时间
		}

		string msgId = GetMsgId();
		
		AddMsgToList((IBaseObject*)pWebUser, msgFrom, MSG_RECV_ERROR, msgId, msgType, (MSG_DATA_TYPE)RecvInfo.nMsgDataType,
			RecvInfo.strmsg, RecvInfo.tMsgTime, pAssistUser, pWxMsg);

		// 同步更新关联词
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
	if (m_userInfo.UserInfo.uid != packhead.uin)//接受者不是当前坐席
	{
		pWebUser->m_nWaitTimer = -20;
		pWebUser->m_bConnected = false;
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
			//是自己接受邀请的访客
			pWebUser->m_nWaitTimer = 0;
			pWebUser->m_bConnected = true;
		}

		//其他人邀请的访客
		pWebUser->RemoveMutiUser(pWebUser->talkuid);
		pWebUser->inviteuid = 0;

		char msg[MAX_256_LEN];
		sprintf(msg, "%s接受了访客%s的会话", pUser->UserInfo.nickname, pWebUser->info.name);

		AddMsgToList((IBaseObject*)pWebUser, MSG_FROM_SYS, MSG_RECV_ERROR, GetMsgId(), MSG_TYPE_NORMAL, MSG_DATA_TYPE_TEXT,
			msg, 0, NULL, NULL);

		//读取会话历史消息，并接受新的消息
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
	if (pAcceptUser == NULL)//获取不到，就不是自己的协助对象
	{
		goto RETURN;
	}

	char msg[MAX_256_LEN];
	if (pWebUser == NULL)
	{
		SendTo_GetWebUserChatInfo(packhead.random, RecvInfo.uAdminId, RecvInfo.chatid);
	}
	else if (RecvInfo.uToKefu == m_userInfo.UserInfo.uid)//当前坐席是接受者，移动到转接中。。。
	{
		if (strcmp(pWebUser->chatid, RecvInfo.chatid) != 0 && strlen(RecvInfo.chatid) > 0)
			strcpy(pWebUser->chatid, RecvInfo.chatid);
		pWebUser->floatadminuid = RecvInfo.uAdminId;
		pWebUser->m_bNewComm = true;
		pWebUser->gpid = packhead.random;
		pWebUser->info.status = STATUS_ONLINE;
		pWebUser->onlineinfo.bInvited = false;//转接过来，就不是邀请协助了
		pWebUser->onlineinfo.talkstatus = TALKSTATUS_TRANSFER;
		pWebUser->transferuid = RecvInfo.uToKefu;

		if (pInviteUser != NULL)
			GetInviteChatSysMsg(msg, pInviteUser, pWebUser, INVITE_TRANSFER, pAcceptUser);
		else
			sprintf(msg, "访客 %s 转移到 %s", pWebUser->info.name, pAcceptUser->UserInfo.nickname);

		AddMsgToList((IBaseObject*)pWebUser, MSG_FROM_SYS, MSG_RECV_ERROR, GetMsgId(), MSG_TYPE_NORMAL, MSG_DATA_TYPE_TEXT,
			msg, 0, NULL, NULL);
		
		m_handlerMsgs->RecvTransferUser(pWebUser, pAcceptUser);
	}
	if (packhead.uin == m_userInfo.UserInfo.uid)//当前坐席是发起转接者
	{
		sprintf(msg, "已发起转接请求到%s", pAcceptUser->UserInfo.nickname);

		AddMsgToList((IBaseObject*)pWebUser, MSG_FROM_SYS, MSG_RECV_ERROR, GetMsgId(), MSG_TYPE_NORMAL, MSG_DATA_TYPE_TEXT,
			msg, 0, NULL, NULL);
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


	//这里需要判断是不是要继续显示该用户
	if (pWebUser->m_bNotResponseUser || hItem == NULL&&pInviteUser != NULL || pInviteUser == NULL)
	{
		pWebUser->cTalkedSatus = HASTALKED;
		//m_pFormUser->m_TreeListUser.DeleteItemByLParam((LPARAM)pWebUser);
	}
	else
	{
		char msg[MAX_256_LEN];
		sprintf(msg, "访客 %s 转移失败", pWebUser->info.name);
		pWebUser->cTalkedSatus = INTALKING;
		pWebUser->transferuid = 0;

		AddMsgToList((IBaseObject*)pWebUser, MSG_FROM_SYS, MSG_RECV_ERROR, GetMsgId(), MSG_TYPE_NORMAL, MSG_DATA_TYPE_TEXT,
			msg, 0, NULL, NULL);

		// 转接超时会话回到发起转移坐席，不是等待应答
		if (m_userInfo.UserInfo.uid != pInviteUser->UserInfo.uid) 
		{
			sprintf(msg, "用户 %s 申请对话!", pWebUser->info.name);
			// 提示
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
		// 被别的客服邀请
		pWebUser->onlineinfo.talkstatus = TALKSTATUS_INVITE;
		pWebUser->inviteuid = m_userInfo.UserInfo.uid;
		if (!pWebUser->m_bNewComm)
		{
			pWebUser->m_bNewComm = true;
		}
		pWebUser->m_nWaitTimer = 0;

		//这里必须先在htmleditor中显示，然后再移动位置，因为移动位置可能会导致区域的切换，其他地方同样考虑
		GetInviteChatSysMsg(msg, pInviteUser, pWebUser, INVITE_HELP, pAcceptUser);
		AddMsgToList(pWebUser, MSG_FROM_SYS, MSG_RECV_WEB, GetMsgId(), MSG_TYPE_NORMAL,
			MSG_DATA_TYPE_TEXT, msg, 0, NULL, NULL);
		m_handlerMsgs->RecvInviteUser(pWebUser, pInviteUser);
	}
	if (packhead.uin == m_userInfo.UserInfo.uid)
	{
		// 自己邀请的别的客服
		GetInviteChatSysMsg(msg, pInviteUser, pWebUser, INVITE_HELP, pAcceptUser);
		AddMsgToList(pWebUser, MSG_FROM_SYS, MSG_RECV_WEB, GetMsgId(), MSG_TYPE_NORMAL,
			MSG_DATA_TYPE_TEXT, msg, 0, NULL, NULL);
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
	if (!RecvInfo.sResult)//拒绝
	{
		if (RecvInfo.uInviteFrom == m_userInfo.UserInfo.uid)
		{
			//发送方收到对方应答
			pWebUser->inviteuid = m_userInfo.UserInfo.uid;
			pWebUser->m_nWaitTimer = 0;
			GetInviteChatSysMsg(msg, pInviteUser, pWebUser, INVITE_HELP_REFUSE, pAcceptUser);
			AddMsgToList(pWebUser, MSG_FROM_SYS, MSG_RECV_WEB, GetMsgId(), MSG_TYPE_NORMAL,
				MSG_DATA_TYPE_TEXT, msg, 0, NULL, NULL);
		}

		if (packhead.uin == m_userInfo.UserInfo.uid)
		{
			//我自己发出的拒绝包
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
	else//接受
	{
		pWebUser->onlineinfo.talkstatus = TALKSTATUS_TALK;

		GetInviteChatSysMsg(msg, pInviteUser, pWebUser, INVITE_HELP_ACCEPT, pAcceptUser);
		AddMsgToList(pWebUser, MSG_FROM_SYS, MSG_RECV_WEB, GetMsgId(), MSG_TYPE_NORMAL,
			MSG_DATA_TYPE_TEXT, msg, 0, NULL, NULL);
		if (!pWebUser->IsMutiUser())
		{
			pWebUser->AddCommonTalkId(RecvInfo.uInviteFrom);
		}

		pWebUser->AddCommonTalkId(packhead.uin);
		m_handlerMsgs->ResultInviteUser(pWebUser, pAcceptUser, INVITE_ACCEPT);
	}

	//这里要考虑如何显示
	if (packhead.uin == m_userInfo.UserInfo.uid)
	{
		//是自己的消息
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
	case REASON_CLIENT_RELEASE:    //客服主动释放了客户
		sprintf(msg, "客服 %s(%u) 主动释放了会话", RecvInfo.szKefuName, RecvInfo.uKefu);
		break;
	default:
		sprintf(msg, "客服 %s(%u) 释放了会话", RecvInfo.szKefuName, RecvInfo.uKefu);
		break;

	}

	//这里需要判断是不是要继续显示该用户
	if (pWebUser->m_bNotResponseUser)
	{
		pWebUser->cTalkedSatus = HASTALKED;
	}
	else
	{
		pWebUser->cTalkedSatus = INTALKING;
		sprintf(msg, "客服 %s(%lu) 对访客 %s 的接待终止", !pUser ? RecvInfo.szKefuName : pUser->UserInfo.nickname, RecvInfo.uKefu, pWebUser->info.name);
	}
	AddMsgToList((IBaseObject*)pWebUser, MSG_FROM_SYS, MSG_RECV_ERROR, GetMsgId(), MSG_TYPE_NORMAL, MSG_DATA_TYPE_TEXT,
		msg, 0, NULL, NULL);
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
		g_WriteLog.WriteLog(C_LOG_TRACE, "RecvFloatCloseChat 被邀请坐席离线(%u)", packhead.uin);

		//被邀请的人退出邀请
		pWebUser->RemoveMutiUser(packhead.uin);

		//添加提示
		char msg[MAX_256_LEN];
		if (pUser != NULL)
		{
			sprintf(msg, "%s(%u)退出会话", pUser->UserInfo.nickname, packhead.uin);
		}
		else
		{
			sprintf(msg, "用户(%u)退出会话", packhead.uin);
		}

		AddMsgToList((IBaseObject*)pWebUser, MSG_FROM_SYS, MSG_RECV_ERROR, GetMsgId(), MSG_TYPE_NORMAL, MSG_DATA_TYPE_TEXT,
			msg, 0, NULL, NULL);
		pWebUser->RemoveMutiUser(packhead.uin);
	}
	else
	{
		//结束会话，清空协助坐席
		if (pWebUser->IsMutiUser())
			pWebUser->RemoveAllMutiUser();
		pWebUser->onlineinfo.bInvited = false;
		pWebUser->m_bNewComm = false;
		pWebUser->m_bConnected = false;
		pWebUser->onlineinfo.talkstatus = TALKSTATUS_NO;
		pWebUser->cTalkedSatus = HASTALKED;
		string strMsg = "";
		if (CHATCLOSE_UNSUBSCRIBE == RecvInfo.usType)
		{
			strMsg = "微信用户取消关注结束会话";
		}
		else
		{
			strMsg = "访客会话已结束";
		}

		AddMsgToList((IBaseObject*)pWebUser, MSG_FROM_SYS, MSG_RECV_ERROR, GetMsgId(), MSG_TYPE_NORMAL, MSG_DATA_TYPE_TEXT,
			strMsg.c_str(), 0, NULL, NULL);

		if (!pWebUser->IsOnline())//这里是不是该用户彻底离线了
		{
			g_WriteLog.WriteLog(C_LOG_TRACE, "RecvFloatCloseChat 坐席离线(%u)访客离线", packhead.uin);

			m_vistor->SetVisitorOffline(pWebUser);
		}
		else
		{
			g_WriteLog.WriteLog(C_LOG_TRACE, "RecvFloatCloseChat 坐席离线(%u)访客访问中", packhead.uin);
			//访客退回到在线列表
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
		SetOfflineStatus();
		if (RecvInfo.type >= SRVNORMAL_IPERROR)
		{
			m_lastError = "该用户已在其他地方进行登陆";
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
	//转移失败
	pWebUser = GetWebUserObjectByUid(RecvInfo.clientinfo.id);
	if (RecvInfo.result != 0)
	{
		sprintf(msg, "转接失败%s", RecvInfo.reason);
		
		AddMsgToList((IBaseObject*)pWebUser, MSG_FROM_SYS, MSG_RECV_ERROR, GetMsgId(), MSG_TYPE_NORMAL, MSG_DATA_TYPE_TEXT,
			msg, 0, NULL, NULL);

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

	//转移成功的会话，要删除掉
	if (pWebUser == NULL)
	{
		
	}
	else if (RecvInfo.sendinfo.id == m_userInfo.UserInfo.uid)
	{
		//当前坐席成功转移出去的会话
		pWebUser->cTalkedSatus = HASTALKED;//通过话
		pWebUser->onlineinfo.talkstatus = TALKSTATUS_TALK;
		pWebUser->m_nWaitTimer = 0;
		pWebUser->m_bConnected = false;
		pWebUser->transferuid = 0;
		pWebUser->talkuid = RecvInfo.recvinfo.id;
		if (pInviteUser != NULL)
			GetInviteChatSysMsg(msg, pInviteUser, pWebUser, INVITE_TRANSFER, pAcceptUser);
		else
			sprintf(msg, "访客 %s 转移到 %s", pWebUser->info.name, pAcceptUser->UserInfo.nickname);
		
		AddMsgToList((IBaseObject*)pWebUser, MSG_FROM_SYS, MSG_RECV_ERROR, GetMsgId(), MSG_TYPE_NORMAL, MSG_DATA_TYPE_TEXT,
			msg, 0, NULL, NULL);
	}

	nError = 0;
RETURN:

	return nError;
}

int CChatManager::RecvTransferClient(PACK_HEADER packhead, char *pRecvBuff, int len)
{
	int nError = 0;

	//待处理
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

	if (pWebUser == NULL)//获取WorkBill
	{
		g_WriteLog.WriteLog(C_LOG_TRACE, "RecvTransferClient pWebUser==NULL");
		SendTo_GetWebUserInfo(RecvInfo.clientinfo.id, RecvInfo.szChatId);
	}
	else
	{
		//把用户移到自己对话中

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

			//转移的会话肯定是在线的
			if (pWebUser->info.status == STATUS_OFFLINE)
				pWebUser->info.status = STATUS_ONLINE;

			//访客邀请
			sprintf(msg, "%s接受了访客%s的会话", m_userInfo.UserInfo.nickname, pWebUser->info.name);
			
			AddMsgToList((IBaseObject*)pWebUser, MSG_FROM_SYS, MSG_RECV_ERROR, GetMsgId(), MSG_TYPE_NORMAL, MSG_DATA_TYPE_TEXT,
				msg, 0, NULL, NULL);
		}
		else
		{
			g_WriteLog.WriteLog(C_LOG_ERROR, "RecvTransferClient recvuid error=接收转移会话坐席不是当前登录坐席,recvuid:%u, localuid:%u",
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
				//如果自己的信息没有获取到，那么需要重复的去获取
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
			chat_manager->m_lastError = "登录超时";
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
	++m_packSeq;
	if (m_packSeq == 0)
	{
		++m_packSeq;
	}
	return m_packSeq;
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

	//30秒发送一次刺激包
	if (m_nSendPing > 30)
	{
		if (m_nOnLineStatus != STATUS_OFFLINE)
		{
			// 发送ping命令包，跟服务端进程能约定的ping包
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
				// 累计3次连接不上，做掉线处理
				g_WriteLog.WriteLog(C_LOG_TRACE, "SendPingFailed3times--SetOfflineStatus ");
				SetOfflineStatus();

				CloseAllSocket();
			}
		}

		if (m_nOnLineStatusEx != STATUS_OFFLINE)
		{
			// 发送网络层级的ping包，确认服务器是否能连接
			if (m_vistor && m_vistor->SendPingToVisitorServer() != 0)
			{
				// 连接不上处理
				m_nOnLineStatusEx = STATUS_OFFLINE;
				m_nLoginToVisitor = 0;
			}
		}

		// 时间清零
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
	m_handlerMsgs->RecvOffline(&m_userInfo);
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
		g_WriteLog.WriteLog(C_LOG_ERROR, "AddWebUserObject name length：%d", strlen(pWebUser->info.name));
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
	g_WriteLog.WriteLog(C_LOG_TRACE, "SendTo_GetWebUserInfo recvuin:%u,senduin:%u，szMsg=%s", SendInfo.msg.recvuin, SendInfo.msg.senduin, szMsg);

	nError = SendPackTo(&SendInfo);

	return nError;
}

WxMsgBase* CChatManager::ParseWxMsg(CWebUserObject* pWebUser, char* msg, CUserObject* pAssistUser, unsigned long time)
{
	//微信消息类型，utf8编码类型 
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
			// 此处注意，如果第二次收到userinfo，应当将前面收到那次信息析构掉
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
				m_handlerMsgs->RecvWebUserInfo(pWebUser,1);
			}
			
			pWebUser->m_bIsGetInfo = true;
			return NULL;
		}
		else if ("wxactoken" == pwxobj->MsgType)
		{
			//todo:收到服务器发过来的微信的access_token消息
			AddToken(pWebUser->m_pWxUserInfo, ((WxAccessTokenInfo*)pwxobj)->szAcToken);
			delete pwxobj;
			return NULL;
		}
		else if ("image" == pwxobj->MsgType)
		{
			string msgId = GetMsgId();
			string imagePath = FullPath("SkinRes\\mainframe\\");
			StringReplace(imagePath, "\\", "/");

			char contentMsg[MAX_1024_LEN];
			sprintf(contentMsg, "<img id=\"%s_msg\" class=\"msg_image\" src=\"%srecv_image_fail.jpg\"><img id=\"%s_image\" class=\"wait_image\" src=\"%smsg_wait.gif\">",
				msgId.c_str(),imagePath.c_str(), msgId.c_str(), imagePath.c_str());

			AddMsgToList((IBaseObject*)pWebUser, msgFormType, MSG_RECV_ERROR, msgId, MSG_TYPE_NORMAL, MSG_DATA_TYPE_IMAGE, contentMsg, time, pAssistUser, NULL, false, true, false);
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

			AddMsgToList((IBaseObject*)pWebUser, msgFormType, MSG_RECV_ERROR, msgId, MSG_TYPE_NORMAL, MSG_DATA_TYPE_VOICE, contentMsg, time, pAssistUser, NULL, false, true, false);

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

			AddMsgToList((IBaseObject*)pWebUser, msgFormType, MSG_RECV_ERROR, msgId, MSG_TYPE_NORMAL, MSG_DATA_TYPE_VIDEO, contentMsg, time, pAssistUser, NULL, false, true, false);

			DownLoadFile(pWebUser, MSG_DATA_TYPE_VIDEO, ((WxMsgVideo*)pwxobj)->MediaUrl, pAssistUser, time, msgId);
			delete pwxobj;
			return NULL;
		}
		else if ("location" == pwxobj->MsgType)
		{
			msgBase = (WxMsgBase*)pwxobj;
			strcpy(msg, ((WxMsgLocation*)pwxobj)->Label.c_str());
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
			msg, 0, NULL, NULL);

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
			"自动回复消息失败！", 0, NULL, NULL);
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
			//打开webui时如果get行没有参数rand，webui将rand设为clientuin
			if (strlen(sid) > 0 || ((unsigned long)atol(sid)) != senduid)
			{
				ChangeWebUserSid(pWebUser, sid, thirdid);
			}
			//来了访客，肯定是在线的
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
				g_WriteLog.WriteLog(C_LOG_ERROR,"RecvComSendWorkBillMsg 添加了空名字访客");
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

			// 微信用户，主动获取微信的userinfo
			if (!pWebUser->m_sWxAppid.empty())
			{
				if (!pWebUser->m_bIsFrWX)
				{
					pWebUser->m_bIsFrWX = true;
					m_handlerMsgs->RecvWebUserInfo(pWebUser,0);
				}
				SendGetWxUserInfoAndToken(pWebUser);
			}

			time_t tnow = time(NULL);

			if (!pWebUser->GetNormalChatHisMsgSuccess
				&&tnow - pWebUser->tGetNormalChatHismsgTime > 3) //判断超时这个最好是有定时器判断下，防止收到不服务器回应吧
			{
				SendGetChatHisMsg(senduid, billid);// 如果是非等待应答的才需要去获取消息记录
				pWebUser->tGetNormalChatHismsgTime = tnow;
			}
		}

		nPos = strReturnParameters.find("transfer");
		if (nPos >= 0)//转接中
		{
			g_WriteLog.WriteLog(C_LOG_ERROR, "RecvComSendWorkBillMsg strReturnParameters: %s", strReturnParameters.c_str());
			//正在转移
			string::iterator new_end1 = remove_if(strReturnParameters.begin(), strReturnParameters.end(), bind2nd(equal_to<char>(), '\\'));
			strReturnParameters.erase(new_end1, strReturnParameters.end());
			string::iterator new_end2 = remove_if(strReturnParameters.begin(), strReturnParameters.end(), bind2nd(equal_to<char>(), '\n'));
			strReturnParameters.erase(new_end2, strReturnParameters.end());
			Json::Value jv;
			if (!ParseJson(strReturnParameters, jv)) return;

			//判断返回成功失败结果
			unsigned long uTransuid = GetIntFromJson(jv, "transfer");
			if (pWebUser->transferuid != m_userInfo.UserInfo.uid)
			{
				pWebUser->onlineinfo.talkstatus = TALKSTATUS_TRANSFER;
				pWebUser->info.status = STATUS_ONLINE;
				pWebUser->transferuid = m_userInfo.UserInfo.uid;
				pWebUser->talkuid = uTransuid;
				char msg[MAX_256_LEN];
				sprintf(msg, "访客 %s 转移到 %s", pWebUser->info.name, m_userInfo.UserInfo.nickname);
				AddMsgToList(pWebUser, MSG_FROM_SYS, MSG_RECV_WEB, GetMsgId(), MSG_TYPE_NORMAL,
					MSG_DATA_TYPE_TEXT, msg, 0, NULL, NULL);
				m_handlerMsgs->RecvTransferUser(pWebUser, &m_userInfo);
				//m_tranferList[pWebUser->webuserid] = 0;
				//KillTimer(TIMER_TRANS_TIMEOUT);
				//SetTimer(TIMER_TRANS_TIMEOUT, 1000, NULL);
			}
		}
		else if ((pWebUser->cTalkedSatus != INTALKING || !pWebUser->m_bConnected)
			&& m_userInfo.UserInfo.uid == recvuid && !pWebUser->m_bNewComm)//非等待应答的会话
		{
			if (m_vistor)
			{
				m_vistor->SendWebuserTalkBegin(pWebUser);
			}
			else
			{
				g_WriteLog.WriteLog(C_LOG_ERROR, "RecvComSendWorkBillMsg visit服务器未登录前，收到新会话");
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
			//	AddMultiWebUserToList(pWebUser);//把接入的会话移动到会话列表中
			
			// 提示
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
					g_WriteLog.WriteLog(C_LOG_ERROR, "SolveWebUserEarlyMsg name length：%d", strlen(pWebUser->info.name));
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
	
	//显示历史消息，并设置已获取历史消息状态
	
	CWebUserObject *pWebUser = GetWebUserObjectBySid(RecvInfo.strChatid);
	if (pWebUser != NULL)
	{
		pWebUser->GetNormalChatHisMsgSuccess = true; // 二次会话需要重置变量
	}
}

int CChatManager::RecvComTransRequest(unsigned long senduid, COM_SEND_MSG& RecvInfo)
{
	int nError = 0;
	CUserObject *pAcceptUser = NULL;
	CWebUserObject *pWebUser = NULL;

	g_WriteLog.WriteLog(C_LOG_TRACE, "RecvComTransRequest ：%s", RecvInfo.msg.strmsg);

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
	else if (RecvInfo.msg.recvuin == m_userInfo.UserInfo.uid && pWebUser->transferuid != m_userInfo.UserInfo.uid)//当前坐席是接受者，移动到转接中。。。
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
		
		//获取历史消息

		//设置超时定时器
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
	CUserObject *pAcceptUser = NULL;//主转接坐席
	CWebUserObject *pWebUser = NULL;
	string strMsg = "";
	g_WriteLog.WriteLog(C_LOG_TRACE, "RecvComTransAnswer ：%s,recvuin:%u", RecvInfo.msg.strmsg, RecvInfo.msg.recvuin);

	pAcceptUser = GetUserObjectByUid(senduid);
	pWebUser = GetWebUserObjectByUid(RecvInfo.msg.senduin);
	if (senduid == 0)
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "RecvComTransAnswer GetUserObject获取被转接人失败：%u", senduid);
		nError = -1;
		goto RETURN;
	}
	if (pWebUser == NULL)
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "RecvComTransAnswer GetWebUserObject获取用户失败：chatid%s", RecvInfo.strChatid);
		nError = -2;
		goto RETURN;
	}

	strMsg = RecvInfo.msg.strmsg;
	if ((int)strMsg.find("OK") >= 0)//同意转接
	{
		//坐席同意后发送CLT_TRANSFERCLIENT包
		strMsg = pWebUser->info.name;
		strMsg = "对方同意您对访客[%s]的转接" + strMsg;
		SendToTransferUser(pAcceptUser, pWebUser, senduid);
	}
	else if ((int)strMsg.find("NO") >= 0)//拒绝转接
	{
		strMsg = pWebUser->info.name;
		strMsg = "对方拒绝您对访客[%s]的转接" + strMsg;
	}
	else if ((int)strMsg.find("TIMEOUT") >= 0)//超时
	{
		strMsg = pWebUser->info.name;
		strMsg = "您对访客[%s]的转接超时" + strMsg;
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
		sprintf(msg, "%s发起了一个快速邀请", pInviteUser->UserInfo.nickname);
		break;
	case APPLY_OPEN:
		if (pInviteUser == NULL)
			return;
		sprintf(msg, "%s请求直接对话", pInviteUser->UserInfo.nickname);
		break;
	case MYGETNOTE:
		if (pInviteUser == NULL)
			return;
		sprintf(msg, "%s请求留言", pInviteUser->UserInfo.nickname);
		break;
	case INVITE_TRANSFER:
		if (pAcceptUser == NULL)
			return;

		if (pInviteUser == NULL)
			sprintf(msg, "%s转接到用户 %s", pWebUser->info.name, pAcceptUser->UserInfo.nickname);
		else
			sprintf(msg, "%s转接用户 %s 到 %s", pInviteUser->UserInfo.nickname, pWebUser->info.name, pAcceptUser->UserInfo.nickname);
		break;
	case INVITE_HELP:
		if (pAcceptUser == NULL)
			return;

		if (pInviteUser == NULL)
			sprintf(msg, "%s被邀请接待 %s", pAcceptUser->UserInfo.nickname, pWebUser->info.name);
		else
			sprintf(msg, "%s邀请用户 %s 接待 %s", pInviteUser->UserInfo.nickname, pAcceptUser->UserInfo.nickname, pWebUser->info.name);
		break;
	case INVITE_HELP_REFUSE:
		if (pAcceptUser == NULL)
			return;

		if (pInviteUser == NULL)
			sprintf(msg, "%s拒绝邀请接待 %s", pAcceptUser->UserInfo.nickname, pWebUser->info.name);
		else
			sprintf(msg, "%s拒绝用户 %s 的邀请 接待 %s", pAcceptUser->UserInfo.nickname, pInviteUser->UserInfo.nickname, pWebUser->info.name);
		break;
	case INVITE_HELP_ACCEPT:
		if (pAcceptUser == NULL)
			return;

		if (pInviteUser == NULL)
			sprintf(msg, "%s同意邀请接待 %s", pAcceptUser->UserInfo.nickname, pWebUser->info.name);
		else
			sprintf(msg, "%s同意用户 %s 的邀请 接待 %s", pAcceptUser->UserInfo.nickname, pInviteUser->UserInfo.nickname, pWebUser->info.name);
		break;
	default:
		sprintf(msg, "邀请协助出现异常");
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

	// 收到登录用户的信息包，登录成功
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

	// 保存默认设置文件
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
		//继续连接
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
						//3	客人答复超时--客人没有发送消息的时间
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
						//2客人等待答复超时--客人发送消息后等待超时发送的消息
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
						//4	客人答复超时关闭对话--客人没有发送消息的时间超过后终止对话
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

	char msg[MAX_256_LEN];
	GetStopChatSysMsg(msg, pWebUser, ntype, &m_userInfo);
	if (pWebUser->onlineinfo.bInvited != CHATCLOSE_INVISTEXIT)
	{

		pWebUser->talkuid = 0;
		pWebUser->RemoveAllMutiUser();

		//这里要加停止对话的标示
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
					msg, 0, NULL, NULL);
			}
			if (m_vistor)
			{
				m_vistor->SendWebuserTalkEnd(pWebUser);
			}
		}
	}
	else
	{
		//我是被邀请进入的
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
		clientName = "客服 ";
		clientName += pSendUser->UserInfo.nickname;
	}
	else
	{
		clientName = "客服";
	}

	switch (type)
	{
	case CHATCLOSE_TIMEOUT:
		sprintf(msg, "因访客 %s 回复消息超时，对话结束", pWebUser->info.name);
		break;
	case CHATCLOSE_USER:
		sprintf(msg, "%s 终止对 %s 的接待", clientName.c_str(), pWebUser->info.name);
		break;
	case CHATCLOSE_INVISTEXIT:
		sprintf(msg, "被邀请 %s 退出访客 %s 的会话", clientName.c_str(), pWebUser->info.name);
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
					msg, 0, NULL, NULL);
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
					msg, 0, NULL, NULL);
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
	SendInfo.msg.senduin = m_userInfo.UserInfo.uid;//客人号码
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
	IBaseObject* pObj = NULL;
	CUserObject* pAssistUser = NULL;
	if (msgFromUserType == MSG_FROM_WEBUSER)
	{
		pObj = GetWebUserObjectByUid(msgFromUserId);
	}
	else if (msgFromUserType == MSG_FROM_ASSIST)
	{
		pObj = GetWebUserObjectByUid(msgFromUserId);
		pAssistUser = GetUserObjectByUid(assistUserId);
	}
	else if (msgFromUserType == MSG_FROM_CLIENT)
	{
		pObj = GetUserObjectByUid(msgFromUserId);
	}
	DownLoadFile(pObj, nMsgDataType, url, pAssistUser, time, msgId);
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
				"该访客已被您屏蔽，请先解除屏蔽后再接受对话请求!", 0, NULL, NULL);
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
				"该访客已被您屏蔽，请先解除屏蔽后再邀请!", 0, NULL, NULL);
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

	g_WriteLog.WriteLog(C_LOG_TRACE, "发送邀请协助请求[chatid:%s, InviteUser:%u,uAdminId:%u,uWebuin:%u]",
		SendInfo.chatid, SendInfo.uInviteUser, SendInfo.uAdminId, SendInfo.uWebuin);

	nError = SendPackTo(&SendInfo);

	return nError;
}

int CChatManager::SendTo_TransferRequestUser(CWebUserObject* pWebUser, CUserObject* pAcceptUser)
{
	int nError = 0;
	if (pWebUser->m_bNewComm)
	{
		//等待应答会话转移
		COM_FLOAT_TRANSREQUEST SendInfo(VERSION, pWebUser->gpid);
		SendInfo.uAdminId = pWebUser->floatadminuid;
		strcpy(SendInfo.chatid, pWebUser->chatid);
		SendInfo.uWebuin = pWebUser->webuserid;
		SendInfo.nTimeOutSecond = 0;
		SendInfo.uToAdminId = 0;
		SendInfo.uToKefu = pAcceptUser->UserInfo.uid;

		g_WriteLog.WriteLog(C_LOG_TRACE, "发送等待应答访客转接请求[chatid:%s,acceptuid:%u,acceptname:%s,sendname:%s,senduid:%u]",
			SendInfo.chatid, SendInfo.uToKefu, pAcceptUser->UserInfo.nickname, m_userInfo.UserInfo.nickname, m_userInfo.UserInfo.uid);

		nError = SendPackTo(&SendInfo);
	}
	else
	{
		//一对一会话转移
		//转接要改成先发送邀请，而不是直接转过去
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
		SendInfo.msg.senduin = pWebUser->webuserid;//客服号码
		strncpy(SendInfo.strChatid, pWebUser->chatid, MAX_CHATID_LEN);
		strncpy(SendInfo.strRand, pWebUser->info.sid, MAX_WEBCLIENID_LEN);
		strncpy(SendInfo.strThirdid, pWebUser->info.thirdid, MAX_THIRDID_LEN);
		g_WriteLog.WriteLog(C_LOG_TRACE, "发送转接请求: recvuin:%u,senduin:%u", SendInfo.msg.recvuin, SendInfo.msg.senduin);

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

	g_WriteLog.WriteLog(C_LOG_TRACE, "发送邀请协助接受包[chatid:%s, sResult:%d,uInviteFrom:%u,uAdminId:%u,uWebuin:%u]",
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
	SendInfo.msg.senduin = m_userInfo.UserInfo.uid;//客人号码

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
	IBaseObject* pObj = NULL;
	if (userType == MSG_RECV_WX || userType == MSG_RECV_WEB)
	{
		pObj = pWebUser = GetWebUserObjectByUid(userId);
		if (pWebUser == NULL)
		{
			g_WriteLog.WriteLog(C_LOG_ERROR, "AfterUpload 为获取到访客信息");
			return;
		}
	}
	else if (userType == MSG_RECV_CLIENT)
	{
		pObj = pUser = GetUserObjectByUid(userId);
		if (pUser == NULL)
		{
			g_WriteLog.WriteLog(C_LOG_ERROR, "AfterUpload 为获取到坐席信息");
			return;
		}
	}

	char contentMsg[MAX_1024_LEN];
	if (mediaID.empty())
	{
		// 上传失败处理,通知界面，文件发送失败
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
			sprintf(contentMsg, "<img id = \"%s_image\" onclick=ReSendFile(\"%s\",\"%d\",\"%s\",\"%d\",\"%lu\",\"%s\") class=\"wait_image\" src=\"%smsg_fail.png\"><span class=\"file_text\">文件发送失败</span>",
				msgId.c_str(), filePath.c_str(), userType, msgId.c_str(), MSG_DATA_TYPE_FILE, userId, imagePath.c_str(), imagePath.c_str());
			filePath = contentMsg;
		}

		AddMsgToList(pObj, MSG_FROM_SELF, userType, msgId, MSG_TYPE_NORMAL,
			msgDataType, filePath, 0, NULL, NULL, true, false ,false);

		g_WriteLog.WriteLog(C_LOG_TRACE, "媒体文件消息发送失败");
	}
	else
	{
		// 上传成功处理	
		Json::Value json;
		string msgSendTo;
		char fileServerMediaUrl[MAX_512_LEN];
		if (msgDataType == MSG_DATA_TYPE_IMAGE)
		{
			if (userType == MSG_RECV_WX)
			{
				if (!fileId.empty())
				{
					// 拼出该url串
					msgSendTo = m_initConfig.fileserver_media_fileid + fileId;

					// 生成发送给微信的消息
					WxMsgImage* imageObj = new WxMsgImage("image");
					imageObj->MediaId = mediaID;
					imageObj->PicUrl = msgSendTo;
					imageObj->MediaUrl = msgSendTo;
					imageObj->ToUserName = pWebUser->info.sid;
					imageObj->FromUserName = pWebUser->info.thirdid;
					imageObj->ToSendJson(json);
					msgSendTo = json.toStyledString();
					delete imageObj;
					if (SendMsg(pWebUser, msgSendTo.c_str(), 0, "JSON=WX") == SYS_SUCCESS)
					{
						m_handlerMsgs->ResultSendMsg(msgId);

						AddMsgToList((IBaseObject*)pWebUser, MSG_FROM_SELF, MSG_RECV_ERROR, msgId, MSG_TYPE_NORMAL,
							msgDataType, filePath, 0, NULL, NULL, true, false);
					}
				}
				else
				{
					// 生成发送给微信的消息
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
							msgDataType, filePath, 0, NULL, NULL, true, false);
					}
				}

			}
			else if (userType == MSG_RECV_WEB)
			{
				// 获取文件服务器文件路径format串
				// 图片在文件服务器的url
				msgSendTo = m_initConfig.fileserver_media_fileid + mediaID;
				// 生成发送给web的消息
				char sSendTo[MAX_1024_LEN];
				sprintf(sSendTo, "<span style=\"font-size:10pt; color:#000000; font-family:微软雅黑\">\
						收到一个图片（点击查看原图）<br></span><a href = \"%s\" target=\"blank\"><img \
						src=\"%s\" width=30%%></a>", msgSendTo.c_str(), msgSendTo.c_str());
				if (SendMsg(pWebUser, sSendTo, 0) == SYS_SUCCESS)
				{
					m_handlerMsgs->ResultSendMsg(msgId);

					AddMsgToList((IBaseObject*)pWebUser, MSG_FROM_SELF, MSG_RECV_ERROR, msgId, MSG_TYPE_NORMAL,
						msgDataType, filePath, 0, NULL, NULL, true, false);
				}
			}
			else if (userType == MSG_RECV_CLIENT)
			{
				// 拼出该url串
				msgSendTo = m_initConfig.fileserver_media_fileid + mediaID;
				if (SendMsg(pUser, msgSendTo.c_str(), 0) == SYS_SUCCESS)
				{
					m_handlerMsgs->ResultSendMsg(msgId);

					AddMsgToList((IBaseObject*)pUser, MSG_FROM_SELF, MSG_RECV_ERROR, msgId, MSG_TYPE_NORMAL,
						msgDataType, filePath, 0, NULL, NULL, true, false);
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
					// 拼出该url串
					msgSendTo = m_initConfig.fileserver_media_fileid + fileId;

					WxMsgVoice* voiceObj = new WxMsgVoice("voice");
					voiceObj->MediaId = mediaID;
					voiceObj->MediaUrl = msgSendTo;
					voiceObj->ToUserName = pWebUser->info.sid;
					voiceObj->FromUserName = pWebUser->info.thirdid;
					voiceObj->ToSendJson(json);
					delete voiceObj;
					msgSendTo = json.toStyledString();

					if (SendMsg(pWebUser, msgSendTo.c_str(), 0, "JSON=WX") == SYS_SUCCESS)
					{
						m_handlerMsgs->ResultSendMsg(msgId);

						AddMsgToList((IBaseObject*)pWebUser, MSG_FROM_SELF, MSG_RECV_ERROR, msgId, MSG_TYPE_NORMAL,
							msgDataType, filePath + ".wav", 0, NULL, NULL, true, false);
					}
				}
				else
				{
					// 拼出该url串
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
							msgDataType, filePath + ".wav", 0, NULL, NULL, true, false);
					}
				}
			}
		}
		else if (msgDataType == MSG_DATA_TYPE_FILE)
		{
			int pos = mediaID.find_last_of("/");
			string fileName = mediaID.substr(pos + 1, mediaID.length() - pos - 1);
			if (userType == MSG_RECV_WX)
			{
				// 上传成功
				char sSendTo[MAX_1024_LEN];
				sprintf(sSendTo, "收到文件 <a href=\"%s\" >%s</a> (建议使用电脑进行打开，链接地址有效时间30分钟)", mediaID.c_str(), mediaID.c_str());
				if (SendMsg(pWebUser, sSendTo, 0) == SYS_SUCCESS)
				{
					m_handlerMsgs->ResultSendMsg(msgId,true,pWebUser->webuserid,userType,msgDataType,mediaID);
					sprintf(sSendTo, "<span class=\"file_text\">发送文件 </span><a class=\"file_link\" href=\"%s\" target=\"_blank\">%s</a>",
						mediaID.c_str(), fileName.c_str());
					AddMsgToList((IBaseObject*)pWebUser, MSG_FROM_SELF, MSG_RECV_ERROR, msgId, MSG_TYPE_NORMAL,
						msgDataType, sSendTo, 0, NULL, NULL);
				}
			}
			else if (userType == MSG_RECV_WEB)
			{
				if (SendMsg(pWebUser, mediaID.c_str(), 0,"") == SYS_SUCCESS)
				{
					char sSendTo[MAX_1024_LEN];
					m_handlerMsgs->ResultSendMsg(msgId, true, pWebUser->webuserid, userType, msgDataType, mediaID);
					sprintf(sSendTo, "<span class=\"file_text\">发送文件 </span><a class=\"file_link\" href=\"%s\" target=\"_blank\">%s</a>",
						mediaID.c_str(), fileName.c_str());
					AddMsgToList((IBaseObject*)pWebUser, MSG_FROM_SELF, MSG_RECV_ERROR, msgId, MSG_TYPE_NORMAL,
						msgDataType, sSendTo, 0, NULL, NULL);
				}
			}
			else if (userType == MSG_RECV_CLIENT)
			{
				// 拼出该url串
				char sSendTo[MAX_1024_LEN];
				sprintf(sSendTo, "<span class=\"file_text\">收到文件 </span><a href=\"%s\">%s</a>", mediaID.c_str(), fileName.c_str());
				if (SendMsg(pUser, sSendTo, 0) == SYS_SUCCESS)
				{
					char sSendTo[MAX_1024_LEN];
					m_handlerMsgs->ResultSendMsg(msgId, true, pUser->UserInfo.uid, userType, msgDataType, mediaID);
					sprintf(sSendTo, "<span class=\"file_text\">发送文件 </span><a href=\"%s\">%s</a>",	mediaID.c_str(), fileName.c_str());
					AddMsgToList((IBaseObject*)pUser, MSG_FROM_SELF, MSG_RECV_ERROR, msgId, MSG_TYPE_NORMAL,
						msgDataType, sSendTo, 0, NULL, NULL);
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

void CChatManager::DownLoadFile(IBaseObject* pObj, MSG_DATA_TYPE nMsgDataType, string url, CUserObject *pAssistUser, unsigned long time, string msgId)
{
	if (pObj && pObj->m_nEMObType == OBJECT_USER)
	{
		CUserObject* pUser = (CUserObject*)pObj;
		if (nMsgDataType == MSG_DATA_TYPE_IMAGE)
		{
			string path = FullPath("temp\\") + GetFileId();
			string fullPath = path + ".jpg";
			if (!PathFileExistsA(fullPath.c_str()))
			{
				CCodeConvert convert;
				DOWNLOAD_INFO* param = new DOWNLOAD_INFO();
				param->pUser = pUser;
				param->pThis = this;
				param->filePath = convert.URLEncode(path.c_str());
				param->downLoadUrl = url;
				param->msgDataType = MSG_DATA_TYPE_IMAGE;
				param->time = time;
				param->pWebUser = NULL;
				param->msgId = msgId;
				param->msgFromType = MSG_FROM_CLIENT;
				_beginthreadex(NULL, 0, DownLoadFileFromServerThread, (void*)param, 0, NULL);
				return;
			}
		}
		g_WriteLog.WriteLog(C_LOG_ERROR, "DownLoadFile download image error");
	}
	else if (pObj && pObj->m_nEMObType == OBJECT_WEBUSER)
	{
		CWebUserObject* pWebUser = (CWebUserObject*)pObj;
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
						//	// 重新拼url是将最新token替换上，防止下载图片无效
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
					param->time = time;
					param->pWebUser = pWebUser;
					param->msgId = msgId;
					param->msgFromType = msgFromType;
					_beginthreadex(NULL, 0, DownLoadFileFromServerThread, (void*)param, 0, NULL);
				}
				else
				{
					g_WriteLog.WriteLog(C_LOG_TRACE, "DownLoadFile 没有拿到token之前不能进行文件下载");

					string imagePath = FullPath("SkinRes\\mainframe\\");
					StringReplace(imagePath, "\\", "/");

					char contentMsg[MAX_1024_LEN];
					sprintf(contentMsg, "<img id=\"%s_msg\" class=\"msg_image\" src=\"%srecv_image_fail.jpg\"><img id = \"%s_image\" onclick=ReRecvFile(\"%s\",\"%d\",\"%s\",\"%d\",\"%lu\",\"%lu\",\"%s\") class=\"wait_image\" src=\"%smsg_fail.png\">",
						msgId.c_str(), imagePath.c_str(), msgId.c_str(), url.c_str(), msgFromType, msgId.c_str(),
						MSG_DATA_TYPE_IMAGE, pWebUser->webuserid, assistUid, imagePath.c_str(), imagePath.c_str());

					// 保存该条未下载记录
					AddMsgToList((IBaseObject*)pWebUser, msgFromType, MSG_RECV_ERROR, msgId, MSG_TYPE_NORMAL,
						nMsgDataType, contentMsg, time, pAssistUser, NULL, true, false, false);
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
					param->time = time;
					param->pWebUser = pWebUser;
					param->msgId = msgId;
					param->msgFromType = msgFromType;
					_beginthreadex(NULL, 0, DownLoadFileFromServerThread, (void*)param, 0, NULL);
				}
				else
				{
					g_WriteLog.WriteLog(C_LOG_ERROR, "DownLoadFile 下载参数异常");

					string imagePath = FullPath("SkinRes\\mainframe\\");
					StringReplace(imagePath, "\\", "/");

					char contentMsg[MAX_1024_LEN];
					sprintf(contentMsg, "<img id=\"%s_msg\" class=\"msg_image\" src=\"%srecv_image_fail.jpg\"><img id = \"%s_image\" onclick=ReRecvFile(\"%s\",\"%d\",\"%s\",\"%d\",\"%lu\",\"%lu\",\"%s\") class=\"wait_image\" src=\"%smsg_fail.png\">",
						msgId.c_str(), imagePath.c_str(), msgId.c_str(), url.c_str(), msgFromType, msgId.c_str(),
						MSG_DATA_TYPE_IMAGE, pWebUser->webuserid, assistUid, imagePath.c_str(), imagePath.c_str());

					// 保存该条未下载记录
					AddMsgToList((IBaseObject*)pWebUser, msgFromType, MSG_RECV_ERROR, msgId, MSG_TYPE_NORMAL,
						nMsgDataType, contentMsg, time, pAssistUser, NULL, true, false, false);
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
					// 重新拼url是将最新token替换上，防止下载图片无效
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
				param->time = time;
				param->pWebUser = pWebUser;
				param->msgId = msgId;
				param->msgFromType = msgFromType;
				_beginthreadex(NULL, 0, DownLoadFileFromServerThread, (void*)param, 0, NULL);
			}
			else
			{
				g_WriteLog.WriteLog(C_LOG_TRACE, "DownLoadFile 没有拿到token之前不能进行文件下载");

				string imagePath = FullPath("SkinRes\\mainframe\\");
				StringReplace(imagePath, "\\", "/");

				char contentMsg[MAX_1024_LEN];
				sprintf(contentMsg, "<audio class=\"msg_voice\" controls=\"controls\" id=\"%s_msg\" src=\"\" type=\"audio/mpeg\"></audio><img id=\"%s_image\" onclick=ReRecvFile(\"%s\",\"%d\",\"%s\",\"%d\",\"%lu\",\"%lu\",\"%s\") class=\"wait_image\" src=\"%smsg_fail.png\">",
					msgId.c_str(), msgId.c_str(), url.c_str(), MSG_FROM_WEBUSER, msgId.c_str(), MSG_DATA_TYPE_VOICE,
					pWebUser->webuserid, assistUid, imagePath.c_str(), imagePath.c_str());

				// 保存该条未下载记录
				AddMsgToList((IBaseObject*)pWebUser, msgFromType, MSG_RECV_ERROR, msgId, MSG_TYPE_NORMAL,
					nMsgDataType, contentMsg, time, pAssistUser, NULL, true, false, false);
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
					// 重新拼url是将最新token替换上，防止下载图片无效
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
				param->time = time;
				param->pWebUser = pWebUser;
				param->msgId = msgId;
				param->msgFromType = msgFromType;
				_beginthreadex(NULL, 0, DownLoadFileFromServerThread, (void*)param, 0, NULL);
			}
			else
			{
				g_WriteLog.WriteLog(C_LOG_TRACE, "DownLoadFile 没有拿到token之前不能进行文件下载");

				string imagePath = FullPath("SkinRes\\mainframe\\");
				StringReplace(imagePath, "\\", "/");

				char contentMsg[MAX_1024_LEN];
				sprintf(contentMsg, "<video class=\"msg_voice\" controls=\"controls\" id=\"%s_msg\" src=\"\" type=\"video/mp4\"></video><img id=\"%s_image\" onclick=ReRecvFile(\"%s\",\"%d\",\"%s\",\"%d\",\"%lu\",\"%lu\",\"%s\") class=\"wait_image\" src=\"%smsg_fail.png\">",
					msgId.c_str(), msgId.c_str(), url.c_str(), MSG_FROM_WEBUSER, msgId.c_str(), MSG_DATA_TYPE_VIDEO,
					pWebUser->webuserid, assistUid, imagePath.c_str(), imagePath.c_str());

				// 保存该条未下载记录
				AddMsgToList((IBaseObject*)pWebUser, msgFromType, MSG_RECV_ERROR, msgId, MSG_TYPE_NORMAL,
					nMsgDataType, contentMsg, time, pAssistUser, NULL, true, false, false);
			}
		}
	}
}

void CChatManager::AddMsgToList(IBaseObject* pObj, MSG_FROM_TYPE msgFrom, MSG_RECV_TYPE recvType,string msgId, 
	MSG_TYPE msgType, MSG_DATA_TYPE msgDataType, string msgContent, unsigned long msgTime, CUserObject* pAssistUser,
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
		// 保存消息记录，界面切换聊天对象时，需要获取这些消息
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
				g_WriteLog.WriteLog(C_LOG_ERROR, "AddMsgToList 消息来源错误");
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
						if ((int)strMsg.find("userfile") > -1 && (int)strMsg.find("收到文件") > -1)
						{
							// 协助对象给微信访客发的
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

									sprintf(formatMsg, "收到文件 <a class=\"file_link\" href=\"%s\" target=\"_blank\">%s</a>",
										fileUrl.c_str(), fileName.c_str());
									msgContent = formatMsg;
								}
							}
						}
						else if ((int)strMsg.find("userfile") > -1)
						{
							// web用户发来
							msgDataType = MSG_DATA_TYPE_FILE;
							string vFileName;
							int pos1 = strMsg.find("userfile/");
							vFileName = strMsg.substr(pos1 + 9, strMsg.length() - pos1 - 9);

							sprintf(formatMsg, "收到文件 <a class=\"file_link\" href=\"%s\" target=\"_blank\">%s</a>",
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
					sprintf(formatMsg, "<img class=\"msg_image\" src=\"%s\" onclick=window.RunMsgList(\"ViewDetails\",\"%s\")>", msgContent.c_str(), msgContent.c_str());
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
					// 获取地图标注图标路径
					CCodeConvert convert;
					string placeMarking = FullPath("SkinRes\\mainframe\\location.png");
					StringReplace(placeMarking,"\\", "/");
					string place_marking;
					convert.Gb2312ToUTF_8(place_marking, placeMarking.c_str(), placeMarking.length());

					string weChatStaticMapFormat = m_initConfig.wechat_static_map;
					char weChatStaticMap[MAX_256_LEN];
					WxMsgLocation* wxLocation = (WxMsgLocation*)msgContentWx;
					sprintf(weChatStaticMap,weChatStaticMapFormat.c_str(), wxLocation->Location_X.c_str(), wxLocation->Location_Y.c_str());

					string weChatMapLocationFormat = m_initConfig.wechat_map_location;
					char weChatMapLocation[MAX_512_LEN];
					sprintf(weChatMapLocation, weChatMapLocationFormat.c_str(), wxLocation->Location_X.c_str(), wxLocation->Location_Y.c_str(), msgContent.c_str(), "当前位置");
					string locationMsg = "<div class=\"location_relative\">";
					locationMsg += "<img class=\"location_map_bg\" src=\"";
					locationMsg += weChatStaticMap;
					locationMsg += "\" ondblclick=window.RunMsgList(\"ViewDetails\",\"";
					locationMsg += weChatMapLocation;
					locationMsg += "\")><img class=\"location_map_middle\" src=\"";
					locationMsg += place_marking.c_str();
					locationMsg += "\" ondblclick = window.RunMsgList(\"ViewDetails\",\"";
					locationMsg += weChatMapLocation;
					locationMsg += "\")><div class=\"location_title_bg\" >";
					locationMsg += "<div class=\"location_title\">";
					locationMsg += msgContent;
					locationMsg += "</div></div></div>";

					convert.Gb2312ToUTF_8(sMsg, locationMsg.c_str(), locationMsg.length());
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
				msgFrom, msgDataType, sName.c_str(), GetTimeByMDAndHMS(msgTime).c_str(), sMsg.c_str(), "0",
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
				g_WriteLog.WriteLog(C_LOG_ERROR, "AddMsgToList 消息来源错误");
				return;
			}

			StringReplace(name, "\\", "\\\\");
			StringReplace(name, "'", "&#039;");
			StringReplace(name, "\r\n", "<br>");
			convert.Gb2312ToUTF_8(sName, name.c_str(), name.length());

			if (bSuccess)
			{
				if (msgDataType == MSG_DATA_TYPE_TEXT)
				{
					StringReplace(msgContent, "\\", "\\\\");
					StringReplace(msgContent, "'", "&#039;");
					StringReplace(msgContent, "\r\n", "<br>");
					convert.Gb2312ToUTF_8(sMsg, msgContent.c_str(), msgContent.length());
				}
				else if (msgDataType == MSG_DATA_TYPE_IMAGE)
				{
					char formatMsg[MAX_1024_LEN];
					sprintf(formatMsg, "<img class=\"msg_image\" src=\"%s\" onclick=window.RunMsgList(\"ViewDetails\",\"%s\")>", msgContent.c_str(), msgContent.c_str());
					sMsg = formatMsg;
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

			sprintf(callJsMsg, "AppendMsgToHistory('%d','%d','%s','%s','%s','%s','%s','%s');",
				msgFrom, msgDataType, sName.c_str(), GetTimeByMDAndHMS(msgTime).c_str(), sMsg.c_str(), "0", head.c_str(), msgId.c_str());
			ongMsg.msg = callJsMsg;

			if (bSave)
			{
				bool isNew = true;
				list<ONE_MSG_INFO>::reverse_iterator iter = pUser->m_strMsgs.rbegin();
				for (iter; iter != pUser->m_strMsgs.rend(); iter++)
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
					pUser->m_strMsgs.push_back(ongMsg);
				}
			}
		}
	}
	else
	{
		strcpy(callJsMsg, msgContent.c_str());
	}
	
	// 界面实时显示消息
	// 1. 自己发的消息，不用推，界面已自行添加
	// 2. 下载文件后的消息也不用推给界面，会通过ResultRecvMsg告之界面
	if (msgFrom != MSG_FROM_SELF && bNotify)
	{
		m_handlerMsgs->RecvMsg(pObj, msgFrom, msgId, msgType, msgDataType,
			callJsMsg, GetTimeByMDAndHMS(msgTime), pAssistUser, msgContentWx);
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

	//生成二次发起微信会话的url
	FormatRequestUrl(strUrl, strNavigateIconURL);

	CCodeConvert convert;
	string strUUrl;
	convert.Gb2312ToUTF_8(strUUrl, strUrl.c_str(), strUrl.length());

	//发起二次会话 
	RepickChatCon(strUUrl, strRet, nUin, strErrMsg);
}

void CChatManager::FormatRequestUrl(string &strUrl,string strMsg)
{
	string strPass, strPass1;//当前坐席密码
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
			sprintf(strLog, "errCode：%s, errMsg: %s", strRet.c_str(), strErrMsg.c_str());
			g_WriteLog.WriteLog(C_LOG_ERROR, "RepickChat err=[%s]\n", strLog);
			sLog = strLog;
			int pos = sLog.find("10001");
			if (pos >= 0)
			{
				MessageBoxA(NULL,"该用户正在与其他客服通话，无法重启会话!",0,0);
			}
			else
			{
				pos = sLog.find("45015");
				if (pos >= 0)//超过48小时的java已经过滤。此处提示取消关注
				{
					MessageBoxA(NULL, "该用户已取消关注，无法重启会话!",0,0);
				}
				else
				{
					MessageBoxA(NULL,"无法重启会话!",0,0);
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
		MessageBoxA(NULL,"重启会话失败，没有接收到后台返回数据", 0,0);
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
	if (pWebUser->m_bNewComm)//等待应答拒绝邀请目前还未开发
	{
	}
	else
	{
		//目前不知道是不是这样发送接受转接包，同意（拒绝）的值怎么传？OK、NO
		COM_SEND_MSG SendInfo(VERSION);
		char strMsg[MAX_1024_LEN];
		sprintf(strMsg,"%s,%u|%s", strReason.c_str(), m_userInfo.UserInfo.uid, m_userInfo.UserInfo.nickname);
		SendInfo.msg.msgtype = MSG_TRANSFER_ANSWER;
		SendInfo.msg.recvuin = pWebUser->talkuid;//回复给主转接客人付
		SendInfo.msg.sendtime = GetTimeLong();
		SendInfo.msg.senduin = pWebUser->webuserid;//客人号码
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
		g_WriteLog.WriteLog(C_LOG_TRACE, "发送comm转接拒绝: recvuin:%u,senduin:%u,strMsg:%s",
			SendInfo.msg.recvuin, SendInfo.msg.senduin, SendInfo.msg.strmsg);

		nError = SendPackTo(&SendInfo);
		if (nError == 0)
		{
			//删掉转接中的会话, 目前服务没有发送的拒绝返回包，无法判断对方是否收到拒绝包，只能在发送成功后删掉会话
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
	if ((int)content.find("[系统消息]") > -1)
	{
		return false;
	}
	string msgId = GetMsgId();
	string imagePath = FullPath("SkinRes\\mainframe\\");
	StringReplace(imagePath, "\\", "/");

	if ((int)content.find("http:") > -1 && (int)content.find("收到一个图片") > -1 && pAssistUser)
	{
		// 协助对象给web访客发的
		int pos = content.find("href =");
		int pos1 = content.find("target=");
		if (pos > -1 && pos1 > -1)
		{
			string imageUrl;
			imageUrl = content.substr(pos + 7, pos1 - pos - 8);

			char contentMsg[MAX_1024_LEN];
			sprintf(contentMsg, "<img id=\"%s_msg\" class=\"msg_image\" src=\"%srecv_image_fail.jpg\"><img id=\"%s_image\" class=\"wait_image\" src=\"%smsg_wait.gif\">",
				msgId.c_str(), imagePath.c_str(), msgId.c_str(), imagePath.c_str());

			AddMsgToList((IBaseObject*)pWebUser, MSG_FROM_ASSIST, MSG_RECV_ERROR, msgId, MSG_TYPE_NORMAL, MSG_DATA_TYPE_IMAGE, contentMsg, 0, pAssistUser, NULL, false, true, false);
			DownLoadFile(pWebUser, MSG_DATA_TYPE_IMAGE, imageUrl, pAssistUser, time, msgId);
		}
		return true;
	}
	else if ((int)content.find("http:") > -1 && ((int)content.find(".jpg") > -1 ||
		(int)content.find(".jpeg") > -1 || (int)content.find(".bmp") > -1 ||
		(int)content.find(".png") > -1))
	{
		// web用户发来
		char contentMsg[MAX_1024_LEN];
		sprintf(contentMsg, "<img id=\"%s_msg\" class=\"msg_image\" src=\"%srecv_image_fail.jpg\"><img id=\"%s_image\" class=\"wait_image\" src=\"%smsg_wait.gif\">",
			msgId.c_str(), imagePath.c_str(), msgId.c_str(), imagePath.c_str());

		AddMsgToList((IBaseObject*)pWebUser, MSG_FROM_WEBUSER, MSG_RECV_ERROR, msgId, MSG_TYPE_NORMAL, MSG_DATA_TYPE_IMAGE, contentMsg, 0, pAssistUser, NULL, false, true, false);
		DownLoadFile(pWebUser, MSG_DATA_TYPE_IMAGE, content, pAssistUser, time, msgId);

		return true;
	}
	return false;
}


// 上传文件到微信服务器
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
		// 上传到文件到微信服务器，微信服务器会返回一个json串，包含一个media_id值
		string returnCode;
		string decodeFilePath = convert.URLDecodeALL(fullFilePath);
		if (load.HttpLoad(string(weChatMediaUpload), "", REQUEST_TYPE_POST, decodeFilePath, returnCode))
		{
			Json::Value json;
			if (ParseJson(returnCode, json))
			{
				// 获取该media_id值
				string mediaID = GetStrFromJson(json, "media_id");
				if (!mediaID.empty())
				{
					string fileServerMediaTaskFormat = pThis->m_initConfig.fileserver_media_task;
					char fileServerMediaTask[MAX_256_LEN];
					returnCode.clear();
					CHttpLoad loadTask;
					sprintf(fileServerMediaTask, fileServerMediaTaskFormat.c_str(), wxToken.c_str(), mediaID.c_str());
					// 根据该media_id值和token值，去自己的文件服务器做一个task访问，也返回一个media值					
					if (loadTask.HttpLoad(string(fileServerMediaTask), "", REQUEST_TYPE_GET, "", returnCode))
					{
						if (msgDataType == MSG_DATA_TYPE_IMAGE || msgDataType == MSG_DATA_TYPE_VOICE)
						{
							pThis->AfterUpload(userId, userType, msgId, mediaID, msgDataType, returnCode, filePath);
						}
						else
						{
							g_WriteLog.WriteLog(C_LOG_ERROR, "未知格式的文件上传：%s", returnCode);
						}
					}
					else
					{
						// 当task访问不成功时，依然认为文件上传成功，将该媒体消息发送给微信方
						if (msgDataType == MSG_DATA_TYPE_IMAGE || msgDataType == MSG_DATA_TYPE_VOICE)
						{
							pThis->AfterUpload(userId, userType, msgId, mediaID, msgDataType, "", filePath);
						}
						else
						{
							g_WriteLog.WriteLog(C_LOG_ERROR, "未知格式的文件上传：%s", returnCode);
						}
						g_WriteLog.WriteLog(C_LOG_ERROR, "没有获取上传文件到微信服务器的task：%s", returnCode.c_str());
					}
					return true;
				}
				else
				{
					g_WriteLog.WriteLog(C_LOG_ERROR, "没有获取上传文件到微信服务器的media_id：%s", returnCode.c_str());
				}
			}
			else
			{
				g_WriteLog.WriteLog(C_LOG_ERROR, "解析上传文件到微信服务器返回码失败：%s", returnCode.c_str());
			}
		}
		else
		{
			g_WriteLog.WriteLog(C_LOG_ERROR, "上传文件到微信服务器失败：%s,当前token值是: %s", returnCode.c_str(), wxToken.c_str());
		}
	}
	else
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "上传文件到微信服务器失败，因为没有token值");
	}

	// 只要失败，一律获取token值
	pThis->SendGetWxToken(userId, pWebUser->chatid);

	// 如果发送失败，将文件路径传入，用作重新发送用的
	pThis->AfterUpload(userId, userType, msgId, "", msgDataType, "", filePath);
	return false;
}

// 上传文件到服务器
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
				// 目前只有对另一个坐席发图片是走这个逻辑
				pThis->AfterUpload(userId, userType, msgId, mediaID, msgDataType, resultCode, filePath);
				return true;
			}
			else
			{
				g_WriteLog.WriteLog(C_LOG_ERROR, "获取上传文件到文件服务器media_id失败：%s", resultCode.c_str());
			}
		}
		else
		{
			g_WriteLog.WriteLog(C_LOG_ERROR, "解析上传文件到文件服务器返回码失败：%s", resultCode.c_str());
		}
	}
	else
	{
		g_WriteLog.WriteLog(C_LOG_ERROR, "上传文件到文件服务器失败：%s", resultCode.c_str());
	}
	pThis->AfterUpload(userId, userType, "", "", msgDataType, "", "");
	return false;
}

// 从服务器下载文件
UINT WINAPI CChatManager::DownLoadFileFromServerThread(void * para)
{
	DOWNLOAD_INFO* param = (DOWNLOAD_INFO*)para;
	string filePath = param->filePath;
	CChatManager* manager = (CChatManager*)param->pThis;
	string url = param->downLoadUrl;
	unsigned long time = param->time;
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
	IBaseObject* pObj = NULL;
	string rs;

	// pUser不为空表示协助对象或坐席发来的媒体文件
	if (pUser)
	{
		sendName = pUser->UserInfo.nickname;
		userId = pUser->UserInfo.uid;
		pObj = pUser;
		webUserId = userId;
	}
	else
	{
		sendName = pWebUser->info.name;
	}

	if (pWebUser)
	{
		webUserId = pWebUser->webuserid;
		pObj = pWebUser;
	}
	
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
			// 转换格式
			manager->Amr2Wav(decodePath.c_str());
		}

		manager->m_handlerMsgs->ResultRecvMsg(msgId, true, url, webUserId, userId, addPath, msgFrom, msgDataType);

		// 保存该条记录
		manager->AddMsgToList(pObj, msgFrom, MSG_RECV_ERROR, msgId, MSG_TYPE_NORMAL, msgDataType, addPath, time, pUser, NULL, true, false, true);
		return true;
	}
	else
	{
		g_WriteLog.WriteLog(C_LOG_TRACE, "DownLoadFileFromServerThread error : %s", rs.c_str());
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

		manager->AddMsgToList(pObj, msgFrom, MSG_RECV_ERROR, msgId, MSG_TYPE_NORMAL, msgDataType,
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
		errInfo = "仅支持的文件格式有：gif|jpg|jpeg|png|bmp|zip|rar|doc|docx|txt|xls|xlsx|swf|ppt|pptx|wps|pdf";
	}
	else if (nRet == -2)
	{
		strURLFile.clear();
		errInfo = "发送文件失败！";
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
	return false;
}


