#if !defined _COMMCOM_EM
#define _COMMCOM_EM

#include "comdef.h"
#include "comstruct.h"
#include "pack.h"
#include "comtran.h"

//ͨѶ����ͷHEADER
class COM_HEAD_PACK
{
public:
	COM_HEAD_PACK(unsigned short ver_t,
		unsigned short sendrandom,
		unsigned int uin_t, 
		unsigned short random_t,
		unsigned short cmd_t, 
		unsigned short seq_t, 
		unsigned short langtype_t,
		unsigned short seqresp_t,
		unsigned short flag);
	COM_HEAD_PACK();
	~COM_HEAD_PACK();

	static int GetHeadLen();
	bool unpack(CPack &pack);  //read
	bool pack(CPack &pack);  //write

	void SetValue(unsigned short ver_t,
		unsigned short sendrandom,
		unsigned int uin_t, 
		unsigned short random_t,
		unsigned short cmd_t, 
		unsigned short seq_t, 
		unsigned short langtype_t,
		unsigned short seqresp_t,
		unsigned short flag);
	void SetValue(PACK_HEADER head);

public:
	PACK_HEADER			head;

public:

};

//ͨѶ������
class CPackInfo
{
public:
	CPackInfo();
	~CPackInfo();

	virtual bool unpack();  //read
	virtual bool pack();  //write

	virtual bool unpackhead();  //read
	virtual bool packhead();  //write

public:
	unsigned short version;
	bool bSendToServer;

	COM_HEAD_PACK	m_Head;	//����ͷ��
	CPack			m_Pack;	//���ṹ

};

//�йش��������
//max size : 48
class CProxyInfo
{
public:
	CProxyInfo();
	~CProxyInfo();

	bool unpack(CPack &pack, unsigned short ver);
	bool pack(CPack &pack, unsigned short ver);  //write

public:
	PROXY_INFO	info;

};

//�й��û�����Ϣ��Ϣ
//max size : 914
class CUserInfo
{
public:
	CUserInfo();
	~CUserInfo();

private:
	//pack and unpack for versin less 4.0
	bool UnpackForm(CPack &pack);
	bool PackForm(CPack &pack);  //write

public:
	//pack and unpack
	bool unpack(CPack &pack, unsigned short ver);
	bool pack(CPack &pack, unsigned short ver);  //write

	void ClearInfo();
	void CopyInfo(CUserInfo *pInfo);
public:
	USER_INFO	info;
};

//�й��û���������Ϣ�����ݾɰ汾
class CTradeInfo
{
public :
	CTradeInfo();
	~CTradeInfo();
	
	//pack and unpack
	bool unpack(CPack &pack, unsigned short ver);
	bool pack(CPack &pack, unsigned short ver);  //write

	void ClearInfo();
	void CopyInfo(CTradeInfo *pInfo);


public:
	TRADE_INFO		info;
};

//ͨ��������Ϣ�������ߵ�������һ�������ͣ���������û���������Ϣ��
//max size : 40(new) 62(old) 
class FRD_COMMON_INFO
{
public:
	FRD_COMMON_INFO();
	~FRD_COMMON_INFO();

	bool unpack(CPack &pack, unsigned short ver);
	bool pack(CPack &pack, unsigned short ver);

	bool ConvertGBToBIG5(bool bGBToBIG5);

	void ClearInfo();
	void CopyInfo(FRD_COMMON_INFO *pInfo);
public:
	unsigned int	uin;						//4 �û���UIN��

	char			mobile[MAX_PHONE_LEN];		//20 ���У��°�δ�ã�
	char			nickname[MAX_USERNAME_LEN]; //20 ����

	unsigned int	deptid;						//4
	//externid =	0 ͬһ��˾���û�
	//				1 ��ͬ��˾���û�
	//				>=1000 ������˾���û���������˾���Ϊ���ڵ���1000�����������ı��Ϊ100
	unsigned short	externid;					//2 ������˾���
	unsigned short	langtype;					//2

//	16 - 23 ���ID
	unsigned char	groupid;					//1
//	24 - 31 ͼ����
	unsigned char	iconid;						//1

