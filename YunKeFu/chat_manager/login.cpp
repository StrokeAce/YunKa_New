#include "../stdafx.h"
#include "login.h"
#include "http_parse.h"
#include "chat_manager.h"
#include "../chat_common/comclt.h"
#include "../chat_common/commsg.h"
#include "auth_error.h"
#include "http_unit.h"

CLogin::CLogin()
{	
	m_nLoginBy = -1;
	m_authAdminid = 0;
	memset(m_szAuthtoken, 0, MAX_256_LEN + 1);
	m_manager = NULL;
	m_pTqAuthClient = NULL;
	m_nSendAuthToken = -20;
}

CLogin::~CLogin()
{
}

void CLogin::StartLogin(string loginName, string password, bool isAutoLogin, bool isKeepPwd)
{
	// 先检查登录信息合法
	if (!CheckLoginInfo(loginName, password, isAutoLogin, isKeepPwd))
	{
		SetLoginProgress(-1);
		return;
	}

	if (!IsNumber(loginName))
	{
		m_nLoginBy = LOGIN_BYSTRING;
	}
	else
	{
		m_nLoginBy = LOGIN_BYUID;
	}

	unsigned int uin(0);
	bool bAutoLogin = true;;

	// 检查配置文件的认证地址(AuthAddr)来确定是否是自动定向(默认值是passport.tq.cn)方式去
	// 认证和登录,即自动登录,否则就按修改后的地址去认证和登录,手动登录
	if (strcmp(m_manager->m_initConfig.sAuthAddr, "passport.tq.cn")==0)
	{
		// 自动方式登录		
		string returnInfo;
		char getUrl[MAX_1024_LEN];		
		CHttpLoad load;

		if (m_nLoginBy == LOGIN_BYUID)
		{
			uin = atol(loginName.c_str());
			sprintf(getUrl, m_manager->m_initConfig.address_by_uin, uin);
		}
		else
		{
			sprintf(getUrl, m_manager->m_initConfig.address_by_rid, loginName.c_str());
		}

		if (load.HttpLoad(string(getUrl), "", REQUEST_TYPE_GET, "", returnInfo))
		{
			char passport[MAX_128_LEN];
			char tcpui[MAX_128_LEN];
			char vip[MAX_128_LEN];
			GetContentBetweenString(returnInfo.c_str(), "passport=", "monitor=", passport);
			GetContentBetweenString(returnInfo.c_str(), "tcp_ui=", "http_ui=", tcpui);
			GetContentBetweenString(returnInfo.c_str(), "vip=", "sysMessage_url=", vip);

			string address = tcpui;
			int pos = address.find(":");
			m_manager->m_server = address.substr(0, pos).c_str();
			string sPort = address.substr(pos + 1, address.length() - pos - 1);
			m_manager->m_port = atol(sPort.c_str());

			address = passport;
			m_manager->m_vip = vip;

			address = address.substr(0, address.length() - 1);
			strcpy(m_manager->m_initConfig.sAuthAddr,address.c_str());
			m_manager->m_initConfig.nAuthPort = 80;
			m_manager->m_vip = m_manager->m_vip.substr(0, m_manager->m_vip.length() - 1);
		}
		else
		{
			SetLoginProgress(-1);
			m_manager->m_lastError = "获取认证地址失败";
			return;
		}
	}
	else
	{
		// 手动方式登录
		if (m_nLoginBy == LOGIN_BYUID)
		{
			uin = atol(loginName.c_str());
		}
		bAutoLogin = false;
	}

	// 认证
	int nState = GetTqAuthToken(uin, loginName.c_str(), password.c_str(), bAutoLogin);
	if (nState != 1)
	{
		char errorInfo[MAX_128_LEN];
		sprintf(errorInfo, "认证失败：%s", GetAuthStrError(nState));
		m_manager->m_lastError = errorInfo;
		SetLoginProgress(-1);
		return;
	}

	SetLoginProgress(20);

	if (CheckLoginFlag(uin, loginName))
	{
		m_manager->m_lastError = "该帐号在本地已经登录";
		SetLoginProgress(-1);
		return;
	}

	// 登录
	if (!LoginToRealServer(m_manager->m_server, m_manager->m_port, uin))
	{
		SetLoginProgress(-1);
	}
}

