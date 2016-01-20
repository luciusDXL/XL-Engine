///////////////////////////
// Incomplete VOC format
// (WIP)
///////////////////////////
#pragma once
#include "../types.h"

namespace Wav
{
	bool read(u8* wav, u32 size);
	void free();

	void* getRawData();
	u32   getRawSize();
	u32   getSampleRate();

	bool  isStereo();
	u32	  getBitsPerSample();
};
