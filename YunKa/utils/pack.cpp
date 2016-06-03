#include "../stdafx.h"
#include "tstring.h"

#include "pack.h"
#include "../chat_common/comtran.h"
#include "encrypt.h"

CPack::CPack()
{
	//��ʼ��
	m_pStart = new unsigned char[PACKMAXLEN+1];

	m_pCurrent=m_pStart;
	m_nTotalLen = PACKMAXLEN;
	m_nPackLen = 0;
	m_nIndex = 0;
}

CPack::CPack(unsigned char *pPack, unsigned short nSize,int langtype)
{
	m_pStart = new unsigned char[PACKMAXLEN+1];
	CopyBuff(pPack, nSize,langtype);
}

CPack::CPack(CPack &pack,int langtype)
{
	m_pStart = new unsigned char[PACKMAXLEN+1];
	CopyBuff(pack,langtype);
}

CPack::~CPack()
{
	if(m_pStart != NULL)
	{
		delete []m_pStart;
		m_pStart = NULL;
	}
}

bool CPack::CopyBuff(unsigned char *pBuff, unsigned short nSize,int langtype)
{
	m_nlangtype=langtype;

	bool bRtn = false;
	if(pBuff == NULL)
		goto RETURN;

	//��������
	if(nSize > PACKMAXLEN)
	{
		//Խ����
		SetLastError(PACK_ERROR_PACKTOOLARGE);
		SetLastErrorInfo("Pack is too large to Unpack!");

		bRtn = false;
	}
	else
	{
		if(m_pStart == NULL)
		{
			m_pStart = new unsigned char[PACKMAXLEN];
		}

		m_nTotalLen = PACKMAXLEN;
		m_pCurrent=m_pStart; //CopyBuff only for read ��m_pCurrentΪ��ָ��ŵ���ͷ

		//���������ݿ��뻺����
		if(nSize > 0)
		{
			memcpy((char *)m_pStart,(char *)pBuff, (size_t)nSize);
		}

		m_nPackLen = nSize;
		m_nIndex = 0;

		bRtn = true;
	}

	bRtn = true;
RETURN:
	return bRtn;
}

bool CPack::CopyBuff(CPack &pack,int langtype)
{
	return CopyBuff(pack.GetPackBuff(), pack.GetPackLength(),langtype);
}

bool CPack::operator<<(unsigned char a)
{	
	int nCurPackLen = sizeof(unsigned char);
	bool bRtn = false;

	AddVariableIndex();
	if(this->m_nIndex + nCurPackLen > m_nTotalLen)
	{
		//Խ����
		SetLastError(PACK_ERROR_SLOPOVER);
		SetLastErrorInfo("Pack unsigned char slop over!");

		bRtn = false;
	}
	else
	{
		//���
		m_pCurrent[0]=(unsigned char)a;
	
		m_pCurrent += nCurPackLen;
		m_nIndex += nCurPackLen;
		m_nPackLen += nCurPackLen;

		bRtn = true;
	}

	return bRtn;
}

bool CPack::operator<<(unsigned short a)
{
	int nCurPackLen = sizeof(unsigned short);
	bool bRtn = false;

	AddVariableIndex();
	if(this->m_nIndex + nCurPackLen > m_nTotalLen)
	{
		//Խ����
		this->SetLastError(PACK_ERROR_SLOPOVER);
		this->SetLastErrorInfo("Pack unsigned short slop over!");

		bRtn = false;
	}
	else
	{
		//���
		m_pCurrent[0]=(unsigned char)a&0xff;
		m_pCurrent[1]=(unsigned char)(a>>8)&0xff;
	
		m_pCurrent += nCurPackLen;
		m_nIndex += nCurPackLen;
		m_nPackLen += nCurPackLen;

		bRtn = true;
	}

	return bRtn;
}

