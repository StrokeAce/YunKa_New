#pragma once

#include "./socket.h"
#include "auth_client.h"
#include "../chat_common/comobject.h"
#include "../chat_common/comfunc.h"
#include "../chat_common/comcom.h"

class CChatManager;

class CLogin
{
public:
	CLogin();
	~CLogin();

public:
	void StartLogin(string loginName, string password, bool isAutoLogin, bool isKeepPwd);	// ��ʼ��¼

	bool CheckLoginFlag(unsigned long uin, const string& strID, bool bCreate = false, bool bFree = false);

	void TimerSolveAuthToken();

	void SetOffline();

private:

	bool CheckLoginInfo(string loginName, string password,bool isAutoLogin, bool isKeepPwd);

	int GetTqAuthToken(unsigned int &uin, const char *szStrid, const char *szPassWord);	

	bool LoginToRealServer(string strServer, int nPort, unsigned int uin);

	int ConnectToServer(string sip, unsigned short port);

	int SendLoginInfo(unsigned int uin);
	
public:
	CChatManager*		m_manager;
	CTqAuthClient*		m_pTqAuthClient;
	char				m_szAuthtoken[MAX_256_LEN + 1];	// �ĳɹ�����
	unsigned long		m_authAdminid;
	int					m_nLoginBy;						// ��ǰ�ĵ�½����
	int					m_nSendAuthToken;				// ����������֤�������Ĵ���
};