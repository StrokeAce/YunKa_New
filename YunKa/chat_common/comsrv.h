#if !defined _COMMSRV_EM
#define _COMMSRV_EM

#include "comcom.h"
#include "comdef.h"

class SRV_ERROR_CLTSEND:public CPackInfo
{
public:
	SRV_ERROR_CLTSEND(unsigned short ver);
	~SRV_ERROR_CLTSEND();
	
	bool unpack();  //read
	bool pack();  //write
	;

public:

	unsigned short type;  //�������� 
/*
		0  ����Ŵ���
		1  uin����
		2  ����С����
*/

	unsigned short cmd;					//�ͻ��˵������
	char	strMemo[MAX_RESPONSE_LEN];	//����
};

//#define CMD_SRV_RESP_FAIL					0x0014	//  20 ��������Ӧ����ĳһ����ʧ��
class SRV_RESP_FAIL:public CPackInfo
{
public:	
	SRV_RESP_FAIL(unsigned short ver);
	SRV_RESP_FAIL(unsigned short ver, unsigned short seq, unsigned short cmd);
	~SRV_RESP_FAIL();

	bool unpack();  //read
	bool pack();  //write
	;

public:
	unsigned short	seq;		//�����յ��İ����
	unsigned short	cmd;		//��һ�������ʧ��
	unsigned short	cmdtype;	//�÷����ʾ,��cmdѡһ��
	unsigned short  type;		//���еķ���
	unsigned short  typecmd;	//���еķ�����cmd��ʾ����typeѡһ��
	unsigned int	uin;		//����һ���û��й�

	unsigned short	langtype;	//���Ա�ʶ
	unsigned short	deny;		//ʧ�ܵ����
	char			reason[MAX_RESPONSE_LEN];		//255  ʧ�ܵ���������
	char			strmemo[MAX_RESPONSE_LEN];		//255  ʧ�ܵ���������
};

//#define CMD_SRV_SERVER_COPY					0x001E	//  30 ����������
class SRV_SERVER_COPY:public CPackInfo
{

public:
	SRV_SERVER_COPY(unsigned short ver);
	~SRV_SERVER_COPY();
	
	bool unpack();  //read
	bool pack();  //write

	;
public:
	unsigned short seq;
	unsigned short type;

	unsigned int uin;
};

//#define CMD_SRV_CONF_LOGOFF					0x0028	//  40 ȷ��ĳ������
//������ȷ���û��Ѿ�����
class SRV_CONF_LOGOFF:public CPackInfo
{
public:
	SRV_CONF_LOGOFF(unsigned short ver);
	~SRV_CONF_LOGOFF();

	bool unpack();  //read
	bool pack();  //write
	;

public:
	unsigned short seq;            //�����������������  
	unsigned short type;
	unsigned int uin;				//�����û��ı��
};

//#define CMD_SRV_SERVER_BUSY					0x003C	//  60 ������æ
class SRV_SERVER_BUSY:public CPackInfo
{
public:
	SRV_SERVER_BUSY(unsigned short ver);
	~SRV_SERVER_BUSY();

	bool unpack();  //read
	bool pack();  //write
	;

public:
	unsigned short seq;            //���������
};

//#define CMD_SRV_CONF_REG					0x0046	//  70 ȷ��ע�����û�
//������ȷ��ע���
//max size : 614
class SRV_CONF_REG:public CPackInfo
{ 
public:
	SRV_CONF_REG(unsigned short ver);
	~SRV_CONF_REG();

	bool unpack();  //read
	bool pack();  //write
	;

public:
	unsigned short seq;					//2
	unsigned short  type_loginby;		//2 0  UID; 1 string

	unsigned int uin;					//4 Ѱ����
	char strid[MAX_SID_LEN];			//60
	char pwd[MAX_PASSWORD_LEN];			//20 ����

	char strMemo[MAX_RESPONSE_LEN];		//255 ����
	char strAddress[MAX_URL_LEN];		//255 ���ص�ַ

	unsigned int videoip;				//4
	unsigned short videoport;			//2
	//14+80+520 = 614
}; 

//#define CMD_SRV_DENY_REG					0x0050	//  80 �ܾ�ע�����û�
//�������ܾ�ע�����û�
//max size : 516
class SRV_DENY_REG:public CPackInfo
{ 
public:
	SRV_DENY_REG(unsigned short ver);
	~SRV_DENY_REG();

	bool unpack();  //read
	bool pack();  //write
	;

public:
	unsigned short seq;					//2
	unsigned short  type_loginby;		//2  0  UID; 1 string

