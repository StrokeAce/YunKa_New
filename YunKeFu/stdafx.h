// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once

#include "targetver.h"
#include "Resource.h"
#define WIN32_LEAN_AND_MEAN             //  �� Windows ͷ�ļ����ų�����ʹ�õ���Ϣ

// C ����ʱͷ�ļ�

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <string>
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <malloc.h>
#include <memory.h>


using namespace std;

#include <objbase.h>


#include "UIlib.h"


#include "global_setting.h"

extern CGlobalSetting _globalSetting;

using namespace DuiLib;

#define lengthof(x) (sizeof(x)/sizeof(*x))
#define MAX max
#define MIN min
#define CLAMP(x,a,b) (MIN(b,MAX(a,x)))

#ifdef _DEBUG
#   ifdef _UNICODE
#       pragma comment(lib, "../lib/DuiLib.lib")
#   else
#       pragma comment(lib, "../lib/DuiLib.lib")
#   endif
#else
#   ifdef _UNICODE
#       pragma comment(lib, "../lib_r/DuiLib.lib")
#   else
#       pragma comment(lib, "../lib_r/DuiLib.lib")
#   endif
#endif


// TODO:  �ڴ˴����ó�����Ҫ������ͷ�ļ�
