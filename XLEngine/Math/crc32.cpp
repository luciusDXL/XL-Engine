#include "crc32.h"
#include <assert.h>

namespace CRC32
{
	static bool s_initCRC32 = false;
	static u32  s_crcTable[256];

	u32 reflectCRC(s32 val, s32 bits)
	{
		s32 result = 0;
		for (s32 bit = 0; bit < bits; bit++)
		{
			if(val & 1) result |= 1 << (bits - 1 - bit);
			val >>= 1;
		}

		return result;
	}

	void initcrc32()
	{
		for (s32 byte = 0; byte < 256; ++ byte)
		{
			u32 crc = reflectCRC(byte, 8) << 24;
			for (s32 offset = 0; offset < 8; ++ offset)
			{
				if (crc & 0x80000000)
				{
					crc = (crc << 1) ^ 0x04c11db7;
				}
				else
				{
					crc <<= 1;
				}
			}
			s_crcTable[byte] = reflectCRC(crc, 32);
		}
		s_initCRC32 = true;
	}

	u32 get(const u8* buffer, size_t size)
	{
		u32 crc = 0xFFFFFFFF;
		if (!s_initCRC32) initcrc32();

		const u8* data = buffer;
		const u8* dataEnd = &data[size];
		while (data < dataEnd)
		{
			crc = (crc >> 8) ^ s_crcTable[(crc & 0xFF) ^ *data++];
		}

		return ~crc;
	}
}
