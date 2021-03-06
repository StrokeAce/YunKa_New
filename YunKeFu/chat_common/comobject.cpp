#include "comobject.h"
#include "comfunc.h"
#include "http_unit.h"
#include "tstring.h"
#include "code_convert.h"
#include "common_utility.h"
#include  <io.h>
#include <fstream>
#include <process.h>
#include <gdiplus.h>  
using namespace Gdiplus;

#pragma comment(lib,"gdiplus")  

CSysConfigFile::CSysConfigFile()
{
	ResetValue();
}

CSysConfigFile::~CSysConfigFile()
{
	DeleteAllLoginInfo();
	DeleteAllAlertInfo();
}

void CSysConfigFile::ResetValue()
{
	m_nKeySendType = 0;
	
	SetWndInitPos(true);

	m_nSendMsgHeight = 100;
	m_nPreSendMsgHeight = 35;

	m_nLastLoginBy = LOGIN_BYSTRING;

	m_sLastLoginUid = 0;
	m_sLastLoginStr = "";
	m_sLastLoginPass = "";

	m_sStrServer = "tcp01.tq.cn";
	m_nServerPort = 443;

	m_sVisitorServer = "vipwebscreen.tq.cn";
	m_nVisitorServerPort = EMVISITOR_PORT;

	m_sStrRealServer = "tcp01.tq.cn";
	m_nRealServerPort = 443;

	m_cLoginInfoList.clear();

	SetAllDefaultAlertInfo();

	m_cServerAddressList.clear();
	m_cKeyWordSearchList.clear();
	m_cForbidWebUserList.clear();

	memset(&m_cWebUserfilter, '\0', sizeof(FILTER_USERDEFINE));
	m_nFilterType = VISITORFILTER_ALL;

	m_bAutoResp = false;
	m_sWellcomeMsg = "您好，我是在线客服，很高兴为您服务。";

	m_nUserTimeoutTime = 6;
	m_sUserTimeoutMsg = "您好，我有事暂时离开，您可以留下联系方式或留言，我会尽快给您回复。";

	m_nVisitorTimeoutTime = 8;
	m_sVisitorTimeoutMsg = "还在吗？有什么问题尽管告诉我，我会尽力解决您的问题。";

	m_bVisotorTimeoutClose = false;
	m_nVisitorTimeoutCloseTime = 10;
	m_sVisitorTimeoutCloseMsg = "已经很久没有收到您的讯息，请问您还在电脑旁吗？如果没有其它问题，系统将主动结束本次对话。<br>如果有问题，欢迎您再次找我咨询，祝您今天工作好心情，再见。";

	m_bAutoRespUnnormalStatus = false;
	m_sUnnormalStatusMsg = "您好，我有事暂时离开，您可以留下联系方式或留言，我会尽快给您回复。";

	m_sInviteWords = "您好！我能为您做点什么?";

	//保存自定义邀请语
	m_cInviteWordsList.clear();
	m_cInviteWordsList.push_back("欢迎光临！");
	m_cInviteWordsList.push_back("您好，欢迎访问本网站！");
	m_cInviteWordsList.push_back("能和您聊聊吗？");
	m_cInviteWordsList.push_back("您好，如您遇到什么问题可以直接与我交流！");
	m_cInviteWordsList.push_back("您好，能为您做些什么呢？");
	m_cInviteWordsList.push_back("您好，我是网站管理员！");

	m_nInviteType = INVITE_USERDEFINE;

	m_nKeywordsSort = 0;
	m_nKeywordsUser = 0;
	m_sKeywordsFind = "";
}