	unsigned char	sex;						//1
	unsigned char	bak;						//1	����

	unsigned char cgroupid;						//�û����ڵĺ��ѷ���
};

//���ߵ�������Ϣ
//max size : 20(new) 40(new ex) 60(old) 
class FRD_ONLINE_INFO
{
public:
	FRD_ONLINE_INFO();
	~FRD_ONLINE_INFO();

	bool unpack(CPack &pack, unsigned short ver);
	bool pack(CPack &pack, unsigned short ver);


	void ClearInfo();
	void CopyInfo(FRD_ONLINE_INFO *pInfo);

public:
	unsigned int	uin;						//4 UIN����
	char			nickname[40];	//20 ����(not use on new copy)

	unsigned int	ip;							//4 ip
	unsigned short	port;						//2 port	

	unsigned short	version;					//2 �û��İ汾
	unsigned short	langtype;					//2 

	unsigned int	onlinestatus;				//4 ����״̬λ
	char 			strstatus[30];	//20 ����״̬����
	char			strcompany[40];

	unsigned short terminaltype;
	
};

//��ѯ����������Ϣ
//size : new 76 size
class FRD_QUERY_INFO
{
public:
	FRD_QUERY_INFO();
	~FRD_QUERY_INFO();

	bool unpack(CPack &pack, unsigned short ver);
	bool pack(CPack &pack, unsigned short ver);


	void ClearInfo();
	void CopyInfo(FRD_QUERY_INFO *pInfo);
public:
	unsigned int	uin;						//4 UIN����
	char			nickname[MAX_USERNAME_LEN];	//20 ����

	char			email[MAX_EMAIL_LEN];		//255 EMAIL	not used

	//���department == 0 ��company���湫˾���ƺͲ�������
	char			company[MAX_COMPANY_LEN];	//40 ��˾����

	unsigned int	deptid;						//4 ����
	unsigned short	country;					//2 ���� not used
	unsigned short	province;					//2 ʡ�� not used

	char			city[MAX_CITY_LEN];			//30 ���� not used
	unsigned int	commstatus;					//4 ��־λ not used
	unsigned int	slavestatus;				//4  not used
	unsigned int	onlinestatus;				//4 ����״̬λnot used

//	unsigned char	sex;						//1 �Ա�
	unsigned short	sex;						//1 �Ա�
//	unsigned char	online;						//1 �Ƿ�����
	unsigned short	langtype;					//2 
	//size : 12 + 20 +40 + 4 = 76
};


//��Ϣ��ͨ�ýṹ
//950
class MSG_INFO
{
public:
	MSG_INFO();
	~MSG_INFO();

	bool unpack(CPack &pack, unsigned short ver);
	bool pack(CPack &pack, unsigned short ver);

public:
	unsigned short	seq;						//2

	unsigned short	msgtype;					//1 ��Ϣ��� 0 ��ͨ��Ϣ  1����
	unsigned int	recvuin;					//4 �����ߵ�UIN����
	unsigned int	senduin;					//4 �����ߵ�UIN����  
	unsigned int	sendtime;					//4 ����ʱ��
	unsigned short	isread;						//2 �Ƿ񱻶���
	char			strmsg[MAX_MSG_RECVLEN+1];		//910 ��Ϣ����
	char			strmobile[MAX_MSGNICKNAME_LEN+1];	//66 �ֻ�����, ������Ϣʹ�ã��û�������Ϣʱ��������
	char			strfontinfo[MAX_MSGFONT_LEN+1]; //HTML,JSON=WX,CHSET=%u,H=%u,EF=%u,CR=%u,FN=%s

	unsigned short	langtype;					//2
	unsigned int	bak;						//4	������Ϣ�ı��
	//40 + 910 = 950
};

//�й�һ���顢���š��û���ͨ��������Ϣ
//max size : 32
class NAMEID_INFO
{
public:
	NAMEID_INFO();
	~NAMEID_INFO();

public:
	unsigned int	id;						//4 �û���UIN��

