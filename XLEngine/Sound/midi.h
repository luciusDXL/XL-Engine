#pragma once
#include "../types.h"

enum MidiFormat
{
	MFMT_GUS_PATCH = 0,
	MFMT_SOUND_FONT,
	MFMT_COUNT
};

namespace Midi
{
	bool init(MidiFormat format, const char* patchLoc);
	void free();
	
	void setVolume(u32 volume);
	void playResume();
	void pause();
	void stop();

	void addMidiData(u8* data, u32 size);
	bool loadMidiFile(const char* file);
};
