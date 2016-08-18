#include "../stdafx.h"
#include "comcom.h"
#include "tstring.h"

//ͨѶ����ͷ��
COM_HEAD_PACK::COM_HEAD_PACK(unsigned short ver_t,
							 unsigned short sendrandom,
							 unsigned int uin_t,
							 unsigned short random_t,
							 unsigned short cmd_t,
							 unsigned short seq_t,
							 unsigned short langtype_t,
							 unsigned short seqresp_t,
							 unsigned short flag)
{
	SetValue(ver_t,	sendrandom, uin_t, random_t, cmd_t, seq_t, langtype_t, seqresp_t, flag);
}

COM_HEAD_PACK::COM_HEAD_PACK()
{
	head.ver=VERSION;
	head.sendrandom = 0;
	head.uin = 0;
	head.random = 0;
	head.cmd = 0;
	head.seq = 0;
	head.langtype = LANGUAGE_NORMAL;
	head.seqresp= 0;
	head.terminal = TERMINAL_PCEX;
}

COM_HEAD_PACK::~COM_HEAD_PACK()
{
}

void COM_HEAD_PACK::SetValue(unsigned short ver_t,
		unsigned short sendrandom,
		unsigned int uin_t, 
		unsigned short random_t,
		unsigned short cmd_t, 
		unsigned short seq_t, 
		unsigned short langtype_t,
		unsigned short seqresp_t,
		unsigned short terminal)
{
	head.ver=ver_t;
	head.sendrandom=sendrandom;
	head.uin = uin_t;
	head.random=random_t;
	head.cmd=cmd_t;
	head.seq=seq_t;
	head.langtype=langtype_t;
	head.seqresp=seqresp_t;

	head.terminal = terminal;
}

void COM_HEAD_PACK::SetValue(PACK_HEADER head)
{
	this->head = head;
}

int COM_HEAD_PACK::GetHeadLen()
{
	return sizeof(PACK_HEADER);
}

bool COM_HEAD_PACK::unpack(CPack &pack)  //read
{
	//read buff
	bool bRtn = false;

	if(!(pack >> head.ver))
		goto RETURN;
	if(!(pack >> head.sendrandom))
		goto RETURN;	
	if(!(pack >> head.uin))
		goto RETURN;
 	if(!(pack >> head.random))
		goto RETURN;
	if(!(pack >> head.cmd))
		goto RETURN;
   	if(!(pack >> head.seq))
		goto RETURN;
	if(!(pack >> head.langtype))
		goto RETURN;
	if(!(pack >> head.seqresp))
		goto RETURN;
	if(!(pack >> head.terminal))
		goto RETURN;

	bRtn = true;
RETURN:
	return bRtn;
}

bool COM_HEAD_PACK::pack(CPack &pack) 
{
	//write buff
	bool bRtn = false;
	if(!(pack << head.ver))
		goto RETURN;
	if(!(pack << head.sendrandom))
		goto RETURN;
	if(!(pack << head.uin))
		goto RETURN;
 	if(!(pack << head.random))
		goto RETURN;
	if(!(pack << head.cmd))
		goto RETURN;
   	if(!(pack << head.seq))
		goto RETURN;
	if(!(pack << head.langtype))
		goto RETURN;
	if(!(pack << head.seqresp))
		goto RETURN;
	if(!(pack << head.terminal))
		goto RETURN;

	bRtn = true;
RETURN:
	return bRtn;
}

//���а��Ļ���
CPackInfo::CPackInfo()
{
	version = VERSION;
	bSendToServer = false;
}

CPackInfo::~CPackInfo()
{
}


bool CPackInfo::unpack()
{
	//read
	return true;
}

bool CPackInfo::pack()
{
	//write
	return true;
}

bool CPackInfo::unpackhead()
{
	return m_Head.unpack(m_Pack);
}

bool CPackInfo::packhead()
{
	//write
	return m_Head.pack(m_Pack);
}

//�йش��������
CProxyInfo::CProxyInfo()
{
	this->info.ip = 0;
	info.port = 0;

	info.proxyport = 1080;

	memset(info.strUser, 0, MAX_USERNAME_LEN);
	memset(info.strPass, 0, MAX_PASSWORD_LEN);
}

CProxyInfo::~CProxyInfo()
{
}

bool CProxyInfo::unpack(CPack &pack, unsigned short ver)
{
	//read
	unsigned short len= 0;
	bool bRtn = false;
	
	if(!(pack >> info.ip))
		goto RETURN;
	if(!(pack >> info.port))
		goto RETURN;

	if(!(pack >> info.proxyport))
		goto RETURN;

	len = MAX_USERNAME_LEN;
	if(!pack.unpackString(info.strUser, len, MAX_USERNAME_LEN))
		goto RETURN;
	
	if(!pack.unpackString(info.strPass, len, MAX_PASSWORD_LEN))
		goto RETURN;

	bRtn = true;

RETURN:
	return bRtn;
}

bool CProxyInfo::pack(CPack &pack, unsigned short ver)
{
	//write
	bool bRtn = false;
	if(!(pack<<info.ip))
		goto RETURN;
	if(!(pack<<info.port))
		goto RETURN;

	if(!(pack<<info.proxyport))
		goto RETURN;

	if(!pack.packString(info.strUser,MAX_USERNAME_LEN))
		goto RETURN;

	if(!pack.packString(info.strPass,MAX_PASSWORD_LEN))
		goto RETURN;

	bRtn = true;

RETURN:
	return bRtn;
}

//�й��û�����Ϣ��Ϣ
CUserInfo::CUserInfo()
{
	this->ClearInfo();
}

CUserInfo::~CUserInfo()
{
}

bool CUserInfo::unpack(CPack &pack, unsigned short ver)
{
	//read

	return UnpackForm(pack);
	
}

