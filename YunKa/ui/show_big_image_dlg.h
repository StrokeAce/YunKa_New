#ifndef __SHOW_BIG_IMAGE_DLG__
#define __SHOW_BIG_IMAGE_DLG__

#pragma once

#include "chat_manager.h"
#include "cef_browser/client_handler.h"



class CShowBigImageDlg : public CWindowWnd, public INotifyUI, public IMessageFilterUI
{
public:
	CShowBigImageDlg();
	~CShowBigImageDlg();
	LPCTSTR GetWindowClassName() const { return _T("UIShowBigImageDlg"); }
	UINT GetClassStyle() const { return UI_CLASSSTYLE_DIALOG; };
	void OnFinalMessage(HWND /*hWnd*/)
	{
		m_pm.RemovePreMessageFilter(this);
		delete this;
	};

	void Notify(TNotifyUI& msg);
	RECT CShowBigImageDlg::GetPos();

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnNcActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnNcCalcSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnNcPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
	LRESULT OnMenuHandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);


	LRESULT HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	
	void ShowWnd(int type);
	void CShowBigImageDlg::ShowBigImage();
public:
	CChatManager* m_manager;

	bool isCreate;


public:

	void OnPrepare(TNotifyUI& msg);
	//HandlerInfo m_pShowImageHandler; // 消息列表

private:
	CPaintManagerUI m_pm;





};




#endif