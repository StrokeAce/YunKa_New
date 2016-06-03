#ifndef _MSG_TRANSFER_H_
#define _MSG_TRANSFER_H_

#include "chat_manager.h"




//typedef   int(*GetLoginState)(int);




class CMessageHandler : public IBaseMsgs
{
public:
	CMessageHandler();
	~CMessageHandler();


	virtual void LoginProgress(int percent);


	// �յ�һ����ϯ�û�����Ϣ,������ʼ����ϯ�б�
	virtual void RecvOneUserInfo(CUserObject* obj);

	// �յ�һ���½��ĻỰ��Ϣ
	virtual void RecvCreateChat(CWebUserObject* obj);

	// �յ�һ���Ự��Ϣ
	virtual void RecvChatInfo(CWebUserObject* obj);

	// �յ������û�������״̬
	virtual void RecvUserStatus(CUserObject* obj);

	virtual string GetLastError(){ return ""; }

	// �յ�һ����Ϣ
	virtual void RecvOneMsg(IBaseObject* pObj, int msgFrom, string msgId, int msgType, int msgDataType,
		string msgContent, string msgTime, CUserObject* pAssistUser, WxMsgBase* msgContentWx, string msgExt);

	// ��ϯ������Ϣ
	virtual void RecvOnline(CUserObject* obj);

	// ��ϯ������Ϣ
	virtual void RecvOffline(CUserObject* obj);

	// �Ự�ر�
	virtual void RecvCloseChat();

	void StartLogin(string loginName, string password, bool isAutoLogin, bool isKeepPwd);

	int GetLoginPercent(int vel)
	{
		return m_loginPercent;
	}
	int SetLoginPercent(int vel)
	{
		return m_loginPercent;
	}



private:

	int m_loginPercent;
	CChatManager* m_manager;

	


};











#endif