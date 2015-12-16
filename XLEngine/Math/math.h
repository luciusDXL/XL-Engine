#pragma once
#include "../types.h"

namespace Math
{
	inline bool isPow2(u32 x)
	{
	  return ((x != 0) && !(x & (x - 1))) != 0;
	}

	inline u32 nextPow2(u32 x)
	{
		x = x-1;
		x = x | (x>>1);
		x = x | (x>>2);
		x = x | (x>>4);
		x = x | (x>>8);
		x = x | (x>>16);
		return x + 1;
	}
}