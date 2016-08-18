#ifndef _SYSTEM_SETTINGS_H_
#define _SYSTEM_SETTINGS_H_

#pragma once

#include "chat_manager.h"

class CSystemSettings : public CWindowWnd, public INotifyUI, public IMessageFilterUI
{
public:
	CSystemSettings();
	~CSystemSettings();

	LPCTSTR GetWindowClassName() const;
	UINT GetClassStyle() const;

	virtual void Notify(TNotifyUI& msg);
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

	CSysConfigFile* m_sysConfig;
private:
	virtual void OnSelectChanged(TNotifyUI &msg);
	void OnPrepare(TNotifyUI& msg);
	void SaveSystemSettings();
	void ResetAlertSettings();

	LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
	CPaintManagerUI m_PaintManager;
};

#endif