bool CUserInfo::UnpackForm(CPack &pack)
{
	bool bRtn = true;
	unsigned short len = 0;
	unsigned short us;
	unsigned int ul;
//	unsigned char uc;
	char sbuff[256];

	if(!pack.unpackString(info.nickname, len, MAX_USERNAME_LEN))
		goto RETURN;
		
	if(!pack.unpackString(info.username, len, MAX_USERNAME_LEN))
		goto RETURN;
	
	pack>>us;
//	pack>>info.sex;

	pack>>ul;
//	pack>>info.birthday;

	if(!pack.unpackString(sbuff, len, MAX_USERMEMO_LEN))
//	if(!pack.unpackString(info.memo, len, MAX_USERMEMO_LEN))
		goto RETURN;

	pack>>ul;
	pack>>ul;
//	pack>>info.companyid;
//	pack>>info.departmentid;

	pack>>us;
	pack>>us;
//	pack>>info.country;
//	pack>>info.province;

	if(!pack.unpackString(sbuff, len, MAX_CITY_LEN))
//	if(!pack.unpackString(info.city, len, MAX_CITY_LEN))
		goto RETURN;
	if(!pack.unpackString(sbuff, len, MAX_ZIPCODE_LEN))
//	if(!pack.unpackString(info.zipcode,len, MAX_ZIPCODE_LEN))
		goto RETURN;
	if(!pack.unpackString(sbuff, len, MAX_PHONE_LEN))
//	if(!pack.unpackString(info.mobile,len, MAX_PHONE_LEN))
		goto RETURN;
	if(!pack.unpackString(sbuff, len, MAX_PHONE_LEN))
//	if(!pack.unpackString(info.phone,len, MAX_PHONE_LEN))
		goto RETURN;
	if(!pack.unpackString(sbuff, len, MAX_PHONE_LEN))
//	if(!pack.unpackString(info.fax,len, MAX_PHONE_LEN))
		goto RETURN;

	if(!pack.unpackString(sbuff, len, MAX_ADDRESS_LEN))
//	if(!pack.unpackString(info.address,len, MAX_ADDRESS_LEN))
		goto RETURN;

	if(!pack.unpackString(sbuff, len, MAX_URL_LEN))
//	if(!pack.unpackString(info.homepage,len, MAX_URL_LEN))
		goto RETURN;
	if(!pack.unpackString(sbuff, len, MAX_EMAIL_LEN))
//	if(!pack.unpackString(info.email,len, MAX_EMAIL_LEN))
		goto RETURN;
	if(!pack.unpackString(sbuff, len, MAX_EMAIL_LEN))
//	if(!pack.unpackString(info.email_bak,len, MAX_EMAIL_LEN))
		goto RETURN;

	pack>>ul;
	pack>>ul;
//	pack>>info.commflag;
//	pack>>info.extflag;          
	pack>>info.onlineflag;

	bRtn = true;
RETURN:
	return bRtn;
}


bool CUserInfo::pack(CPack &pack, unsigned short ver)
{
	return PackForm(pack);
}

bool CUserInfo::PackForm(CPack &pack)
{
	 //write
	bool bRtn = false;
	unsigned short us=0;
	unsigned int ul=0;
//	unsigned char uc=0;
	char sbuff[256];

	if(!pack.packString(info.nickname, MAX_USERNAME_LEN))
		goto RETURN;
	if(!pack.packString(info.username,MAX_USERNAME_LEN))
		goto RETURN;
//	pack<<info.sex;
	pack<<us;

	pack<<ul;
//	pack<<info.birthday;
	if(!pack.packString(sbuff,MAX_USERMEMO_LEN))
//	if(!pack.packString(info.memo,MAX_USERMEMO_LEN))
		goto RETURN;

	pack<<ul;
//	pack<<info.companyid;

	//�������ע�ᣬ�����޸Ĳ���
	pack<<ul;
//	pack<<info.departmentid;

	pack<<us;
	pack<<us;
//	pack<<info.country;
//	pack<<info.province;
	
	if(!pack.packString(sbuff,MAX_CITY_LEN))
//	if(!pack.packString(info.city,MAX_CITY_LEN))
		goto RETURN;
	if(!pack.packString(sbuff,MAX_ZIPCODE_LEN))
//	if(!pack.packString(info.zipcode,MAX_ZIPCODE_LEN))
		goto RETURN;
	if(!pack.packString(sbuff,MAX_PHONE_LEN))
//	if(!pack.packString(info.mobile,MAX_PHONE_LEN))
		goto RETURN;
	if(!pack.packString(sbuff,MAX_PHONE_LEN))
//	if(!pack.packString(info.phone, MAX_PHONE_LEN))
		goto RETURN;
	if(!pack.packString(sbuff,MAX_PHONE_LEN))
//	if(!pack.packString(info.fax,MAX_PHONE_LEN))
		goto RETURN;

	if(!pack.packString(sbuff,MAX_ADDRESS_LEN))
//	if(!pack.packString(info.address,MAX_ADDRESS_LEN))
		goto RETURN;
	if(!pack.packString(sbuff,MAX_URL_LEN))
//	if(!pack.packString(info.homepage, MAX_URL_LEN))
		goto RETURN;
	if(!pack.packString(sbuff,MAX_EMAIL_LEN))
//	if(!pack.packString(info.email,MAX_EMAIL_LEN))
		goto RETURN;
	if(!pack.packString(sbuff,MAX_EMAIL_LEN))
//	if(!pack.packString(info.email_bak, MAX_EMAIL_LEN))
		goto RETURN;

	pack<<ul;
	pack<<ul;
	pack<<ul;
//	pack<<info.commflag;
//	pack<<info.extflag;
//	pack<<info.onlineflag;

	bRtn = true;

RETURN:
	return bRtn;
}


void CUserInfo::ClearInfo()
{
	//������Ϣ
	memset(&info, '\0', sizeof(info));
}

void CUserInfo::CopyInfo(CUserInfo *pInfo)
{
	if(pInfo == NULL)
		return;

	memcpy(&info, pInfo, sizeof(info));
}

//�й��û���������Ϣ�����ݾɰ汾
CTradeInfo::CTradeInfo()
{
	this->ClearInfo();
}

CTradeInfo::~CTradeInfo()
{
}