bool CSysConfigFile::ReadFile(ifstream& fout)
{
	byte count = 0;

	Read(fout, m_nKeySendType);
	Read(fout, m_nX);
	Read(fout, m_nY);
	Read(fout, m_nWidth);
	Read(fout, m_nHeight);
	Read(fout, m_nSendMsgHeight);
	Read(fout, m_nPreSendMsgHeight);
	Read(fout, m_nLastLoginBy);
	Read(fout, m_sLastLoginUid);
	Read(fout, m_sLastLoginStr);
	Read(fout, m_sLastLoginPass);
	Read(fout, m_sStrServer);
	Read(fout, m_nServerPort);
	Read(fout, m_sVisitorServer);
	Read(fout, m_nVisitorServerPort);
	Read(fout, m_sStrRealServer);
	Read(fout, m_nRealServerPort);

	// 登录过的信息
	DeleteAllLoginInfo();
	Read(fout, count);
	for (int i = 0; i < (int)count; i++)
	{
		LOGIN_INFO *plg = new LOGIN_INFO();
		Read(fout, plg->uid);
		Read(fout, plg->sid);
		Read(fout, plg->pass);
		Read(fout, plg->bAutoLogin);
		Read(fout, plg->bKeepPwd);
		m_cLoginInfoList.push_back(plg);
	}
	
	DeleteAllAlertInfo();
	Read(fout, count);
	int type;
	for (int i = 0; i < (int)count; i++)
	{
		ALERT_INFO *par = new ALERT_INFO();
		Read(fout, type);
		Read(fout, par->bTray);
		Read(fout, par->bShowwnd);
		Read(fout, par->bSound);
		Read(fout, par->soundfilename);
		par->type = (ALERT_TYPE)type;
		m_cAlertInfoList.push_back(par);
	}

	m_cServerAddressList.clear();
	Read(fout, count);
	for (int i = 0; i < (int)count; i++)
	{
		string address;
		Read(fout, address);
		m_cServerAddressList.push_back(address);
	}

	m_cKeyWordSearchList.clear();
	Read(fout, count);
	for (int i = 0; i < (int)count; i++)
	{
		string address;
		Read(fout, address);
		m_cKeyWordSearchList.push_back(address);
	}

	m_cForbidWebUserList.clear();
	Read(fout, count);
	for (int i = 0; i < (int)count; i++)
	{
		string address;
		Read(fout, address);
		m_cForbidWebUserList.push_back(address);
	}

	Read(fout, m_cWebUserfilter.num);
	Read(fout, m_cWebUserfilter.include);
	for (int i = 0; i < m_cWebUserfilter.num; i++)
	{
		Read(fout, m_cWebUserfilter.text[i]);
	}

	Read(fout, m_nFilterType);
	Read(fout, m_bAutoResp);
	Read(fout, m_sWellcomeMsg);
	Read(fout, m_nUserTimeoutTime);
	Read(fout, m_sUserTimeoutMsg);
	Read(fout, m_nVisitorTimeoutTime);
	Read(fout, m_sVisitorTimeoutMsg);
	Read(fout, m_bVisotorTimeoutClose);
	Read(fout, m_nVisitorTimeoutCloseTime);
	Read(fout, m_sVisitorTimeoutCloseMsg);
	Read(fout, m_bAutoRespUnnormalStatus);
	Read(fout, m_sUnnormalStatusMsg);
	Read(fout, m_sInviteWords);

	m_cInviteWordsList.clear();
	Read(fout, count);
	for (int i = 0; i < (int)count; i++)
	{
		string address;
		Read(fout, address);
		m_cInviteWordsList.push_back(address);
	}

	Read(fout, m_nInviteType);
	Read(fout, m_nKeywordsSort);
	Read(fout, m_nKeywordsUser);
	Read(fout, m_sKeywordsFind);

	return false;
}

