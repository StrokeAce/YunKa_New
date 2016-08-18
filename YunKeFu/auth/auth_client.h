#ifndef TQAUTHCLIENT_H_
#define TQAUTHCLIENT_H_

#include "sockbase.h"

#define  MAXDOMAINLEN 100

class CTqAuthClient
{

	char m_domain[MAXDOMAINLEN+1];
	unsigned short m_port;
	char m_proxyip[MAXDOMAINLEN+1];
	unsigned short m_proxyport;
	int            m_cversion;
public:
	CTqAuthClient(
		const char *domain,      //IN ��֤��������ַ
		unsigned short port,     //IN ��֤�������˿�
		int	 cversion			 //IN  ���ذ汾��

		);
	~CTqAuthClient(void);


	//!��֤
	int Login(unsigned int uin,  //IN �û�����
		const char *strid,		 //uinΪ0��ʹ��strid,uin>0ʱ���Դ˲���
		const char *pass,		 //IN  ����
		int type,				 //IN  ���ͣ� 1.�ͻ��� ��2 java
		const char* myip,		 //IN  ��Դip��ַ
		char * recvbuf,			 //OUT ��ŷ�������������
		int &nbuflen,			 //INOUT ����recvbuf�Ĵ�С�����ؽ��ܵ����ݴ�С
		bool& butf8,				 //OUT   recvbuf�ı��뷽ʽ��1��utf-8   0��gbk
		const char* appendinfo = NULL);// ���ӿͻ��˵�½��Ϣ��������Ŀ

	//!������������Сʱ����һ��
	int Hearbeat(const char *ptoken,//IN Login�з��ص�token����
		char * recvbuf,				//OUT ��ŷ�������������
		int &nbuflen,				//INOUT ����recvbuf�Ĵ�С�����ؽ��ܵ����ݴ�С
		bool& butf8);				//OUT   recvbuf�ı��뷽ʽ��1��utf-8   0��gbk

	//!ȡsession����
	int  Validate(const char *ptoken,//IN Login�з��ص�token����
		char * recvbuf,				//OUT ��ŷ�������������
		int &nbuflen,				//INOUT ����recvbuf�Ĵ�С�����ؽ��ܵ����ݴ�С
		bool& butf8					//OUT   recvbuf�ı��뷽ʽ��1��utf-8   0��gbk
		);

	//!ע��session
	int Logout(const char *ptoken,	//IN Login�з��ص�token����
		char * recvbuf,				//OUT ��ŷ�������������
		int &nbuflen,				//INOUT ����recvbuf�Ĵ�С�����ؽ��ܵ����ݴ�С
		bool& butf8					//OUT   recvbuf�ı��뷽ʽ��1��utf-8   0��gbk
		);

	void setproxyinfo(const char* proxyip, unsigned short proxyport);

private:
	//������֤��Ҫ��У�鴮       nbuflen�������46
	static char* CreateLoginTocken(char * recvbuf,int nbuflen);

	//���ܷ��������ص�http����
	bool RecvHttpPack(SOCKET s,char * recvbuf,int &nbuflen,bool &butf8);

};

#endif 