bool CLogin::CheckLoginInfo(string loginName, string password, bool isAutoLogin, bool isKeepPwd)
{
	char str[MAX_256_LEN];

	if (loginName.empty())
	{
		m_manager->m_lastError = "登录名不能为空!";
		return false;
	}
	
	if (password.length() > MAX_PASSWORD_LEN)
	{
		sprintf(str, "密码超过其最大的长度：%d个字符", MAX_PASSWORD_LEN);
		m_manager->m_lastError = str;
		return false;
	}
	return true;
}

int CLogin::GetTqAuthToken(unsigned int &uin, const char *szStrid, const char *szPassWord, bool bAutoLogin)
{
	int nlen = MAX_4096_LEN;
	char recvbuf[MAX_4096_LEN] = { 0 };
	bool butf8(true);
	char myip[100];

	if (this->m_pTqAuthClient == NULL)
		m_pTqAuthClient = new CTqAuthClient(m_manager->m_initConfig.sAuthAddr, m_manager->m_initConfig.nAuthPort, VERSION);
	else if (strlen(m_szAuthtoken) > 0)
	{
		m_pTqAuthClient->Logout(m_szAuthtoken, recvbuf, nlen, butf8);
		m_szAuthtoken[0] = 0;
	}

	GetLocalHost(myip);

	nlen = 4096;
	int nstate = SESSION_ACTION_ERR_NOTCONNECT;
	if (0 != m_pTqAuthClient->Login(uin, szStrid, szPassWord, 1, myip, recvbuf, nlen, butf8))
	{
		return nstate;
	}

	CHttpParse  p;
	p.ParseHead(recvbuf, nlen);

	ConvertMsg(recvbuf, sizeof(recvbuf)-1);

	string t;
	if (!p.GetPostBodyParams(recvbuf, "state", t))
	{
		return nstate;
	}

	nstate = atoi(t.c_str());
	if (nstate != 1)
	{
		return nstate;
	}

	if (p.GetPostBodyParams(recvbuf, "token", t))
	{
		strncpy(m_szAuthtoken, t.c_str(), MAX_256_LEN);
	}

	if (p.GetPostBodyParams(recvbuf, "adminid", t))
	{
		m_authAdminid = atol(t.c_str());
	}

	if (p.GetPostBodyParams(recvbuf, "uin", t))
	{
		uin = atol(t.c_str());
	}

	// 手动登录的时候才使用认证返回的服务器地址
	if (!bAutoLogin && p.GetPostBodyParams(recvbuf, "uilist", t))
	{
		char server[MAX_128_LEN];
		if (GetContentBetweenString(t.c_str(), "TCP:", ",", server));
		{
			string info = server;
			int pos = info.find(":");
			m_manager->m_server = info.substr(0, pos);
			info = info.substr(pos + 1, info.length() - 1);
			m_manager->m_port = atol(info.c_str());
		}
	}

	return nstate;
}

