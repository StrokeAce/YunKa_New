// CodeConvert.h: interface for the CCodeConvert class.
/***********************************************************
����:������

��������:����ת��
1��gb2312 to uft-8
2��utf-8 to gb2312
3��text to  bit()
���ʱ��:2006/12/21

  ԭʼ�루16���ƣ���UTF��8���루�����ƣ�
--------------------------------------------
0000 - 007F       0xxxxxxx 
0080 - 07FF       110xxxxx 10xxxxxx 
0800 - FFFF       1110xxxx 10xxxxxx 10xxxxxx 
����
--------------------------------------------
************************************************************/

#if !defined(AFX_CODECONVERT_H__2788AA9D_C1F3_4510_9761_7923B68A85CC__INCLUDED_)
#define AFX_CODECONVERT_H__2788AA9D_C1F3_4510_9761_7923B68A85CC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <string>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

//typedef unsigned short WORD;
//typedef WORD WCHAR;

class CCodeConvert 
{
public:
	CCodeConvert();
	virtual ~CCodeConvert();
	//�ı���ʽתΪ ��������ʽ(�磺%C7%A2%CC%B8%CD%A8123->Ǣ̸ͨ123)
	//����:0 ansi; 1 utf-8
	int TextToBit(char *out,const char *pText);

	//�ж�һ���ַ����ǲ���utf8����
	bool IsUtf8(const char *pdata,int len);


	//GB2312 תΪ UTF-8
	void Gb2312ToUTF_8(std::string& pOut,const char *pText, int pLen);
	//ansi תΪ UTF-8
	void AnsiToUTF_8(std::string& pOut,const char *pText, int pLen);

	//UTF-8 תΪ GB2312
	void UTF_8ToAnsi(std::string &pOut,const char *pText, int pLen);
	//
	int utf8ToUcs2(const  char *s, int len, WCHAR *wbuf, int wbuf_len);
	int Ucs2Toutf8(const WCHAR *wpin, int inlen, char *pout, int outlen);
	std::string URLEncode(const char *code);
	std::string URLDecode(const std::string code);
	// ����ʱ����������ĸ�ı�����ȫ������
	std::string URLDecodeALL(const std::string code);
private:
	void LocalToUTF_8(UINT CodePage,std::string& pOut,const char *pText, int pLen);

	char hexTodec(char ch);
	void UnicodeToUTF_8(char* pOut,const WCHAR* pText,int utf8len=3);
	void UTF_8ToUnicode(WCHAR* pOut,const char *pText,int utf8len=3);

	void UnicodeToAnsi(char* pOut,WCHAR uData);
	void LocalToUnicode(UINT CodePage,WCHAR* pOut,const char *gbBuffer);
	
};

#endif // !defined(AFX_CODECONVERT_H__2788AA9D_C1F3_4510_9761_7923B68A85CC__INCLUDED_)
