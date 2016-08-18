#include <string>
#include "Lock.h"
#include <map>
using namespace std;


struct NHisMsgItem
{
	unsigned int fromuin;
	string   fromname;
	time_t   tmsgtime;
	string   sFont;
	string   sMsg;
};

class CNChatHisMsg
{
public:
	CNChatHisMsg() :m_nNeedTotalNum(-1){}
	CNChatHisMsg(const string& cid, int nt) 
		:m_schatid(cid)
		,m_nNeedTotalNum(nt){}

	~CNChatHisMsg(){ Clear(); }

	void SetTotalMsgNum(int nt);
	void AddMsg(int id, NHisMsgItem *phm);
	bool Completed(){ return m_nNeedTotalNum <= (int)m_msgs.size(); }
	void Clear();
	static void Move(CNChatHisMsg&l,CNChatHisMsg &r);

	string				   m_schatid;
	int                    m_nNeedTotalNum; //��ʷ��¼�ܹ�����������ȫ�Ϳ�����ʾ��
	map<int, NHisMsgItem*> m_msgs;            //id->msg ��Ϣ�б�
	time_t                 m_recvbegintime; //���տ�ʼʱ��
private:
	CNChatHisMsg(const CNChatHisMsg&); 
	CNChatHisMsg& operator= (const CNChatHisMsg &r);
};


class CNomalChatHistoryMsgManage
{

public:
	enum NHISMSGTYPE
	{
		SUCCESS,
		TIMEOUT,
		NOTFIND
	};

	bool ParseJsonHisMsg(const char* chatid, const char* pjsondata);

	/*
	return:  NHisMsgType
	msgs������������Ҫdelete
	*/
	NHISMSGTYPE GetCompleteMsgs(const string& schatid,CNChatHisMsg &nchatmsgs);
private:
	CLock  m_lock;
	map < string, CNChatHisMsg >  m_chatmsgs;  //chatid ->chatmsgs
};

extern CNomalChatHistoryMsgManage g_NomalChatHistoryMsgManage;