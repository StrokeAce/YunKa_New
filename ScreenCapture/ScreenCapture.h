// ScreenCapture.h : ScreenCapture DLL ����ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// ������


// CScreenCaptureApp
// �йش���ʵ�ֵ���Ϣ������� ScreenCapture.cpp
//

class CScreenCaptureApp : public CWinApp
{
public:
	CScreenCaptureApp();

// ��д
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
