#include "memoryPool.h"
#include "log.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <vector>

using namespace std;

namespace MemoryPool
{
	///////////////////////////////////////////////
	// Structures
	///////////////////////////////////////////////
	//TO-DO: I could remove the next pointer and calculate it when needed, saving 4-8 bytes in the header.
	//However the prev pointer would add a fair amount of overhead to calculate [jump to the beginning of the list and
	//iterate until it is found], but could be changed to an offset fixing the size to 4 bytes even in 64bit.
	//This would fix the size at 8 bytes (verus 12 in 32bit and 20 in 64bit).
	struct Header
	{
		u32      size;
		Header*  next;
		Header*  prev;
	};

	struct MemPool
	{
		size_t      allocCount;
		Header*     head;
		u8*         top;
	};

	///////////////////////////////////////////////
	// Constants and Defines
	///////////////////////////////////////////////
	const u32 c_topBit    = 1<<31;
	const u32 c_clearMask = ~c_topBit;
	const u32 c_sizeMask  = c_topBit - 1;
	const size_t c_memChunkSize = 32 * 1024 * 1024;	//32MB
	const size_t c_headerSize = sizeof(Header);

	#define IS_USED(s)    ( (s)->size &  c_topBit )
	#define GET_SIZE(s)   ( (s)->size &  c_sizeMask )
	#define SET_USED(s)   ( (s)->size |= c_topBit )
	#define CLEAR_USED(s) ( (s)->size &= c_clearMask )
	#define SET_SIZE_USED(s, n) (s)->size = ( (n) | c_topBit )
	#define GET_POINTER(p) (void*)(  (u8*)p + c_headerSize);
	#define GET_HEADER(p)  (Header*)((u8*)p - c_headerSize);

	///////////////////////////////////////////////
	// Internal Variables
	///////////////////////////////////////////////
	static vector<MemPool*> s_memPools;
	static s32 s_memUsed;
	static s32 s_lastReported;

	///////////////////////////////////////////////
	// Forward Declarations
	///////////////////////////////////////////////
	Header* getAllocation(MemPool* pool, u32 size);
	MemPool* allocatePool();
	void deallocatePools();


	///////////////////////////////////////////////
	// API Implementation
	///////////////////////////////////////////////
	bool init()
	{
		//allocate a single chunk to start out with.
		s_memPools.reserve( 16 );
		s_memUsed = 0;
		s_lastReported = 0;
		MemPool* pool = allocatePool();
		return (pool != NULL);
	}

	void destroy()
	{
		deallocatePools();
	}

	//Reset the memory pool
	void reset()
	{
		LOG( LOG_MESSAGE, "Memory Pool Reset [previous allocations = %d bytes]", s_memUsed );
		s_memUsed = 0;
		s_lastReported = 0;

		const size_t poolCount = s_memPools.size();
		MemPool** poolList = s_memPools.data();
		for (size_t p=0; p<poolCount; p++)
		{
			MemPool* pool = poolList[p];

			memset(pool->top, 0, c_memChunkSize);
			pool->allocCount = 1;
			pool->head = NULL;

			getAllocation(pool, c_memChunkSize - c_headerSize);
		}
	}

	u32 getMemUsed()
	{
		assert(s_memUsed >= 0);
		return u32( s_memUsed );
	}

	void* xlMalloc(size_t size)
	{
		if (size == 0) { return NULL; }

		//find the first pool with a large enough slot...
		void* out = NULL;

		const size_t poolCount = s_memPools.size();
		MemPool** poolList = s_memPools.data();
		for (size_t p=0; p<poolCount; p++)
		{
			Header* alloc = getAllocation( poolList[p], size );
			if (alloc)
			{
				out = GET_POINTER(alloc);
				break;
			}
		}

		//allocate another pool...
		if (!out)
		{
			MemPool* newPool = allocatePool();
			Header* alloc = getAllocation( newPool, size );
			out = GET_POINTER(alloc);
		}

		return out;
	}

	void* xlCalloc(size_t size, size_t num)
	{
		if (size == 0 || num == 0) { return NULL; }

		size = size * num;

		//find the first pool with a large enough slot...
		void* out = NULL;

		const size_t poolCount = s_memPools.size();
		MemPool** poolList = s_memPools.data();
		for (size_t p=0; p<poolCount; p++)
		{
			Header* alloc = getAllocation( poolList[p], size );
			if (alloc)
			{
				out = GET_POINTER(alloc);
				break;
			}
		}

		//allocate another pool...
		if (!out)
		{
			MemPool* newPool = allocatePool();
			Header* alloc = getAllocation( newPool, size );

			if (alloc)
			{
				out = GET_POINTER(alloc);
			}
		}

		if (out)
		{
			memset(out, 0, size);
		}

		return out;
	}

