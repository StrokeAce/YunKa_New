#include "../stdafx.h"
#include "login.h"
#include "http_parse.h"
#include "./chat_manager.h"
#include "../chat_common/comclt.h"
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
	if (!CheckLoginInfo(loginName, password, isAutoLogin, isKeepPwd))
	{
		m_manager->m_handlerLogin->LoginProgress(-1);
		return;
	}

	unsigned int uin(0);
	string addressInfo;
	char getIpInfoUrl[MAX_1024_LEN];
	string authIp;
	CHttpLoad load;
	unsigned short authPort = 80;

	if (IsNumber(loginName))
	{
		uin = atol(loginName.c_str());
		sprintf(getIpInfoUrl, m_manager->m_initConfig.address_by_uin, uin);
	}
	else
	{
		sprintf(getIpInfoUrl, m_manager->m_initConfig.address_by_rid, loginName.c_str());
	}

	if (load.HttpLoad(string(getIpInfoUrl), "", REQUEST_TYPE_GET, "", addressInfo))
	{
		char passport[MAX_128_LEN];
		char tcpui[MAX_128_LEN];
		char vip[MAX_128_LEN];
		GetContentBetweenString(addressInfo.c_str(), "passport=", "monitor=", passport);
		GetContentBetweenString(addressInfo.c_str(), "tcp_ui=", "http_ui=", tcpui);
		GetContentBetweenString(addressInfo.c_str(), "vip=", "sysMessage_url=", vip);
		string address = tcpui;
		int pos = address.find(":");
		m_manager->m_server = address.substr(0, pos).c_str();
		string sPort = address.substr(pos + 1, address.length() - pos - 1);
		m_manager->m_port = atol(sPort.c_str());

		authIp = passport;
		m_manager->m_vip = vip;

		authIp = authIp.substr(0, authIp.length() - 1);
		m_manager->m_vip = m_manager->m_vip.substr(0, m_manager->m_vip.length() - 1);
	}
	else
	{
		m_manager->m_handlerLogin->LoginProgress(-1);
		m_manager->m_lastError = "��ȡ��������Ϣʧ��";
		return;
	}

	// ��֤
	if (GetTqAuthToken(uin, loginName.c_str(), password.c_str(), authIp.c_str(), authPort) != 1)
	{
		m_manager->m_lastError = "��֤ʧ��";
		m_manager->m_handlerLogin->LoginProgress(-1);
		return;
	}

	m_manager->m_handlerLogin->LoginProgress(20);

	if (!IsNumber(loginName))
	{
		m_nLoginBy = LOGIN_BYSTRING;
	}
	else
	{
		m_nLoginBy = LOGIN_BYUID;
	}

	if (CheckLoginFlag(uin, loginName))
	{
		m_manager->m_lastError = "���ʺ��ڱ����Ѿ���¼";
		m_manager->m_handlerLogin->LoginProgress(-1);
		return;
	}

	// ��¼
	if (!LoginToRealServer(m_manager->m_server, m_manager->m_port, uin))
	{
		m_manager->m_handlerLogin->LoginProgress(-1);
	}
}

bool CLogin::CheckLoginInfo(string loginName, string password, bool isAutoLogin, bool isKeepPwd)
{
	char str[MAX_256_LEN];

	if (loginName.empty())
	{
		m_manager->m_lastError = "��¼������Ϊ��!";
		return false;
	}
	
	if (password.length() > MAX_PASSWORD_LEN)
	{
		sprintf(str, "���볬�������ĳ��ȣ�%d���ַ�", MAX_PASSWORD_LEN);
		m_manager->m_lastError = str;
		return false;
	}
	return true;
}

int CLogin::GetTqAuthToken(unsigned int &uin, const char *szStrid, const char *szPassWord,const char* ip, unsigned short port)
{
	int nlen = MAX_4096_LEN;
	char recvbuf[MAX_4096_LEN] = { 0 };
	bool butf8(true);
	char myip[100];

	if (this->m_pTqAuthClient == NULL)
		m_pTqAuthClient = new CTqAuthClient(ip, port, VERSION);
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

	m_manager->m_handlerLogin->LoginProgress(40);

	if ((nError = SendLoginInfo(uin)) != 0)
		return false;

	m_manager->m_handlerLogin->LoginProgress(60);
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

	if (m_nSendAuthToken > 600)	//1800 ��Լ30���� -> ��Ϊ600��10����
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