bool CSysConfigFile::WriteFile(ofstream& fin)
{
	byte count = 0;
	Write(fin, m_nKeySendType);
	Write(fin, m_nX);
	Write(fin, m_nY);
	Write(fin, m_nWidth);
	Write(fin, m_nHeight);
	Write(fin, m_nSendMsgHeight);
	Write(fin, m_nPreSendMsgHeight);
	Write(fin, m_nLastLoginBy);
	Write(fin, m_sLastLoginUid);
	Write(fin, m_sLastLoginStr.c_str());
	Write(fin, m_sLastLoginPass.c_str());
	Write(fin, m_sStrServer.c_str());
	Write(fin, m_nServerPort);
	Write(fin, m_sVisitorServer.c_str());
	Write(fin, m_nVisitorServerPort);
	Write(fin, m_sStrRealServer.c_str());
	Write(fin, m_nRealServerPort);

	// 登录信息列表
	Write(fin, (byte)m_cLoginInfoList.size());
	ListLoginedInfo::iterator iter_login = m_cLoginInfoList.begin();
	for (iter_login; iter_login != m_cLoginInfoList.end(); iter_login++)
	{
		Write(fin, (*iter_login)->uid);
		Write(fin, (*iter_login)->sid);
		Write(fin, (*iter_login)->pass);
		Write(fin, (*iter_login)->bAutoLogin);
		Write(fin, (*iter_login)->bKeepPwd);
	}

	// 提示音信息
	Write(fin, (byte)m_cAlertInfoList.size());
	ListAlertInfo::iterator iter_alert = m_cAlertInfoList.begin();
	for (iter_alert; iter_alert != m_cAlertInfoList.end(); iter_alert++)
	{
		Write(fin, (*iter_alert)->type);
		Write(fin, (*iter_alert)->bTray);
		Write(fin, (*iter_alert)->bShowwnd);
		Write(fin, (*iter_alert)->bSound);
		Write(fin, (*iter_alert)->soundfilename);
	}

	// 服务器地址列表
	Write(fin, (byte)m_cServerAddressList.size());
	list<string>::iterator iter_Address = m_cServerAddressList.begin();
	for (iter_Address; iter_Address != m_cServerAddressList.end(); iter_Address++)
	{
		Write(fin, (*iter_Address).c_str());
	}

	Write(fin, (byte)m_cKeyWordSearchList.size());
	list<string>::iterator iter_wordSearch = m_cKeyWordSearchList.begin();
	for (iter_wordSearch; iter_wordSearch != m_cKeyWordSearchList.end(); iter_wordSearch++)
	{
		Write(fin, (*iter_wordSearch).c_str());
	}

	Write(fin, (byte)m_cForbidWebUserList.size());
	list<string>::iterator iter_forbidWebUser = m_cForbidWebUserList.begin();
	for (iter_forbidWebUser; iter_forbidWebUser != m_cForbidWebUserList.end(); iter_forbidWebUser++)
	{
		Write(fin, (*iter_forbidWebUser).c_str());
	}

	if (m_cWebUserfilter.num > MAX_STRINGFILTER_NUM)
		m_cWebUserfilter.num = MAX_STRINGFILTER_NUM;
	Write(fin, m_cWebUserfilter.num);
	Write(fin, m_cWebUserfilter.include);

	for (int i = 0; i < m_cWebUserfilter.num; i++)
	{
		string tempStr = m_cWebUserfilter.text[i];
		Write(fin, tempStr.c_str());
	}

	Write(fin, m_nFilterType);
	Write(fin, m_bAutoResp);
	Write(fin, m_sWellcomeMsg.c_str());
	Write(fin, m_nUserTimeoutTime);
	Write(fin, m_sUserTimeoutMsg.c_str());
	Write(fin, m_nVisitorTimeoutTime);
	Write(fin, m_sVisitorTimeoutMsg.c_str());
	Write(fin, m_bVisotorTimeoutClose);
	Write(fin, m_nVisitorTimeoutCloseTime);
	Write(fin, m_sVisitorTimeoutCloseMsg.c_str());
	Write(fin, m_bAutoRespUnnormalStatus);
	Write(fin, m_sUnnormalStatusMsg.c_str());
	Write(fin, m_sInviteWords.c_str());

	Write(fin, (byte)m_cInviteWordsList.size());
	list<string>::iterator iter_invite = m_cInviteWordsList.begin();
	for (iter_invite; iter_invite != m_cInviteWordsList.end(); iter_invite++)
	{
		Write(fin, (*iter_invite).c_str());
	}

	Write(fin, m_nInviteType);
	Write(fin, m_nKeywordsSort);
	Write(fin, m_nKeywordsUser);
	Write(fin, m_sKeywordsFind.c_str());

	return false;
}

bool CSysConfigFile::LoadData(char *sFilename)
{
	assert(sFilename != NULL && strlen(sFilename) > 0);

	if (sFilename != NULL && strlen(sFilename) > 0)
	{
		strcpy(m_sConfigname, sFilename);
	}	

	if (_access(m_sConfigname, 0) == -1)
	{
		return false;
	}

	ifstream outfile(sFilename, std::ios::binary);
	if (!outfile)
	{
		return false;
	}

	if (ReadFile(outfile))
	{
		outfile.close();
		return false;
	}

	outfile.close();

	if (m_sStrServer.empty())
	{
		m_sStrServer = "tcp01.tq.cn";
	}

	if (m_nServerPort == 0)
	{
		m_nServerPort = 443;
	}
	return true;
}

bool CSysConfigFile::SaveData(char *sFilename)
{
	if (sFilename != NULL)
	{
		strcpy(m_sConfigname, sFilename);
	}

	assert(m_sConfigname);

	char sTempFile[256];
	sprintf(sTempFile, "%s.bak", m_sConfigname);

	ofstream infile(sTempFile, std::ios::binary);
	if (!infile)
	{
		return false;
	}

	if (!WriteFile(infile))
	{
		infile.close();
		if (CopyFileA(sTempFile, m_sConfigname, false))
		{
			DeleteFileA(sTempFile);
			return true;
		}
		else
		{
			return false;
		}
	}

	infile.close();
	return true;
}

