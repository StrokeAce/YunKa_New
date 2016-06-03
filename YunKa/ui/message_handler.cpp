#include "../StdAfx.h"
#include "message_handler.h"




CMessageHandler::CMessageHandler()
{
}

CMessageHandler::~CMessageHandler()
{
}

void CMessageHandler::LoginProgress(int percent)
{
	if (percent == 100)
	{
		m_manager->SendTo_GetShareList();
	}
	else
	{
	}

	SetLoginPercent(percent);



}



// �յ�һ����ϯ�û�����Ϣ,������ʼ����ϯ�б�
void CMessageHandler::RecvOneUserInfo(CUserObject* obj)
{}

// �յ�һ���½��ĻỰ��Ϣ
void CMessageHandler::RecvCreateChat(CWebUserObject* obj)
{}

// �յ�һ���Ự��Ϣ
void CMessageHandler::RecvChatInfo(CWebUserObject* obj)
{}

// �յ������û�������״̬
void CMessageHandler::RecvUserStatus(CUserObject* obj)
{}


// �յ�һ����Ϣ
void CMessageHandler::RecvOneMsg(IBaseObject* pObj, int msgFrom, string msgId, int msgType, int msgDataType,
	string msgContent, string msgTime, CUserObject* pAssistUser, WxMsgBase* msgContentWx, string msgExt)
{

}

// ��ϯ������Ϣ
void CMessageHandler::RecvOnline(CUserObject* obj)
{}

// ��ϯ������Ϣ
void CMessageHandler::RecvOffline(CUserObject* obj)
{}

// �Ự�ر�
void CMessageHandler::RecvCloseChat()
{}

void CMessageHandler::StartLogin(string loginName, string password, bool isAutoLogin, bool isKeepPwd)
{
	m_manager = CChatManager::GetInstance(this);

	//bool isAutoLogin = false;
	//string loginName = "9692111";
	//string password = "123";
	//bool isKeepPwd = false;

	//string error;

	m_manager->StartLogin(loginName, password, isAutoLogin, isKeepPwd);

}