bool CTradeInfo::unpack(CPack &pack,unsigned short ver)
{
	bool bRtn = false;
	unsigned short len = 0;

	if(!pack.unpackString(info.company, len, MAX_COMPANY_LEN))
		goto RETURN;
	if(!pack.unpackString(info.leader,len,MAX_USERNAME_LEN))
		goto RETURN;
	
	pack>>info.duty;
	pack>>info.size;
	pack>>info.trade;

	if(!pack.unpackString(info.bankcode,len,MAX_BANK_LEN))
		goto RETURN;
	if(!pack.unpackString(info.bank_num,len,MAX_BANKNO_LEN))
		goto RETURN;
	if(!pack.unpackString(info.tax_num,len,MAX_PHONE_LEN))
		goto RETURN;

	if(!pack.unpackString(info.mobile,len,MAX_PHONE_LEN))
		goto RETURN;
	if(!pack.unpackString(info.phone,len,MAX_PHONE_LEN))
		goto RETURN;
	if(!pack.unpackString(info.fax,len,MAX_PHONE_LEN))
		goto RETURN;
	if(!pack.unpackString(info.homepage,len,MAX_URL_LEN))
		goto RETURN;
	if(!pack.unpackString(info.mailbox,len,MAX_EMAIL_LEN))
		goto RETURN;

	if(!pack.unpackString(info.bak1,len,MAX_USERNAME_LEN))
		goto RETURN;

	pack >> info.langtype;
	pack >> info.companylangtype;
	if(!pack.unpackString(info.memo,len,MAX_TRADEMEMO_LEN))
		goto RETURN;

	bRtn = true;
RETURN:
	return bRtn;
}


bool CTradeInfo::pack(CPack &pack, unsigned short ver)
{
	 //write
	bool bRtn = false;

	if(!pack.packString(info.company, MAX_COMPANY_LEN))
		goto RETURN;
	if(!pack.packString(info.leader,MAX_USERNAME_LEN))
		goto RETURN;
	
	pack<<info.duty;
	pack<<info.size;
	pack<<info.trade;

	if(!pack.packString(info.bankcode,MAX_BANK_LEN))
		goto RETURN;
	if(!pack.packString(info.bank_num, MAX_BANKNO_LEN))
		goto RETURN;
	if(!pack.packString(info.tax_num,MAX_PHONE_LEN))
		goto RETURN;

	if(!pack.packString(info.mobile,MAX_PHONE_LEN))
		goto RETURN;
	if(!pack.packString(info.phone,MAX_PHONE_LEN))
		goto RETURN;
	if(!pack.packString(info.fax,MAX_PHONE_LEN))
		goto RETURN;
	if(!pack.packString(info.homepage,MAX_URL_LEN))
		goto RETURN;
	if(!pack.packString(info.mailbox,MAX_EMAIL_LEN))
		goto RETURN;

	if(!pack.packString(info.bak1,MAX_USERNAME_LEN))
		goto RETURN;
	pack << info.langtype;
	pack << info.companylangtype;
	if(!pack.packString(info.memo,MAX_TRADEMEMO_LEN))
		goto RETURN;

	bRtn = true;
RETURN:
	return bRtn;
}


void CTradeInfo::ClearInfo()
{		
	memset(info.company, 0, MAX_COMPANY_LEN);//��˾����   40
	memset(info.leader, 0, MAX_USERNAME_LEN);

	info.duty = 0;									//ְ������   2
	info.size = 0;									//��˾��ģ   2
	info.trade = 0;									//������ҵ   2
					
	memset(info.bankcode, 0, MAX_BANK_LEN);			//��������	40		
	memset(info.bank_num, 0, MAX_BANKNO_LEN);		//�����˺�	30				
	memset(info.tax_num, 0, MAX_TAXNO_LEN);			//˰�����	30				
	memset(info.mobile, 0, MAX_PHONE_LEN);			//�绰		δ��					
	memset(info.phone, 0, MAX_PHONE_LEN);			//�绰		20				
	memset(info.fax, 0, MAX_PHONE_LEN);				//����		20				
	memset(info.homepage, 0, MAX_URL_LEN);			//��ҳ����  60				
	memset(info.mailbox, 0, MAX_EMAIL_LEN);			//			60			
	memset(info.bak1, 0, MAX_VERIFYMSG_LEN);		//			40
	info.langtype = LANGUAGE_NORMAL;				//������Ϣ�����԰汾
	info.companylangtype = LANGUAGE_NORMAL;			//			4				
	memset(info.memo, 0, MAX_TRADEMEMO_LEN);		//��˾���  500
}

void CTradeInfo::CopyInfo(CTradeInfo *pInfo)
{
	if(pInfo == NULL)
		return;

	strcpy(info.company, pInfo->info.company);				//��˾����   40
	strcpy(info.leader, pInfo->info.leader);				//����		 δ��(30)

	info.duty = pInfo->info.duty;							//ְ������   2
	info.size = pInfo->info.size;							//��˾��ģ   2
	info.trade = pInfo->info.trade;							//������ҵ   2

	strcpy(info.bankcode, pInfo->info.bankcode);			//��������	40		   
	strcpy(info.bank_num, pInfo->info.bank_num);			//�����˺�	30
	strcpy(info.tax_num, pInfo->info.tax_num);				//˰�����	30
	strcpy(info.mobile, pInfo->info.mobile);				//�绰		δ��
	strcpy(info.phone, pInfo->info.phone);					//�绰		20
	strcpy(info.fax, pInfo->info.fax);						//����		20
	strcpy(info.homepage, pInfo->info.homepage);			//��ҳ����  60
	strcpy(info.mailbox, pInfo->info.mailbox);				//			60
	strcpy(info.bak1, pInfo->info.bak1);					//			40
	info.langtype = pInfo->info.langtype;					//������Ϣ�����԰汾
	info.companylangtype = pInfo->info.companylangtype;		//			4
	strcpy(info.memo, pInfo->info.memo);					//��˾���  500
}

//ͨ��������Ϣ�������ߵ�������һ�������ͣ���������û���������Ϣ��
//max size : 42(new) 62(old) 
FRD_COMMON_INFO::FRD_COMMON_INFO()
{
	ClearInfo();
}

FRD_COMMON_INFO::~FRD_COMMON_INFO()
{
	;
}

bool FRD_COMMON_INFO::unpack(CPack &pack, unsigned short ver)
{
	bool bRtn = false;
	unsigned short len = 0;

	//read
	if(!(pack>>uin))
		goto RETURN;


	if(!pack.unpackString(mobile,len,MAX_PHONE_LEN))
		goto RETURN;

	if(!pack.unpackString(nickname,len,MAX_USERNAME_LEN))
		goto RETURN;

	if(!(pack>>deptid))
		goto RETURN;
	if(!(pack>>externid))
		goto RETURN;

	if(!(pack>>groupid))
		goto RETURN;
	if(!(pack>>iconid))
		goto RETURN;

	if(!(pack>>sex))
		goto RETURN;
	if(!(pack>>bak))
		goto RETURN;

	if(!(pack>>langtype))
		goto RETURN;

	if(!(pack>>cgroupid))
		goto RETURN;

	bRtn = true;
RETURN:
	return bRtn;
}