	char			name[MAX_USERNAME_LEN+1]; //20 ����

};

//#define CMD_COM_ACK							0x000A	//  10 �յ������Ӧ
class COM_ACK : public CPackInfo
{
protected:
	bool bClient;

public:
	COM_ACK(unsigned short version);
	COM_ACK();
	~COM_ACK();
	
	bool unpack();  //read
	bool pack();  //write

public:
	
};

//#define CMD_COM_ACKEX						0x000A	//  11 ����Ļ�Ӧ���������յ��󣬱��뷢��һ��ack������һ���û���
class COM_ACKEX : public CPackInfo
{
protected:

public:
	COM_ACKEX(unsigned short version);
	COM_ACKEX();
	~COM_ACKEX();
	
	bool unpack();  //read
	bool pack();  //write

public:
	

public:
	unsigned short seq;		//��Ӧ�İ������

	unsigned int uin;		//ת���ı��
	unsigned int ip;		//Ҫת��ack���������û��ĵ�ַ
	unsigned short port;	//Ҫת��ack���������û��Ķ˿�
};

union  ONLINEFLAGUNION
{

	unsigned int    online_flag;
	struct STATUS
	{
		enum PH_TYPE  //�绰��¼����	
		{
			DIRECT = 0, // ֱ��
			GATEWAY,	// ����
			SIP			// SIP
		};

		enum PH_STATUS  //�绰״̬
		{
			OFFLINE = 0, // ����
			IDLE,		 // ����
			BUSY		 // ��æ
		};

		unsigned int nConnecttype : 4;   //���ӷ�ʽ
		unsigned int ch_amt : 1;        //1:�Ự�ֶ����÷�æ��0Ϊ�Զ���æ

		unsigned int nbak : 3;
		unsigned int nOnlineStatus : 4;  //����״̬
		unsigned int nVideoStatus : 1;   //��Ƶ״̬

		//ph_type �绰��¼����	
		unsigned int ph_type : 2;


		//ph_amt: 1Ϊ�Ự�ֶ����÷�æ��0Ϊ�Զ���æ
		unsigned int ph_amt : 1;

		// �绰״̬
		unsigned int ph_status : 4;
		unsigned int nbak2 : 12;
	}stStatus;
};

class COM_SEND_PING : public CPackInfo
{
public:
	COM_SEND_PING(unsigned short ver);
	~COM_SEND_PING();
	bool unpack();
	bool pack();
public:
	unsigned short seq;

	unsigned short connecttype;//��������
	unsigned int localip;  //����ip��http��������Ϊ0����
	unsigned short localport;//�����˿�
	unsigned int  onlinestatus; // ����״̬

	ONLINEFLAGUNION  o;
};

class COM_SEND_MULTIPING : public CPackInfo
{
public:
	COM_SEND_MULTIPING(unsigned short ver);
	~COM_SEND_MULTIPING();
	bool unpack();
	bool pack();
public:
	unsigned short seq;

	unsigned short	first;
	unsigned int uin;
	char nickname[MAX_USERNAME_LEN];

	unsigned int ip;
	unsigned short port;
};

//������Ϣ
class COM_SEND_MSG:public CPackInfo
{
public:
	COM_SEND_MSG(unsigned short version);
	~COM_SEND_MSG();

	bool unpack();  //read
	bool pack();  //write

	
public:
	MSG_INFO		msg;			//950;

	//���sendip sendportΪ0����ʾΪ������Ϣ
	unsigned int	sendip;			//4;
	unsigned short	sendport;		//2;

	//�ͻ��˰汾����6000
	char   strChatid[MAX_CHATID_LEN+1]; //�Ự���  �滻��ǰ��sbak
	char	strRand[MAX_WEBCLIENID_LEN+1];  	//�ÿ�Ψһ��ʾ �������汾>=3073֧��
	char	strThirdid[MAX_THIRDID_LEN+1];	//������id �������汾>=3073֧��

};