ALERT_INFO *CSysConfigFile::SetAllDefaultAlertInfo()
{
	DeleteAllAlertInfo();

	string strPath = GetCurrentPath();
	strPath += "\\res\\sound";

	ALERT_INFO *pInfo, *pRtn;
	pRtn = NULL;
	
	{
		pInfo = new ALERT_INFO();
		memset(pInfo, '\0', sizeof(ALERT_INFO));
		pInfo->type = ALERT_NEW_VISIT;
		pInfo->bSound = 1;
		pInfo->bTray = 0;
		pInfo->bShowwnd = 0;
		sprintf(pInfo->soundfilename, "%s%s", strPath.c_str(), "\\invite.wav");
		m_cAlertInfoList.push_back(pInfo);
	}

	{
		pInfo = new ALERT_INFO;
		memset(pInfo, '\0', sizeof(ALERT_INFO));
		pInfo->type = ALERT_NEW_CHAT;
		pInfo->bSound = 1;
		pInfo->bTray = 0;
		pInfo->bShowwnd = 0;
		sprintf(pInfo->soundfilename, "%s%s", strPath.c_str(), "\\online.wav");
		m_cAlertInfoList.push_back(pInfo);
	}

	{
		pInfo = new ALERT_INFO;
		memset(pInfo, '\0', sizeof(ALERT_INFO));
		pInfo->type = ALERT_NEW_MSG;
		pInfo->bSound = 1;
		pInfo->bTray = 0;
		pInfo->bShowwnd = 0;
		sprintf(pInfo->soundfilename, "%s%s", strPath.c_str(), "\\msg.wav");
		m_cAlertInfoList.push_back(pInfo);
	}

	{
		pInfo = new ALERT_INFO;
		memset(pInfo, '\0', sizeof(ALERT_INFO));
		pInfo->type = ALERT_NEW_TRANSFER;
		pInfo->bSound = 1;
		pInfo->bTray = 0;
		pInfo->bShowwnd = 0;
		sprintf(pInfo->soundfilename, "%s%s", strPath.c_str(), "\\invite.wav");
		m_cAlertInfoList.push_back(pInfo);
	}

	{
		pInfo = new ALERT_INFO;
		memset(pInfo, '\0', sizeof(ALERT_INFO));
		pInfo->type = ALERT_NEW_OTHER;
		pInfo->bSound = 0;
		pInfo->bTray = 0;
		pInfo->bShowwnd = 0;
		sprintf(pInfo->soundfilename, "%s%s", strPath.c_str(), "\\ring.wav");
		m_cAlertInfoList.push_back(pInfo);
	}

	return pRtn;
}

ALERT_INFO *CSysConfigFile::GetAlertInfo(ALERT_TYPE type)
{
	int i, len;

	len = m_cAlertInfoList.size();
	if (len == 0)
	{
		SetAllDefaultAlertInfo();
	}

	len = m_cAlertInfoList.size();
	for (i = 0; i < len; i++)
	{
		ALERT_INFO *pInfo = m_cAlertInfoList[i];

		if (pInfo->type == type)
			return pInfo;
	}

	return NULL;
}

void CSysConfigFile::SetWndInitPos(bool bAlways)
{
	int scx, scy;
	scx = GetSystemMetrics(SM_CXSCREEN);
	scy = GetSystemMetrics(SM_CYSCREEN);

	if (bAlways || ((m_nX < 0 || m_nX > scx) || (m_nY < 0 || m_nY >scy) ||
		(m_nWidth < 0 || m_nWidth > scx) || (m_nHeight < 0 || m_nHeight >scy)))
	{
		m_nWidth = 264;
		m_nHeight = 450;
		m_nX = scx - m_nWidth;
		m_nY = (scy - m_nHeight) / 2;
	}
}

void CSysConfigFile::DeleteAllLoginInfo()
{
	ListLoginedInfo::iterator iter = m_cLoginInfoList.begin();
	for (iter; iter != m_cLoginInfoList.end(); iter++)
	{
		delete *iter;
	}
	m_cLoginInfoList.clear();
}

void CSysConfigFile::DeleteAllAlertInfo()
{
	ListAlertInfo::iterator iter = m_cAlertInfoList.begin();
	for (iter; iter != m_cAlertInfoList.end(); iter++)
	{
		delete *iter;
	}
	m_cAlertInfoList.clear();
}

void CSysConfigFile::AddServerAddress(string strServer, bool bMoveToTop)
{
	if (strServer.empty())
		return;

	list<string>::iterator iter;
	for (iter = m_cServerAddressList.begin(); iter != m_cServerAddressList.end(); ++iter)
	{
		if (*iter == strServer)
		{
			return;
		}
	}

	m_cServerAddressList.push_back(strServer);
}

bool CSysConfigFile::IsWebuserSidForbid(string sid)
{
	if (sid.empty())
		return false;

	bool brtn = false;
	
	list<string>::iterator iter = m_cForbidWebUserList.begin();

	for (iter; iter != m_cForbidWebUserList.end(); iter++)
	{
		if ((*iter) == sid)
		{
			brtn = true;
			break;
		}
	}

	return brtn;
}

