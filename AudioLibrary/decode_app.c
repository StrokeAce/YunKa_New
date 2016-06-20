#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <mmsystem.h>
#include <io.h>
#include "sound_record.h"
#pragma comment(lib,"winmm.lib")

// 转码并播放
_declspec(dllexport) void RunAudio( char* amrID)
{
	char wavPath[MAX_PATH];
	ZeroMemory(wavPath, MAX_PATH);		
	strcpy(wavPath, amrID);
	strcat(wavPath, ".wav");
	if (access(wavPath, 0) != 0)
	{
		char amrPath[MAX_PATH];
		char pcmPath[MAX_PATH];

		strcpy(amrPath, amrID);
		strcat(amrPath, ".amr");
		strcpy(pcmPath, amrID);
		strcat(pcmPath, ".pcm");

		// amr文件存在时才进行转码和播放
		if (access(amrPath, 0) == 0)
		{
			AMR_To_PCM(amrPath,pcmPath);
			PCM_To_WAV(pcmPath,wavPath);
			PlaySound((LPCSTR)wavPath, NULL, SND_FILENAME|SND_ASYNC);
		}
	}
	else
	{
		PlaySound((LPCSTR)wavPath, NULL, SND_FILENAME|SND_ASYNC);
	}	
}

// 仅仅是转码
_declspec(dllexport) void AMRToWAV(const char* amrID)
{
	char wavPath[MAX_PATH];
	ZeroMemory(wavPath, MAX_PATH);
	strcpy(wavPath, amrID);
	strcat(wavPath, ".wav");
	if (access(wavPath, 0) != 0)
	{
		char amrPath[MAX_PATH];
		char pcmPath[MAX_PATH];

		strcpy(amrPath, amrID);
		strcat(amrPath, ".amr");
		strcpy(pcmPath, amrID);
		strcat(pcmPath, ".pcm");

		// amr文件存在时才进行转码和播放
		if (access(amrPath, 0) == 0)
		{
			AMR_To_PCM(amrPath, pcmPath);
			PCM_To_WAV(pcmPath, wavPath);
		}
	}
}

// 录制WAV格式的音频
_declspec(dllexport) int StartRecordWAV(const char* voicePath, const char* voiceName)
{
	return Start_Record_WAV(voicePath, voiceName);
}

// 停止音频录制
_declspec(dllexport) int StopRecordWAV()
{
	return Stop_Record_WAV();
}

_declspec(dllexport) void CancelRecordWAV()
{
	Cancel_Record_WAV();
}

// 转码
_declspec(dllexport) void WAVToAMR(char* wavPath)
{
	char t_wavPath[MAX_PATH];
	char t_amrPath[MAX_PATH];
	ZeroMemory(t_wavPath, MAX_PATH);
	ZeroMemory(t_amrPath, MAX_PATH);
	strcpy(t_wavPath, wavPath);
	strcpy(t_amrPath, wavPath);
	strcat(t_wavPath, ".wav");
	strcat(t_amrPath, ".amr");
	WAV_To_AMR(t_wavPath, t_amrPath, 1, 16);
}