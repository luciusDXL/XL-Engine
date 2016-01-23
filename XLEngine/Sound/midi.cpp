#define WILDMIDI_STATIC 1
#define AUDIODRV_OPENAL 1

#include "midi.h"
#include "sound.h"
#include "../log.h"
#include "../Threads/mutex.h"
#include "wildmidi_lib.h"
#include "fluidsynthDLL.h"

#include <stdlib.h>

namespace Midi
{
	#define LOCK()	 s_mutex->lock()
	#define UNLOCK() s_mutex->unlock()
	const  f32 c_volumeScale[] = { 0.5f, 1.0f };

	static MidiFormat s_midiFormat;

	static void*			  s_song			= NULL;
	static fluid_settings_t*  s_fluidSettings	= NULL;
	static fluid_synth_t*     s_fluidSynth		= NULL;
	static fluid_sequencer_t* s_fluidSeq		= NULL;
	static fluid_player_t*    s_fluidPlayer		= NULL;
	static bool				  s_fluidPlaying    = false;

	static bool  s_initialized = false;
	static Mutex* s_mutex = NULL;
		
	static f64 s_sampleRate = 0.0;
	static f32 s_volume = c_volumeScale[0];

	void midiStreamCallback(void* userData, u32 requestedChunkSize, u8* chunkData);
		
	bool init(MidiFormat format, const char* patchLoc)
	{
		free();
		s_mutex = Mutex::create();
		s_midiFormat = format;

		if (s_midiFormat == MFMT_GUS_PATCH)
		{
			s_volume = c_volumeScale[0];
			s_sampleRate = 32072.0;
			if (WildMidi_Init(patchLoc, 32072, WM_MO_ENHANCED_RESAMPLING) >= 0)
			{
				s_initialized = true;
				WildMidi_MasterVolume(100);
				
				return true;
			}
		}
		else if (s_midiFormat == MFMT_SOUND_FONT)
		{
			s_volume = c_volumeScale[1];
			if (!loadFluidsythDLL())
			{
				LOG( LOG_ERROR, "cannot find or load the \"libfluidsynth\" dynamic library." );
				return false;
			}
			
			s_fluidSettings = new_fluid_settings();
			fluid_settings_setstr(s_fluidSettings, "player.timing-source", "sample");
			fluid_settings_setstr(s_fluidSettings, "synth.lock-memory", 0);
			fluid_settings_setstr(s_fluidSettings, "synth.chorus-active", "0");

			s_fluidSynth = new_fluid_synth(s_fluidSettings);
			if (fluid_synth_sfload(s_fluidSynth, patchLoc, 1) < 0)
			{
				LOG( LOG_ERROR, "cannot load sound font \"%s\"", patchLoc );
				unloadFluidsynthDLL();
				return false;
			}
			
			s_fluidSeq = new_fluid_sequencer2(false);
			fluid_sequencer_register_fluidsynth(s_fluidSeq, s_fluidSynth);
			fluid_settings_getnum(s_fluidSettings, "synth.sample-rate", &s_sampleRate);

			s_fluidPlayer = new_fluid_player(s_fluidSynth);
			return true;
		}

		return false;
	}
		
	void free()
	{
		if (s_initialized)
		{
			stop();
			if (s_midiFormat == MFMT_GUS_PATCH)
			{
				WildMidi_Shutdown();
			}
			else if (s_midiFormat == MFMT_SOUND_FONT)
			{
				delete_fluid_settings(s_fluidSettings);
				delete_fluid_synth(s_fluidSynth);
				delete_fluid_sequencer(s_fluidSeq);
				delete_fluid_player(s_fluidPlayer);

				unloadFluidsynthDLL();
			}

			s_initialized = false;

			delete s_mutex;
		}
	}
	
	void setVolume(u32 volume)
	{
		s_volume = f32(volume)*0.01f * c_volumeScale[ s_midiFormat ];
	}

	void playResume()
	{
		Sound::startMusic( midiStreamCallback, NULL, s_volume, true, 16, u32(s_sampleRate) );
	}

	void pause()
	{
		Sound::pauseMusic();
	}

	void stop()
	{
		Sound::stopMusic();
		LOCK();
			if (s_song && s_midiFormat == MFMT_GUS_PATCH)
			{
				WildMidi_Close(s_song);
			}
			else if (s_song && s_midiFormat == MFMT_SOUND_FONT)
			{
				fluid_player_stop(s_fluidPlayer);
				s_fluidPlaying = false;
			}
			s_song = NULL;
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
			if (s_midiFormat == MFMT_GUS_PATCH)
			{
				s_song = WildMidi_Open(file);
			}
			else if (s_midiFormat == MFMT_SOUND_FONT)
			{
				s_song = (void*)(fluid_player_add(s_fluidPlayer, file) >= 0 ? 1 : 0);
				s_fluidPlaying = false;
			}
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
				if (s_midiFormat == MFMT_GUS_PATCH)
				{
					soundSize = WildMidi_GetOutput(s_song, (char*)&chunkData[totalSize], requestedChunkSize-totalSize);
				}
				else if (s_midiFormat == MFMT_SOUND_FONT)
				{
					if (!s_fluidPlaying)
					{
						fluid_player_play( s_fluidPlayer );
						fluid_player_set_loop(s_fluidPlayer, -1);
						s_fluidPlaying = true;
					}
					soundSize = fillFluidBuffer(requestedChunkSize, chunkData, s_sampleRate, s_fluidSynth, s_fluidPlayer);
				}

				//we've finished the loop, start the song all over again.
				if (soundSize == 0)
				{
					if (s_midiFormat == MFMT_GUS_PATCH)
					{
						unsigned long beginning = 0;
						WildMidi_FastSeek(s_song, &beginning);
						soundSize = WildMidi_GetOutput(s_song, (char*)&chunkData[totalSize], requestedChunkSize-totalSize);
					}
					else if (s_midiFormat == MFMT_SOUND_FONT)
					{
						LOG( LOG_ERROR, "midi synthesizer should handle looping." );
					}
				}

				totalSize += soundSize;
			};
		UNLOCK();
	}
};