bool FRD_COMMON_INFO::pack(CPack &pack, unsigned short ver)
{
	//write
	bool bRtn = false;

	if(!(pack << uin))
		goto RETURN;


	if(!pack.packString(mobile, MAX_PHONE_LEN))
		goto RETURN;

	if(!pack.packString(nickname, MAX_USERNAME_LEN))
		goto RETURN;

	if(!(pack << deptid))
		goto RETURN;
	if(!(pack << externid))
		goto RETURN;

	if(!(pack << groupid))
		goto RETURN;
	if(!(pack << iconid))
		goto RETURN;

	if(!(pack << sex))
		goto RETURN;
	if(!(pack << bak))
		goto RETURN;

	if(!(pack << langtype))
		goto RETURN;
	if(!(pack << cgroupid))
		goto RETURN;

	bRtn = true;
RETURN:
	return bRtn;
}

bool FRD_COMMON_INFO::ConvertGBToBIG5(bool bGBToBig5)
{
	return true;
}

void FRD_COMMON_INFO::ClearInfo()
{
	uin = 0;					//4 �û���UIN��

	strcpy(mobile, "");			//20 ���У��°�δ�ã�
	strcpy(nickname, "");		//30 ����

	deptid = 0;						//4
	externid = 0;						//2 ����

//	16 - 23 ���ID
	groupid = 0;					//1
//	24 - 31 ͼ����
	iconid = 0;						//1

	bak = 0;						//1	����
	sex = 0;						//1

	langtype = LANGUAGE_NORMAL;					//2
	cgroupid=0;
}

void FRD_COMMON_INFO::CopyInfo(FRD_COMMON_INFO *pInfo)
{
	if(pInfo == NULL)
		return;

	this->uin = pInfo->uin;

	strcpy(this->mobile, pInfo->mobile);
	strcpy(this->nickname, pInfo->nickname);		//30 ����

	this->deptid = pInfo->deptid;						//4
	this->externid = pInfo->externid;

//	16 - 23 ���ID
	this->groupid = pInfo->groupid;					//1
//	24 - 31 ͼ����
	this->iconid = pInfo->iconid;						//1
	this->bak = pInfo->bak;						//1	����

	this->sex = pInfo->sex;						//1

	this->langtype =  pInfo->langtype;
	cgroupid= pInfo->cgroupid;
}

//���ߵ�������Ϣ
//max size : 20(new) 40(new ex) 60(old) 
FRD_ONLINE_INFO::FRD_ONLINE_INFO()
{
	this->ClearInfo();
}

FRD_ONLINE_INFO::~FRD_ONLINE_INFO()
{
}



bool FRD_ONLINE_INFO::unpack(CPack &pack, unsigned short ver)
{
	bool bRtn = false;
	unsigned short len = 0;

	if(!(pack >> uin))
		goto RETURN;


	if(!pack.unpackString(nickname,len,30))
		goto RETURN;

	if(!(pack >> ip))
		goto RETURN;
	if(!(pack >> port))
		goto RETURN;

	if(!(pack >> version)) //��ǰ���ѵİ汾
		goto RETURN;
	if(!(pack >> langtype))
		goto RETURN;

	if(!(pack >> onlinestatus))
		goto RETURN;
	if(!pack.unpackString(strstatus, len, 40))
		goto RETURN;

	if (VERSION>=4213)
	{//�Լ��İ汾

		if(!(pack >> terminaltype))
			goto RETURN;
	}

	bRtn = true;
RETURN:
	return bRtn;
}

bool FRD_ONLINE_INFO::pack(CPack &pack, unsigned short ver)
{
	//write
	bool bRtn = true;
	unsigned short len = 0;

	if(!(pack << uin))
		goto RETURN;

	if(!pack.packString(nickname, MAX_USERNAME_LEN))
		goto RETURN;

	if(!(pack << ip))
		goto RETURN;
	if(!(pack << port))
		goto RETURN;

	if(!(pack << version))
		goto RETURN;
	if(!(pack << langtype))
		goto RETURN;

	if(!(pack << onlinestatus))
		goto RETURN;
	if(!pack.packString(strstatus, MAX_STATUS_LEN))
		goto RETURN;

	if (VERSION>=4213)
	{//�Լ��汾
		if(!(pack << terminaltype))
			goto RETURN;
	}

	bRtn = true;
RETURN:
	return bRtn;
}

void FRD_ONLINE_INFO::ClearInfo()
{
	uin = 0;					//4 �û���UIN��

	strcpy(nickname, "");		//30 ����

	ip = 0;						//4
	port = 0;						//2 ����

	version = VERSION;					//2
	langtype = LANGUAGE_NORMAL;					//2

	onlinestatus = 0;						//4
	strcpy(strstatus, "");
}

void FRD_ONLINE_INFO::CopyInfo(FRD_ONLINE_INFO *pInfo)
{
	if(pInfo == NULL)
		return;

	this->uin = pInfo->uin;
	strcpy(this->nickname, pInfo->nickname);		//30 ����

	this->ip = pInfo->ip;						//4
	this->port = pInfo->port;

	this->version = pInfo->version;
	this->langtype = langtype;

	this->onlinestatus = pInfo->onlinestatus;						//1
	strcpy(this->strstatus, pInfo->strstatus);
	strcpy(this->strcompany,pInfo->strcompany);						//1	����
}

//��ѯ����������Ϣ
FRD_QUERY_INFO::FRD_QUERY_INFO()
{
	this->ClearInfo();
}

FRD_QUERY_INFO::~FRD_QUERY_INFO()
{
}