	unsigned short deny;				//2  �ܾ������� 0  �ַ�����¼�Ŵ���    5 �汾̫��
	char reason[MAX_RESPONSE_LEN];		//255  �ܾ�����������

	char strAddress[MAX_URL_LEN];		//255 ���ص�ַ
}; 

//#define CMD_SRV_CONF_LOGON					0x005A	//  90 ȷ���û���¼
//������ȷ���û���¼
//max size : 997 
class SRV_CONF_LOGON:public CPackInfo
{
public:
	SRV_CONF_LOGON(unsigned short ver);
	~SRV_CONF_LOGON();

	bool unpack();  //read
	bool pack();  //write
	;

public:
	unsigned short	seq;						//2  ��¼���	
	unsigned char	type_loginby;				//1  0  UID; 1 string 2, phone, 3 mail, 50 web
	//�Ƿ��һ�ε�½�������һ�ε�½����Ҫ����������ϵ���б�
	unsigned char	type_loginflag;				//1  0  ������½��1 ��һ�ε�½

	//���ߵ�¼�ɹ��� �ͷ���¼�ɹ������������Ѱ�
	unsigned short type_loginsort;				//2  ��¼���  0  normal;  1  offline

	unsigned int	uin;						//4  �û�UIN
	char			strid[MAX_SID_LEN];		//60  �ַ����͵ĵĵ�¼
	char			stridmobile[MAX_PHONE_LEN];	//20  �ַ����͵ĵĵ�¼
	char			stridmail[MAX_EMAIL_LEN];	//255 �ַ����͵ĵĵ�¼
	char			stridbak[MAX_SID_LEN];	//60  �ַ����͵ĵĵ�¼

	unsigned short status ;						//2  ״̬
	char strStatus[MAX_STATUS_LEN];				//40  ״̬����

	char strMemo[MAX_RESPONSE_LEN];				//255 ����
	char strAddress[MAX_URL_LEN];				//255 ���ص�ַ

	//��Ƶ��ת�ĵ�ַ
	unsigned int videoip;						//4
	unsigned short videoport;					//2

	//���ǵ���������������û��ĵ�½�ķ�������ַ
	unsigned int ip;							//4
	unsigned short port;						//2

	//��������TraCQ����OAM�������ڵ�½���̵Ĵ���һ����
	unsigned char servertype;					//1
	unsigned char sublogon;						//1

	//������͵�½����Ҫ���͵İ���Ϣ�ܶ࣬��ʾ�û�ͨ��tcp����ȡ
	unsigned int tcpip;						//4
	unsigned short tcpport;						//2

	//90+255+60+42+510+12 = 345 + 105 + 522 = 445 + 552 = 997
};

//#define CMD_SRV_DENY_LOGON					0x0064	// 100 �ܾ��û���¼
//�������ܾ���¼
//max size : 582
class SRV_DENY_LOGON:public CPackInfo
{
public:
	SRV_DENY_LOGON(unsigned short ver);
	~SRV_DENY_LOGON();

	bool unpack();  //read
	bool pack();  //write
	;

public:
	unsigned short seq;						//2

	unsigned short type_loginby;			//2 0  UID; 1 string
	unsigned short type_loginsort;			//2 ��¼���  0  normal;  1  offline

	unsigned int	uin;					//4 �û�UIN
	char 			strid[MAX_SID_LEN];	//60 �ַ����͵ĵĵ�¼

	unsigned short deny;					//2 �ܾ���¼��ԭ��0: �û��Ѿ����ߣ�1��UIN���벻���ڣ� 2���������  3: �����������  4:�����½ʧ��  5:�������� 10 �汾̫��  100 ͬһ�����������tq�����Ѿ��ﵽ3�� ������������
	char reason[MAX_RESPONSE_LEN];			//255

	char strAddress[MAX_URL_LEN];			//255���ص�ַ
	//72+510 = 582
};

//#define CMD_SRV_STATUS_USER					0x01A4	// 130 �û�����״̬
class SRV_STATUS_USER:public CPackInfo
{
public:
	SRV_STATUS_USER(unsigned short ver);
	~SRV_STATUS_USER();

	bool unpack();  //read
	bool pack();  //write
	;

	int GetStatus();
	void SetStatus(int status);

	int GetConnectType();
	void SetConnectType(int type);

	int GetClientType();
	void SetClientType(int type);

	int GetVideoType();
	void SetVideo(int type);
public:
	unsigned  int	uin;                  //�û���UIN

	char			nickname[MAX_USERNAME_LEN];						//�û�������
	unsigned  int	ip;                   //�û�������IP    
	unsigned  short port;                 //�û�������PORT

