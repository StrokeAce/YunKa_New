#ifndef _pcm_wave_h_
#define _pcm_wave_h_


#define DWORD unsigned long
#define WORD unsigned short

struct RIFF_HEADER  
{  
	char szRiffID[4];  // 'R','I','F','F'  
	DWORD dwRiffSize;  
	char szRiffFormat[4]; // 'W','A','V','E'  
};  

struct WAVE_FORMAT  
{  
	WORD wFormatTag;  
	WORD wChannels;  
	DWORD dwSamplesPerSec;  
	DWORD dwAvgBytesPerSec;  
	WORD wBlockAlign;  
	WORD wBitsPerSample;
};

struct WAVE_FORMATX
{
	WORD wFormatTag;
	WORD wChannels;
	DWORD dwSamplesPerSec;
	DWORD dwAvgBytesPerSec;
	WORD wBlockAlign;
	WORD wBitsPerSample;
	WORD nExSize;
};

struct FMT_BLOCK  
{  
	char  szFmtID[4]; // 'f','m','t',' '  
	DWORD  dwFmtSize;  
	struct WAVE_FORMAT wavFormat;
};

struct XCHUNK_HEADER
{
	char chChunkID[4];
	int nChunkSize;
};

struct DATA_BLOCK  
{  
	char szDataID[4]; // 'd','a','t','a'  
	DWORD dwDataSize;  
};  

union DWORD_CHAR
{  
	int  nValue;  
	char charBuf[4];
};

enum Mode {
	MR475 = 0,/* 4.75 kbps */
	MR515,    /* 5.15 kbps */
	MR59,     /* 5.90 kbps */
	MR67,     /* 6.70 kbps */
	MR74,     /* 7.40 kbps */
	MR795,    /* 7.95 kbps */
	MR102,    /* 10.2 kbps */
	MR122,    /* 12.2 kbps */
	MRDTX,    /* DTX       */
	N_MODES   /* Not Used  */
};

// amr转pcm
int AMR_To_PCM(char* amrPath, char* pcmPath);

// pcm转amr
int PCM_To_WAV(char* pcmPath, char* wavPath);

// wav转amr
// WAVE音频采样频率是8khz 
// 音频样本单元数 = 8000*0.02 = 160 (由采样频率决定)
// 声道数 1 : 160
//        2 : 160*2 = 320
// bps决定样本(sample)大小
// bps = 8 --> 8位 unsigned char
//       16 --> 16位 unsigned short
int WAV_To_AMR(const char* pchWAVEFilename, const char* pchAMRFileName, int nChannels, int nBitsPerSample);

#endif