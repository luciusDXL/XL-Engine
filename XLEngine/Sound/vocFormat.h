///////////////////////////
// Incomplete VOC format
// (WIP)
///////////////////////////
#pragma once
#include "../types.h"

namespace Voc
{
	bool read(u8* voc, u32 size);
	void free();

	void* getRawData();
	u32   getRawSize();
	u32   getSampleRate();
};
