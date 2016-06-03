#ifndef _SWITCH_SOCK_H_
#define _SWITCH_SOCK_H_

#include "sock.h"
#include "mutex.h"

#define HTTP_SWITCH_PORT 8080

void http_useswitch(BOOL bUse=true);
void http_useproxy(bool bUse ,const string& Host="", unsigned short nPort=0);
void http_setswitch(string strSwitchHost, unsigned short nSwitchPort);
//---------------------------------------------------------------------------
//!��������Ϊ��ͨSocket���ֿ��Զ��������ݰ������HttpЭ�鴫�ͣ�������֧��Http����
class SwitchSock
{
public:
	SwitchSock();
	~SwitchSock();

	//!�������ݰ�������HTTPЭ�������͵�HttpSwitch��ת����ȥ, �ͻ��� <--> Http Switch <--> ������
	void SetUseHttpSwitch(BOOL bUse=true);

	//!ϵͳ�ڲ�ʹ�õ�����,��HttpSwitch�˴������Կͻ��˵İ�
	void SetAsHttpSwitch(BOOL bAs=true);

	//!����Socket�������HttpSwitchģʽ�������ӵ�Switch�������ҪHttp���������ӵ�����
	bool Create(bool bUDP=false);

	//!��������
	virtual bool Connect(const string& host,unsigned short port);
	//!�Ƿ��Ѿ�����
	virtual bool isConnected();
	//!�Ƿ���UDP��
	virtual bool isUDP();
	//!����Ŀ��ĵ�ַ
	virtual bool GetPeerName(string& strIP,unsigned short &nPort);
	//!���ӱ��صĵ�ַ�����������HttpSwitch������Switch��������Ip�Ͷ˿�
	virtual bool GetLocalName(string& strIP,unsigned short &nPort);

	//!�󶨷���˿�
	bool Bind(unsigned short nPort);
	//!���տͻ���
	bool Accept(SwitchSock& client);

	//!�ر�
	virtual void Close();

	//!����
	virtual long Send(const char* buf,long buflen);
	//!����
	virtual long Recv(char* buf,long buflen);
	//!����
	virtual long SendTo(const char* buf,int len,const struct sockaddr_in* toaddr,int tolen);
	virtual long SendTo(const char* buf,int len,const string& strHost,unsigned short nPort);
	//!����
	virtual long RecvFrom(char* buf,int len,struct sockaddr_in* fromaddr,int* fromlen);
	virtual long RecvFrom(char* buf,int len,string& strHost,unsigned short & nPort);

	//!���Socket�������������ϵͳ�����շ������õ�Socket�����Խ���select,fcntl�Ȳ���������ֱ��send recv
	SOCKET GetHandle();
private:
	//!��׼��Socket�����������շ�����
	BaseSock m_sock;

	//!�����ݰ�����HttpЭ����
	bool _SendHttpPacket(const string& param,const string& data);
	//!�����ݰ�����HTTPЭ����
	bool _RecvHttpPacket(string& param,string& data);

	//!�����յ���Httpͷ
	void _ParseParam(const string& param, map<string,string>& paramlist);
private:
	BOOL m_bUseHttpSwitch;
	BOOL m_bAsHttpSwitch;

	string m_strCurSwitch;

	char* m_pHttpRecvBuffer;
	int m_nHttpRecvLen;

	string m_strPeerHost;
	unsigned short m_nPeerPort;
	friend class CHttpSwitch;

	MMutex m_SendRecvLock;

};

//////////////////////////////////API�����б�///////////////////////////////////
SOCKET http_socket( int af,  int type,  int protocol);

void http_closesocket(SOCKET s);


int http_bind(  SOCKET s,  const struct sockaddr* name,  int namelen);

SOCKET http_accept(  SOCKET s,  struct sockaddr* addr,  int* addrlen);

int http_connect(  SOCKET s,  const struct sockaddr* name,  int namelen);

int http_recv(  SOCKET s,  char* buf,  int len,  int flags);

int http_recvfrom(  SOCKET s,  char* buf,  int len,  int flags,  struct sockaddr* from,  int* fromlen);

int http_send(  SOCKET s,  const char* buf,  int len,  int flags);

int http_sendto(  SOCKET s,  const char* buf,  int len,  int flags,  const struct sockaddr* to,  int tolen);

#endif