void CSysConfigFile::Write(ofstream& fin, bool bVal)
{
	fin.write((char*)(&bVal), sizeof(bool));
}

void CSysConfigFile::Write(ofstream& fin, int iVal)
{
	fin.write((char*)(&iVal), sizeof(int));
}

void CSysConfigFile::Write(ofstream& fin, unsigned long lVal)
{
	fin.write((char*)(&lVal), sizeof(unsigned long));
}

void CSysConfigFile::Write(ofstream& fin, const char* sVal)
{
	byte count = 0;	
	count = strlen(sVal);
	fin.write((char*)(&count), sizeof(byte));
	fin.write(sVal, (int)count);
}

void CSysConfigFile::Write(ofstream& fin, byte byVal)
{
	fin.write((char*)(&byVal), sizeof(byte));
}

void CSysConfigFile::Write(ofstream& fin, unsigned int Val)
{
	fin.write((char*)(&Val), sizeof(unsigned int));
}

void CSysConfigFile::Read(ifstream& fout, bool& bVal)
{
	fout.read((char*)(&bVal), sizeof(bool));
}

void CSysConfigFile::Read(ifstream& fout, int& iVal)
{
	fout.read((char*)(&iVal), sizeof(int));
}

void CSysConfigFile::Read(ifstream& fout, unsigned long& lVal)
{
	fout.read((char*)(&lVal), sizeof(unsigned long));
}

void CSysConfigFile::Read(ifstream& fout, unsigned int& Val)
{
	fout.read((char*)(&Val), sizeof(unsigned int));
}

void CSysConfigFile::Read(ifstream& fout, string& sVal)
{
	char sTemp[MAX_256_LEN];
	byte count = 0;
	fout.read((char*)(&count), sizeof(byte));
	fout.read(sTemp, (int)count);
	sTemp[(int)count] = '\0';
	sVal = sTemp;
}

void CSysConfigFile::Read(ifstream& fout, byte& byVal)
{
	fout.read((char*)(&byVal), sizeof(byte));
}

void CSysConfigFile::Read(ifstream& fout, char* chVal)
{
	byte count = 0;
	fout.read((char*)(&count), sizeof(byte));
	fout.read(chVal, (int)count);
	chVal[(int)count] = '\0';
}

LOGIN_INFO * CSysConfigFile::AddLatestLoginInfo(unsigned long uid, string sid, string pass, bool bAutoLogin, bool bKeepPwd)
{
	ListLoginedInfo::iterator iter = m_cLoginInfoList.begin();
	for (iter; iter != m_cLoginInfoList.end(); iter++)
	{
		if ((*iter)->uid == uid && uid != 0)
		{
			m_cLoginInfoList.erase(iter);
			break;
		}
	}

	LOGIN_INFO* plg = new LOGIN_INFO;
	plg->uid = uid;
	strcpy(plg->sid, sid.c_str());
	strcpy(plg->pass, pass.c_str());
	plg->bAutoLogin = bAutoLogin;
	plg->bKeepPwd = bKeepPwd;
	m_cLoginInfoList.push_back(plg);

	return plg;
}

CUserObject::CUserObject()
{
	m_nEMObType = OBJECT_USER;
	memset(&UserInfo, '\0', sizeof(USER_INFO));
	m_nWaitTimer = -20;
	talkstatus = TALK_STATUS_NO;
	status = USER_STATUS_OFFLINE;
	strcpy(sstatus, "");
	nTimer = 0;
	nFlashFirstOnline = 0;
	nVisitNum = 0;
	nTalkNum = 0;
	nLastVisitTime = 0;
	bTalkMeAfterLogon = false;
	m_nFlag = 0;
	m_bOpen = false;
	m_bReset = false;
	m_bConnected = false;
	talkuid = 0;
	m_bFriend = false;
	m_bInnerTalk = false;
	m_bKeywordsChange = false;
	m_bKeywordsGet = false;
	CCodeConvert convert;
	string strPath;
	strPath = FullPath("res\\headimage\\default.png");
	StringReplace(strPath,"\\", "/");
	convert.Gb2312ToUTF_8(m_headPath, strPath.c_str(), strPath.length());
}

