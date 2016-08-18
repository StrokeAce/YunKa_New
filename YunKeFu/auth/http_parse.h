#ifndef HTTPPARSE_H_
#define  HTTPPARSE_H_

#include <string>
#include <map>
using namespace std;

class CHttpParse
{
public:
	CHttpParse();
	~CHttpParse();
	int     m_nCmdType;			//��������0: GET �� 1:POST

	string m_strUrl;
	map<string,string> m_mapParamsList;			//url����

	string  m_strRefererLine;			//Referer������
	string  m_strAcceptLanguageLine;	//Accept-Language������
	string  m_strUserAgentLine;		    //User-Agent������
	bool    m_bKeepAlive;		        //Connection������
	bool    m_bGzip;					//Accept-Encoding������
	size_t  m_nBodySize;				//Content-Length
	string  m_strCollkieLine;			//Cookie������
	string  m_strXForwardedFor;			//X-Forwarded-For������
	string  m_strHost;                  //


	/**
	��httpͷ��ȡ�� url ��url�в���,������ǰ��ӡ�&���������

	phttppack�����������ͷ��http���ַ�����������null����
	nhttppacklen����ͷ����
	*/

	bool ParseHead(const char *phttppack,size_t nhttppacklen);

	/**
	�Ӳ�������ȡ��ĳ������
	pstrkey: ����get�в���key��key������'\0'����
	pvaluebuf:���ֵ������
	nvaluebuflen:�������������
	����ֵ: NULL keyû���ҵ��������ҵ���key��pvaluebuf����key��Ӧ���ַ���ֵ,��key��Ӧ��value����С��nvaluebuflenʱ����pvaluebufδ���0

	*/
	bool GetHeadParams(const char *pstrkey,string & strValue) const;

	/**
	��post����ȡ��ĳ������,Ҫ��m_strPostBody��һ��һ������
	*/
	static bool GetPostBodyParams(const string& strPostBody,const char *pstrkey,string & strValue);

private:

	//һ���Խ�������url����
	void ParseHeadParams(const char *pparmstr,size_t nlen);

};

#endif
