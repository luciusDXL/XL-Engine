#pragma once
#include "types.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Memory Pool used by the Games
// Freeing a game consists of resetting the whole memory pool.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

//wacky template magic, used to safely determine array size or generate a compile error if used incorrectly.
template <typename T,unsigned S>
inline u32 arraysize(const T (&v)[S]) { return S; }

namespace MemoryPool
{
	bool init();
	void destroy();

	//Reset the memory pool
	void reset();

	void* xlMalloc(size_t size);
	void* xlCalloc(size_t size, size_t num);
	void* xlRealloc(void* ptr, size_t size);
	void  xlFree(void* ptr);

	u32 getMemUsed();

	void test();
};