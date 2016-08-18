#ifndef __UI_USER_LIST_H
#define __UI_USER_LIST_H

#pragma once
#include <string>

typedef struct UserListItemStruct
{
	int type;
	unsigned long _uid;
	std::string   _sid;
	CDuiString  userImage;
	CDuiString nickName;
	CDuiString talkMsg;
	CDuiString time;

	CListContainerElementUI* plistElement;
}UserListItemInfo;


class CUIUserList : public CListUI
{

public:
	enum { SCROLL_TIMERID = 10 };

	CUIUserList(CPaintManagerUI& paint_manager);
	~CUIUserList();

public:
	void AddUser(UserListItemInfo *info);
	void CUIUserList::UpdateUserInfo(UserListItemInfo info);
	void CUIUserList::UpdateUserInfo(UserListItemInfo info, int type);

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