CUserObject::~CUserObject()
{

}

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT  num = 0;          // number of image encoders  
	UINT  size = 0;         // size of the image encoder array in bytes  

	ImageCodecInfo* pImageCodecInfo = NULL;

	//2.获取GDI+支持的图像格式编码器种类数以及ImageCodecInfo数组的存放大小  
	GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1;  // Failure  

	//3.为ImageCodecInfo数组分配足额空间  
	pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL)
		return -1;  // Failure  

	//4.获取所有的图像编码器信息  
	GetImageEncoders(num, size, pImageCodecInfo);

	//5.查找符合的图像编码器的Clsid  
	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  // Success  
		}
	}

	//6.释放步骤3分配的内存  
	free(pImageCodecInfo);
	return -1;
}

void TransImageFormat(const WCHAR* srcName, const WCHAR* destName)
{
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;

	//1.初始化GDI+，以便后续的GDI+函数可以成功调用  
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	CLSID   encoderClsid;
	Status  stat;

	//7.创建Image对象并加载图片  
	Image* image = new Image(srcName);

	// Get the CLSID of the PNG encoder.  
	GetEncoderClsid(_T("image/png"), &encoderClsid);

	//8.调用Image.Save方法进行图片格式转换，并把步骤3)得到的图像编码器Clsid传递给它  
	stat = image->Save(destName, &encoderClsid, NULL);

	//9.释放Image对象  
	delete image;
	//10.清理所有GDI+资源  
	GdiplusShutdown(gdiplusToken);
}

// 头像下载线程
static UINT WINAPI DownLoadFaceThread(void * para)
{
	char strFaceLink[MAX_256_LEN];
	char strStorePath[MAX_256_LEN];
	char strPNG[MAX_256_LEN];
	string strFaceLinkPath = ((CUserObject*)para)->m_loadHeadUrl;
	string strPath;
	strPath = FullPath("res\\headimage");
	sprintf(strPNG, "%s\\%lu.png", strPath.c_str(), ((CUserObject*)para)->UserInfo.uid);

	static char* szExt[] = { "jpg", "gif", "png", "jpeg", NULL };
	DWORD dwRetVal = 0;

	for (int i = 0; szExt[i] != NULL; i++)
	{
		sprintf(strFaceLink, "%slogo_%lu.%s", strFaceLinkPath.c_str(), ((CUserObject*)para)->UserInfo.uid, szExt[i]);
		sprintf(strStorePath, "%s\\%lu.%s", strPath.c_str(), ((CUserObject*)para)->UserInfo.uid, szExt[i]);
		
		CHttpLoad downLoad;
		dwRetVal = HttpDownloadFile(((CUserObject*)para)->UserInfo.uid, strFaceLink, strStorePath);
		if (dwRetVal == 0)
		{
			if (i != 2)
			{
				// 将图片换成png格式
				TransImageFormat(convertstring(strStorePath).c_str(), convertstring(strPNG).c_str());
				DeleteFileA(strStorePath);								
			}
			if (_access(strPNG, 0) == 0)
			{
				((CUserObject*)para)->m_headPath = strPNG;
				StringReplace(((CUserObject*)para)->m_headPath, "\\", "/");
			}
			return true;
		}
	}

	return false;
}

void CUserObject::DownLoadFace(char* loadUrl)
{
	m_loadHeadUrl = loadUrl;
	char filePath[MAX_PATH];
	sprintf(filePath, "%s\\%lu.png", FullPath("res\\headimage").c_str(), UserInfo.uid);
	if (_access(filePath, 0) == 0)
	{
		m_headPath = filePath;
		StringReplace(m_headPath, "\\", "/");
		return;
	}
		
	HANDLE hDownLoadThread = (HANDLE)_beginthreadex(NULL, 0, DownLoadFaceThread, this, CREATE_SUSPENDED, NULL);
	if (hDownLoadThread)
	{
		SetThreadPriority(hDownLoadThread, THREAD_PRIORITY_LOWEST);
		ResumeThread(hDownLoadThread);
	}
}

CWebUserObject::CWebUserObject()
{
	m_nEMObType = OBJECT_WEBUSER;
	memset(&exinfo, '\0', sizeof(WEBUSEREX_INFO));
	memset(&info, '\0', sizeof(WEBUSER_INFO));
	memset(&onlineinfo, '\0', sizeof(WEBONLINE_INFO));
	info.uid = SYS_WEBUSER;

	m_bNewComm = false;
	m_sNewSeq = 0;

	webuserid = 0;
	talkuid = 0;
	strcpy(prevurl, "");
	strcpy(prevurlhost, "");
	strcpy(prevurlvar, "");

	m_nWaitTimer = -20;
	m_resptimeoutmsgtimer = -20;
	m_resptimeoutclosetimer = -20;
	m_waitresptimeouttimer = -20;

	nTimer = 0;
	nVisitNum = 0;
	nTalkNum = 0;
	nLastVisitTime = 0;

	m_nFlag = 0;

	m_bConnected = false;
	m_bIsGetInfo = false;

	m_onlinetime = 0;
	m_bIsShow = false;

	talkuid = 0;
	transferuid = 0;
	inviteuid = SYS_WEBUSER;
	m_bNotResponseUser = 0;

	floatadminuid = 0;

	floatfromadminuid = 0;
	floatfromsort = 0;

	strcpy(chatid, "");
	gpid = 0;

	nVisitNum = 0;
	nTalkNum = 0;

	m_refuseinvite = 0;
	nVisitFrom = WEBUSERICON_DIRECTURL;
	m_bIsFrWX = false;
	cTalkedSatus = 0;
	m_pWxUserInfo = NULL;

	GetNormalChatHisMsgSuccess = false;
	tGetNormalChatHismsgTime = 0;
}