bool FRD_QUERY_INFO::unpack(CPack &pack, unsigned short ver)
{
	//read
	bool bRtn = false;
	unsigned short len = 0;

	if(!(pack>>uin))
		goto RETURN;

	if(!(pack.unpackString(nickname,len,MAX_USERNAME_LEN)))
		goto RETURN;

	if(!(pack.unpackString(email,len,MAX_EMAIL_LEN)))
		goto RETURN;

	if(!(pack.unpackString(company,len, MAX_COMPANY_LEN)))
		goto RETURN;

	if(!(pack >> deptid))
		goto RETURN;
	if(!(pack>>country))
		goto RETURN;
	if(!(pack>>province))
		goto RETURN;

	if(	!(pack.unpackString(city,len,MAX_CITY_LEN)))
		goto RETURN;
	
	if(!(pack >> commstatus))
		goto RETURN;
	if(!(pack >> slavestatus))
		goto RETURN;
	if(!(pack >> onlinestatus))
		goto RETURN;

	if(!(pack >> sex))
		goto RETURN;
	if(!(pack >> langtype))
		goto RETURN;

	bRtn = true;
RETURN:
	return bRtn;
}

bool FRD_QUERY_INFO::pack(CPack &pack, unsigned short ver)
{
	//read
	bool bRtn = false;
	unsigned short len = 0;

	if(!(pack << uin))
		goto RETURN;

	if(!(pack.packString(nickname, MAX_USERNAME_LEN)))
		goto RETURN;


	if(!(pack.packString(email,MAX_EMAIL_LEN)))
		goto RETURN;

	if(!(pack.packString(company,MAX_COMPANY_LEN)))
		goto RETURN;

	if(!(pack << deptid))
		goto RETURN;
	
	
	if(!(pack << country))
		goto RETURN;
	if(!(pack << province))
		goto RETURN;
	
	if(	!pack.packString(city,MAX_CITY_LEN))
		goto RETURN;
	
	if(!(pack << commstatus))
		goto RETURN;
	if(!(pack << slavestatus))
		goto RETURN;
	if(!(pack << onlinestatus))
		goto RETURN;
	
	if(!(pack << sex))
		goto RETURN;
//	if(!(pack << online))
//		goto RETURN;
	if(!(pack << langtype))
		goto RETURN;

	bRtn = true;
RETURN:
	return bRtn;
}

void FRD_QUERY_INFO::ClearInfo()
{
	uin = 0;					//4 �û���UIN��

	strcpy(nickname, "");		//30 ����
	strcpy(email, "");
	strcpy(company, "");

	deptid = 0;						//4
	country = 0;						//2 ����
	province = 0;

	strcpy(city, "");
	commstatus = 0;
	slavestatus = 0;
	onlinestatus = 0;

	sex = 0;						//4
//	online = STATUS_OFFLINE;
	langtype = LANGUAGE_NORMAL;					//2
}

void FRD_QUERY_INFO::CopyInfo(FRD_QUERY_INFO *pInfo)
{
	if(pInfo == NULL)
		return;

	this->uin = pInfo->uin;
	strcpy(this->nickname, pInfo->nickname);
	strcpy(this->email, pInfo->email);
	strcpy(this->company, pInfo->company);

	this->deptid = pInfo->deptid;
	this->country = pInfo->country;
	this->province = pInfo->province;

	strcpy(this->city, pInfo->city);
	this->commstatus = pInfo->commstatus;
	this->slavestatus = pInfo->slavestatus;
	this->onlinestatus = pInfo->onlinestatus;

	this->sex = pInfo->sex;
//	this->online = pInfo->online;//

	this->langtype = pInfo->langtype;
}

//��Ϣ��
MSG_INFO::MSG_INFO()
{
	langtype = LANGUAGE_NORMAL;

	msgtype = 0;			//1 ��Ϣ���
///	msgformat = MSG_TXT;			//1 ��Ϣ��ʽ
	recvuin = 0;					//4 �����ߵ�UIN����
	senduin = 0;					//4 �����ߵ�UIN����  
	sendtime = 0;					//4 ����ʱ��
	isread = 0;						//2 �Ƿ񱻶���
	strcpy(strmsg, "");		//910 ��Ϣ����
	strcpy(strmobile, "");	//20 �ֻ�����, ������Ϣʹ�ã��û�������Ϣʱ��������

	strfontinfo[0]='\0';
	seq = 0; 
	bak = 0;
}

MSG_INFO::~MSG_INFO()
{
}


bool MSG_INFO::unpack(CPack &pack, unsigned short ver)
{
	//read
	bool bRtn = false;
	unsigned short len = 0;

	if(!(pack >> msgtype))
		goto RETURN;

	if(!(pack >> recvuin))
		goto RETURN;
	if(!(pack >> senduin))
		goto RETURN;

	if(!(pack >> sendtime))
		goto RETURN;
	if(!(pack >> isread))
		goto RETURN;

	if(!pack.unpackString(strmsg,len, MAX_MSG_RECVLEN+1))
		goto RETURN;
	strmsg[MAX_MSG_RECVLEN] = '\0';
	if(!pack.unpackString(strmobile,len,MAX_MSGNICKNAME_LEN+1))
		goto RETURN;

	if(ver > 2700)
	{
		if(!(pack >> langtype))
			goto RETURN;

		if(!(pack >> seq))
			goto RETURN;

		if(!(pack >> bak))
			goto RETURN;
	}

	if (ver>=4212)
	{
		if(!pack.unpackString(strfontinfo,len,MAX_MSGFONT_LEN+1))
			goto RETURN;

	}else
	{
		strfontinfo[0]='\0';
	}

	bRtn = true;
RETURN:
	return bRtn;
}

bool MSG_INFO::pack(CPack &pack, unsigned short ver)
{
	//write
	bool bRtn = false;


	if(!(pack << msgtype))
		goto RETURN;

	if(!(pack << recvuin))
		goto RETURN;
	if(!(pack << senduin))
		goto RETURN;

	if(!(pack << sendtime))
		goto RETURN;
	if(!(pack << isread))
		goto RETURN;
	
	if (ver>=4212)
	{
		//utf8����
		if(!pack.packString(strmsg,MAX_MSG_SENDLEN))
			goto RETURN;
		if(!pack.packString(strmobile,MAX_MSGNICKNAME_LEN))
			goto RETURN;
	}else
	{
		// ��ϰ���ͷlangtype��Ϊ0�������ϰ汾,
		if(!pack.packString(strmsg,MAX_MSG_SENDLEN*2))
			goto RETURN;
		
		if(!pack.packString(strmobile,MAX_MSGNICKNAME_LEN*2))
			goto RETURN;

/*		string sansi=std::UnicodeToAnsi(strmsg);
		if(!pack.packString(sansi.c_str(),MAX_MSG_SENDLEN*2))
			goto RETURN;

		sansi=std::UnicodeToAnsi(strmobile);
		if(!pack.packString(sansi.c_str(),MAX_MSGNICKNAME_LEN*2))
			goto RETURN;
*/
	}


	if(ver > 2700)
	{
		if(!(pack << langtype))
			goto RETURN;


		if(!(pack << seq))
			goto RETURN;

		if(!(pack << bak))
			goto RETURN;
	}
	if (ver>=4212)
	{
		if(!pack.packString(strfontinfo,MAX_MSGFONT_LEN+1))
			goto RETURN;

	}

	bRtn = true;
RETURN:
	return bRtn;
}

