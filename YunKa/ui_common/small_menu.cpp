#include "../stdafx.h"
#include "small_menu.h"
#include <WinUser.h>
#include <io.h>
#include "ui_menu.h"



CSmallMenu::CSmallMenu()
{

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


void CSmallMenu::CreateSmallIcon(HWND hWnd, WCHAR *path)
{

	m_hMenuWnd = hWnd;
	HICON hIconSmall = (HICON)::LoadImage(GetModuleHandle(NULL),
		path,
		IMAGE_ICON,
		48,//::GetSystemMetrics(SM_CXSMICON),
		48,//::GetSystemMetrics(SM_CYSMICON),
		LR_LOADFROMFILE);

	nid.cbSize = sizeof(NOTIFYICONDATA);//����structure��С
	nid.hWnd = m_hMenuWnd;                  //��ǰ���򴰿ھ����������CreateWindowEx�ķ���ֵ���
	nid.uID = NULL;                     //���ж��ico�ļ�ʱʹ�ã��������NULL(û���Թ����Ժ���Գ���һ�£�
	nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP; //ѡ���Ǽ���������ʹ��
	nid.uCallbackMessage = WM_MY_MESSAGE_NOTIFYICON;     //�û��Զ����¼� #define WM_NOTIFYICON WM_USER+1(�������κ�������
	nid.hIcon = hIconSmall;                 //����ͼ�꣬�����Ǻ�windowͼ����ͬ��Ҳ������LoadIcon������
	lstrcpyn(nid.szTip, _T("������Գ���"), sizeof(nid.szTip));
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

	cpoint.y -= 65;
	//ClientToScreen(m_hMenuWnd, &cpoint);
	pMenu->SetPath(L"menu\\menutest.xml");
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
			SendMessage(m_hMenuWnd, WM_ACTIVATE, SC_RESTORE, 0);
			ShowWindow(m_hMenuWnd, SW_SHOW);
			return 0L;
		}
		//�Ҽ�����ʱ����ʾ�˵�
		else if (uMouseMsg == WM_RBUTTONUP)
		{
			POINT lpoint;
			GetCursorPos(&lpoint);//�õ����λ��
			CreateMyAppMenu(lpoint);
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