	void* xlRealloc(void* ptr, size_t size)
	{
		if (ptr == NULL || size == 0) { return NULL; }

		Header* header = GET_HEADER(ptr);
		if (GET_SIZE(header) >= size)
		{
			return ptr;
		}
		else if (header->next && !IS_USED(header->next) && GET_SIZE(header)+GET_SIZE(header->next)+c_headerSize >= size)
		{
			s_memUsed += GET_SIZE(header->next);

			SET_SIZE_USED( header, GET_SIZE(header) + GET_SIZE(header->next) + c_headerSize );
			header->next = header->next->next;
			header->next->prev = header;

			return ptr;
		}

		void* newPtr = xlMalloc(size);
		memcpy(newPtr, ptr, GET_SIZE(header));

		xlFree(ptr);
		return newPtr;
	}

	void xlFree(void* ptr)
	{
		if (ptr == NULL)
			return;

		Header* header = GET_HEADER(ptr);
		s_memUsed -= GET_SIZE(header);
		assert( s_memUsed >= 0 );

		//merge blocks
		if (header->next && !IS_USED(header->next))
		{
			s_memUsed -= c_headerSize;
			assert( s_memUsed >= 0 );

			header->size = GET_SIZE(header) + GET_SIZE(header->next) + c_headerSize;
			header->next = header->next->next;
			if (header->next)
			{
				header->next->prev = header;
			}
		}
		if (header->prev && !IS_USED(header->prev))
		{
			s_memUsed -= c_headerSize;
			assert( s_memUsed >= 0 );

			const u32 newSize = GET_SIZE(header) + GET_SIZE(header->prev) + c_headerSize;

			Header* n = header->next;
			header    = header->prev;
			header->next = n;
			header->size = newSize;
			if (n)
			{
				n->prev = header;
			}
		}

		//make it available for allocations.
		CLEAR_USED(header);
	}

	void test()
	{
		void* ptr0 = xlMalloc( 295 );
		void* ptr1 = xlMalloc( 3095 );
		xlFree(ptr1);
		ptr1 = xlMalloc(3056);
		ptr0 = xlRealloc(ptr0, 406);
	}

	/////////////////////////////////////
	// Internal
	/////////////////////////////////////
	Header* getAllocation(MemPool* pool, u32 size)
	{
		Header* alloc = NULL;
		if (pool->head == NULL)
		{
			alloc = (Header*)pool->top;
			alloc->size = size;
			alloc->next = NULL;
			alloc->prev = NULL;
			pool->head  = alloc;

			s_memUsed += c_headerSize;
		}
		else
		{
			alloc = pool->head;
			while (alloc)
			{
				const u32 allocSize = GET_SIZE(alloc);
				if (allocSize >= size && !IS_USED(alloc))
				{
					//decide whether to split or not...
					if (allocSize >= size+c_headerSize)
					{
						Header* n = alloc->next;
						Header* newAlloc = (Header*)( (u8*)alloc + c_headerSize + size );
						s_memUsed += c_headerSize;

						newAlloc->size = allocSize - size - c_headerSize;
						newAlloc->next = n;
						newAlloc->prev = alloc;
						if (n)
						{
							n->prev = newAlloc;
						}

						alloc->next = newAlloc;
						alloc->size = size;
					}

					SET_USED(alloc);
					s_memUsed += GET_SIZE(alloc);

					//Log memory usage every 1MB
					if ( s_memUsed >= s_lastReported+(1<<20) )
					{
						LOG( LOG_MESSAGE, "Memory watermark - used = %d bytes", s_memUsed );
						s_lastReported = s_memUsed;
					}

					return alloc;
				}

				alloc = alloc->next;
			};
		}

		return alloc;
	}

	MemPool* allocatePool()
	{
		MemPool* pool = new MemPool;
		pool->allocCount = 1;
		pool->head = NULL;
		pool->top  = (u8*)malloc( c_memChunkSize );

		getAllocation(pool, c_memChunkSize - c_headerSize);
		s_memPools.push_back( pool );

		LOG( LOG_MESSAGE, "Memory Pool Allocated, count = %u.", (u32)s_memPools.size() );
		return pool;
	}

	void deallocatePools()
	{
		const size_t poolCount = s_memPools.size();
		MemPool** poolList = s_memPools.data();
		for (size_t p=0; p<poolCount; p++)
		{
			free(poolList[p]->top);
			delete poolList[p];
		}
		s_memPools.clear();
	}
};