CWebUserObject::~CWebUserObject()
{
	
}

void CWebUserObject::AddCommonTalkId(unsigned long uid)
{
	if (uid == NULL)
		return;

	if (IsExistCommonTalkId(uid))
		return;

	m_listCommonTalkId.push_back(uid);
}

bool CWebUserObject::IsExistCommonTalkId(unsigned long uid)
{
	list<unsigned long>::iterator iter = m_listCommonTalkId.begin();
	for (iter; iter != m_listCommonTalkId.end(); iter++)
	{
		if ((*iter) == uid)
		{
			return true;
		}
	}
	return false;
}

bool CWebUserObject::IsOnline()
{
	if (cTalkedSatus == INTALKING || !m_mapUrlAndScriptFlagOb.empty())
	{
		return true;
	}
	return false;
}

int CWebUserObject::IsForbid()
{
	return ::GetByte(exinfo.comauth, WEBUSERAUTH_FORBID);
}

bool CWebUserObject::IsDisplay(CSysConfigFile *pConfig, unsigned long uid)
{
	if (pConfig == NULL)
		return true;

	bool bDisplay = false;
	int i;
	unsigned long curtime = ::GetTimeLong();

	switch (pConfig->m_nFilterType)
	{
	case VISITORFILTER_ALL:
		bDisplay = true;
		break;
	case VISITORFILTER_MYVISITOR:
		if (this->talkuid == uid)
		{
			bDisplay = true;
		}
		else
		{
			if (IsIDIsMutiUser(uid))
				bDisplay = true;
		}
		break;
	case VISITORFILTER_ALLVISITOR:
		bDisplay = true;
		break;

	case VISITORFILTER_1MINUTES:
		if (curtime - this->m_onlinetime > 60)
			bDisplay = true;
		break;
	case VISITORFILTER_3MINUTES:
		if (curtime - this->m_onlinetime > 180)
			bDisplay = true;
		break;
	case VISITORFILTER_5MINUTES:
		if (curtime - this->m_onlinetime > 300)
			bDisplay = true;
		break;
	case VISITORFILTER_10MINUTES:
		if (curtime - this->m_onlinetime > 600)
			bDisplay = true;
		break;
	case VISITORFILTER_USERDEFINE:
		bDisplay = true;
		if (pConfig->m_cWebUserfilter.include == 0)
		{
			for (i = 0; i < MAX_STRINGFILTER_NUM; i++)
			{
				if (strlen(pConfig->m_cWebUserfilter.text[i]) <= 0)
					break;

				if (strstr(this->info.ipfromname, pConfig->m_cWebUserfilter.text[i]) == NULL)
				{
					bDisplay = false;
					break;
				}
			}
		}
		else
		{
			for (i = 0; i < MAX_STRINGFILTER_NUM; i++)
			{
				if (strlen(pConfig->m_cWebUserfilter.text[i]) <= 0)
					break;

				if (strstr(this->info.ipfromname, pConfig->m_cWebUserfilter.text[i]) != NULL)
				{
					bDisplay = false;
					break;
				}
			}
		}
		break;
	}

	return bDisplay;
}

void CWebUserObject::SetForbid(bool bForbid)
{
	exinfo.comauth = ::SetByte(exinfo.comauth, WEBUSERAUTH_FORBID, bForbid);
}

bool CWebUserObject::IsIDIsMutiUser(unsigned long uid)
{
	list<unsigned long>::iterator iter = m_listCommonTalkId.begin();

	for (iter; iter != m_listCommonTalkId.end(); iter++)
	{
		if (*iter == uid)
		{
			return true;
		}
	}
	return false;
}

void CWebUserObject::AddMutiUser(unsigned long uid)
{
	if (!IsIDIsMutiUser(uid))
	{
		m_listCommonTalkId.push_back(uid);
	}
}

