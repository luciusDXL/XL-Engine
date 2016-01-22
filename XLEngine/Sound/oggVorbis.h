#pragma once
#include "../types.h"

namespace oggVorbis
{
	bool init();
	void free();
	
	void setVolume(u32 volume);
	void playResume();
	void pause();
	void stop();

	bool loadOGG(const char* file);
};