//�й�һ���顢���š��û���ͨ��������Ϣ
//max size : 32
NAMEID_INFO::NAMEID_INFO()
{
	id = 0;
	strcpy(name, "");

}

NAMEID_INFO::~NAMEID_INFO()
{
}


//�յ������ȷ�ϰ�
COM_ACK::COM_ACK(unsigned short version)
{
	this->version = version;
	bClient = true;
}

COM_ACK::COM_ACK()
{
	bClient = true;
}

COM_ACK::~COM_ACK()
{
}


bool COM_ACK::unpack()
{
	//read
	return true;
}

bool COM_ACK::pack()
{
	//write
	return true;
}

//����Ļ�Ӧ���������յ��󣬱��뷢��һ��ack������һ���û���
COM_ACKEX::COM_ACKEX(unsigned short version)
{
	this->version = version;

	seq = 0;		//��Ӧ�İ������
	uin = 0;

	ip = 0;		//Ҫת��ack���������û��ĵ�ַ
	port = 0;	//Ҫת��ack���������û��Ķ˿�

	this->m_Head.head.cmd = CMD_COM_ACKEX;
}

COM_ACKEX::COM_ACKEX()
{
	this->version = VERSION;

	seq = 0;		//��Ӧ�İ������
	uin = 0;

	ip = 0;		//Ҫת��ack���������û��ĵ�ַ
	port = 0;	//Ҫת��ack���������û��Ķ˿�

	this->m_Head.head.cmd = CMD_COM_ACKEX;
}

COM_ACKEX::~COM_ACKEX()
{
}


bool COM_ACKEX::unpack()
{
	//read
	bool bRtn = false;

	if(!(m_Pack >> seq))
		goto RETURN;
	if(!(m_Pack >> uin))
		goto RETURN;
	if(!(m_Pack >> ip))
		goto RETURN;	
	if(!(m_Pack >> port))
		goto RETURN;

	bRtn = true;
RETURN:
	return bRtn;
}

bool COM_ACKEX::pack()
{
	//write
	bool bRtn = false;

	if(!(m_Pack << seq))
		goto RETURN;
	if(!(m_Pack << uin))
		goto RETURN;
	if(!(m_Pack << ip))
		goto RETURN;	
	if(!(m_Pack << port))
		goto RETURN;

	bRtn = true;
RETURN:
	return bRtn;
}

//#define CMD_COM_SEND_MSG					0x0F00	// 3840 �û���������
//������Ϣ
COM_SEND_MSG::COM_SEND_MSG(unsigned short version)
{
	this->version = version;

	sendip = 0;
	sendport = 0;

	memset(strChatid,0,sizeof(strChatid));
	memset(strRand,0,sizeof(strRand));
	memset(strThirdid,0,sizeof(strThirdid));

	this->m_Head.head.cmd = CMD_COM_SEND_MSG;
}

COM_SEND_MSG::~COM_SEND_MSG()
{
}

bool COM_SEND_MSG::unpack()
{
	//read
	bool bRtn = false;
	
	if(!(msg.unpack(m_Pack, version)))
		return false;

	if(version >= 2602)
	{
		if(!(m_Pack >> sendip))
			return false;
		if(!(m_Pack >> sendport)) 
			return false;
	}
	if (version>=6000)
	{
		unsigned short len = 0;
		m_Pack.unpackString(strChatid,len,MAX_CHATID_LEN);
		m_Pack.unpackString(strRand,len,MAX_WEBCLIENID_LEN);
		m_Pack.unpackString(strThirdid,len,MAX_THIRDID_LEN);
	}
	return true;
}

bool COM_SEND_MSG::pack()
{
	//write
	bool bRtn = false;

	if(!(msg.pack(m_Pack, version)))
		goto RETURN;

	if(version >= 2602)
	{
		if(!(m_Pack << sendip))
			goto RETURN;
		if(!(m_Pack << sendport)) 
			goto RETURN;
	}
	if (version>=6000)
	{
		if(!(m_Pack.packString(strChatid,MAX_CHATID_LEN)))
			goto RETURN;
		
		if(!(m_Pack.packString(strRand,MAX_WEBCLIENID_LEN)))
			goto RETURN;
		
		if(!(m_Pack.packString(strThirdid,MAX_THIRDID_LEN)))
			goto RETURN;
	}
	bRtn = true;
RETURN:
	return bRtn;
}

//�������ӵ�����
//#define CMD_COM_P2PCONNECTNAT				0x1388  // 5000	�������ӵĲ�������
COM_P2PCONNECTNAT::COM_P2PCONNECTNAT(unsigned short version)
{
	this->version = version;

	seq = 0;
	type1 = P2PCONNECTNAT_SEND;
	type2 = P2PCONNECTNAT_NORMAL;

	this->sendip = 0;
	this->sendport = 0;
	this->senduin = 0;

	this->recvip = 0;
	this->recvport = 0;
	this->recvuin = 0;

	this->sendlocalip = 0;
	this->sendlocalport = 0;

	this->recvlocalip = 0;
	this->recvlocalport = 0;

	this->m_Head.head.cmd = CMD_COM_P2PCONNECTNAT;
}

COM_P2PCONNECTNAT::~COM_P2PCONNECTNAT()
{
}