	unsigned short ver;
	unsigned short langtype;

	unsigned int status;               //�û�������״̬
	//0-3 ����״̬  
	//4-5 ���뷽ʽ
	//6-7 web/�ͻ����û�

	char	strstatus[MAX_STATUS_LEN];			//����״̬������

	unsigned short	externid;					//2 ������˾���
	unsigned short	bak;		//
};

//#define CMD_SRV_SEARCH_SUCC					0x008C	// 140 ��ѯ���ѽ��
//size : 930
class SRV_SEARCH_SUCC:public CPackInfo
{
public:	
	SRV_SEARCH_SUCC(unsigned short ver);
	~SRV_SEARCH_SUCC();

	bool unpack();  //read
	bool pack();  //write
	;

public:
	unsigned short	        seq;						//2	
	unsigned int			onlineusernum;				//4 online user num
	unsigned int			onlineclientnum;			//4 online client num
	unsigned short          isnext;						//2 1 is the next 

	unsigned short          num;						//2
	FRD_QUERY_INFO			ListInfo[MAX_LIST_QUERYFRD];//76*12

	unsigned int			nextpos;					//4

public:
	bool AddInfo(FRD_QUERY_INFO Info);
	void ClearInfo();

	unsigned short GetMaxNum()
	{
		return maxnum;
	}
private:
	unsigned short maxnum;

	//size : 18+912 = 930
};

//�����û���Ϣ, ��������, ����״̬�ķ�����������ɹ���ʧ��
//���³ɹ�
//#define CMD_SRV_UPDATE_SUCC					0x00C8	// 200 ���³ɹ�
class SRV_UPDATE_SUCC:public CPackInfo
{
public:
	SRV_UPDATE_SUCC(unsigned short ver);
	~SRV_UPDATE_SUCC();

	bool unpack();  //read
	bool pack();  //write
	;

public:
	unsigned short seq;			//2
	unsigned short type;		//2

	//���µ�״̬
	unsigned int commstatus;	//4
	unsigned int extstatus;	//4
	unsigned int onlinestatus;	//4

	char strstatus[MAX_STATUS_LEN];	//120  //����״̬����������,�µ������

	unsigned int uin;			//4
};

//������Ϣ
//#define CMD_SRV_SEND_MSG_OFFLINE		0x00DC	//220
class SRV_OFFLINE_MSG : public CPackInfo
{
public:
	SRV_OFFLINE_MSG(unsigned short ver);
	~SRV_OFFLINE_MSG();

	bool unpack();
	bool pack();
public:
	unsigned short	MsgNum;          //��Ϣ��Ŀ
	MSG_INFO MsgList[MAX_OFFLINE_MSG];         //��Ϣ�б�

};

//���������ص��û�����
//max size : 974
class SRV_REP_USERINFO:public CPackInfo
{
public:
	SRV_REP_USERINFO(unsigned short ver);
	~SRV_REP_USERINFO();

	bool unpack();  //read
	bool pack();  //write
	;

public:
	unsigned short          seq;			//2 ���ز�ѯ��
	unsigned short			nUpdateType;	//2
	unsigned int			uin;			//4 ���µ��û���

	unsigned int			ip;				//4
	unsigned short			port;			//2
	
	CUserInfo				UserInfo;		//934	�û���Ϣ
	CTradeInfo				TradeInfo;		//0			

	char strStatus[MAX_STATUS_LEN];			//20�Զ���״̬����������

	unsigned short cltVer;					//2 �ͻ��˰汾��
	unsigned short terminaltype;			//2 �ն����� 0��pc�ͻ��ˣ�1 �ֻ��ͻ��� 2 ���ͻ���
	char strFaceLink[MAX_FACELINK_LEN];

	unsigned int onlinetime;	//����ʱ��
	char strid[MAX_SID_LEN];	// strid
};

//#define CMD_SRV_REP_SMS						0x0122	// 290 ���ض����յ�����Ϣ
// 290 ���ض����յ�����Ϣ
class SRV_REP_SMS:public CPackInfo
{

public:
	SRV_REP_SMS(unsigned short version);
	~SRV_REP_SMS();

	bool unpack();  //read
	bool pack();  //write
	;

public:
	unsigned short	seq;

	char	mobile[MAX_PHONE_LEN];
	char	sms[MAX_SMS_LEN];

	unsigned int sendtime;
	unsigned int recvtime;
};

