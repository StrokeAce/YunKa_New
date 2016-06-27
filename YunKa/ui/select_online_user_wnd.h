#ifndef _CHAT_INSIDE_WND_
#define _CHAT_INSIDE_WND_

#pragma once

#include "chat_manager.h"
#include "user_list.h"



class CSelectOnlineUserWnd : public CWindowWnd, public INotifyUI, public IMessageFilterUI
{
public:
	CSelectOnlineUserWnd();
	~CSelectOnlineUserWnd();
	LPCTSTR GetWindowClassName() const { return _T("UIChatInsideWnd"); };
	UINT GetClassStyle() const { return UI_CLASSSTYLE_DIALOG; };
	void OnFinalMessage(HWND /*hWnd*/)
	{
		m_pm.RemovePreMessageFilter(this);
		delete this;
	};

	void Notify(TNotifyUI& msg);
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


public:
	CChatManager* m_manager;

	CGroupObject* m_pGroup;

	UserListUI* m_pOnlineUserList;


public:

	void OnPrepare(TNotifyUI& msg);


private:
	CPaintManagerUI m_pm;



};




#endif