#if !defined _COMMCLT_EM
#define _COMMCLT_EM

//�û���¼
//max size : 196
class CLT_LOGON : public CPackInfo
{
public:
	CLT_LOGON(unsigned short ver);
	virtual ~CLT_LOGON();


	bool unpack();  //read
	bool pack();  //write
 
	void SetOnlineStatus(int nStatus);

	int GetOnlineStatus();
	CLT_LOGON& operator =(CLT_LOGON& pLogon);

public:
	unsigned short	seq;					//2  ��¼���			

	unsigned char	type_loginby;			//1  0  UID; 1 tstring 2, phone, 3 mail, 50 web
	//�Ƿ��һ�ε�½�������һ�ε�½����Ҫ����������ϵ���б�
	unsigned char	type_loginflag;			//1  0  ������½��1 ��һ�ε�½
	//���ߵ�½�ɹ��� �ͷ���½�ɹ������������Ѱ�

	/*��¼��ʽ
	#define LOGIN_NORMAL			0  //������¼
	#define LOGIN_OFFLINE			1  //���ߺ��ٵ�¼
	#define LOGIN_SERVERDOWN		2  //�������������Զ���¼
	#define LOGIN_OFFLINEAUTO		3  //���ߺ��Զ��ٵ�¼
	#define LOGIN_LITE				10  //�������ߣ�����ʱmpֻ����½�ɹ���ʧ�ܰ������������������ѷ��飬�����б������б� 2010.1.12
	*/
	unsigned short  type_loginsort;	

	unsigned int 	uin;					//4  Tra�����½				4
	char			strid[MAX_SID_LEN];	//60 �ַ�����½				60
	char			pwd[MAX_PASSWORD_LEN];  //20 �û����� 

/*	  0 - 4  online status
	    5    video status
      6 - 7  bak
*/
	unsigned short	status;					//2  ����״̬ 3 hide,  2 online
	char			strstatus[MAX_STATUS_LEN];	//40 ״̬

	unsigned short	connecttype;				//2  connect type :0 udp; 1 mutiping udp; 2 proxy
	CProxyInfo		proxyinfo;					//58

	unsigned short langtype;				//2
	unsigned short terminaltype;			//2  
	unsigned short bak;                     //

	char   szRand[MAX_WEBCLIENID_LEN+1];//�ÿ�Ψһ��ʾ
	char   szThirdid[MAX_THIRDID_LEN+1];//������id

};

//#define CMD_CLT_REG							0x03FC  // 1020 �û�ע��
//max size :  1078
class CLT_REG : public CPackInfo
{
public:
	CLT_REG(unsigned short ver);
	~CLT_REG();
	
	bool unpack();  //read
	bool pack();  //write

	

public:
	unsigned short	seq;					//2 ע�����к�

	unsigned short	type_loginby;			//2  0  UID; 1 tstring 2 mobile 3 mail, ���Ϊ2��3��½���ַ�����user_info��
	char			strid[MAX_SID_LEN];	//60 �ַ�����½
	char			pwd[MAX_PASSWORD_LEN];	//20 passowrd

	unsigned short	ConnectType;			//2  connect type :0 udp; 1 mutiping udp; 2 proxy
	CProxyInfo		ProxyInfo;				//58

	CUserInfo		UserInfo;				//934  �û���Ϣ
	CTradeInfo		TradeInfo;				//0    �û�������Ϣ

};

//#define CMD_CLT_LOGOFF						0x0438  // 1080 �û�����
//�û����߷����İ�
//CMD_CLT_LOGOFF
class CLT_LOGOFF:public CPackInfo
{
public:
	CLT_LOGOFF(unsigned short ver);
	~CLT_LOGOFF();

	bool unpack();  //read
	bool pack();  //write

public:
	unsigned short seq;              //������ţ�ȱʡΪ1

	unsigned int uin;						//�����û����.

	unsigned int online_flag;				//���ߺ�������߱�־λ	
};

//#define CMD_CLT_SEARCH_FRD					0x041A  // 1050 ��UIN��ѯ
//ͨ��UIN������½��, �ʼ���ַ������
//��ѯ˭����
class CLT_SEARCH_FRD:public CPackInfo
{
public:
	CLT_SEARCH_FRD(unsigned short ver);
	~CLT_SEARCH_FRD();

