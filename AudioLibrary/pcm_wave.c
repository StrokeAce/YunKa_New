#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include "pcm_wave.h"

#define PCM_FRAME_SIZE 160 // 8khz 8000*0.02=160
#define MAX_AMR_FRAME_SIZE 32
 
int writeFile2Int(FILE *fp,int nWhere,int nValue)  
{  
	union DWORD_CHAR dc;

	if(fp==NULL)  
	{  
		return 0;  
	}  

	fseek(fp,nWhere,SEEK_SET);
	
	dc.nValue=nValue;  
	fwrite(dc.charBuf,1,4,fp);        
	return 1;  
}  

void writeWaveHead(FILE *fp) 
{  
	if (fp)  
	{  
		//写WAV文件头  
		struct RIFF_HEADER rh;
		struct FMT_BLOCK fb;
		char buf[]={"data0000"};

		memset(&rh,0,sizeof(rh));  
		strncpy(rh.szRiffFormat,"WAVE",4);  
		strncpy(rh.szRiffID,"RIFF",4); 

		fwrite(&rh,1,sizeof(rh),fp); 
		  
		strncpy(fb.szFmtID,"fmt ",4);  
		fb.dwFmtSize = 16;  
		fb.wavFormat.wFormatTag = 0x0001;  
		fb.wavFormat.wChannels = 1;  
		fb.wavFormat.wBitsPerSample = 16;         
		fb.wavFormat.dwSamplesPerSec = 8000;         
		fb.wavFormat.wBlockAlign = fb.wavFormat.wChannels*fb.wavFormat.wBitsPerSample/8;
		fb.wavFormat.dwAvgBytesPerSec = fb.wavFormat.dwSamplesPerSec * fb.wavFormat.wBlockAlign;


		fwrite(&fb,1,sizeof(fb),fp);
		 
		fwrite(buf,1,sizeof(buf),fp);  
	}  
}  

void writeWaveBody(FILE *fp,long filelength)  
{  
	//更新WAV文件dwRiffSize字段中的值  
	int nWhere = 4;  
	writeFile2Int(fp,nWhere, filelength - 8);  

	//更新WAV文件DataChunk中Size字段的值  
	nWhere=sizeof(struct RIFF_HEADER)+sizeof(struct FMT_BLOCK)+4;  
	writeFile2Int(fp,nWhere,filelength - (sizeof(struct RIFF_HEADER)+sizeof(struct FMT_BLOCK)+8) );  
}

#ifndef ETSI
#ifndef IF2
#include <string.h>
#define AMR_MAGIC_NUMBER "#!AMR\n"
#endif
#endif

#define UNIT 4

