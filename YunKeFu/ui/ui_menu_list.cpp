#include "../stdafx.h"
#include "ui_menu_list.h"



const int kFriendMenuItemNormalHeight      = 60;
const int kFriendMenuItemSelectedHeight    = 60;



static double CalculateDelay(double state)
{
	return pow(state, 2);
}

static bool OnLogoButtonEvent(void* event) {
	if (((TEventUI*)event)->Type == UIEVENT_BUTTONDOWN) {
		CControlUI* pButton = ((TEventUI*)event)->pSender;
		if (pButton != NULL) {
			CListContainerElementUI* pListElement = (CListContainerElementUI*)(pButton->GetTag());
			if (pListElement != NULL) pListElement->DoEvent(*(TEventUI*)event);
		}
	}
	return true;
}


CUIMenuList::CUIMenuList(CPaintManagerUI& paint_manager)
	:m_paint_manager_(paint_manager)
	, delay_deltaY_(0)
	, delay_number_(0)
	, delay_left_(0)
{

	SetItemShowHtml(true);

}

CUIMenuList::~CUIMenuList()
{

}



bool CUIMenuList::Add(CControlUI* pControl)
{

	if (!pControl)
		return false;

	if (_tcsicmp(pControl->GetClass(), _T("ListContainerElementUI")) != 0)
		return false;

	return CListUI::Add(pControl);
}


bool CUIMenuList::AddAt(CControlUI* pControl, int iIndex)
{
	if (!pControl)
		return false;

	if (_tcsicmp(pControl->GetClass(), _T("ListContainerElementUI")) != 0)
		return false;

	return CListUI::AddAt(pControl, iIndex);
}

bool CUIMenuList::Remove(CControlUI* pControl)
{
	if (!pControl) return false;
	if (_tcscmp(pControl->GetClass(), _T("ListContainerElementUI")) != 0) return false;


	return CListUI::Remove(pControl);
}

bool CUIMenuList::RemoveAt(int iIndex)
{
	CControlUI* pControl = GetItemAt(iIndex);
	if (!pControl) return false;
	if (_tcscmp(pControl->GetClass(), _T("ListContainerElementUI")) != 0) return false;

	return CListUI::RemoveAt(iIndex);

}

void CUIMenuList::RemoveAll()
{
	CListUI::RemoveAll();
}

void CUIMenuList::DoEvent(TEventUI& event)
{
	if (!IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND)
	{
		CVerticalLayoutUI::DoEvent(event);
		return;
	}

	if (event.Type == UIEVENT_TIMER && event.wParam == SCROLL_TIMERID)
	{
		if (delay_left_ > 0)
		{
			--delay_left_;
			SIZE sz = GetScrollPos();
			LONG lDeltaY = (LONG)(CalculateDelay((double)delay_left_ / delay_number_) * delay_deltaY_);
			if ((lDeltaY > 0 && sz.cy != 0) || (lDeltaY < 0 && sz.cy != GetScrollRange().cy))
			{
				sz.cy -= lDeltaY;
				SetScrollPos(sz);
				return;
			}
		}
		delay_deltaY_ = 0;
		delay_number_ = 0;
		delay_left_ = 0;
		m_pManager->KillTimer(this, SCROLL_TIMERID);
		return;
	}
	if (event.Type == UIEVENT_SCROLLWHEEL)
	{
		LONG lDeltaY = 0;
		if (delay_number_ > 0)
			lDeltaY = (LONG)(CalculateDelay((double)delay_left_ / delay_number_) * delay_deltaY_);
		switch (LOWORD(event.wParam))
		{
		case SB_LINEUP:
			if (delay_deltaY_ >= 0)
				delay_deltaY_ = lDeltaY + 8;
			else
				delay_deltaY_ = lDeltaY + 12;
			break;
		case SB_LINEDOWN:
			if (delay_deltaY_ <= 0)
				delay_deltaY_ = lDeltaY - 8;
			else
				delay_deltaY_ = lDeltaY - 12;
			break;
		}
		if
			(delay_deltaY_ > 100) delay_deltaY_ = 100;
		else if
			(delay_deltaY_ < -100) delay_deltaY_ = -100;

		delay_number_ = (DWORD)sqrt((double)abs(delay_deltaY_)) * 5;
		delay_left_ = delay_number_;
		m_pManager->SetTimer(this, SCROLL_TIMERID, 50U);
		return;
	}


	CListUI::DoEvent(event);
}

