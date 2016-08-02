// MDuiTest.cpp : ����Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "YunKa.h"
#include "login_wnd.h"
#include "main_frame.h"
#include "path.h"
#include "utils.h"
#include "chat_manager.h"
#include "cef_browser/cefclient.h"
#include "common_utility.h"


CefRefPtr<ClientApp> m_cefApp;

Gdiplus::GdiplusStartupInput g_gdiplusStartupInput;
ULONG_PTR g_gdiplusToken;

CGlobalSetting _globalSetting;




int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPTSTR    lpCmdLine,
	_In_ int       nCmdShow)
{

	m_cefApp = new ClientApp();
	if (!(m_cefApp->Init(hInstance) < 0))
		return FALSE;


	wstring strFileName = ZYM::CPath::GetAppPath() + _T("ImageOleCtrl.dll");

	BOOL bRet = DllRegisterServer(strFileName.c_str());	// ע��COM���
	if (!bRet)
	{
		::MessageBox(NULL, _T("COM���ע��ʧ�ܣ�Ӧ�ó����޷���ɳ�ʼ��������"), _T("��ʾ"), MB_OK);
		return 0;
	}

	HRESULT hr = ::OleInitialize(NULL);
	if (FAILED(hr))
		return 0;

	GdiplusStartup(&g_gdiplusToken, &g_gdiplusStartupInput, NULL);	// ��ʼ��GDI+
	HMODULE hRichEditDll = ::LoadLibrary(_T("Riched20.dll"));	// ����RichEdit�ؼ�DLL

	CPaintManagerUI::SetInstance(hInstance);
	CPaintManagerUI::SetResourcePath(CPaintManagerUI::GetInstancePath() + _T("SkinRes"));

	HRESULT Hr = ::CoInitialize(NULL);
	if (FAILED(Hr)) return 0;

	CLoginWnd* pLoginFrame = new CLoginWnd();
	CMainFrame *pWndFrame = new CMainFrame(pLoginFrame->m_manager);
	pLoginFrame->Create(NULL, _T(""), UI_WNDSTYLE_DIALOG, 0, 0, 0, 0, 0, NULL);
	pLoginFrame->CenterWindow();

	pWndFrame->SetHandler();
	int result = pLoginFrame->ShowModal();

	if (result == 1)
	{
		
		pWndFrame->Create(NULL, _T(""), UI_WNDSTYLE_FRAME, WS_EX_WINDOWEDGE | WS_EX_ACCEPTFILES);
		pWndFrame->CenterWindow();
		pWndFrame->ShowModal();
		

	}
	else
	{
		//��¼ʧ��
	}

	

	CPaintManagerUI::MessageLoop();
	::CoUninitialize();


	if (hRichEditDll != NULL)					// ж��RichEdit�ؼ�DLL
		::FreeLibrary(hRichEditDll);

	Gdiplus::GdiplusShutdown(g_gdiplusToken);	// ����ʼ��GDI+
	::OleUninitialize();


	m_cefApp->Exit();
	m_cefApp = NULL;

	
	if (_globalSetting.m_logoutState == 1)
	{
		CDuiString path = GetCurrentPathW();
		path += L"\\YunKa.exe";
		ShellExecute(NULL, L"open", path.GetData(), NULL, NULL, SW_SHOWNOACTIVATE);
	}


	return 0;
}