//max size : 295
class COM_ADD_FRD:public CPackInfo
{
public:
	COM_ADD_FRD(unsigned short version);
	~COM_ADD_FRD();

	bool unpack();  //read
	bool pack();  //write
	
	
public:
	unsigned short	seq;		//2
	unsigned int	ip;			//4 �����ߵĵ�ַ
	unsigned short	port;		//2	�����ߵĶ˿�

	unsigned int	senduin;			//4 �����ߵ�UIN����
	char sendname[MAX_USERNAME_LEN];	//20 �����ߵ�����
	unsigned int	recvuin;				//4 ����UIN����
	unsigned short	addtype;				//2 0��ɾ������, 1:�������� , 2 delete add me
	unsigned char cgroupid;					//��Ӻ��ѵ������id
	//û�м���֤���ͼ�����֤����֤��ϢΪ�����岻һ����ǰ�߱�ʾ�ͻ�����Ҫ��֤��
	//������֪ͨ�����˼���Ϊ����
    unsigned short	isauth;				//2 �Ƿ������֤  0  û�����ӣ�1 ����
    char	strauth[MAX_RESPONSE_LEN];	//255 ��֤��Ϣ 

	//max sze : 20 + 255 + 20 = 295
};

//�������ӵ�����
class COM_P2PCONNECTNAT:public CPackInfo
{
public :
	COM_P2PCONNECTNAT(unsigned short version);
	~COM_P2PCONNECTNAT();

	bool unpack();  //read
	bool pack();  //write

public:
	unsigned short seq;
	unsigned short type1;			//send or resp
	unsigned short type2;			//msg video audio file... 

	unsigned int senduin;
	unsigned int sendip;
	unsigned short sendport;

	unsigned int recvuin;
	unsigned int recvip;
	unsigned short recvport;

	unsigned int sendlocalip;
	unsigned short sendlocalport;

	unsigned int recvlocalip;
	unsigned short recvlocalport;
};

class CLT_GET_AP_LIST:public CPackInfo
{
public:
	CLT_GET_AP_LIST(unsigned short ver);
	~CLT_GET_AP_LIST();

	bool unpack();  //read
	bool pack();  //write

	
public:
	unsigned short	seq;		//�����յ��İ����
	
};

//#define CMD_SRV_GROUP_LIST					0x02C6 	//��ȡ��½�û��ĺ��ѷ������б�
class CLT_GET_FRIENDGROUP_LIST:public CPackInfo
{
public:
	CLT_GET_FRIENDGROUP_LIST()
	{
		m_Head.head.cmd=CMD_SRV_GROUP_LIST;
	};

	~CLT_GET_FRIENDGROUP_LIST();

	bool unpack(){return true;};  //read
	bool pack(){return true;};  //write

};

//#define CMD_SRV_LIST_FRD					0x0212 		//��ȡ��½�û��ĺ����б�
class CLT_GET_FRIEND_LIST:public CPackInfo
{
public:
	CLT_GET_FRIEND_LIST()
	{
		m_Head.head.cmd=CMD_SRV_LIST_FRD;
	};

	~CLT_GET_FRIEND_LIST();

	bool unpack(){return true;};  //read
	bool pack(){return true;};  //write

};

//#define CMD_SRV_LIST_ONLINEFRD					 0x0226  //��ȡ��½�û������ߺ����б�
class CLT_GET_ONLINEFRIEND_LIST:public CPackInfo
{
public:
	CLT_GET_ONLINEFRIEND_LIST()
	{
		m_Head.head.cmd=CMD_SRV_LIST_ONLINEFRD;
	};

	~CLT_GET_ONLINEFRIEND_LIST();

	bool unpack(){return true;};  //read
	bool pack(){return true;};  //write
};

//#define CMD_SRV_SEND_MSG_OFFLINE					 0x00DC   //��ȡ��½�û�����������
class CLT_GET_OFFLINEMSG:public CPackInfo
{
public:
	CLT_GET_OFFLINEMSG()
	{
		m_Head.head.cmd=CMD_SRV_SEND_MSG_OFFLINE;
	};

