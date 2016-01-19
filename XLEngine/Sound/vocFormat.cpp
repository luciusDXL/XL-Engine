///////////////////////////
// Incomplete VOC format
// (WIP)
///////////////////////////

#include "vocFormat.h"
#include "../log.h"
#include <stdlib.h>

namespace Voc
{
#pragma pack(push, 1)
	struct VocHeader
	{ 
		u8  Description[20];
		u16 DataBlockOffset;
		u16 Version;
		u16 IDCode;
	};

	struct VocBlock1_Header
	{
		u8 tc;
		u8 pack;
	};

	struct VocBlock9_Header
	{
		u32  samplesPerSecond;
		u8   bitsPerSample;
		u8   channel;
		u16  format;
		u8   reserved[4]; 
	};

	enum VocType
	{
		VT_8BIT_UNSIGNED=0,
		VT_4BIT_ADPCM,
		VT_2_6BIT_ADPCM,
		VT_2_BIT_ADPCM,
		VT_COUNT
	};
#pragma pack(pop)

	void* s_rawData = NULL;
	u32   s_rawSize = 0;
	u32   s_sampleRate = 0;

	bool readVocBlock1(u8* soundData, u32& index, u8*& outData, u32& outLength, u32& sampleRate)
	{
		VocType type = VT_COUNT;
		u32 a = soundData[index]; index++;
		u32 b = soundData[index]; index++;
		u32 c = soundData[index]; index++;
		u32 len = (c<<16) | (b<<8) | a;

		VocBlock1_Header* header = (VocBlock1_Header*)&soundData[index]; index += sizeof(VocBlock1_Header);

		sampleRate = 1000000/256 - (u32)header->tc;
		len -= 2;

		u8* temp = &soundData[index]; index += len;

		switch (header->pack)
		{
			case 0:
				type = VT_8BIT_UNSIGNED;
				break;
			case 1:
				type = VT_4BIT_ADPCM;
				break;
			case 2:
				type = VT_2_6BIT_ADPCM;
				break;
			case 3:
				type = VT_2_BIT_ADPCM;
				break;
		}

		outLength = len;
		outData   = temp;

		return true;
	}

	bool readVocBlock9(u8* soundData, u32& index, u8*& outData, u32& outLength, u32& sampleRate)
	{
		u32 a = soundData[index]; index++;
		u32 b = soundData[index]; index++;
		u32 c = soundData[index]; index++;
		u32 len = (c<<16) | (b<<8) | a;

		VocBlock9_Header* header = (VocBlock9_Header*)&soundData[index]; index += sizeof(VocBlock9_Header);
		sampleRate = header->samplesPerSecond;

		len -= 12;
		switch(header->format)
		{
			case 0x0000:
				//cout<<" PCM ";
				break;
			case 0x0001:
			case 0x0002: 
			case 0x0200:
			case 0x0003:
				//cout<<" ADPCM ";
				break;
			case 0x0004:
				//cout<<" Signed ";
				break;
			case 0x0006:
				//cout<<" ALAW ";
				break;
			case 0x0007:
				//cout<<" MULAW ";
				break;
			default:
				//cout<<"Unsupported Format!\n";
				break;
		}

		outData   = &soundData[index];
		outLength = len;

		return true;
	}
		
	bool read(u8* voc, u32 size)
	{
		u32 index = 0;
		VocHeader* header = (VocHeader*)&voc[index]; index += sizeof(VocHeader);
		index = header->DataBlockOffset;

		u8* rawData = NULL;
		u32 rawLen  = 0;
		u32 sampleRate = 0;

		char eof = 0;
		while (eof == 0)
		{
			char blocktype = voc[index]; index++;
			switch (blocktype)
			{
				case 0:
					eof = 1;
					break;
				case 1:
					readVocBlock1(voc, index, rawData, rawLen, sampleRate);
					break;
				case 9:
					readVocBlock9(voc, index, rawData, rawLen, sampleRate);
					break;
				default:
					eof = 1;
			};
		};

		s_rawData = rawData;
		s_rawSize = rawLen;
		s_sampleRate = sampleRate;

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
};
