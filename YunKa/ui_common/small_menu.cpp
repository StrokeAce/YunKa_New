#include "../stdafx.h"
#include "small_menu.h"
#include <WinUser.h>
#include <io.h>
#include "ui_menu.h"



CSmallMenu::CSmallMenu()
{
	m_showType = 0;
}

CSmallMenu::~CSmallMenu()
{

}

void CSmallMenu::Init()
{

	//hMenu = CreatePopupMenu();

	//AppendMenu(hMenu, MF_STRING, IDM_MENU_QUIT, _T("�˳�"));
	//AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
	//AppendMenu(hMenu, MF_STRING, IDM_MENU_RELOGIN, _T("���µ�¼"));

}

void CSmallMenu::SetMenuType(int type)
{
	m_showType = type;
}

void CSmallMenu::CreateSmallIcon(HWND hWnd, WCHAR *path)
{

	m_hMenuWnd = hWnd;
	HICON hIconSmall = (HICON)::LoadImage(GetModuleHandle(NULL),
		path,
		IMAGE_ICON,
		16,//::GetSystemMetrics(SM_CXSMICON),
		16,//::GetSystemMetrics(SM_CYSMICON),
		LR_LOADFROMFILE);

	nid.cbSize = sizeof(NOTIFYICONDATA);//����structure��С
	nid.hWnd = m_hMenuWnd;                  //��ǰ���򴰿ھ����������CreateWindowEx�ķ���ֵ���
	nid.uID = NULL;                     //���ж��ico�ļ�ʱʹ�ã��������NULL(û���Թ����Ժ���Գ���һ�£�
	nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP; //ѡ���Ǽ���������ʹ��
	nid.uCallbackMessage = WM_MY_MESSAGE_NOTIFYICON;     //�û��Զ����¼� #define WM_NOTIFYICON WM_USER+1(�������κ�������
	nid.hIcon = hIconSmall;                 //����ͼ�꣬�����Ǻ�windowͼ����ͬ��Ҳ������LoadIcon������
	lstrcpyn(nid.szTip, _T("�ƿ�"), sizeof(nid.szTip));
	//szTip��char[64] ����ֱ�Ӻ� string���Ⱥţ�����Ҫ����strcpy��������ֵ
	//���ú�DOTIFYICONDATA֮�󣬾Ϳ��Ե���Shell_NotifyIcon������ ���� dwMessage ���й̶��ļ���ѡ��Ҫ�������ͼ��������NIM_ADD���ڶ���������һ��ָ��NOTIFYICONDATA�Ľṹָ��
	Shell_NotifyIcon(NIM_ADD, &nid);


}

void CSmallMenu::DeleteSmallIcon()
{

	Shell_NotifyIcon(NIM_DELETE, &nid);

}



void CSmallMenu::CreateMyAppMenu(POINT point)
{
	//TrackPopupMenu(hMenu, TPM_LEFTALIGN, point.x, point.y - 5, 0, m_hMenuWnd, NULL);
	CMenuWnd* pMenu = new CMenuWnd(m_hMenuWnd);
	CPoint cpoint = point;
	cpoint.y -= 180;


	//ClientToScreen(m_hMenuWnd, &cpoint);
	if (m_showType == 0)
		pMenu->SetPath(L"menu\\small_menu_hide.xml");
	else if (m_showType == 1)
		pMenu->SetPath(L"menu\\small_menu_show.xml");

	pMenu->Init(NULL, _T(""), _T("xml"), cpoint);

}




LRESULT CSmallMenu::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int id = wParam;
	int uMouseMsg = lParam;

	if (uMsg == WM_MY_MESSAGE_NOTIFYICON)
	{
		//����ǵ�����������л�������ʾ״̬
		if (uMouseMsg == WM_LBUTTONDOWN)
		{
			printf("�л�����״̬");
			::PostMessage(m_hMenuWnd, WM_DOUBLE_CLICK_SHOW_WND_MSG, 0, 0);
		}

		// ����ǵ����Ҽ�������ʾ�˵�
		else if (uMouseMsg == WM_RBUTTONDOWN)
		{
			// ��ʾ��Ӧ�˵�
			printf("��ʾ�����˵�");
		}
		//��������˫��,��ʾ������
		else if (uMouseMsg == WM_LBUTTONDBLCLK)
		{
			printf("��ʾ������");
			//SendMessage(m_hMenuWnd, WM_ACTIVATE, SC_RESTORE, 0);
			//ShowWindow(m_hMenuWnd, SW_SHOW);


		
			return 0L;
		}
		//�Ҽ�����ʱ����ʾ�˵�
		else if (uMouseMsg == WM_RBUTTONUP)
		{
	
			GetCursorPos(&m_lpoint);//�õ����λ��
			CreateMyAppMenu(m_lpoint);
		}

	}
	else if (uMsg == WM_COMMAND)
	{

		switch (LOWORD(wParam))
		{
		case IDM_MENU_QUIT:

			PostQuitMessage(0);
			break;

		case IDM_MENU_RELOGIN:

			break;

		default:
			break;
		}
	}

	return 0L;
}