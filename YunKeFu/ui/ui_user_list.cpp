#include "../stdafx.h"
#include "ui_user_list.h"



const int kUserListItemNormalHeight      = 60;
const int kUserListItemSelectedHeight    = 60;


const TCHAR* const kLogoButtonControlName = _T("logo");
const TCHAR* const kLogoContainerControlName = _T("logo_container");
const TCHAR* const kNickNameControlName = _T("nickname");
const TCHAR* const kDescriptionControlName = _T("description");
const TCHAR* const kOperatorPannelControlName = _T("operation");

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


CUIUserList::CUIUserList(CPaintManagerUI& paint_manager)
	:m_paint_manager_(paint_manager)
	, delay_deltaY_(0)
	, delay_number_(0)
	, delay_left_(0)
{

	SetItemShowHtml(true);


	/*
	CContainerUI* pDesk = NULL;
	
		if (!m_dlgBuilder.GetMarkup()->IsValid())
			pDesk = static_cast<CContainerUI*>(m_dlgBuilder.Create(_T("friend_list_item.xml"), (UINT)0));
		else
			pDesk = static_cast<CContainerUI*>(m_dlgBuilder.Create(_T(""), (UINT)0));
		//Add(pDesk);
		//Add(pDesk);
		this->Add(pDesk);
*/




}

CUIUserList::~CUIUserList()
{



}



bool CUIUserList::Add(CControlUI* pControl)
{

	if (!pControl)
		return false;

	if (_tcsicmp(pControl->GetClass(), _T("ListContainerElementUI")) != 0)
		return false;

	return CListUI::Add(pControl);

}


bool CUIUserList::AddAt(CControlUI* pControl, int iIndex)
{

	if (!pControl)
		return false;

	if (_tcsicmp(pControl->GetClass(), _T("ListContainerElementUI")) != 0)
		return false;

	return CListUI::AddAt(pControl, iIndex);
}

bool CUIUserList::Remove(CControlUI* pControl)
{
	if (!pControl) return false;
	if (_tcscmp(pControl->GetClass(), _T("ListContainerElementUI")) != 0) return false;


	return CListUI::Remove(pControl);
}

bool CUIUserList::RemoveAt(int iIndex)
{
	CControlUI* pControl = GetItemAt(iIndex);
	if (!pControl) return false;
	if (_tcscmp(pControl->GetClass(), _T("ListContainerElementUI")) != 0) return false;

	return CListUI::RemoveAt(iIndex);

}

void CUIUserList::RemoveAll()
{
	CListUI::RemoveAll();
}