bool CPack::operator<<(unsigned int a)
{
	int nCurPackLen = sizeof(unsigned int);
	bool bRtn = false;

	AddVariableIndex();
	if(this->m_nIndex + nCurPackLen > m_nTotalLen)
	{
		//Խ����
		this->SetLastError(PACK_ERROR_SLOPOVER);
		this->SetLastErrorInfo("Pack unsigned long slop over!");

		bRtn = false;
	}
	else
	{
		//���
		m_pCurrent[0]=(unsigned char)a&0xff;
		m_pCurrent[1]=(unsigned char)(a>>8)&0xff;
		m_pCurrent[2]=(unsigned char)(a>>16)&0xff;
		m_pCurrent[3]=(unsigned char)(a>>24)&0xff;

		m_pCurrent += nCurPackLen;
		m_nIndex += nCurPackLen;
		m_nPackLen += nCurPackLen;

		bRtn = true;
	}

	return bRtn;
}

bool CPack::packString(const char *a, int maxlen)
{
	bool bRtn = true;

	AddVariableIndex();

	if(a == NULL)
	{
		//validate tstring
		this->SetLastError(PACK_ERROR_NULLSTRING);
		this->SetLastErrorInfo("pack's tstring is NULL!");

		bRtn = false;
	}
	else
	{
		if( maxlen == 0)
		{
			maxlen = BUFFMAXLEN;
		}	
		unsigned short slen = strlen(a);//������ַ���ʵ�ʳ���
		int nCurPackLen = sizeof(unsigned short);	//���δ������


		//�����ַ����Ĵ�С
		if(slen > maxlen)
		{
			//validate tstring
			this->SetLastError(PACK_WARNING_STRINGTOOLONG);
			this->SetLastErrorInfo("pack's tstring is more long than max length and only max length is pack!");

			slen = maxlen;
		}
		
		//�������Ĵ�С
		nCurPackLen += slen ;
		if(this->m_nIndex + nCurPackLen > m_nTotalLen)
		{
			//Խ����
			this->SetLastError(PACK_ERROR_SLOPOVER);
			this->SetLastErrorInfo("Pack buffer slop over!");

			bRtn = false;
		}
		else
		{
			//���
			//�ַ�������
			if(!((*this)<<(unsigned short)(slen)))
			{
				//Խ����
				this->SetLastError(PACK_ERROR_SLOPOVER);
				this->SetLastErrorInfo("Pack buffer length slop over!");

				bRtn = false;
			}
			else
			{
				memcpy((char *)m_pCurrent,(char *)a, slen);

				m_pCurrent += slen;
				m_nIndex += slen;
				m_nPackLen += slen;

				bRtn = true;
			}
			AddVariableIndex(-1);
		}
	}

	return bRtn;
}
/*
//��Unicode������ת����ٴ��
bool CPack::packString(const TCHAR *a, int maxLen)
{
	string s;
	if (m_nlangtype == LANGUAGE_UTF8)
	{
		s=convertstring(a);
		maxLen = (maxLen*3+1);
	}
	else
	{
		s=UnicodeToAnsi(a);
		maxLen = maxLen*2+1;
	}
	return packString(s.c_str(),maxLen);
}
*/
bool CPack::operator>>(unsigned char &a)
{
	int nCurUnpackLen = sizeof(unsigned char);
	bool bRtn = false;

	AddVariableIndex();
	//Խ����
	if(m_nIndex + nCurUnpackLen > m_nPackLen)
	{
		//Խ����
		SetLastError(PACK_ERROR_SLOPOVER);
		SetLastErrorInfo("Unpack unsigned char slop over!");

		bRtn = false;
	}
	else
	{
		a = m_pCurrent[0];

		m_pCurrent += nCurUnpackLen;
		m_nIndex += nCurUnpackLen;

		bRtn = true;
	}

	return bRtn;
}

bool CPack::operator>>(unsigned short &a)
{
	int nCurUnpackLen = sizeof(unsigned short);
	bool bRtn = false;

	AddVariableIndex();
	//Խ����
	if(m_nIndex + nCurUnpackLen > m_nPackLen)
	{
		//Խ����
		SetLastError(PACK_ERROR_SLOPOVER);
		SetLastErrorInfo("Unpack unsigned short slop over!");

		bRtn = false;
	}
	else
	{
		a = (unsigned short)m_pCurrent[0] + 
			(unsigned short)((m_pCurrent[1]<<8)&0xff00);

		m_pCurrent += nCurUnpackLen;
		m_nIndex += nCurUnpackLen;

		bRtn = true;
	}

	return bRtn;
}

