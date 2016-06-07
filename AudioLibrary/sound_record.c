#include "sound_record.h"

#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <time.h>
#include <mmsystem.h>
#include <windows.h>
#include <string.h>
#include "pcm_wave.h"
#pragma comment(lib, "winmm.lib")

#define MAX_BUFF_SOUNDSIZE 8000
#define TIMER_WAIT WM_USER+232
#define TIME_STOP 60000

#define CODE_AUDIO_SUCCESS 0 // �ɹ�
#define CODE_AUDIO_NO_DEVICE 1 // û��¼���豸
#define CODE_AUDIO_OPEN_FAIL 2 // ��¼���豸ʧ��
#define CODE_AUDIO_FILE_FAIL 3 // ���ļ�ʧ��

int Init();
void SaveWav();//����ΪWAV�ļ�
DWORD CALLBACK waveInProc(HWAVEIN hwi, UINT uMsg,DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);//�ص���������������¼���ͻ���øú���
DWORD CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime);

static int IsRecord = 0;//ֹͣ��־����Ϊ������WIM_DATA �ǲ���AddBuff��Ҫ����������ȥ���ƣ�ֹͣ
WAVEFORMATEX  m_soundFormat;//������ʽ
HWAVEIN     m_hWaveIn;
WAVEHDR     m_pWaveHdrIn[3];
CHAR     m_cBufferIn[3][MAX_BUFF_SOUNDSIZE];
CHAR g_cFilePath[MAX_PATH];
CHAR g_cVoicePath[MAX_PATH];
UINT g_timerid = -1;
DWORD g_start_time = 0;
FILE*fp;  //����Ϊ�ļ���ֻ��Ϊ�˸���һ�ݴ���ʹ�ã�

int Start_Record_WAV(const char* voicePath, const char* voiceName)
{
	memset(g_cFilePath, 0, sizeof(g_cFilePath));
	memset(g_cVoicePath, 0, sizeof(g_cVoicePath));
	strcat(g_cFilePath, voicePath);
	strcat(g_cFilePath, voiceName);
	strcat(g_cVoicePath, g_cFilePath);
	strcat(g_cVoicePath, ".wav");
	int returnCode = Init();
	if (returnCode == CODE_AUDIO_SUCCESS)
	{
		g_timerid = SetTimer(NULL, TIMER_WAIT, TIME_STOP, TimerProc);
		g_start_time = GetTickCount();
	}
	return returnCode;
}

int Stop_Record_WAV()
{
	if (g_timerid != -1)
	{
		// ��ʱ��δ�ر�ʱ���رն�ʱ��
		KillTimer(NULL, g_timerid);
		g_timerid = -1;
	}

	if (IsRecord != 0)
	{
		IsRecord = 0;		

		// ����1�����Ƶ�ű��棬��������Ч��Ƶ�����豣��
		DWORD end_time = GetTickCount();
		if (end_time - g_start_time < 1000)
			return -1;
		else
			SaveWav();
	}
	return 0;
}

void Cancel_Record_WAV()
{
	if (g_timerid != -1)
	{
		// �رն�ʱ��
		KillTimer(NULL, g_timerid);
		g_timerid = -1;
	}

	if (IsRecord != 0)
	{
		// ȡ��¼��
		IsRecord = 0;

		waveInStop(m_hWaveIn);
		waveInReset(m_hWaveIn);
		waveInClose(m_hWaveIn);
		fclose(fp);
	}
}

DWORD CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
	if (IsRecord != 0)
	{
		IsRecord = 0;
		SaveWav();
	}
	IsRecord = 0;

	if (g_timerid != -1)
	{
		KillTimer(NULL, g_timerid);
		g_timerid = -1;
	}
	return 1;
}

