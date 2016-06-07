#include "../stdafx.h"
#include "global_setting.h"
#include "common_utility.h"
#include <Shlwapi.h>


CGlobalSetting::CGlobalSetting()
{
	memset(m_passWord, 0, sizeof(m_passWord));
	memset(m_userName, 0, sizeof(m_userName));


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

	SYSTEMTIME t = { 0 };
	GetLocalTime(&t);

	//CTime ct1 = CTime::GetCurrentTime();
	//CTime ct2 = ct1 - CTimeSpan(nday, 0, 0, 0);

	//strFrom.Format("%04d-%02d-%02d", ct2.GetYear(), ct2.GetMonth(), ct2.GetDay());
	//strEnd.Format("%04d-%02d-%02d", ct1.GetYear(), ct1.GetMonth(), ct1.GetDay());

	return;
}