///////////////////////////
// WAVE format.
///////////////////////////

#include "wavFormat.h"
#include "../log.h"
#include <stdlib.h>
#include <memory.h>

namespace Wav
{
	void* s_rawData = NULL;
	u32   s_rawSize = 0;
	u32   s_sampleRate = 0;

	bool s_isStereo = false;
	u32 s_bitsPerSample = 0;

	bool read(u8* wav, u32 size)
	{
		u32 index = 0;
		char* riff = (char*)&wav[index]; index += 4;
		if (riff[0] != 'R' || riff[1] != 'I' || riff[2] != 'F' || riff[3] != 'F')
		{
			//invalid wav format
			return false;
		}

		//skip 4 bytes?
		index +=4;

		char* wave = (char*)&wav[index]; index+=4;
		if (wave[0] != 'W' || wave[1] != 'A' || wave[2] != 'V' || wave[3] != 'E')
		{
			//invalid wav format
			return false;
		}

		char* fmt = (char*)&wav[index]; index+=4;
		if (fmt[0] != 'f' || fmt[1] != 'm' || fmt[2] != 't' || fmt[3] != ' ')
		{
			//invalid wav format
			return false;
		}

		//skip 4 bytes.
		index += 4;

		//read audio format.
		s16 audioFormat = *((u16*)&wav[index]); index +=2;
		s16 channels    = *((u16*)&wav[index]); index +=2;
		s32 sampleRate  = *((s32*)&wav[index]); index +=4;
		s32 byteRate    = *((s32*)&wav[index]); index +=4;
		index += 2;
		s16 bitsPerSample = *((u16*)&wav[index]); index +=2;

		//search for the 'data' block
		while (index < size)
		{
			char* dataStr = (char*)&wav[index];
			if (dataStr[0] == 'd' && dataStr[1] == 'a' && dataStr[2] == 't' && dataStr[3] == 'a')
			{
				index+=4;
				break;
			}
			else
			{
				index++;
			}
		}
		if (index >= size)
		{
			return false;
		}

		char* dataStr = (char*)&wav[index-4];
		if (dataStr[0] != 'd' || dataStr[1] != 'a' || dataStr[2] != 't' || dataStr[3] != 'a')
		{
			//invalid wav format
			return false;
		}

		const s32 dataChunkSize = *((s32*)&wav[index]); index +=4;
		s_rawData = (void*)&wav[index];
		s_rawSize = dataChunkSize;

		s_sampleRate = sampleRate;

		s_isStereo = (channels > 1);
		s_bitsPerSample = bitsPerSample;

		return true;
	}

	void free()
	{
	}

	void* getRawData()
	{
		return s_rawData;
	}

	u32 getRawSize()
	{
		return s_rawSize;
	}

	u32 getSampleRate()
	{
		return s_sampleRate;
	}
		
	bool isStereo()
	{
		return s_isStereo;
	}

	u32 getBitsPerSample()
	{
		return s_bitsPerSample;
	}
};