int Init()
{
	fp = fopen(g_cFilePath, "wb");
	if (fp == NULL)
	{
		return CODE_AUDIO_FILE_FAIL;
	}
	int n = waveInGetNumDevs(); //��ȡ���������豸������û�о��˳�
	if (n < 1)
	{
		return CODE_AUDIO_NO_DEVICE;
	}
	memset(&m_soundFormat, 0, sizeof(m_soundFormat));//����������ʽ
	m_soundFormat.wFormatTag = WAVE_FORMAT_PCM;
	m_soundFormat.nChannels = 1;
	m_soundFormat.nSamplesPerSec = 8000;
	m_soundFormat.nAvgBytesPerSec = 16000;
	m_soundFormat.nBlockAlign = 2;
	m_soundFormat.wBitsPerSample = 16;
	m_soundFormat.cbSize = 0;

	MMRESULT m_res;

	m_res = waveInOpen(&m_hWaveIn, WAVE_MAPPER, &m_soundFormat, (DWORD)(waveInProc), 0, CALLBACK_FUNCTION);//���豸
	if (m_res != 0)
	{
		return CODE_AUDIO_OPEN_FAIL;
	}
	unsigned int id;
	waveInGetID(m_hWaveIn, &id);//����ʹ�õ������豸ID��-1ΪĬ��
	int i;
	for (i = 0; i < 3; i++) //�����ڴ���ʽ
	{
		m_pWaveHdrIn[i].lpData = m_cBufferIn[i];
		m_pWaveHdrIn[i].dwBufferLength = MAX_BUFF_SOUNDSIZE;
		m_pWaveHdrIn[i].dwBytesRecorded = 0;
		m_pWaveHdrIn[i].dwUser = i;
		m_pWaveHdrIn[i].dwFlags = 0;
		waveInPrepareHeader(m_hWaveIn, &m_pWaveHdrIn[i], sizeof(WAVEHDR)); //׼���ڴ��¼��
		waveInAddBuffer(m_hWaveIn, &m_pWaveHdrIn[i], sizeof(WAVEHDR)); //�����ڴ��
	}
	IsRecord = 1;
	waveInStart(m_hWaveIn);//��ʼ¼��
	return CODE_AUDIO_SUCCESS;
}

DWORD CALLBACK waveInProc(HWAVEIN hwi, UINT uMsg,
	DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	if (uMsg == WIM_DATA)
	{
		//����������Ļ��Ϳ����������ȡ���ݣ�ѹ������
		WAVEHDR* p = (WAVEHDR*)dwParam1;//dwParam1ָ��WAVEHDR�ĵ�ַ
		printf("getdate id:%d size:%d timestemp:%d\n", p->dwUser, p->dwBytesRecorded, clock());
		int i = p->dwUser;
		if (!IsRecord) //0��ʾֹͣ��
		{
			return 0;
		}
		else
		{
			fwrite(&m_cBufferIn[i], 1, p->dwBytesRecorded, fp);
			waveInUnprepareHeader(m_hWaveIn, p, sizeof(WAVEHDR)); //�ͷ�
		}
		p->lpData = m_cBufferIn[i];
		p->dwBufferLength = MAX_BUFF_SOUNDSIZE;
		p->dwBytesRecorded = 0;
		p->dwUser = i;
		p->dwFlags = 0;
		waveInPrepareHeader(m_hWaveIn, p, sizeof(WAVEHDR)); //׼���ڴ��¼��
		waveInAddBuffer(m_hWaveIn, p, sizeof(WAVEHDR)); //�����ڴ��
	}
	return 0;
}

void SaveWav()
{
	waveInStop(m_hWaveIn);
	waveInReset(m_hWaveIn);
	waveInClose(m_hWaveIn);
	fclose(fp);

	struct RIFF_HEADER m_riff = { 0 };
	struct FMT_BLOCK m_fmt = { 0 };
	struct DATA_BLOCK m_data = { 0 };
	FILE*fwav = fopen(g_cVoicePath, "wb");
	fp = fopen(g_cFilePath, "rb");
	if (fwav == NULL || fp == NULL)
	{
		printf("err\n");
		return;
	}
	long filesize = 0;
	long n = 0;
	//��ȡ��������
	fseek(fp, 0, SEEK_END);
	filesize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	
	//RIFF
	strncpy(m_riff.szRiffID, "RIFF", 4);
	m_riff.dwRiffSize = 4 + sizeof(struct FMT_BLOCK) + sizeof(struct DATA_BLOCK) + filesize;
	strncpy(m_riff.szRiffFormat, "WAVE", 4);
	//fmt
	strncpy(m_fmt.szFmtID, "fmt ", 4);
	m_fmt.dwFmtSize = sizeof(struct WAVE_FORMAT);
	m_fmt.wavFormat = *(struct WAVE_FORMAT*)&m_soundFormat;
	//Data
	strncpy(m_data.szDataID, "data", 4);
	m_data.dwDataSize = filesize;
	fwrite(&m_riff, 1, sizeof(struct RIFF_HEADER), fwav);
	fwrite(&m_fmt, 1, sizeof(struct FMT_BLOCK), fwav);
	fwrite(&m_data, 1, sizeof(struct DATA_BLOCK), fwav);

	while ((n = fread(m_cBufferIn[0], 1, MAX_BUFF_SOUNDSIZE, fp)) > 0)
		fwrite(m_cBufferIn[0], 1, n, fwav);

	fclose(fwav);
	fclose(fp);
}