bool COM_P2PCONNECTNAT::unpack()
{
	//read
	bool bRtn = false;

	if(!(m_Pack >> seq))
		goto RETURN;
	if(!(m_Pack >> type1))
		goto RETURN;
	if(!(m_Pack >> type2))
		goto RETURN;

	if(!(m_Pack >> senduin))
		goto RETURN;
	if(!(m_Pack >> sendip))
		goto RETURN;
	if(!(m_Pack >> sendport))
		goto RETURN;

	if(!(m_Pack >> recvuin))
		goto RETURN;
	if(!(m_Pack >> recvip))
		goto RETURN;
	if(!(m_Pack >> recvport))
		goto RETURN;

	if(!(m_Pack >> sendlocalip))
		goto RETURN;
	if(!(m_Pack >> sendlocalport))
		goto RETURN;

	if(!(m_Pack >> recvlocalip))
		goto RETURN;
	if(!(m_Pack >> recvlocalport))
		goto RETURN;
	
	bRtn = true;
RETURN:
	return bRtn;
}
	
bool COM_P2PCONNECTNAT::pack()
{
	//write
	bool bRtn = false;

	if(!(m_Pack << seq))
		goto RETURN;
	if(!(m_Pack << type1))
		goto RETURN;
	if(!(m_Pack << type2))
		goto RETURN;

	if(!(m_Pack << senduin))
		goto RETURN;
	if(!(m_Pack << sendip))
		goto RETURN;
	if(!(m_Pack << sendport))
		goto RETURN;

	if(!(m_Pack << recvuin))
		goto RETURN;
	if(!(m_Pack << recvip))
		goto RETURN;
	if(!(m_Pack << recvport))
		goto RETURN;

	if(!(m_Pack << sendlocalip))
		goto RETURN;
	if(!(m_Pack << sendlocalport))
		goto RETURN;

	if(!(m_Pack << recvlocalip))
		goto RETURN;
	if(!(m_Pack << recvlocalport))
		goto RETURN;
	
	bRtn = true;
RETURN:
	return bRtn;
}


COM_SEND_PING::COM_SEND_PING(unsigned short ver)
{
	seq = 0;
	
	connecttype=0xffff;
	localip = 0;
	localport = 0;
	version=ver;
	o.online_flag = 0;
	this->m_Head.head.cmd = CMD_COM_PING;
}

COM_SEND_PING::~COM_SEND_PING()
{}

bool COM_SEND_PING::unpack()
{
	bool bRet = false;
	unsigned short len = 0;
	if(!(m_Pack >> seq))
		goto RET;
	if(!(m_Pack >> connecttype))
		goto RET;
	if(!(m_Pack >> localip))
		goto RET;
	if(!(m_Pack >> localport))
		goto RET;


	if ( !(m_Pack >> onlinestatus) )
		goto RET;

	m_Pack >> o.online_flag;

	bRet = true;
RET:
	return bRet;
}

bool COM_SEND_PING::pack()
{
	bool bRet = false;
	unsigned short len = 0;
	if(!(m_Pack << seq))
		goto RET;
	if(!(m_Pack << connecttype))
		goto RET;
	if(!(m_Pack << localip))
		goto RET;
	if(!(m_Pack << localport))
		goto RET;
	if(!(m_Pack << onlinestatus))
		goto RET;
	bRet = true;

	if (!(m_Pack << o.online_flag))
		goto RET;
	bRet = true;

RET:
	return bRet;
}


COM_SEND_MULTIPING::COM_SEND_MULTIPING(unsigned short ver)
{
	seq = 0;

	first = 0;
	uin = 0;
	strcpy(nickname, "");

	ip = 0;
	port = 0;

	this->m_Head.head.cmd = CMD_COM_MULTIPING;
}

COM_SEND_MULTIPING::~COM_SEND_MULTIPING()
{
}

bool COM_SEND_MULTIPING::unpack()
{
	bool bRet = false;
	unsigned short len = 0;
	if(!(m_Pack >> seq))
		goto RET;
	if(!(m_Pack >> first))
		goto RET;
	if(!(m_Pack >> uin))
		goto RET;
	if(!(m_Pack.unpackString(nickname, len, MAX_USERNAME_LEN)))
		goto RET;
	if(!(m_Pack >> ip))
		goto RET;
	if(!(m_Pack >> port))
		goto RET;
	bRet = true;
RET:
	return bRet;
}

bool COM_SEND_MULTIPING::pack()
{
	bool bRet = false;
	unsigned short len = 0;
	if(!(m_Pack << seq))
		goto RET;
	if(!(m_Pack << first))
		goto RET;
	if(!(m_Pack << uin))
		goto RET;
	if(!(m_Pack.packString(nickname,MAX_USERNAME_LEN)))
		goto RET;
	if(!(m_Pack << ip))
		goto RET;
	if(!(m_Pack << port))
		goto RET;
	bRet = true;
RET:
	return bRet;
}


CLT_GET_AP_LIST::CLT_GET_AP_LIST(unsigned short version)
{
	seq = 0;
	this->m_Head.head.cmd = CMD_COM_AP_LIST;
}
CLT_GET_AP_LIST::~CLT_GET_AP_LIST()
{
}

bool CLT_GET_AP_LIST::pack()
{
	bool bRtn = false;
	if(!(m_Pack << this->seq))
		goto RETURN;
	bRtn = true;
RETURN:
	return bRtn;

}
bool CLT_GET_AP_LIST::unpack()
{
	bool bRtn = false;
	if(!(m_Pack >> seq))
		goto RETURN;
	bRtn = true;
RETURN:
	return bRtn;
}

AP_LIST::AP_LIST(unsigned short ver)
{
	seq = 0;
	this->m_Head.head.cmd = CMD_COM_AP_LIST;

	ap_audiovideo_ip=0;
	ap_audiovideo_port=0;
	
	ap_file_ip=0;
	ap_file_port=0;

	ap_circle_num=0;
	max_ap_circle_num=MAX_CIRCLE_AP_NUM;
	int i;
	for(i=0;i<max_ap_circle_num;i++)
	{
		ap_circle_ip[i]=0;
		ap_circle_port[i]=0;
		begin_id[i]=0;
		end_id[i]=0;
	}

}
AP_LIST::~AP_LIST()
{
}
bool AP_LIST::unpack()
{
	bool bRtn = false;
	
	if(!(m_Pack >> seq))
		goto RETURN;

	if(!(m_Pack>>ap_audiovideo_ip))
		goto RETURN;
	if(!(m_Pack>>ap_audiovideo_port))
		goto RETURN;

	if(!(m_Pack>>ap_file_ip))
		goto RETURN;
	if(!(m_Pack>>ap_file_port))
		goto RETURN;

	if(!(m_Pack>>ap_circle_num))
		goto RETURN;
	if(ap_circle_num>max_ap_circle_num)
		ap_circle_num=max_ap_circle_num;
	int i;
	for(i=0;i<ap_circle_num;i++)
	{
		if(!(m_Pack>>ap_circle_ip[i]))
			goto RETURN;
		if(!(m_Pack>>ap_circle_port[i]))
			goto RETURN;
		if(!(m_Pack>>begin_id[i]))
			goto RETURN;
		if(!(m_Pack>>end_id[i]))
			goto RETURN;
	}

	bRtn = true;
RETURN:
	return bRtn;
}