	~CLT_GET_OFFLINEMSG();

	bool unpack(){return true;};  //read
	bool pack(){return true;};  //write

};

#define MAX_CIRCLE_AP_NUM	30  //���Ȧ��AP����
//#define CMD_COM_AP_LIST					0x8100		//��ȡ�Լ�����AP�������б�
class AP_LIST:public CPackInfo
{
public:
	AP_LIST(unsigned short ver);
	~AP_LIST();

	bool unpack();  //read
	bool pack();  //write

	
public:
	unsigned short	seq;		//�����յ��İ����
	unsigned int  ap_audiovideo_ip;  //������Ƶ��������
	unsigned short  ap_audiovideo_port;

	unsigned int  ap_file_ip;	//�ļ���������
	unsigned short  ap_file_port;


	unsigned short ap_circle_num; // Ȧ�ӷ���������
	unsigned int ap_circle_ip[MAX_CIRCLE_AP_NUM];	// Ȧ�ӷ�����i��ip     	
	unsigned short ap_circle_port[MAX_CIRCLE_AP_NUM];	//Ȧ�ӷ�����i��port
	unsigned int begin_id[MAX_CIRCLE_AP_NUM];		//�÷���������Ŀ�ʼȦ�ӱ��
	unsigned int end_id[MAX_CIRCLE_AP_NUM];		//�÷�������������Ȧ�ӱ��
private:
	unsigned short max_ap_circle_num;		//Ȧ�ӷ�����������
						
		
	
};

//#define CMD_COM_SENDINVITE					0x2010	//8208	����
//#define CMD_COM_RESPINVITE					0x2020	//8224	�ظ�
class COM_INVITE:public CPackInfo
{
public:
	COM_INVITE(unsigned short version, unsigned short cmd);
	//	COM_INVITE(unsigned short version, bool binvite = true);
	~COM_INVITE();
	
	bool unpack();  //read
	bool pack();  //write
	
public:
	//	unsigned int	inviteid;	//����������û����
	unsigned int	recvuid;	//�ڻظ�ʱ������ϢΪ���͵��û������Ǹ��û��յ���Ϊ֪ͨ��Ϣ
	unsigned int	senduid;
	unsigned int	compid;
	
	//	#define	INVITE_PTPTALK					0	//����Է��Ի�
	//	#define	INVITE_MUTITALK					1	//����Է����뵽��ĳһ���ÿ͵ĶԻ�
	unsigned char	type;
	unsigned char	ucbak;
	unsigned short	usbak;
	
	//���recvuid=9999, visitorid��ʾΪ�ÿ͵�Ψһ��ʾ
	//���senduid recvuid����Ϊ9999, visitoridҲ��Ϊ�գ���ʾ�����û����뵽��ͬ�Ӵ�һ���ÿ�
	//������ÿ�����ÿͷ�����Ϣ,recvuid senduid����ͬʱΪ9999
	char			visitorid[MAX_VISITORID_LEN+2];	//26
	char			scritpflag[MAX_SCRIPTFLAG_LEN+2];	//26
	char			msg[MAX_MSG_LEN];//���븽��
	char			msgex[MAX_MSG_LEN];//���븽��
	
	unsigned char	type1;
	unsigned char	type2;
	
	unsigned char	error;		//0��ȷ��> 0����		
	unsigned char	response;	//0 �յ���  1  ͬ��  2 �ܾ� 3 ��ʱ
};

//#define CMD_EVENT_ANNOUNCEMENT:				0x7fff  //!�¼�ͨ�������� djz 2010-3-24
class COM_EVENT_ANNOUNCEMENT:public CPackInfo
{
public:
	COM_EVENT_ANNOUNCEMENT(unsigned short version);
	~COM_EVENT_ANNOUNCEMENT();
	
	bool unpack();  //read
	bool pack();  //write
	
public:
	unsigned int myuin;
	unsigned short gpid;
	
	unsigned short type;

	char msgbuf[MAX_MSG_RECVLEN+1];
};

#endif