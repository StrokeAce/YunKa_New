// Pack.h: interface for the CPack class.
//
//////////////////////////////////////////////////////////////////////

#if !defined _PACK_EM34563456
#define _PACK_EM34563456

#include "error_info.h"
#include "../chat_common/comstruct.h"
#include <tchar.h>

class CPack : public CErrorInfo
{

public:
	CPack();
	CPack(unsigned char *pBuff, unsigned short nSize,int langtype);
	CPack(CPack &pack,int langtype);
	 ~CPack();

private:
	//ָ�򻺳���ͷ��
	unsigned char *m_pStart;
	
	//ָ�򻺳�����ǰ����λ��
	unsigned char *m_pCurrent;
	
	//����������
	unsigned short	m_nTotalLen;
	//���ĳ���
	unsigned short	m_nPackLen;
	//��������ǰ��λ��
	unsigned short	m_nIndex;

public:
	int     m_nlangtype;//�ַ������뷽ʽ��1utf8 ,0 gbk
	int Encode();
	unsigned short GetPackLength();
	unsigned char * GetPackBuff();
	unsigned char * GetCurrentPackBuff();

	//ֱ�ӿ���ĳһ������һ����
	bool CopyBuff(unsigned char *pBuff, unsigned short nSize,int langtype);
	bool CopyBuff(CPack &pack,int langtype);

	//�������
	int Decode();
	bool operator<<(unsigned char a);
	bool operator<<(unsigned short a);
	bool operator<<(unsigned int a);
	bool packString(const char *a, int maxLen);

	//�������
	bool operator>>(unsigned char &a);
	bool operator>>(unsigned short &a);
	bool operator>>(unsigned int &a);
	bool unpackString(char *a, unsigned short &len, int maxLen=BUFFMAXLEN);
	void Clear();
};

#endif