bool AP_LIST::pack()
{
	bool bRtn = false;
	
	if(!(m_Pack << seq))
		goto RETURN;

	if(!(m_Pack<<ap_audiovideo_ip))
		goto RETURN;
	if(!(m_Pack<<ap_audiovideo_port))
		goto RETURN;

	if(!(m_Pack<<ap_file_ip))
		goto RETURN;
	if(!(m_Pack<<ap_file_port))
		goto RETURN;

	if(!(m_Pack<<ap_circle_num))
		goto RETURN;
	if(ap_circle_num>max_ap_circle_num)
		ap_circle_num=max_ap_circle_num;
	int i;
	for(i=0;i<ap_circle_num;i++)
	{
		if(!(m_Pack<<ap_circle_ip[i]))
			goto RETURN;
		if(!(m_Pack<<ap_circle_port[i]))
			goto RETURN;
		if(!(m_Pack<<begin_id[i]))
			goto RETURN;
		if(!(m_Pack<<end_id[i]))
			goto RETURN;
	}

	bRtn = true;
RETURN:
	return bRtn;

}


//#define CMD_COM_SENDINVITE					0x2010	//8208	����
//#define CMD_COM_RESPINVITE					0x2020	//8224	�ظ�
COM_INVITE::COM_INVITE(unsigned short version, unsigned short cmd)
//COM_INVITE::COM_INVITE(unsigned short version, bool binvite)
{
	this->m_Head.head.cmd = cmd;

	type = 0;
	ucbak = 0;
	usbak = 0;

//	inviteid = 0;
	senduid = 0;
	recvuid = 0;
	compid = 0;

	strcpy(visitorid, "");
	strcpy(msg, "");
	strcpy(msgex, "");

	type1 = 0;
	type2 = 0;
	error = 0;
	response = 0;
}

COM_INVITE::~COM_INVITE()
{
}

bool COM_INVITE::unpack()
{
	bool bRtn = false;
	unsigned short len = 0;

	if(!(m_Pack >> recvuid))
		goto RETURN;
	if(!(m_Pack >> senduid))
		goto RETURN;
	if(!(m_Pack >> compid))
		goto RETURN;
	
	if(!(m_Pack >> type))
		goto RETURN;
	if(!(m_Pack >> ucbak))
		goto RETURN;
	if(!(m_Pack >> usbak))
		goto RETURN;

	if(!m_Pack.unpackString(visitorid, len, MAX_VISITORID_LEN))
		goto RETURN;
	if(!m_Pack.unpackString(msg, len, MAX_MSG_LEN))
		goto RETURN;
	if(!m_Pack.unpackString(msgex, len, MAX_MSG_LEN))
		goto RETURN;

	if(!(m_Pack >> type1))
		goto RETURN;
	if(!(m_Pack >> type2))
		goto RETURN;
	if(!(m_Pack >> error))
		goto RETURN;
	if(!(m_Pack >> response))
		goto RETURN;
	bRtn = true;
RETURN:
	return bRtn;
}

bool COM_INVITE::pack()
{
	bool bRtn = false;

	if(!(m_Pack << recvuid))
		goto RETURN;
	if(!(m_Pack << senduid))
		goto RETURN;
	if(!(m_Pack << compid))
		goto RETURN;
	

	if(!(m_Pack << type))
		goto RETURN;
	if(!(m_Pack << ucbak))
		goto RETURN;
	if(!(m_Pack << usbak))
		goto RETURN;

	if(!m_Pack.packString(visitorid, MAX_VISITORID_LEN))
		goto RETURN;
	if(!m_Pack.packString(msg, MAX_MSG_LEN))
		goto RETURN;
	if(!m_Pack.packString(msgex, MAX_MSG_LEN))
		goto RETURN;

	if(!(m_Pack << type1))
		goto RETURN;
	if(!(m_Pack << type2))
		goto RETURN;
	if(!(m_Pack << error))
		goto RETURN;
	if(!(m_Pack << response))
		goto RETURN;

	bRtn = true;
RETURN:
	return bRtn;
}


//#define CMD_EVENT_ANNOUNCEMENT:				0x7fff  //!�¼�ͨ�������� djz 2010-3-24
COM_EVENT_ANNOUNCEMENT::COM_EVENT_ANNOUNCEMENT(unsigned short version)
{
	this->m_Head.head.cmd = CMD_EVENT_ANNOUNCEMENT;

	myuin = 0;
	gpid = 0;
	
	type = 0;
	
	strcpy( msgbuf, "" );
}

COM_EVENT_ANNOUNCEMENT::~COM_EVENT_ANNOUNCEMENT()
{
}

bool COM_EVENT_ANNOUNCEMENT::unpack()
{
	bool bRtn = false;
	unsigned short len = 0;

	if(!(m_Pack >> myuin))
		goto RETURN;
	if(!(m_Pack >> gpid))
		goto RETURN;
	if(!(m_Pack >> type))
		goto RETURN;
	
	if(!m_Pack.unpackString(msgbuf, len, MAX_MSG_RECVLEN))
		goto RETURN;

	bRtn = true;
RETURN:
	return bRtn;
}

bool COM_EVENT_ANNOUNCEMENT::pack()
{
	bool bRtn = false;

	if(!(m_Pack << myuin))
		goto RETURN;
	if(!(m_Pack << gpid))
		goto RETURN;
	if(!(m_Pack << type))
		goto RETURN;
	
	if(!m_Pack.packString(msgbuf, MAX_MSG_RECVLEN))
		goto RETURN;

	bRtn = true;
RETURN:
	return bRtn;
}