void CUIMenuList::UpdateUserInfo(MenuItemInfo info, int type)
{
	CListContainerElementUI* pListElement = info.plistElement;
	CDuiString html_text;
	TCHAR szBuf[MAX_PATH] = { 0 };

	CLabelUI* pLabelUI = NULL;

	if (type == 1)
	{
		pLabelUI = static_cast<CLabelUI*>(m_paint_manager_.FindSubControlByName(pListElement, _T("show_menu_group_image")));
		if (pLabelUI != NULL)
		{
			pLabelUI->SetBkImage(info._image.GetData());
		}

	}
	else if (type == 2)
	{
		_stprintf_s(szBuf, MAX_PATH - 1, _T("%s"), info._name.GetData());
		html_text += szBuf;

		pLabelUI = static_cast<CLabelUI*>(m_paint_manager_.FindSubControlByName(pListElement, _T("show_group_name_text")));
		if (pLabelUI != NULL)
		{
			//	nick_name->SetFixedWidth(0);
			pLabelUI->SetShowHtml(true);
			pLabelUI->SetText(html_text);
		}

	}

}


void CUIMenuList::UpdateUserInfo(MenuItemInfo info)
{
	UpdateUserInfo(info,1);
	UpdateUserInfo(info, 2);


}


void CUIMenuList::AddUser(MenuItemInfo *info)
{
	CListContainerElementUI* pListElement = NULL;

	CDuiString html_text;

	info->plistElement = NULL;

	if (!m_dlgBuilder.GetMarkup()->IsValid()) {
		pListElement = static_cast<CListContainerElementUI*>(m_dlgBuilder.Create(_T("menu_list.xml"), (UINT)0, NULL, &m_paint_manager_));
	}
	else {
		pListElement = static_cast<CListContainerElementUI*>(m_dlgBuilder.Create((UINT)0, &m_paint_manager_));
	}

	pListElement->SetFixedHeight(kFriendMenuItemNormalHeight);

	//CListContainerElementUI* pListElement = new CListContainerElementUI;
	if (pListElement == NULL)
		return ;

	int index = pListElement->GetIndex() + 1;

	TCHAR szBuf[MAX_PATH] = { 0 };
	pListElement->SetVisible(true);
	CDuiRect rcPadding(1, 1, 1, 1);
	pListElement->SetPadding(rcPadding);

	
	CLabelUI* image_label = static_cast<CLabelUI*>(m_paint_manager_.FindSubControlByName(pListElement, _T("show_menu_group_image")));
	if (image_label != NULL)
	{
		image_label->SetBkImage(info->_image.GetData());
	}

	CLabelUI* nick_name = static_cast<CLabelUI*>(m_paint_manager_.FindSubControlByName(pListElement, _T("show_group_name_text")));
	if (nick_name != NULL)
	{
		//if (info.folder)
		//	nick_name->SetFixedWidth(0);

		_stprintf_s(szBuf, MAX_PATH - 1, _T("%s"), info->_name.GetData());
		html_text += szBuf;
		nick_name->SetShowHtml(true);
		nick_name->SetText(html_text);
	}

	info->plistElement = pListElement;
	//pListElement->SetTag((UINT_PTR)data);

	pListElement->SetTag((UINT_PTR)info);
	AddAt(pListElement, index);

	this->SetHotItemBkColor(0xFFEBEBEB);
	this->SetSelectedItemBkColor(0xFFE5E5E5);

	//this->SetHotItemBkColor(0xFFE2E4E6);
	//this->SetSelectedItemBkColor(0xFFCACDD3);
	//this->SetDisabledItemBkColor(0xFF000000);
	//this->SetBkColor(0xFF04F45F);
	//this->SetHotItemBkColor(0xFFFF45FF);
	//this->SetDisabledItemBkColor(0xFF000000);
	//this->SetSelectedItemBkColor(0xFFFFFFFF);
	//userId->SetFixedWidth(0);

}