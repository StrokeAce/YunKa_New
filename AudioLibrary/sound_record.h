#ifndef _sound_record_h_
#define _sound_record_h_

// 开始录音，最多录制60秒
int Start_Record_WAV(char* voicePath, char* voiceName);

// 结束录音
int Stop_Record_WAV();

// 取消录音
void Cancel_Record_WAV();

#endif