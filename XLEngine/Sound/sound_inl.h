////////////////////////////////////
// Internal inline functions used
// by the sound system.
// This should ONLY be included by
// "sound.cpp"
////////////////////////////////////
#pragma once
#include "../types.h"

namespace Sound
{
	extern u32* s_activeSounds;

	inline u32 getHandleBuffer(u32 handle)  { return handle&0xff; }
	inline u32 getHandleSource(u32 handle)  { return (handle>>8)&0x1f; }
	inline u32 getHandleAllocID(u32 handle) { return (handle>>13)&0x7ffff; }

	inline u32 getActiveBuffer(u32 sourceID) { return s_activeSounds[sourceID]&0xff; }
	inline u32 getActiveAllocID(u32 sourceID) { return (s_activeSounds[sourceID]>>8)&0x7ffff; }
	inline bool checkActiveFlag(u32 sourceID, u32 flag) { return (s_activeSounds[sourceID]&flag)!=0; }

	inline void setActiveBuffer(u32 sourceID, u32 bufferID) { s_activeSounds[sourceID] &= ~0xff; s_activeSounds[sourceID] |= bufferID; }
	inline void setActiveAllocID(u32 sourceID, u32 allocID) { s_activeSounds[sourceID] &= ~(0x7ffff<<8); s_activeSounds[sourceID] |= (allocID<<8); }
	inline void setActiveFlag(u32 sourceID, u32 flag) { s_activeSounds[sourceID] |= flag; }
	inline void clearActiveFlag(u32 sourceID, u32 flag) { s_activeSounds[sourceID] &= ~flag; }
};
