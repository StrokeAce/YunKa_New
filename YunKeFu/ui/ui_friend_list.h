#ifndef __UI_FRIEND_LIST_H
#define __UI_FRIEND_LIST_H

#pragma once
#include <string>

typedef struct FriendListItemStruct
{
	int type;

	CDuiString    _name;
	CDuiString    _image;

	CListContainerElementUI* plistElement;
}FriendListItemInfo;


class CUIFriendList : public CListUI
{

public:
	enum { SCROLL_TIMERID = 10 };

	CUIFriendList(CPaintManagerUI& paint_manager);
	~CUIFriendList();

public:
	void AddUser(FriendListItemInfo *info);
	void CUIFriendList::UpdateUserInfo(FriendListItemInfo info);
	void CUIFriendList::UpdateUserInfo(FriendListItemInfo info, int type);

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