static HANDLE NameMutex = NULL;
static HANDLE UinMutex = NULL;
bool CLogin::CheckLoginFlag(unsigned long uin, const string& strID, bool bCreate, bool bFree)
{
	bool bRetVal = false;
	HANDLE hUinFlag = NULL, hNameFlag = NULL;

	if (uin != 0)
	{
		char strUin[MAX_256_LEN];
		sprintf_s(strUin, "_TQ_%lu_", uin);

		if (bCreate)
		{
			UinMutex = CreateMutexA(NULL, FALSE, strUin);
		}
		else
		{
			hUinFlag = OpenMutexA(MUTEX_ALL_ACCESS, FALSE, strUin);
		}

		if (hUinFlag)
		{
			bRetVal = TRUE;
			if (hUinFlag)
			{
				CloseHandle(hUinFlag);
				hUinFlag = NULL;
			}
		}
	}

	if (!strID.empty())
	{
		char strName[MAX_256_LEN];
		sprintf_s(strName, "_TQ_%s_", strID.c_str());

		if (bCreate)
		{
			NameMutex = CreateMutexA(NULL, FALSE, strName);
		}
		else
		{
			hNameFlag = OpenMutexA(MUTEX_ALL_ACCESS, FALSE, strName);
		}

		if (hNameFlag)
		{
			bRetVal = TRUE;
			if (hNameFlag)
			{
				CloseHandle(hNameFlag);
				hNameFlag = NULL;
			}
		}
	}

	if (bFree)
	{
		if (UinMutex)
		{
			CloseHandle(UinMutex);
			UinMutex = NULL;
		}

		if (NameMutex)
		{
			CloseHandle(NameMutex);
			NameMutex = NULL;
		}
	}

	return bRetVal;
}

bool CLogin::LoginToRealServer(string strServer, int nPort, unsigned int uin)
{
	int nError;

	if ((nError = ConnectToServer(strServer, nPort)) != 0)
		return false;

	SetLoginProgress(40);

	if ((nError = SendLoginInfo(uin)) != 0)
		return false;

	SetLoginProgress(60);
	return true;
}

int CLogin::ConnectToServer(string sip, unsigned short port)
{
	int nError = 0;

	m_manager->m_socket.Close();

	for (int i = 0; i < 2; i++)
	{
		if (m_manager->m_socket.Connect(sip.c_str(), port))
		{
			return nError;
		}
	}

	nError = SYS_ERROR_SOCKETCONNECTFAIL;
	return nError;
}

int CLogin::SendLoginInfo(unsigned int uin)
{
	int nError = 0;

	CLT_LOGON SendInfo(VERSION);


	SendInfo.uin = uin;
	strcpy(SendInfo.strid, "");
	SendInfo.type_loginby = LOGIN_BYUID;

	strcpy(SendInfo.pwd, m_manager->m_password.c_str());

	SendInfo.status = USER_STATUS_ONLINE;
	SendInfo.langtype = 0;
	SendInfo.bak = 0;


	SendInfo.terminaltype = TERMINAL_PCEX;
	SendInfo.connecttype = MSG_TCPUI;

	SendInfo.type_loginsort = 10;

	m_manager->m_userInfo.UserInfo.uid = uin;
	nError = m_manager->SendPackTo(&SendInfo);
	if (nError != 0)
	{
		goto FAIL;
	}

FAIL:

	return nError;
}

void CLogin::TimerSolveAuthToken()
{
	if (m_nSendAuthToken < 0)
		return;
	m_nSendAuthToken++;

	if (m_nSendAuthToken > 600)	//1800 大约30分钟 -> 改为600秒10分钟
	{
		if (m_pTqAuthClient != NULL && strlen(m_szAuthtoken) > 0)
		{

			int nlen = 4096;
			char recvbuf[4096];
			bool butf8(true);

			m_pTqAuthClient->Hearbeat(m_szAuthtoken, recvbuf, nlen, butf8);
			printf("%d,%s\n", butf8, recvbuf);
		}

		m_nSendAuthToken = 0;
	}
}

void CLogin::SetOffline()
{
	if (m_pTqAuthClient != NULL && strlen(m_szAuthtoken) > 0)
	{
		int nlen = 200;
		char recvbuf[201];
		bool butf8(true);
		m_pTqAuthClient->Logout(m_szAuthtoken, recvbuf, nlen, butf8);
		strcpy(m_szAuthtoken, "");
	}
}

void CLogin::SetLoginProgress(int percent)
{
	if (m_manager && m_manager->m_handlerLogin)
	{
		m_manager->m_handlerLogin->LoginProgress(percent);
	}
	
	if (m_manager && m_manager->m_timers && percent == -1)
	{
		m_manager->m_timers->KillTimer(TIMER_LOGIN);
	}
}