	bool unpack();  //read
	bool pack();  //write

	

public:
	unsigned short	seq;
	unsigned short	type;					//0  UID; 1 tstring  2 strmobile  3 strmail

	unsigned int	uin;

	char			strid[MAX_EMAIL_LEN];	//�ַ�����½				60
};

//#define CMD_CLT_SEARCH_MUTIFRD				0x0424  // 1060 ����Ϣ��ѯ
//ͨ��������Ϣ��ѯ�û�����ϲ�ѯ�����������Ϊ�յ�)
class CLT_SEARCH_MUTIFRD:public CPackInfo
{
public:
	CLT_SEARCH_MUTIFRD(unsigned short ver);
	~CLT_SEARCH_MUTIFRD();

	bool unpack();  //read
	bool pack();  //write

	

public:
	unsigned short	seq;				//��ѯ���
	unsigned short  sort;				//��ѯ��ҳ

	char	nickname[MAX_USERNAME_LEN];	//�û�����
	char	company[MAX_COMPANY_LEN];	//��˾����

	unsigned int		deptid;		//����
	unsigned short		country;		//���� 0��ʾȫ��	
	unsigned short		province;		//ʡ�� 0��ʾȫ��
	unsigned short		sex;			//�Ա� 0 all, 1 man ,  2feman
	unsigned short		onlineuser;		//0:�����ˣ�1�������û�

	unsigned int		nextpos;		//������ѯ�ı�־ֵ

	//add at 8.23 phenix
	unsigned int agentID;
};

//#define CMD_CLT_GET_INFO					0x044C  // 1100 
//��÷�������ĳһЩ��Ϣ����(�����б���Ϣ)
class CLT_GET_INFO:public CPackInfo
{
public:
	CLT_GET_INFO(unsigned short ver, unsigned short cmd);
	CLT_GET_INFO(unsigned short ver);
	~CLT_GET_INFO();

	bool unpack();  //read
	bool pack();  //write

	
public:
	unsigned short	seq;			//��ѯ��
	unsigned short	cmd;			//���صİ�������
	unsigned short	cmdtype;		//��ѯ���ͣ��û���Ⱥ�顢���桢�����,��cmdѡһ��
	unsigned short	type;			//ÿһ�������ڲ��ķ���

	unsigned int   id;				//Ҫ��ѯ�Ķ���ı��
	char strid[MAX_SID_LEN];		


};

//#define CMD_CLT_CHECKPROXY					0x0474  // 1140 �����������
class CLT_CHECKPROXY:public CPackInfo
{
public :
	CLT_CHECKPROXY(unsigned short ver);
	~CLT_CHECKPROXY();

	bool unpack();  //read
	bool pack();  //write

public:
	unsigned short seq;
	unsigned short connecttype; //connect type :0 udp; 1 mutiping udp; 2 proxy

	CProxyInfo proxyinfo;
};

//#define CMD_CLT_TRANSFERCLIENT				0x047E  // 1150 ת����ʱ�û�
class CLT_TRANSFERCLIENT:public CPackInfo
{
public :
	CLT_TRANSFERCLIENT(unsigned short ver);
	~CLT_TRANSFERCLIENT();

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

//#define CMD_CLT_GROUP_INVITE				0x05A0	// 1440 �����������,(0��2)������
//	CMD_COM_UPDATE_FRDLIST	0x2000
class CLT_UPDATE_FRDLIST:public CPackInfo
{
public:
	CLT_UPDATE_FRDLIST(unsigned short ver);
	~CLT_UPDATE_FRDLIST();

	bool unpack();
	bool pack();

public:
	unsigned short seq;
	unsigned int uin;		//

	unsigned short num;
	unsigned int IDList[MAX_UPDATE_FRDLIST_NUM];
	char NameList[MAX_UPDATE_FRDLIST_NUM][MAX_USERNAME_LEN];
	unsigned short sex[MAX_UPDATE_FRDLIST_NUM];
};

#endif