int AMR_To_PCM(char* amrPath, char* pcmPath)
{
	FILE * file_speech;
	FILE * file_analysis;
	short synth[160];
	int frames = 0;
	int * destate;
	int read_size;
#ifndef ETSI
	unsigned char analysis[32];
	enum Mode dec_mode;
#ifdef IF2
	short block_size[16]={ 12, 13, 15, 17, 18, 20, 25, 30, 5, 0, 0, 0, 0, 0, 0, 0 };
#else
	char magic[8];
	short block_size[16]={ 12, 13, 15, 17, 19, 20, 26, 31, 5, 0, 0, 0, 0, 0, 0, 0 };
#endif
#else
	short analysis[250];
#endif

	file_speech = fopen(pcmPath, "wb");
	if (file_speech == NULL)
	{
		return 0;
	}

	file_analysis = fopen(amrPath, "rb");
	if (file_analysis == NULL)
	{
		fclose(file_speech);
		return 0;
	}

	/* init decoder */
	destate = Decoder_Interface_init();

#ifndef ETSI
#ifndef IF2
	/* read and verify magic number */
	fread( magic, sizeof( char ), strlen( AMR_MAGIC_NUMBER ), file_analysis );
	if ( strncmp( magic, AMR_MAGIC_NUMBER, strlen( AMR_MAGIC_NUMBER ) ) ) 
	{
		fclose( file_speech );
		fclose( file_analysis );
		return 0;
	}
#endif
#endif

#ifndef ETSI

	/* find mode, read file */
	while (fread(analysis, sizeof (unsigned char), 1, file_analysis ) > 0)
	{
#ifdef IF2
		dec_mode = analysis[0] & 0x000F;
#else
		dec_mode = (analysis[0] >> 3) & 0x000F;
#endif
		read_size = block_size[dec_mode];

		fread(&analysis[1], sizeof (char), read_size, file_analysis );
#else

	read_size = 250;
	/* read file */
	while (fread(analysis, sizeof (short), read_size, file_analysis ) > 0)
	{
#endif

		frames ++;

		/* call decoder */
		Decoder_Interface_Decode(destate, analysis, synth, 0);

		fwrite( synth, sizeof (short), 160, file_speech );
	}

	Decoder_Interface_exit(destate);

	fclose(file_speech);
	fclose(file_analysis);
	return 1;
}

int PCM_To_WAV(char* pcmPath, char* wavPath)
{
	FILE *fpS;  
	FILE *fpD;
	long filelength;  
	char buf[UNIT]; 

	fpS = fopen(pcmPath,"rb");  
	fpD = fopen(wavPath,"wb+");  
	if(fpS==NULL||fpD==NULL)  
	{
		return 0;  
	}  
	fseek(fpS, 0, SEEK_END);  
	filelength = ftell(fpS);  
	writeWaveHead(fpD);  
	writeWaveBody(fpD,filelength);  

	fseek(fpS,44,SEEK_SET);  

	while(UNIT==fread(buf,1,UNIT,fpS))  
	{  
		fwrite(buf,1,UNIT,fpD);  
	}  
	fclose(fpS);  
	fclose(fpD);

	remove(pcmPath);
	return 1;
}

// 从WAVE文件中跳过WAVE文件头，直接到PCM音频数据
void Skip_PCM_Data(FILE* fpwave)
{
	struct RIFF_HEADER riff;
	struct FMT_BLOCK fmt;
	struct XCHUNK_HEADER chunk;
	struct WAVE_FORMAT wfx;
	int bDataBlock = 0;

	// 1. 读RIFF头
	fread(&riff, 1, sizeof(struct RIFF_HEADER), fpwave);

	// 2. 读FMT块 - 如果 fmt.nFmtSize>16 说明需要还有一个附属大小没有读
	fread(&chunk, 1, sizeof(struct XCHUNK_HEADER), fpwave);
	if (chunk.nChunkSize > 16)
	{
		fread(&wfx, 1, sizeof(struct WAVE_FORMAT), fpwave);
	}
	else
	{
		memcpy(fmt.szFmtID, chunk.chChunkID, 4);
		fmt.dwFmtSize = chunk.nChunkSize;
		fread(&fmt.wavFormat, 1, sizeof(struct WAVE_FORMAT), fpwave);
	}

	// 3.转到data块 - 有些还有fact块等。
	while (!bDataBlock)
	{
		fread(&chunk, 1, sizeof(struct XCHUNK_HEADER), fpwave);
		if (!memcmp(chunk.chChunkID, "data", 4))
		{
			bDataBlock = 1;
			break;
		}
		// 因为这个不是data块,就跳过块数据
		fseek(fpwave, chunk.nChunkSize, SEEK_CUR);
	}
}

// 从WAVE文件读一个完整的PCM音频帧
// 返回值: 0-错误 >0: 完整帧大小
int Read_PCM_Frame(short speech[], FILE* fpwave, int nChannels, int nBitsPerSample)
{
	int nRead = 0;
	int x = 0, y = 0;
	unsigned short ush1 = 0, ush2 = 0, ush = 0;

	// 原始PCM音频帧数据
	unsigned char  pcmFrame_8b1[PCM_FRAME_SIZE];
	unsigned char  pcmFrame_8b2[PCM_FRAME_SIZE << 1];
	unsigned short pcmFrame_16b1[PCM_FRAME_SIZE];
	unsigned short pcmFrame_16b2[PCM_FRAME_SIZE << 1];

	if (nBitsPerSample == 8 && nChannels == 1)
	{
		nRead = fread(pcmFrame_8b1, (nBitsPerSample / 8), PCM_FRAME_SIZE*nChannels, fpwave);
		for (x = 0; x < PCM_FRAME_SIZE; x++)
		{
			speech[x] = (short)((short)pcmFrame_8b1[x] << 7);
		}
	}
	else
		if (nBitsPerSample == 8 && nChannels == 2)
		{
			nRead = fread(pcmFrame_8b2, (nBitsPerSample / 8), PCM_FRAME_SIZE*nChannels, fpwave);
			for (x = 0, y = 0; y < PCM_FRAME_SIZE; y++, x += 2)
			{
				// 1 - 取两个声道之左声道
				speech[y] = (short)((short)pcmFrame_8b2[x + 0] << 7);
				// 2 - 取两个声道之右声道
				//speech[y] =(short)((short)pcmFrame_8b2[x+1] << 7);
				// 3 - 取两个声道的平均值
				//ush1 = (short)pcmFrame_8b2[x+0];
				//ush2 = (short)pcmFrame_8b2[x+1];
				//ush = (ush1 + ush2) >> 1;
				//speech[y] = (short)((short)ush << 7);
			}
		}
		else
			if (nBitsPerSample == 16 && nChannels == 1)
			{
				nRead = fread(pcmFrame_16b1, (nBitsPerSample / 8), PCM_FRAME_SIZE*nChannels, fpwave);
				for (x = 0; x < PCM_FRAME_SIZE; x++)
				{
					speech[x] = (short)pcmFrame_16b1[x + 0];
				}
			}
			else
				if (nBitsPerSample == 16 && nChannels == 2)
				{
					nRead = fread(pcmFrame_16b2, (nBitsPerSample / 8), PCM_FRAME_SIZE*nChannels, fpwave);
					for (x = 0, y = 0; y < PCM_FRAME_SIZE; y++, x += 2)
					{
						//speech[y] = (short)pcmFrame_16b2[x+0];
						speech[y] = (short)((int)((int)pcmFrame_16b2[x + 0] + (int)pcmFrame_16b2[x + 1])) >> 1;
					}
				}

	// 如果读到的数据不是一个完整的PCM帧, 就返回0
	if (nRead < PCM_FRAME_SIZE*nChannels) return 0;

	return nRead;
}

int WAV_To_AMR(const char* pchWAVEFilename, const char* pchAMRFileName, int nChannels, int nBitsPerSample)
{
	FILE* fpwave;
	FILE* fpamr;

	/* input speech vector */
	short speech[160];

	/* counters */
	int byte_counter, frames = 0, bytes = 0;

	/* pointer to encoder state structure */
	void *enstate;

	/* requested mode */
	enum Mode req_mode = MR122;
	int dtx = 0;

	/* bitstream filetype */
	unsigned char amrFrame[MAX_AMR_FRAME_SIZE];

	fpwave = fopen(pchWAVEFilename, "rb");
	if (fpwave == NULL)
	{
		return 0;
	}

	// 创建并初始化amr文件
	fpamr = fopen(pchAMRFileName, "wb");
	if (fpamr == NULL)
	{
		fclose(fpwave);
		return 0;
	}
	/* write magic number to indicate single channel AMR file storage format */
	bytes = fwrite(AMR_MAGIC_NUMBER, sizeof(char), strlen(AMR_MAGIC_NUMBER), fpamr);

	/* skip to pcm audio data*/
	Skip_PCM_Data(fpwave);

	enstate = Encoder_Interface_init(dtx);

	while (1)
	{
		// read one pcm frame
		if (!Read_PCM_Frame(speech, fpwave, nChannels, nBitsPerSample)) break;

		frames++;

		/* call encoder */
		byte_counter = Encoder_Interface_Encode(enstate, req_mode, speech, amrFrame, 0);

		bytes += byte_counter;
		fwrite(amrFrame, sizeof(unsigned char), byte_counter, fpamr);
	}

	Encoder_Interface_exit(enstate);

	fclose(fpamr);
	fclose(fpwave);

	return frames;
}