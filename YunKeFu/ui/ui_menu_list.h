#ifndef __UI_FRIEND_MENU_H
#define __UI_FRIEND_MENU_H

#pragma once
#include <string>

typedef struct MenuItemStruct
{
	int _menuIndex;
	int type;
	CDuiString    _name;
	CDuiString    _image;

	CListContainerElementUI* plistElement;
}MenuItemInfo;


class CUIMenuList : public CListUI
{

public:
	enum { SCROLL_TIMERID = 10 };

	CUIMenuList(CPaintManagerUI& paint_manager);
	~CUIMenuList();

public:
	void AddUser(MenuItemInfo *info);
	void CUIMenuList::UpdateUserInfo(MenuItemInfo info);
	void CUIMenuList::UpdateUserInfo(MenuItemInfo info, int type);

	bool Remove(CControlUI* pControl);

	bool RemoveAt(int iIndex);

	void RemoveAll();

protected:
	bool Add(CControlUI* pControl);

	bool AddAt(CControlUI* pControl, int iIndex);



	void DoEvent(TEventUI& event);





private:
	LONG	delay_deltaY_;
	DWORD	delay_number_;
	DWORD	delay_left_;


	CDialogBuilder m_dlgBuilder;
	CPaintManagerUI& m_paint_manager_;

};
















#endif