void CUIUserList::DoEvent(TEventUI& event)
{
	if (!IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND)
	{
		//if (m_pParent != NULL)
			//m_pParent->DoEvent(event);
		//else
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

void CUIUserList::UpdateUserInfo(UserListItemInfo *info, int type)
{
	CListContainerElementUI* pListElement = info->plistElement;
	CDuiString html_text;
	TCHAR szBuf[MAX_PATH] = { 0 };

	CLabelUI* pLabelUI = NULL;

	if (type == 1)
	{
		pLabelUI = static_cast<CLabelUI*>(m_paint_manager_.FindSubControlByName(pListElement, _T("show_user_head_label")));
		if (pLabelUI != NULL)
		{
			pLabelUI->SetBkImage(info->userImage.GetData());
		}

	}
	else if (type == 2)
	{
		_stprintf_s(szBuf, MAX_PATH - 1, _T("%s"), info->nickName);
		html_text += szBuf;

		pLabelUI = static_cast<CLabelUI*>(m_paint_manager_.FindSubControlByName(pListElement, _T("nickname_label")));
		if (pLabelUI != NULL)
		{
			//	nick_name->SetFixedWidth(0);
			pLabelUI->SetShowHtml(true);
			pLabelUI->SetText(html_text);
		}

	}

	else if (type == 3)
	{
		pLabelUI = static_cast<CButtonUI*>(m_paint_manager_.FindSubControlByName(pListElement, _T("show_small_text_label")));
		html_text = info->smallText;
		if (pLabelUI != NULL)
		{
			pLabelUI->SetShowHtml(true);
			pLabelUI->SetText(html_text);
		}
	}

	else if (type == 4)
	{
		pLabelUI = static_cast<CButtonUI*>(m_paint_manager_.FindSubControlByName(pListElement, _T("show_right_text_label")));
		html_text = info->rightText;
		if (pLabelUI != NULL)
		{
			pLabelUI->SetShowHtml(true);
			pLabelUI->SetText(html_text);
		}
	}
}


void CUIUserList::UpdateUserInfo(UserListItemInfo *info)
{
	UpdateUserInfo(info,1);
	UpdateUserInfo(info, 2);
	UpdateUserInfo(info, 3);
	UpdateUserInfo(info, 4);

}


void CUIUserList::AddUser(UserListItemInfo *info)
{
	CListContainerElementUI* pListElement = NULL;

	CDuiString html_text;

	info->plistElement = NULL;

	if (!m_dlgBuilder.GetMarkup()->IsValid()) {
		pListElement = static_cast<CListContainerElementUI*>(m_dlgBuilder.Create(_T("user_list.xml"), (UINT)0, NULL, &m_paint_manager_));
	}
	else {
		pListElement = static_cast<CListContainerElementUI*>(m_dlgBuilder.Create((UINT)0, &m_paint_manager_));
	}

	pListElement->SetFixedHeight(kUserListItemNormalHeight);

	//CListContainerElementUI* pListElement = new CListContainerElementUI;
	if (pListElement == NULL)
		return ;

	int index = pListElement->GetIndex() + 1;

	TCHAR szBuf[MAX_PATH] = { 0 };
	pListElement->SetVisible(true);
	CDuiRect rcPadding(1, 1, 1, 1);
	pListElement->SetPadding(rcPadding);

	
	CLabelUI* image_label= static_cast<CLabelUI*>(m_paint_manager_.FindSubControlByName(pListElement, _T("show_user_head_label")));
	if (image_label != NULL)
	{
		image_label->SetBkImage(info->userImage.GetData());
	}

	CLabelUI* nick_name = static_cast<CLabelUI*>(m_paint_manager_.FindSubControlByName(pListElement, _T("nickname_label")));
	if (nick_name != NULL)
	{
		//if (info.folder)
		//	nick_name->SetFixedWidth(0);

		_stprintf_s(szBuf, MAX_PATH - 1, _T("%s"), info->nickName);
		html_text += szBuf;
		nick_name->SetShowHtml(true);
		nick_name->SetText(html_text);
	}


	CLabelUI* labelMsg = static_cast<CButtonUI*>(m_paint_manager_.FindSubControlByName(pListElement, _T("show_small_text_label")));
	if (labelMsg != NULL)
	{
		html_text = info->smallText;
		labelMsg->SetShowHtml(true);
		labelMsg->SetText(html_text);
	}

	CLabelUI* labelTime = static_cast<CButtonUI*>(m_paint_manager_.FindSubControlByName(pListElement, _T("show_right_text_label")));
	if (labelTime != NULL)
	{
		html_text = info->rightText;
		labelTime->SetShowHtml(true);
		labelTime->SetText(html_text);
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




	/*
	//_stprintf_s(szBuf, MAX_PATH - 1, _T("User\\groups.png"));
	//labelImage = static_cast<CButtonUI*>(m_paint_manager_.FindSubControlByName(pListElement, _T("image1")));
	_stprintf_s(szBuf, MAX_PATH - 1, _T("£¨543234£©"));
	html_text = szBuf;
	CLabelUI* userId = static_cast<CLabelUI*>(m_paint_manager_.FindSubControlByName(pListElement, _T("userid")));
	if (userId != NULL)
	{
		if (info.folder)
			userId->SetFixedWidth(0);

		userId->SetShowHtml(true);
		userId->SetText(html_text);
	}
	*/

#if 0
	for (int i = 0; i < 4; i++)
	{
		CLabelUI* labelImage;
		if (i == 0)
		{
			_stprintf_s(szBuf, MAX_PATH - 1, _T("User\\groups.png"));

			labelImage = static_cast<CButtonUI*>(m_paint_manager_.FindSubControlByName(pListElement, _T("image1")));
		}
		else if (i == 1)
		{
			_stprintf_s(szBuf, MAX_PATH - 1, _T("User\\game.png"));
			labelImage = static_cast<CButtonUI*>(m_paint_manager_.FindSubControlByName(pListElement, _T("image2")));
		}
		else if (i == 2)
		{
			_stprintf_s(szBuf, MAX_PATH - 1, _T("User\\icon_f.png"));
			labelImage = static_cast<CButtonUI*>(m_paint_manager_.FindSubControlByName(pListElement, _T("image3")));
		}

		else if (i == 3)
		{
			_stprintf_s(szBuf, MAX_PATH - 1, _T("User\\icon_home.png"));
			labelImage = static_cast<CButtonUI*>(m_paint_manager_.FindSubControlByName(pListElement, _T("image4")));
		}

		if (labelImage != NULL)
		{

			
			labelImage->SetBkImage(szBuf);
		}
	}

#endif

#if 0

	TCHAR szBuf[MAX_PATH] = { 0 };

	//this->SetAttribute(_T("itemselectedbkcolor"), _T("#FFFF4589"));
	pListElement->SetVisible(true);

	CDuiRect rcPadding(1, 1, 1, 1);
	pListElement->SetPadding(rcPadding);

	CButtonUI* log_button = static_cast<CButtonUI*>(m_paint_manager_.FindSubControlByName(pListElement, kLogoButtonControlName));
	if (log_button != NULL)
	{
		if (!info.folder && !info.logo.IsEmpty())
		{
#if defined(UNDER_WINCE)
			_stprintf(szBuf, _T("%s"), item.logo);
#else
			_stprintf_s(szBuf, MAX_PATH - 1, _T("%s"), info.logo);
#endif
			log_button->SetNormalImage(szBuf);
		}
		else
		{
			CContainerUI* logo_container = static_cast<CContainerUI*>(m_paint_manager_.FindSubControlByName(pListElement, kLogoContainerControlName));
			if (logo_container != NULL)
				logo_container->SetVisible(true);
		}
		log_button->SetTag((UINT_PTR)pListElement);
		log_button->OnEvent += MakeDelegate(&OnLogoButtonEvent);
	}


	CDuiString html_text;


#if defined(UNDER_WINCE)
		_stprintf(szBuf, _T("%s"), item.nick_name);
#else
		_stprintf_s(szBuf, MAX_PATH - 1, _T("%s"), info.nick_name);
#endif
		html_text += szBuf;

	CLabelUI* nick_name = static_cast<CLabelUI*>(m_paint_manager_.FindSubControlByName(pListElement, kNickNameControlName));
	if (nick_name != NULL)
	{
		if (info.folder)
			nick_name->SetFixedWidth(0);

		nick_name->SetShowHtml(true);
		nick_name->SetText(html_text);
	}

	if (!info.folder && !info.description.IsEmpty())
	{
		CLabelUI* description = static_cast<CLabelUI*>(m_paint_manager_.FindSubControlByName(pListElement, kDescriptionControlName));
		if (description != NULL)
		{
#if defined(UNDER_WINCE)
			_stprintf(szBuf, _T("<x 20><c #808080>%s</c>"), item.description);
#else
			_stprintf_s(szBuf, MAX_PATH - 1, _T("<x 20><c #808080>%s</c>"), info.description);
#endif
			description->SetShowHtml(true);
			description->SetText(szBuf);
		}
	}

	pListElement->SetFixedHeight(kUserListItemNormalHeight);

	pListElement->SetTag(index);


#endif


}