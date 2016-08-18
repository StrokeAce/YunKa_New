#ifndef _CHAT_VISITOR_H_
#define _CHAT_VISITOR_H_

#include "socket.h"

class CChatManager;

class CChatVisitor : public IBaseReceive
{
public:
	CChatVisitor();
	~CChatVisitor();

public:
	// ���Ϳ�ʼͨ�����ÿͽӴ�������
	int SendWebuserTalkBegin(CWebUserObject *pWebUser);

	// ���ͽ���ͨ�������ÿͽӴ�������
	int SendWebuserTalkEnd(CWebUserObject *pWebUser);

	int SendPingToVisitorServer();

	int SendBuffToVisitorServer(char *sbuff, int len);

	int SendStartRecvMsgToVisitorServer();

	bool ConnectAndLoginToVisitorServer();

	bool ConnectToVisitorServer();

	int LoginToVisitorServer();

	void SolveVisitorSystem(char *pInitBuff);

	void SolveVisitorSCRIPTMSG(char *pInitBuff);

	//�ÿͲ�ͬ��Ի�
	void SolveVisitorSCRIPTMSGApplyFail(char *pInitBuff);

	//���ķÿ�����
	void SolveVisitorSCRIPTMSGModiName(char *pInitBuff);

	//��ʼͨ��
	void SolveVisitorSCRIPTMSGTalkBegin(char *pInitBuff);

	//����ͨ��
	void SolveVisitorSCRIPTMSGTalkEnd(char *pInitBuff);

	void SolveVisitorSystemAdmin(char *pInitBuff);

	//�ÿ�����
	void SolveVisitorSystemUp(char *pInitBuff);

	void SetVisitorOffline(CWebUserObject *pWebUser);

	//�ÿ�����
	void SolveVisitorSystemDown(char *pInitBuff);

	void SolveVisitorSystemStopRecvMsg(char *pInitBuff);

	void SolveVisitorSystemAlreadyApply(char *pInitBuff);

	void GetInviteSysMsg(char* msg, CWebUserObject *pWebUser, char *nickname, int result);

	void GetWebUserSysMsg(char* msg, CWebUserObject *pWebUser, WEBUSER_UPINFO *pInfo, const string& strscriptflag);

	void RecvSrvRespVisitorInfo(CWebUserObject *pWebUser, WEBUSER_UPINFO *pInfo);

	string SolveVisitorInfoHtmlTxt(CWebUserObject *pWebUser, WEBUSER_UPINFO *pInfo);

	void SolvePrevURL(CWebUserObject *pWebUser, WEBUSER_UPINFO *pInfo);

	void SolveWebUserTipsTail(CWebUserObject *pWebUser, WEBUSER_UPINFO *pInfo);

	void SolveWebUserOnlineTipsTail(CWebUserObject *pWebUser, WEBUSER_UPINFO *pInfo);

private:
	/***************     �̳нӿڵĺ���ʵ��    *****************/

	virtual void OnReceive(void* wParam, void* lParam);

	virtual void OnReceiveEvent(int wParam, int lParam);

public:
	CChatManager*	m_manager;
	CMySocket		m_socketEx;				// �ÿͽӴ�����
	time_t			m_tResentVisitPackTime;	//
private:
	friend class CMySocket;

private:
	
};

#endif