//#define CMD_SRV_REP_TRANSFERCLIENT			0x01B8  // 440 ת����ʱ�û��ɹ�ʧ��
class SRV_REP_TRANSFERCLIENT:public CPackInfo
{
public:
	SRV_REP_TRANSFERCLIENT(unsigned short ver);
	~SRV_REP_TRANSFERCLIENT();

	bool unpack();  //read
	bool pack();  //write
	;

public:
	unsigned short seq;

	NAMEID_INFO	recvinfo;
	NAMEID_INFO clientinfo;
	NAMEID_INFO	sendinfo;

	//0 success, 1 fail
	unsigned short result;

	//when result = 1
	unsigned short deny;
	//error memo
	char reason[MAX_RESPONSE_LEN];

	char   szChatId[MAX_CHATID_LEN+1];  //�Ựid
	char   szRand[MAX_WEBCLIENID_LEN+1];//�ÿ�Ψһ��ʾ
	char   szThirdid[MAX_THIRDID_LEN+1];//������id

};

//#define CMD_SRV_TRANSFERCLIENT				0x01C2  // 450 ֪ͨ�����ߣ�ת����ʱ�û�
class SRV_TRANSFERCLIENT:public CPackInfo
{
public:
	SRV_TRANSFERCLIENT(unsigned short version);
	~SRV_TRANSFERCLIENT();

	bool unpack();  //read
	bool pack();  //write


public:
	unsigned short seq;

	NAMEID_INFO	recvinfo;
	NAMEID_INFO clientinfo;
	NAMEID_INFO	sendinfo;

	char   szChatId[MAX_CHATID_LEN+1];  //�Ựid
	char   szRand[MAX_WEBCLIENID_LEN+1];//�ÿ�Ψһ��ʾ
	char   szThirdid[MAX_THIRDID_LEN+1];//������id
	
};

struct  GROUP_LIST
{
	unsigned char id;//��id
	char groupname[MAX_GROUPNAME_LEN+1]; //����
};

//#define CMD_SRV_GROUP_LIST					0x02C6 	//  ���ѷ����б�
class SRV_LIST_FRIENDGROUP:public CPackInfo
{
public:
	SRV_LIST_FRIENDGROUP(unsigned short ver);

	bool unpack();  //read
public:
	unsigned short seq;

	unsigned short all;
	unsigned short index;

	unsigned short	num;                                        //��������û���Ŀ
	unsigned short cur;	//��ǰ����

	GROUP_LIST ListInfo[MAX_LIST_FRDGROUP];		//40*MAX_LIST_FRD

public:
	unsigned short GetMaxNum()
	{
		return maxnum;
	}

private:
	unsigned short maxnum;

};

//#define CMD_SRV_LIST_FRD					0x0212	// 530 ���������б�
class SRV_LIST_FRD:public CPackInfo
{
public:
	SRV_LIST_FRD(unsigned short ver);
	~SRV_LIST_FRD();

	bool unpack();  //read
	bool pack();  //write
	;
public:
	unsigned short seq;

	unsigned short all;
	unsigned short index;

	unsigned short	num;                                        //��������û���Ŀ
	unsigned short cur;	//��ǰ����

	FRD_COMMON_INFO ListInfo[MAX_LIST_FRD];		//40*MAX_LIST_FRD

public:
	bool AddInfo(FRD_COMMON_INFO Info);
	void ClearInfo();
	unsigned short GetMaxNum()
	{
		return maxnum;
	}

private:
	unsigned short maxnum;

};

//#define CMD_SRV_LIST_ONLINEFRD				0x0226  // 550 ���������б�
//���������������û��б�
//�Է��Զ���״̬��ûһ������50���û���Ϣ�� 
//���Զ���״̬��ûһ������25���û���Ϣ�� 
//���������ȴ�����Զ���İ���Ȼ�����Զ���İ���
//max size : 6 + 50 * 20 = 1006
class SRV_LIST_ONLINEFRD:public CPackInfo
{
public:
	SRV_LIST_ONLINEFRD(unsigned short ver);
	~SRV_LIST_ONLINEFRD();

	bool unpack();  //read
	bool pack();  //write

public:
	unsigned short seq;

	unsigned short all;
	unsigned short index;

	unsigned short	num;                                        //��������û���Ŀ

	unsigned short cur;

	FRD_ONLINE_INFO ListInfo[MAX_LIST_ONLINEFRD];		//40*MAX_LIST_ONLINEFRD

public:
	bool AddInfo(FRD_ONLINE_INFO Info);
//	bool AddInfo(USER_TABLE_PT pInfo);
	void ClearInfo();

	unsigned short GetMaxNum()
	{
		return maxnum;
	}
private:
	unsigned short maxnum;
};


#endif