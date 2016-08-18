#pragma  once

#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
using namespace  std;


//ȥ���ַ�����ͷ�Ŀհ��ַ�
void TrimString(string &str);

//�ַ����滻
void string_replace(string & strBig, const string & strsrc, const string &strdst);

//��Ϊ��д�ַ�
void string_toupper(string & str);

//��ΪСд�ַ�
void string_tolower(string & str);


//�з��ַ���,����cSepCharList��ĳ���ַ��ָ�
int SplitStringByChar(const string& strSrc,const string& cSepCharList,vector<string>& vecDest,bool includeEmpties=true);

//�з��ַ���,����cSepStr�����ַ����ָ�
int SplitStringByStr(const string& strSrc  ,const string& cSepStr ,vector<string>& vecDest,bool includeEmpties=true);


//Replaces sets like <, >, ", ' and & with the &lt;, &gt;, &quot;, &#039; and &amp; sets.
string HtmlEscape(const string & str);
string HtmlUnEscape(const string & str);


//Replaces sets like <, >, ", ' and & to &lt;, &gt;, &quot;, &apos; and &amp; sets.
string XmlEscape(const string & str);

//Replaces sets like <, >, ", ' and & to &lt;, &gt;, &quot;, &apos; and &amp; sets.
string WmlEscape(const string & str);

//JSESCAPE :Replaces sets like\, \r,\n, ", ' with the \\, \\r,\\n,\",\' and &amp; sets.
string JsEscape(const string & str);

//���ļ�·������ȡĿ¼����
string FileDirectory(const string & str_path);

//���ַ�����hashֵ
unsigned int BKDRHash(const string &str);

//ת��16�������
string hexstr(unsigned char *str,int len) ;

//��s1��ǰs1Len�ַ��в���
const char* strnstr(const char* s1,size_t s1Len,const char* s2, size_t s2len);

//�����ַ���ʧ��js����
string FormatJs(const string &szIn,string &szOut);
 

//�����ִ�Сд����
char* stristr(const char *pcString1, const char *pcString2);


//�ַ���hash������ֱ��ɢ�бȽϺ�
unsigned int BKDRHash(const char *str,size_t len);

//��ֹԽ�翽��
void strMaxNumCp(char * strOut,const char* pin,int nmaxcplen);