bool CPack::operator>>(unsigned int &a)
{
	int nCurUnpackLen = sizeof(unsigned int);
	bool bRtn = false;

	AddVariableIndex();
	//Խ����
	if(m_nIndex + nCurUnpackLen > m_nPackLen)
	{
		//Խ����
		SetLastError(PACK_ERROR_SLOPOVER);
		SetLastErrorInfo("Unpack unsigned long slop over!");

		bRtn = false;
	}
	else
	{
		a = (unsigned long)(m_pCurrent[0])
			+(unsigned long)((m_pCurrent[1]<<8)&0xff00)
			+(unsigned long)((m_pCurrent[2]<<16)&0xff0000)
			+(unsigned long)((m_pCurrent[3]<<24)&0xff000000);

		m_pCurrent += nCurUnpackLen;
		m_nIndex += nCurUnpackLen;

		bRtn = true;
	}

	return bRtn;
}

bool CPack::unpackString(char *a, unsigned short &len, int maxLen)
{
	bool bRtn = true;
	if(maxLen == 0)
	{
		//0����Ϊ��󳤶�
		maxLen = BUFFMAXLEN;
	}
//	AddVariableIndex();

	unsigned short reallen;
	if(!((*this)>>(unsigned short)len ))
	{
		//��ȡ�ַ������ȳ���
		SetLastError(PACK_ERROR_SLOPOVER);
		SetLastErrorInfo("Unpack tstring length slop over!");

		bRtn = false;
	}
	else
	{
		if (len > this->m_nPackLen - this->m_nIndex)
		{
			SetLastError(PACK_ERROR_STRINGTOOLONG);
			SetLastErrorInfo("Unpack tstring length is too large!");

			bRtn = false;
		}
		else
		{
			if(len  < 0 || len > maxLen)
			{
				SetLastError(PACK_WARNING_STRINGTOOLONG);
				SetLastErrorInfo("Unpack tstring length is too large!");

				reallen = maxLen;
			}
			else
			{
				reallen = len;
			}

			bRtn = true;
			memcpy(a, m_pCurrent, reallen);
			a[reallen]='\0';

			m_pCurrent += len;
			this->m_nIndex += len;

			len = reallen;
		}
	}

	return bRtn;
}
/*
bool CPack::unpackString(TCHAR *a, unsigned short &len, int maxLen)
{
	char  buf[201] = {0},*pdata=NULL;
	if (maxLen*3>200)
	{
		pdata=new char[maxLen*3];
		memset(pdata, 0, maxLen*3);
	}else
	{
		pdata=buf;
	}

	unpackString(pdata,len,maxLen*3);

	if (m_nlangtype==LANGUAGE_UTF8)
	{
		_tcsncpy(a,convertstring(pdata).c_str(),maxLen);
	}else
	{
		_tcsncpy(a,Gb2312ToUnicode(pdata).c_str(),maxLen);

	}
	a[maxLen-1]='\0';
	if (pdata!=buf)
	{
		delete [] pdata;
		pdata = NULL;
	}
	
	return true;
}
*/

unsigned char * CPack::GetPackBuff()
{
	return m_pStart;
}

unsigned char * CPack::GetCurrentPackBuff()
{
	return this->m_pCurrent;
}

unsigned short CPack::GetPackLength()
{
	return this->m_nPackLen;
}

int CPack::Encode()
{
	::lock((char *)m_pStart, m_nPackLen);
	return m_nPackLen;
}

int CPack::Decode()
{
	::lock((char *)m_pStart, m_nPackLen);
	return m_nPackLen;
}

void CPack::Clear()
{
	this->m_nTotalLen = 0;
	
	m_pCurrent = m_pStart;
	m_nTotalLen = PACKMAXLEN;
	m_nPackLen = 0;
	m_nIndex = 0;
	
	memset(m_pStart, '\0', PACKMAXLEN);
}
