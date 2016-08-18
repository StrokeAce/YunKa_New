/************************************************************************

		Submodule	: log.h
		Depict		: Write log Interface
		Language	: ANSI C++
		Version		: 3.0
		Compile		: make
		Compat		: Linux/Windows
		Editor		: yanyg
		Time		: 2007 07 15

************************************************************************/

//log.h: interface for the CLog class.

/************************************************************************

		ʹ	��	˵	��

1. ȷ��ʹ��ǰ��ʼ��;
2. ���Զ���������༶Ŀ¼�Ҳ�����ĳ�����Ŀ¼;
2. ����־���Ѿ�����һ����Ϊg_WriteLogʵ�����;
3. ����֧��C/C++��������;����:
	int iExample(19831107);
	string strExample("19831107");
	g_WriteLog.WriteLog( C_LOG_SYSNOTICE , "��־��������������.������C�÷�����:\r\n");
	g_WriteLog.WriteLog( C_LOG_SYSNOTICE , "i = %d ; str = %s .\r\n" , iExample , strExample.c_str() );
	g_WriteLog<<CPP_LOG_SYSNOTICE<<"��־��������������.";
	g_WriteLog<<CPP_LOG_SYSNOTICE<<"i = "<<iExample<<"; str = "<<strExample<<CPP_LOG_END;
   ע : ������CPP_LOG_END��β,��������!!!
4. �����Ը���д��־Ч��Ϊ 200000��/��(C++��ʽ����) �� 125000��/��(Cʽ����)
	������Ϣ:
		ϵͳ���� : CPU 2.4HZ / MEM 512M 
		ÿ���ֽ� : 68Byte
		����ϵͳ : Linux Redhat9

************************************************************************/

#ifndef __WRITELOG_H_
#define __WRITELOG_H_

#include "mutex.h"

#include <string>
#include <fstream>

enum
{
	C_LOG_NONE = 0 ,		//	no any log will be write
	C_LOG_SYSNOTICE = 1 ,		//	system informational notices
	C_LOG_FATAL = 2 ,		//	fatal errors
	C_LOG_ERROR = 3 ,		//	error!
	C_LOG_WARNING = 4 ,		//	exceptional events
	C_LOG_NOTICE = 5 ,		//	informational notices
	C_LOG_DEBUG = 6 ,		//	full debugging
	C_LOG_TRACE = 7 ,		//	program tracing  
	C_LOG_ALL = 8 ,			//	everything
};

#define CPP_LOG_NONE		CLogLevel<C_LOG_NONE>()		//	no any log will be write
#define CPP_LOG_SYSNOTICE	CLogLevel<C_LOG_SYSNOTICE>()	//	system informational notices
#define CPP_LOG_FATAL		CLogLevel<C_LOG_FATAL>()	//	fatal errors
#define CPP_LOG_ERROR		CLogLevel<C_LOG_ERROR>()	//	error!
#define CPP_LOG_WARNING		CLogLevel<C_LOG_WARNING>()	//	exceptional events
#define CPP_LOG_NOTICE		CLogLevel<C_LOG_NOTICE>()	//	informational notices
#define CPP_LOG_DEBUG		CLogLevel<C_LOG_DEBUG>()	//	full debugging
#define CPP_LOG_TRACE		CLogLevel<C_LOG_TRACE>()	//	program tracing
#define CPP_LOG_ALL		CLogLevel<C_LOG_ALL>()		//	everything
#define CPP_LOG_END		CLogUnLockType()		//	unLock the log stream
#define CPP_LOG_FLUSH_END		CLogFlushUnLockType()		//	 flush and unLock the log stream

template<int ilevel>struct CLogLevel{};
struct CLogUnLockType{};
struct CLogFlushUnLockType{};

class CLog
{
public:
	CLog():m_bWrite(false),m_iLogDay(-1),m_iFlush(C_LOG_ALL){}

	void SetLogLevel( unsigned int uiLogLevel ) { m_uiLogLevel = uiLogLevel <= C_LOG_ALL ? uiLogLevel : C_LOG_ALL; }
	bool InitLog( const std::string& strLogPath , const std::string& strLogName , unsigned int uiLogLevel );
	void WriteLog( unsigned int uiLogLevel , const char* fmt , ... );
	void WriteStr(  const char* strerror ,unsigned int uiLogLevel);


	template< typename T > CLog& operator<<( const T& t ) { if(m_bWrite)m_LogStream<<t; return *this;  }
	CLog& operator<<( const CLogUnLockType ) { return m_LogLock.Unlock(), *this; }
	CLog& operator<<( const CLogFlushUnLockType ) { return m_LogStream.flush(), m_LogLock.Unlock(), *this; }

	template<int iLevel> CLog& operator<<( const CLogLevel<iLevel> ) { return m_LogLock.Lock(),MakeLogTag(iLevel),*this; }

private:
	void MakeLogTag(const unsigned int iLevel );
	bool CreateDir( );
private:
	bool m_bWrite;
	unsigned int m_uiLogLevel;	//��־����
	int m_iLogDay;			//����(��):ÿ�����һ��
	int m_iFlush;			//ˢ�¼���
	std::string m_strLogPath;	//��־·��
	std::string m_strlogName;	//��־��
	std::string m_strAllName;	//��־ȫ�� : path+(year+month+day)+name
	std::string m_strBakeName;	//�ڶ����ƶ���һ����־��bake
	std::ofstream m_LogStream;	//��־�ļ���
	MMutex m_LogLock;		//Linux��־�ļ���
};

extern CLog g_WriteLog,g_VisitLog;

#endif
