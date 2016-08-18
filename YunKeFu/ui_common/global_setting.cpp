#include "../stdafx.h"
#include "global_setting.h"
#include "common_utility.h"
#include <Shlwapi.h>

#include "atltime.h" 



CGlobalSetting::CGlobalSetting()
{
	memset(m_passWord, 0, sizeof(m_passWord));
	memset(m_userName, 0, sizeof(m_userName));

	m_showWnd = 0;

	m_logoutState = 0;
}


CGlobalSetting::~CGlobalSetting()
{
}

//查看指定文件是否存在
int CGlobalSetting::FindFileExist(char *str)
{
	WCHAR path[1024] = { 0 };

	ANSIToUnicode(str, path);
	return PathFileExists(path);
}

int CGlobalSetting::FindFileExist(WCHAR *str)
{

	return PathFileExists(str);
}


void CGlobalSetting::GetCurTimeString(string &strFrom, string &strEnd, int nday)
{
	char from[128] = { 0 };
	char end[128] = { 0 };
#if 0

	struct tm curTm;
	time_t tmData;

	SYSTEMTIME t = { 0 };
	GetLocalTime(&t);


	long curTime = GetTickCount(); // 1000;
	long startTime = curTime - nday * 24 * 60 * 60;

	tmData = curTime;
	curTm = *localtime(&tmData);
	sprintf(end, "%4d-%2d-%2d",	curTm.tm_year + 1900, curTm.tm_mon + 1, curTm.tm_mday);
	strEnd = end;

	tmData = startTime;
	curTm = *localtime(&tmData);
	sprintf(from, "%4d-%2d-%2d", curTm.tm_year + 1900, curTm.tm_mon + 1, curTm.tm_mday);
	strFrom = from;
#endif

	CTime ct1 = CTime::GetCurrentTime();
	CTime ct2 = ct1 - CTimeSpan(nday, 0, 0, 0);

	sprintf(from, "%04d-%02d-%02d", ct2.GetYear(), ct2.GetMonth(), ct2.GetDay());
	sprintf(end, "%04d-%02d-%02d", ct1.GetYear(), ct1.GetMonth(), ct1.GetDay());

	strFrom = from;
	strEnd = end;

	return;
}