void CWebUserObject::RemoveMutiUser(unsigned long uid)
{
	list<unsigned long>::iterator iter = m_listCommonTalkId.begin();

	for (iter; iter != m_listCommonTalkId.end(); iter++)
	{
		if (*iter == uid)
		{
			m_listCommonTalkId.erase(iter);
			return;
		}
	}
}

bool CWebUserObject::IsMutiUser()
{
	return m_listCommonTalkId.size() > 1;
}

void CWebUserObject::RemoveAllMutiUser()
{
	m_listCommonTalkId.clear();
}

bool CWebUserObject::ScriptFlagIsExist(char *scriptflag)
{
	MapWebUserFLag::iterator iter = m_mapUrlAndScriptFlagOb.find(scriptflag);
	if (iter != m_mapUrlAndScriptFlagOb.end())
	{
		return true;
	}
	return false;
}

void CWebUserObject::AddScriptFlag(char *scriptflag, char *url)
{
	if (scriptflag == NULL || strlen(scriptflag) <= 0)
		return;
	if (url == NULL)
		return;

	if (!ScriptFlagIsExist(scriptflag))
	{
		WEBUSER_URL_INFO* info = new WEBUSER_URL_INFO();
		info->dwtime = GetTimeLong();
		info->url = url;
		info->scriptFlag = scriptflag;
		m_mapUrlAndScriptFlagOb.insert(MapWebUserFLag::value_type(scriptflag, info));
	}
}

void CWebUserObject::DeleteScriptFlag(char *scriptflag)
{
	MapWebUserFLag::iterator iter = m_mapUrlAndScriptFlagOb.find(scriptflag);
	if (iter != m_mapUrlAndScriptFlagOb.end())
	{
		delete iter->second;
		m_mapUrlAndScriptFlagOb.erase(iter);
	}
}

void CWebUserObject::DeleteAllScriptFlag()
{
	MapWebUserFLag::iterator iter = m_mapUrlAndScriptFlagOb.begin();
	for (iter; iter != m_mapUrlAndScriptFlagOb.end(); iter++)
	{
		delete iter->second;
	}
	m_mapUrlAndScriptFlagOb.clear();
}

WEBUSER_URL_INFO* CWebUserObject::GetScriptFlagOb(char *scriptflag)
{
	MapWebUserFLag::iterator iter = m_mapUrlAndScriptFlagOb.find(scriptflag);
	if (iter != m_mapUrlAndScriptFlagOb.end())
	{
		return iter->second;
	}
	return NULL;
}

WEBUSER_URL_INFO* CWebUserObject::GetLastScriptFlagOb()
{
	unsigned long dwtime = 0;
	WEBUSER_URL_INFO* pUrl = NULL;
	MapWebUserFLag::iterator iter = m_mapUrlAndScriptFlagOb.begin();
	for (iter; iter != m_mapUrlAndScriptFlagOb.end(); iter++)
	{
		if (iter->second->dwtime > dwtime)
		{
			pUrl = iter->second;
			dwtime = iter->second->dwtime;
		}
	}
	return pUrl;
}

//组对象

CGroupObject::CGroupObject()
{
	m_nEMObType = OBJECT_GROUP;
	id = 0;
}

CGroupObject::CGroupObject(unsigned long id, string name)
{
	id = id;
	strName = name;
}

void CGroupObject::DeleteAll()
{
	DeleteAllGroup();
	DeleteAllUser();
}

void CGroupObject::DeleteAllGroup()
{
	list<CGroupObject *>::iterator iter = m_ListGroupInfo.begin();
	for (iter; iter != m_ListGroupInfo.end(); iter++)
	{
		delete *iter;
	}

	m_ListGroupInfo.clear();
}

void CGroupObject::DeleteAllUser()
{
	list<CUserObject*>::iterator iter = m_ListUserInfo.begin();
	for (iter; iter != m_ListUserInfo.end(); iter++)
	{
		delete *iter;
	}

	m_ListUserInfo.clear();
}

CGroupObject::~CGroupObject()
{
	DeleteAllUser();
}

CGroupObject *CGroupObject::AddGroupObject(unsigned long id, string name)
{
	CGroupObject *pGroup = new CGroupObject(id, name);

	m_ListGroupInfo.push_back(pGroup);
	return pGroup;
}

void CGroupObject::AddGroupObject(CGroupObject * pGroup)
{
	m_ListGroupInfo.push_back(pGroup);
}

CUserObject *CGroupObject::AddUserObject(unsigned long id, string name, char sex)
{
	CUserObject *pUser = new CUserObject();

	pUser->UserInfo.uid = id;
	sprintf(pUser->UserInfo.nickname, "%s", name.c_str());

	m_ListUserInfo.push_back(pUser);
	return pUser;
}