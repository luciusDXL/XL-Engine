#define WILDMIDI_STATIC 1
#define AUDIODRV_OPENAL 1

#include "midi.h"
#include "sound.h"
#include "../Threads/mutex.h"
#include "wildmidi_lib.h"
#include <stdlib.h>

namespace Midi
{
	#define LOCK()	 s_mutex->lock()
	#define UNLOCK() s_mutex->unlock()

	static midi* s_song = NULL;
	static bool  s_initialized = false;
	static Mutex* s_mutex = NULL;

	const  s32   s_volumeScale = 128;

	void midiStreamCallback(void* userData, u32 requestedChunkSize, u8* chunkData);

	bool init(MidiFormat format, const char* patchLoc)
	{
		free();

		if (WildMidi_Init(patchLoc, 32072, WM_MO_ENHANCED_RESAMPLING) >= 0)
		{
			s_initialized = true;
			WildMidi_MasterVolume( (100 * s_volumeScale) >> 8 );
			s_mutex = Mutex::create();

			return true;
		}

		return false;
	}

	void free()
	{
		if (s_initialized)
		{
			stop();
			WildMidi_Shutdown();
			s_initialized = false;

			delete s_mutex;
		}
	}
	
	void setVolume(u32 volume)
	{
		LOCK();
			WildMidi_MasterVolume( (volume * s_volumeScale) >> 8 );
		UNLOCK();
	}

	void playResume()
	{
		Sound::startMusic( midiStreamCallback, NULL, 1.0f, true, 16, 32072 );
	}

	void pause()
	{
		Sound::pauseMusic();
	}

	void stop()
	{
		Sound::stopMusic();
		LOCK();
			if (s_song)
			{
				WildMidi_Close(s_song);
				s_song = NULL;
			}
		UNLOCK();
	}

	//This is the proper way to handle midi in many cases (depending on the game)
	//Using this function the game can provide the midi data, for example when using iMuse.
	void addMidiData(u8* data, u32 size)
	{
		//Stub...
		//Obviously I need to fill this in, but I am going to wait until I have a test case.
	}

	bool loadMidiFile(const char* file)
	{
		LOCK();
			s_song = WildMidi_Open(file);
		UNLOCK();
		return (s_song!=NULL);
	}

	void midiStreamCallback(void* userData, u32 requestedChunkSize, u8* chunkData)
	{
		LOCK();
			s32 soundSize = 0;
			s32 totalSize = 0;
			while (totalSize < (s32)requestedChunkSize)
			{
				soundSize = WildMidi_GetOutput(s_song, (char*)&chunkData[totalSize], requestedChunkSize-totalSize);

				//we've finished the loop, start the song all over again.
				if (soundSize == 0)
				{
					unsigned long beginning = 0;
					WildMidi_FastSeek(s_song, &beginning);
					soundSize = WildMidi_GetOutput(s_song, (char*)&chunkData[totalSize], requestedChunkSize-totalSize);
				}

				totalSize += soundSize;
			};
		UNLOCK();
	}
};
