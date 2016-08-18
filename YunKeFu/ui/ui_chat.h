#ifndef  __UI_CHAT__
#define  __UI_CHAT__



#define CHAT_FONT_NAME (_T("΢���ź�"))

using namespace std;
//using namespace DuiLib2;

class CFontInfo				// ������Ϣ
{
public:
	CFontInfo(void)
	{
		m_nSize = 9;
		m_clrText = RGB(0, 0, 0);
		m_strName = CHAT_FONT_NAME;
		m_bBold = FALSE;
		m_bItalic = FALSE;
		m_bUnderLine = FALSE;
	}
	//~CFontInfo(void);

public:
	int m_nSize;			// �����С
	COLORREF m_clrText;		// ������ɫ
	wstring m_strName;		// ��������
	BOOL m_bBold;			// �Ƿ�Ӵ�
	BOOL m_bItalic;			// �Ƿ���б
	BOOL m_bUnderLine;		// �Ƿ���»���
};

class CUIChat : public IMessageFilterUI, public INotifyUI
{

public:

	CUIChat();
	~CUIChat();

	virtual LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
	virtual void Notify(TNotifyUI& msg);
	void init();



	void SetPaintManager(CPaintManagerUI *val);

	bool OnMsg(void *pMsg);

	void NotifyMsg(TNotifyUI& msg);
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);



private:
	CPaintManagerUI *m_PaintManager;
	HWND m_hMainWnd;



};




#endif