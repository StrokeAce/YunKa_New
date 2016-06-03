#if !defined(AUTHERROR_H_)
#define AUTHERROR_H_

#define  SESSION_ACTION_SUCCESS 1    //1:�ɹ�
static char   szSESSION_ACTION_SUCCESS[]="�ɹ�";

#define  SESSION_ACTION_ERR_PASS 0    //0: ���벻��
static char   szSESSION_ACTION_ERR_PASS []="�û����벻��";

#define  SESSION_ACTION_ERR_TOKEN -1    //-1��token����
static char   szSESSION_ACTION_ERR_TOKEN[]="token����";

#define  SESSION_ACTION_ERR_UIN  -2     //-2�� uin���Ϸ�
static char   szSESSION_ACTION_ERR_UIN []="�û����� ���Ϸ�";

#define  SESSION_ACTION_ERR_TOKENLEN -3    //-3��token���Ȳ���
static char   szSESSION_ACTION_ERR_TOKENLEN[]="token���Ȳ���";

#define  SESSION_ACTION_ERR_SERVER  -4     //-4���������ڲ�����
static char   szSESSION_ACTION_ERR_SERVER []="�������ڲ�����";

#define  SESSION_ACTION_ERR_NOUSER  -5     //-5���û�������
static char   szSESSION_ACTION_ERR_NOUSER []="�û�������";

#define  SESSION_ACTION_ERR_NORIGHT  -8     //-8���û�����TQ��ͬʱΪ����
static char   szSESSION_ACTION_ERR_NORIGHT[]="�û������û���ͬʱΪ����";

#define  SESSION_ACTION_ERR_TYPE    -14     //-14����֤���Ͳ���
static char   szSESSION_ACTION_ERR_TYPE[]="��֤���Ͳ���ȷ";

#define  SESSION_ACTION_ERR_TIME    -15     //-15��ʱ�����
static char   szSESSION_ACTION_ERR_TIME[]="����ʱ�����";

#define  SESSION_ACTION_ERR_SOURCEIP    -16       //-16����ԴIP����
static char   szSESSION_ACTION_ERR_SOURCEIP []="��֤��ԴIP����";

#define  SESSION_ACTION_ERR_GETADMIN    -17       //-17����ȡ����Ա�������
static char   szSESSION_ACTION_ERR_GETADMIN []="��ȡ����Ա�������";

#define  SESSION_ACTION_ERR_NOTCONNECT    -100       //-100������֤������ʧ��
static char   szSESSION_ACTION_ERR_NOTCONNECT[]="������֤������ʧ��";

static char   szszSESSION_ACTION_ERR_DEFAULT[]="δ֪����";


inline const char* GetAuthStrError(int nErrrid)
{
	const char* pe=NULL;
	switch (nErrrid)
	{
	case SESSION_ACTION_SUCCESS:
		pe=szSESSION_ACTION_SUCCESS;
		break;
	case SESSION_ACTION_ERR_PASS:
		pe=szSESSION_ACTION_ERR_PASS;
		break;
	case SESSION_ACTION_ERR_UIN:
		pe=szSESSION_ACTION_ERR_UIN;
		break;
	case SESSION_ACTION_ERR_TOKENLEN:
		pe=szSESSION_ACTION_ERR_TOKENLEN;
		break;
	case SESSION_ACTION_ERR_SERVER:
		pe=szSESSION_ACTION_ERR_SERVER;
		break;
	case SESSION_ACTION_ERR_NOUSER:
		pe=szSESSION_ACTION_ERR_NOUSER;
		break;
	case SESSION_ACTION_ERR_NORIGHT:
		pe=szSESSION_ACTION_ERR_NORIGHT;
		break;
	case SESSION_ACTION_ERR_TYPE:
		pe=szSESSION_ACTION_ERR_TYPE;
		break;
	case SESSION_ACTION_ERR_TIME:
		pe=szSESSION_ACTION_ERR_TIME;
		break;
	case SESSION_ACTION_ERR_SOURCEIP:
		pe=szSESSION_ACTION_ERR_SOURCEIP;
		break;
	case SESSION_ACTION_ERR_GETADMIN:
		pe=szSESSION_ACTION_ERR_GETADMIN;
		break;
	case SESSION_ACTION_ERR_NOTCONNECT:
		pe=szSESSION_ACTION_ERR_NOTCONNECT;
		break;
	default :
		pe=szszSESSION_ACTION_ERR_DEFAULT;
		break;
	}

	return pe;
}


#endif
