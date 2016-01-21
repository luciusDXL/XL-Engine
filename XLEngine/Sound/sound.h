////////////////////////////////////////////////////
// XL Engine Sound System
// The sound system manages resources such as 
// buffers and source (i.e. voices).
// Fire and forget sounds, looping sounds and 
// sounds that are tweaked over time are supported.
////////////////////////////////////////////////////
// The Sound System does not manage file I/O, sound
// data is passed into the system directly.
////////////////////////////////////////////////////

#pragma once
#include "../types.h"
#include "../xlServices.h"

typedef u32 SoundHandle;
#define INVALID_SOUND_HANDLE 0xffffffff

typedef void (*MusicCallback)(void* userData, u32 requestedChunkSize, u8* chunkData);

namespace Sound
{
	//system setup
	bool init();
	void free();
	void update();
	//stops all sources, marks all buffers as reusable.
	void reset();
	//optional callback - called whenever sounds "end"
	void setCallback(XLSoundCallback callback);

	//////////////////////////////////////////////////////////////////
	//Sets the global sound volume (affects all sounds).
	//////////////////////////////////////////////////////////////////
	// Inputs:
	// volume - the volume level from 0 = no sound to 1 = max volume.
	//////////////////////////////////////////////////////////////////
	void setGlobalVolume(f32 volume);

	//////////////////////////////////////////////////////////////////
	//Starts streaming music playback.
	//////////////////////////////////////////////////////////////////
	// Inputs:
	// streamCallback - this callback is called whenever data needs to
	//		be loaded into the streaming sound buffers.
	//////////////////////////////////////////////////////////////////
	bool startMusic(MusicCallback streamCallback, void* userData, Bool stereo, u32 bitsPerSample, u32 samplingRate);
	// Stops music playback and pauses music update.
	void stopMusic();
	// Pause the music, it can be resumed from the same spot.
	void pauseMusic();
	// Resume paused music.
	void resumeMusic();
	
	//////////////////////////////////////////////////////////////////
	//Generic 2D sound play function
	//////////////////////////////////////////////////////////////////
	// Inputs:
	// name - unique name of the sound, used to share sound data.
	// data - the sound data to play.
	// size - the size of the sound data.
	// type - the type of sound data provided (see SoundType)
	// info - information required to play the sound, such as sampling
	//			rate (see SoundInfo).
	// looping - if set to true, the sound will loop until stopped.
	//-----------------------------------------------------------------
	// Output:
	// SoundHandle - used by the client to check on, adjust, pause or
	//					stop the sound.
	//////////////////////////////////////////////////////////////////
	SoundHandle playSound2D(const char* name, const void* data, u32 size, u32 type, SoundInfo* info, Bool looping);

	//////////////////////////////////////////////////////////////////
	//Play a "fire and forget" 2D sound. 
	//////////////////////////////////////////////////////////////////
	// Output:
	// bool - returns true if the sound was successfully played.
	//////////////////////////////////////////////////////////////////
	Bool playOneShot2D(const char* name, const void* data, u32 size, u32 type, SoundInfo* info);

	//////////////////////////////////////////////////////////////////
	//Play a looping 2D sound. 
	//////////////////////////////////////////////////////////////////
	SoundHandle playSoundLooping(const char* name, const void* data, u32 size, u32 type, SoundInfo* info);

	//////////////////////////////////////////////////////////////////
	//Check sound state.
	//////////////////////////////////////////////////////////////////
	Bool isActive(SoundHandle handle);
	Bool isPlaying(SoundHandle handle);
	Bool isLooping(SoundHandle handle);
	//get the number of sounds currently playing.
	s32  soundsPlaying();

	//////////////////////////////////////////////////////////////////
	//Change sound state.
	//////////////////////////////////////////////////////////////////
	//stops the sound, after calling this function the handle will no
	//longer be valid.
	void stopSound(SoundHandle handle);
	void stopAllSounds();

	//pauses a sound. The handle is still valid and the source still
	//used. Do not leave sounds paused for too long.
	void pauseSound(SoundHandle handle);
	void resumeSound(SoundHandle handle);

	//for 2D sounds, this allows the stereo panning to be adjusted.
	//pan: -1 = left, 0 = middle, 1 = right
	void setPan(SoundHandle handle, f32 pan);

	//change the sound volume (0 = not audible, 1 = full volume)
	void setVolume(SoundHandle handle, f32 volume);
};
