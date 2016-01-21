#include "sound.h"
#include "../osUtil.h"
#include "../log.h"
#include "../Threads/mutex.h"
#include "../Threads/thread.h"
#include <al.h>
#include <alc.h>
#include <assert.h>
#include <string>
#include <map>

//internal inlines
#include "sound_inl.h"
#include "vocFormat.h"
#include "wavFormat.h"

namespace Sound
{
	//////////////////////////////////////////////////////////////////
	//Structures, enums and defines.
	//////////////////////////////////////////////////////////////////
	enum ActiveSoundFlags
	{
		SOUND_ACTIVE  = 0x08000000,
		SOUND_PLAYING = 0x10000000,
		SOUND_LOOPING = 0x20000000,
		SOUND_PAUSED  = 0x40000000,
		SOUND_UNUSED  = 0x80000000,	//reserved for future expansion.
	};

	enum BufferFlags
	{
		BUFFER_ACTIVE = 1,
	};

	struct SoundBuffer
	{
		//hold the data name associated with the buffered data.
		std::string name;

		u32 flags;			//buffer flags.
		u32 index;			//buffer index
		s32 refCount;		//number of sounds current referencing this buffer.
		u64 lastUsed;		//last "frame" referenced.
		ALuint oalBuffer;	//OpenAL buffer.
	};

	typedef std::map<std::string, u32> BufferMap;

	#define VALIDATE() \
	{  \
		ALenum error = alGetError();\
		if (error != AL_NO_ERROR)	\
		{ \
			LOG( LOG_ERROR, "alError = 0x%x", error ); \
			assert(0); \
		} \
	}

	//////////////////////////////////////////////////////////////////
	//Variables
	//////////////////////////////////////////////////////////////////
	static s32 s_numBuffers		= 256;
	static s32 s_maxSimulSounds	= 32;
	static const s32 c_musicBufferCount = 4;

	static bool			s_init   = false;
	static ALCdevice*	s_device = NULL;
	static ALCcontext*	s_context = NULL;

	//sounds
	static SoundBuffer*	s_buffers;
	static ALuint*		s_sources;
	static f32*			s_sourceVolume;
	
	//music - for now only support a single track.
	static ALuint		s_musicBuffers[c_musicBufferCount];
	static ALuint		s_musicSource;

	static BufferMap    s_bufferMap;

	static u64 s_currentFrame;
	static f32 s_globalVolume;

	static Mutex* s_mutex;
	static XLSoundCallback s_callback = NULL;
	static u32* s_userValue;
	static f32 s_pos2D[3] = { 0.0f, 0.0f, 0.0f };

	u32* s_activeSounds;

	//music
	static bool s_musicPlaying = false;
	static s32  s_musicBuffersProcessed = 0;
	static Thread* s_musicThread = NULL;
	static MusicCallback s_musicCallback = NULL;
	static void* s_musicUserData = NULL;
	static ALenum s_musicFormat;
	static u32 s_musicSamplingRate;
	static bool s_musicPaused = false;
		
	//////////////////////////////////////////////////////////////////
	//Forward function declarations.
	//////////////////////////////////////////////////////////////////
	bool isActiveNoLock(SoundHandle handle);
	SoundBuffer* getSoundBuffer(const char* name);
	SoundHandle allocateSound(u32 bufferID);
	bool playSoundInternal(SoundHandle sound, f32 volume, f32 pan, Bool loop, Bool is3D);
	const void* getRawSoundData(const void* data, u32 size, u32 type, u32& rawSize);
	const f32* calculatePan(f32 pan);
	u32 XL_STDCALL streamMusic(void* userData);
	
	//////////////////////////////////////////////////////////////////
	//API implementation
	//////////////////////////////////////////////////////////////////
	bool init()
	{
		//setup the device.
		s_device = alcOpenDevice(NULL);
		if (!s_device)
		{
			LOG( LOG_ERROR, "Cannot open the audio device, no sound will be available." );
			return false;
		}

		//create the context
		s_context = alcCreateContext(s_device, 0);
		if (!s_context)
		{
			alcCloseDevice(s_device);
			LOG( LOG_ERROR, "Cannot create an audio context, no sound will be available." );
			return false;
		}
		alcMakeContextCurrent(s_context);

		//reset error handling.
		alGetError();

		//allocate buffer pool.
		s_buffers = new SoundBuffer[ s_numBuffers ];
		for (s32 i=0; i<s_numBuffers; i++)
		{
			s_buffers[i].flags    = 0;
			s_buffers[i].index	  = i;
			s_buffers[i].lastUsed = 0;
			s_buffers[i].refCount = 0;
			alGenBuffers(1, &s_buffers[i].oalBuffer);
		}
		if ( alGetError() != AL_NO_ERROR )
		{
			delete[] s_buffers;
			alcMakeContextCurrent(0);
			alcDestroyContext(s_context);
			alcCloseDevice(s_device);

			LOG( LOG_ERROR, "Cannot allocate space for audio buffers, no sound will be available." );
			return false;
		}

		//allocate sources
		s_sources = new ALuint[s_maxSimulSounds];
		s_sourceVolume = new f32[s_maxSimulSounds];
		alGenSources(s_maxSimulSounds, s_sources);
		if ( alGetError() != AL_NO_ERROR )
		{
			//free allocated memory.
			for (s32 i=0; i<s_numBuffers; i++)
			{
				alDeleteBuffers(1, &s_buffers[i].oalBuffer);
			}
			alcMakeContextCurrent(0);
			alcDestroyContext(s_context);
			alcCloseDevice(s_device);

			delete[] s_buffers;
			delete[] s_sources;

			LOG( LOG_ERROR, "Cannot allocate space for audio sources, no sound will be available." );
			return false;
		}

		s_activeSounds = new u32[ s_maxSimulSounds ];
		memset(s_activeSounds, 0, sizeof(u32)*s_maxSimulSounds);

		s_userValue = new u32[ s_maxSimulSounds ];
		memset(s_userValue, 0, sizeof(u32)*s_maxSimulSounds);

		//allocate music buffers
		alGenBuffers(c_musicBufferCount, s_musicBuffers);
		alGenSources(1, &s_musicSource);

		s_mutex = Mutex::create();

		s_musicThread = Thread::create("Music Thread", streamMusic, NULL);
		s_musicThread->run();

		s_init = true;
		s_currentFrame = 1;
		s_globalVolume = 0.50f;
		LOG( LOG_MESSAGE, "Sound System initialized." );
				
		return true;
	}

	void free()
	{
		if (!s_init) { return; }

		reset();
		stopMusic();

		delete s_musicThread;
		delete s_mutex;

		for (s32 i=0; i<s_numBuffers; i++)
		{
			alDeleteBuffers(1, &s_buffers[i].oalBuffer);
		}
		alDeleteSources(s_maxSimulSounds, s_sources);

		//music buffers
		alDeleteBuffers(c_musicBufferCount, s_musicBuffers);
		alDeleteSources(1, &s_musicSource);

		alcMakeContextCurrent(0);
		alcDestroyContext(s_context);
		alcCloseDevice(s_device);

		delete[] s_buffers;
		delete[] s_sources;
		delete[] s_activeSounds;
	}

	void reset()
	{
		if (!s_init) { return; }

		//first stop all sounds.
		s_mutex->lock();

		s_callback = NULL;
		for (s32 s=0; s<s_maxSimulSounds; s++)
		{
			alSourceStop( s_sources[s] );
			alSourcei( s_sources[s], AL_BUFFER, 0 );
			s_activeSounds[s] = 0;
		}

		for (s32 b=0; b<s_numBuffers; b++)
		{
			s_buffers[b].refCount = 0;
			s_buffers[b].flags    = 0;
			s_buffers[b].lastUsed = 0;
		}

		s_mutex->unlock(); 
	}
		
	void setCallback( XLSoundCallback callback )
	{
		s_callback = callback;
	}

	void update()
	{
		if (!s_init) { return; }
		s_mutex->lock();

		f32 totalVolume = 0.0f;
		s32 activeSources[32] = {0};
		s32 activeCount = 0;

		for (s32 s=0; s<s_maxSimulSounds; s++)
		{
			if ( checkActiveFlag(s, SOUND_PLAYING) )
			{
				s32 state;
				alGetSourcei( s_sources[s], AL_SOURCE_STATE, &state );
				if ( state != AL_PLAYING )
				{
					//was this playing up until now? - fire off the callback...
					if (s_callback && checkActiveFlag(s, SOUND_PLAYING))
					{
						if (s_userValue[s] != XL_SOUND_NO_CALLBACK) 
						{ 
							s_callback( s_userValue[s] ); 
						}
					}

					const u32 bufferID = getActiveBuffer(s);
					s_buffers[bufferID].refCount--;
					assert(s_buffers[bufferID].refCount >= 0);

					clearActiveFlag(s, SOUND_PLAYING);
				}
				else if ( state != AL_PAUSED )
				{
					totalVolume += s_sourceVolume[s];
					activeSources[ activeCount++ ] = s;
					clearActiveFlag(s, SOUND_PAUSED);
				}
			}
		}

		//reduce the overall volume.
#if 0
		f32 scale = totalVolume > 0.0f ? s_globalVolume/totalVolume : 1.0f;
		scale = sqrtf(scale);

		for (s32 s=0; s<activeCount; s++)
		{
			const u32 sourceID = activeSources[s];
			const f32 gain = std::min(s_sourceVolume[sourceID] * scale, 1.0f);
			alSourcef( s_sources[sourceID], AL_GAIN, gain );
		}
#endif
		
		//check the music track.
		if (s_musicPlaying)
		{
			s32 buffersProcessed = 0;
			alGetSourcei( s_musicSource, AL_BUFFERS_PROCESSED, &buffersProcessed );
			s_musicBuffersProcessed += buffersProcessed;
		}

		s_currentFrame++;
		s_mutex->unlock();
	}

	void setGlobalVolume(f32 volume)
	{
		if (!s_init) { return; }

		if (volume != s_globalVolume && s_init)
		{
			s_mutex->lock();

			//change the volume of all the currently playing sounds.
			for (s32 s=0; s<s_maxSimulSounds; s++)
			{
				const f32 gain = std::min(s_sourceVolume[s] * volume, 1.0f);
				alSourcef( s, AL_GAIN, gain );
			}
			
			s_globalVolume = volume;

			s_mutex->unlock();
		}
	}
	
	Bool isActive(SoundHandle handle)
	{
		if (!s_init || handle == INVALID_SOUND_HANDLE) { return false; }

		s_mutex->lock();
			const bool soundActive = isActiveNoLock(handle);
		s_mutex->unlock();

		return soundActive;
	}

	Bool isPlaying(SoundHandle handle)
	{
		if (!s_init || handle == INVALID_SOUND_HANDLE) { return false; }

		s_mutex->lock();
			const u32 sourceID = getHandleSource(handle);
			const bool soundIsPlaying = ( isActiveNoLock(handle) && checkActiveFlag(sourceID, SOUND_PLAYING) );
		s_mutex->unlock();

		return soundIsPlaying;
	}

	Bool isLooping(SoundHandle handle)
	{
		if (!s_init || handle == INVALID_SOUND_HANDLE) { return false; }
		
		s_mutex->lock();
			const u32 sourceID = getHandleSource(handle);
			const bool soundIsLooping = ( isActiveNoLock(handle) && checkActiveFlag(sourceID, SOUND_LOOPING) );
		s_mutex->unlock();
		
		return soundIsLooping;
	}
	
	SoundHandle playSound2D(const char* name, const void* data, u32 size, u32 type, SoundInfo* info, Bool looping)
	{
		if (!s_init || name==NULL || data==NULL || info==NULL) 
		{ 
			return INVALID_SOUND_HANDLE; 
		}

		s_mutex->lock();

		//allocate a free buffer if needed
		SoundBuffer* buffer = getSoundBuffer(name);
		if (!buffer)
		{
			LOG( LOG_WARNING, "Sound \"%s\" failed to get a sound buffer.", name );

			s_mutex->unlock();
			return INVALID_SOUND_HANDLE;
		}

		//allocate a sound
		SoundHandle sound = allocateSound(buffer->index);
		if (sound == INVALID_SOUND_HANDLE)
		{
			s_mutex->unlock();
			return INVALID_SOUND_HANDLE;
		}

		//load the sound buffer (if not already active).
		if ( !(buffer->flags&BUFFER_ACTIVE) )
		{
			ALenum bufferFmt = AL_FORMAT_MONO8;
			if (info->bitRate == 8)
			{
				bufferFmt = (info->stereo) ? AL_FORMAT_STEREO8 : AL_FORMAT_MONO8;
			}
			else if (info->bitRate == 16)
			{
				bufferFmt = (info->stereo) ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
			}

			u32 rawSize = 0;
			const void* rawData = getRawSoundData(data, size, type, rawSize);

			u32 samplingRate = info->samplingRate;
			if (type == STYPE_VOC)
			{
				samplingRate = Voc::getSampleRate() * 3;
			}
			else if (type == STYPE_WAV)
			{
				samplingRate = Wav::getSampleRate();
				u32 wavBitsPerSample = Wav::getBitsPerSample();
				Bool wavStereo = Wav::isStereo();

				if (wavBitsPerSample == 8)
				{
					bufferFmt = (wavStereo) ? AL_FORMAT_STEREO8 : AL_FORMAT_MONO8;
				}
				else if (wavBitsPerSample == 16)
				{
					bufferFmt = (wavStereo) ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
				}
			}

			alBufferData( buffer->oalBuffer, bufferFmt, rawData, rawSize, samplingRate );
			ALenum error = alGetError();
			if (error != AL_NO_ERROR)
			{
				LOG( LOG_ERROR, "Sound \"%s\" has invalid data.", name );

				s_mutex->unlock();
				return false;
			}

			buffer->flags |= BUFFER_ACTIVE;
		}

		//finally play the sound.
		bool playResult = playSoundInternal( sound, info->volume, info->pan, looping, false );
		if (!playResult)
		{
			LOG( LOG_ERROR, "Cannot play sound \"%s\"", name );

			clearActiveFlag( getHandleSource(sound), SOUND_ACTIVE );
			clearActiveFlag( getHandleSource(sound), SOUND_PLAYING );
			clearActiveFlag( getHandleSource(sound), SOUND_LOOPING );

			sound = INVALID_SOUND_HANDLE;
		}
		else
		{
			s_userValue[ getHandleSource(sound) ] = info->userValue;
		}

		s_mutex->unlock();

		return sound;
	}

	Bool playOneShot2D(const char* name, const void* data, u32 size, u32 type, SoundInfo* info)
	{
		SoundHandle sound = playSound2D(name, data, size, type, info, false);
		return (sound != INVALID_SOUND_HANDLE);
	}

	SoundHandle playSoundLooping(const char* name, const void* data, u32 size, u32 type, SoundInfo* info)
	{
		return playSound2D(name, data, size, type, info, true);
	}

	void stopSound(SoundHandle handle)
	{
		if (!s_init || handle == INVALID_SOUND_HANDLE) { return; }

		s_mutex->lock();
		
		if (!isActiveNoLock(handle)) { s_mutex->unlock(); return; }
		const u32 sourceID = getHandleSource(handle);

		//if the sound is not playing then it doesn't need to be stopped.
		if (!checkActiveFlag(sourceID, SOUND_PLAYING))
		{
			s_mutex->unlock(); 
			return;
		}

		//stop the sound
		alSourceStop( s_sources[sourceID] );
		alSourcei( s_sources[sourceID], AL_BUFFER, 0 );

		//clear the playing flag (and looping).
		clearActiveFlag(sourceID, SOUND_PLAYING);
		clearActiveFlag(sourceID, SOUND_LOOPING);
		clearActiveFlag(sourceID, SOUND_PAUSED);

		const u32 bufferID = getHandleBuffer(handle);
		s_buffers[bufferID].refCount--;
		assert(s_buffers[bufferID].refCount >= 0);

		s_mutex->unlock(); 
	}

	void stopAllSounds()
	{
		if (!s_init) { return; }

		s_mutex->lock();

		for (s32 s=0; s<s_maxSimulSounds; s++)
		{
			alSourceStop( s_sources[s] );
			alSourcei( s_sources[s], AL_BUFFER, 0 );

			setActiveBuffer(s, 0);
			clearActiveFlag(s, SOUND_PLAYING);
			clearActiveFlag(s, SOUND_LOOPING);
			clearActiveFlag(s, SOUND_PAUSED);
		}

		for (s32 b=0; b<s_numBuffers; b++)
		{
			s_buffers[b].refCount = 0;
		}

		s_mutex->unlock(); 
	}

	s32 soundsPlaying()
	{
		if (!s_init)
		{
			return 0;
		}

		s32 numSoundsPlaying = 0;

		s_mutex->lock();
		for (s32 s=0; s<s_maxSimulSounds; s++)
		{
			if (checkActiveFlag(s, SOUND_PLAYING))
			{
				numSoundsPlaying++;
			}
		}
		s_mutex->unlock(); 

		return numSoundsPlaying;
	}

	void pauseSound(SoundHandle handle)
	{
		if (!s_init || handle == INVALID_SOUND_HANDLE) { return; }

		s_mutex->lock();

		if (!isActiveNoLock(handle)) { s_mutex->unlock(); return; }
		const u32 sourceID = getHandleSource(handle);

		//if the sound is not playing then it doesn't need to be stopped.
		if (!checkActiveFlag(sourceID, SOUND_PLAYING))
		{
			s_mutex->unlock();
			return;
		}

		//stop the sound but leave the source intact.
		alSourcePause( s_sources[sourceID] );

		//clear the playing flag.
		clearActiveFlag(sourceID, SOUND_PLAYING);
		setActiveFlag(sourceID, SOUND_PAUSED);

		const u32 bufferID = getHandleBuffer(handle);
		s_buffers[bufferID].lastUsed = s_currentFrame;

		s_mutex->unlock();
	}

	void resumeSound(SoundHandle handle)
	{
		if (!s_init || handle == INVALID_SOUND_HANDLE) { return; }

		s_mutex->lock();

		if (!isActiveNoLock(handle)) { s_mutex->unlock(); return; }
		const u32 sourceID = getHandleSource(handle);

		//if the sound is not playing then it doesn't need to be stopped.
		if (!checkActiveFlag(sourceID, SOUND_PAUSED))
		{
			s_mutex->unlock();
			return;
		}

		//resume the sound
		alSourcePlay( s_sources[sourceID] );

		//set the playing flag.
		setActiveFlag(sourceID, SOUND_PLAYING);
		clearActiveFlag(sourceID, SOUND_PAUSED);

		const u32 bufferID = getHandleBuffer(handle);
		s_buffers[bufferID].lastUsed = s_currentFrame;

		s_mutex->unlock();
	}

	void setPan(SoundHandle handle, f32 pan)
	{
		if (!s_init || handle == INVALID_SOUND_HANDLE) { return; }

		s_mutex->lock();

		if (!isActiveNoLock(handle)) { s_mutex->unlock(); return; }
		const u32 sourceID = getHandleSource(handle);

		alSourcefv(s_sources[sourceID], AL_POSITION, calculatePan(pan));

		const u32 bufferID = getHandleBuffer(handle);
		s_buffers[bufferID].lastUsed = s_currentFrame;

		s_mutex->unlock();
	}

	void setVolume(SoundHandle handle, f32 volume)
	{
		if (!s_init || handle == INVALID_SOUND_HANDLE) { return; }

		s_mutex->lock();

		if (!isActiveNoLock(handle)) { s_mutex->unlock(); return; }
		//set the gain.
		const u32 sourceID = getHandleSource(handle);
		const f32 gain = std::min(volume * s_globalVolume, 1.0f);
		s_sourceVolume[ sourceID ] = volume;
		alSourcef( s_sources[sourceID], AL_GAIN, gain );

		const u32 bufferID = getHandleBuffer(handle);
		s_buffers[bufferID].lastUsed = s_currentFrame;

		s_mutex->unlock();
	}

	//////////////////////////////////////////////////////////////////
	//Starts streaming music playback.
	//////////////////////////////////////////////////////////////////
	// Inputs:
	// streamCallback - this callback is called whenever data needs to
	//		be loaded into the streaming sound buffers.
	//////////////////////////////////////////////////////////////////
	bool startMusic(MusicCallback streamCallback, void* userData, Bool stereo, u32 bitsPerSample, u32 samplingRate)
	{
		if (!s_init || !streamCallback)
		{
			LOG( LOG_ERROR, "A streaming music callback must be provided in order to play any music." );
			return false;
		}

		s_mutex->lock();
			s_musicCallback = streamCallback;
			s_musicUserData = userData;
			s_musicPlaying  = false;

			s_musicSamplingRate = samplingRate;
			if (bitsPerSample == 8)
			{
				s_musicFormat = (stereo) ? AL_FORMAT_STEREO8 : AL_FORMAT_MONO8;
			}
			else if (bitsPerSample == 16)
			{
				s_musicFormat = (stereo) ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
			}
		s_mutex->unlock();

		return true;
	}

	// Stops music playback and pauses music update.
	void stopMusic()
	{
		if (!s_init) { return; }

		s_mutex->lock();
			alSourceStop(s_musicSource);
			s_musicPlaying = false;
		s_mutex->unlock(); 
	}

	// Pause the music, it can be resumed from the same spot.
	void pauseMusic()
	{
		if (!s_init) { return; }

		s_mutex->lock();
			s_musicPaused = true;
			alSourcePause(s_musicSource);
		s_mutex->unlock(); 
	}

	// Resume paused music.
	void resumeMusic()
	{
		if (!s_init) { return; }

		s_mutex->lock();
			s_musicPaused = false;
			alSourcePlay(s_musicSource);
		s_mutex->unlock(); 
	}

	//////////////////////////////////////////////////////////////////
	//Internal implementation
	//////////////////////////////////////////////////////////////////
	bool isActiveNoLock(SoundHandle handle)
	{
		if (!s_init || handle == INVALID_SOUND_HANDLE)
		{
			return false;
		}

		const u32 sourceID = getHandleSource(handle);
		const u32 bufferID = getHandleBuffer(handle);
		const u32 allocID  = getHandleAllocID(handle);

		const bool isActive = checkActiveFlag(sourceID, SOUND_ACTIVE);
		const u32 activeAllocID = getActiveAllocID(sourceID);
		const u32 activeBufferID = getActiveBuffer(sourceID);

		const bool soundIsActive = (allocID == activeAllocID && bufferID == activeBufferID && isActive);
		return soundIsActive;
	}

	SoundBuffer* getSoundBuffer(const char* name)
	{
		//is a buffer already loaded with this data?
		BufferMap::iterator iBuffer = s_bufferMap.find(name);
		if (iBuffer != s_bufferMap.end())
		{
			return &s_buffers[ iBuffer->second ];
		}

		//if not then find a free buffer.
		for (s32 i=0; i<s_numBuffers; i++)
		{
			if ( !(s_buffers[i].flags&BUFFER_ACTIVE) )
			{
				s_buffers[i].name = name;
				s_bufferMap[name] = i;

				return &s_buffers[i];
			}
		}

		//if all buffers are active, try to find the oldest one that is no longer referenced.
		u64 oldestFrame = 0xffffffffffffffffULL;
		s32 oldestIndex = -1;
		for (s32 i=0; i<s_numBuffers; i++)
		{
			if ( s_buffers[i].refCount == 0 )
			{
				if (s_buffers[i].lastUsed < oldestFrame)
				{
					oldestFrame = s_buffers[i].lastUsed;
					oldestIndex = i;
				}
			}
		}

		if (oldestIndex >= 0)
		{
			//free the buffer.
			SoundBuffer* buffer = &s_buffers[oldestIndex];
			buffer->refCount = 0;
			buffer->flags = 0;

			//remove the old name from the map
			BufferMap::iterator iOldBuffer = s_bufferMap.find(buffer->name);
			if (iOldBuffer != s_bufferMap.end())
			{
				s_bufferMap.erase(iOldBuffer);
			}
			s_bufferMap[name] = oldestIndex;

			buffer->name = name;
			
			//return it.
			return buffer;
		}

		return NULL;
	}

	SoundHandle allocateSound(u32 bufferID)
	{
		//find an active sound that is no longer playing.
		s32 soundID = -1;
		for (s32 i=0; i<s_maxSimulSounds; i++)
		{
			if (!checkActiveFlag(i, SOUND_PLAYING) && !checkActiveFlag(i, SOUND_PAUSED))
			{
				soundID = s32(i);
				break;
			}
		}

		//for now do not overwrite sounds already playing.
		if (soundID < 0)
		{
			return INVALID_SOUND_HANDLE;
		}

		u32 allocID = getActiveAllocID(soundID);
		allocID = (allocID + 1) & 0x7ffff;

		setActiveBuffer(soundID, bufferID);
		setActiveAllocID(soundID, allocID);

		clearActiveFlag(soundID, SOUND_PLAYING);
		clearActiveFlag(soundID, SOUND_LOOPING);
		setActiveFlag(soundID, SOUND_ACTIVE);
		
		//create a sound handle.
		return SoundHandle( bufferID | (soundID<<8) | (allocID<<13) );
	}

	bool playSoundInternal(SoundHandle sound, f32 volume, f32 pan, Bool loop, Bool is3D)
	{
		const u32 sourceID = getHandleSource(sound);
		const u32 bufferID = getHandleBuffer(sound);
		const ALuint oalSource = s_sources[sourceID];

		alSourceStop(oalSource);
		alSourcei( oalSource, AL_BUFFER, s_buffers[bufferID].oalBuffer );
		alSourcef( oalSource, AL_ROLLOFF_FACTOR, 1.0f );

		//this is a "2D" source.
		if (!is3D)
		{
			alSourcei( oalSource, AL_SOURCE_RELATIVE, AL_TRUE );
			alSourcef( oalSource, AL_REFERENCE_DISTANCE, 15.0f );
			alSourcef( oalSource, AL_MAX_DISTANCE, 200.0f );
			
			alSourcefv( oalSource, AL_POSITION, calculatePan(pan) );
		}
		else
		{
			//adjust the hearing distance...
			f32 distScale = std::max(volume, 1.0f);
			alSourcef( oalSource, AL_REFERENCE_DISTANCE, 15.0f*distScale );
			alSourcef( oalSource, AL_MAX_DISTANCE, 200.0f*distScale );
		}

		//set looping.
		alSourcei( oalSource, AL_LOOPING, loop ? AL_TRUE : AL_FALSE );

		//set the gain.
		f32 gain = std::min(volume * s_globalVolume, 1.0f);
		s_sourceVolume[ sourceID ] = volume;
		alSourcef( oalSource, AL_GAIN, gain );

		//finally play the sound.
		alSourcePlay( oalSource );

		//mark the sound as playing.
		setActiveFlag(sourceID, SOUND_PLAYING);
		if (loop)
		{
			setActiveFlag(sourceID, SOUND_LOOPING);
		}

		s_buffers[bufferID].lastUsed = s_currentFrame;
		s_buffers[bufferID].refCount++;

		return true;
	}

	const void* getRawSoundData(const void* data, u32 size, u32 type, u32& rawSize)
	{
		if (type == STYPE_RAW)
		{
			rawSize = size;
			return data;
		}
		else if (type == STYPE_VOC)
		{
			if (!Voc::read((u8*)data, size))
			{
				LOG( LOG_ERROR, "Cannot read VOC data for sound." );
				return NULL;
			}

			rawSize = Voc::getRawSize();
			const void* outData = Voc::getRawData();
			Voc::free();

			return outData;
		}
		else if (type == STYPE_WAV)
		{
			if (!Wav::read((u8*)data, size))
			{
				LOG( LOG_ERROR, "Cannot read VOC data for sound." );
				return NULL;
			}

			rawSize = Wav::getRawSize();
			const void* outData = Wav::getRawData();
			Wav::free();

			return outData;
		}

		return NULL;
	}

	const f32* calculatePan(f32 pan)
	{
		//"half circle" position, as suggested by kcat (though z needs to be positive).
		s_pos2D[0] = pan;
		s_pos2D[2] = sqrtf(1.0f - pan*pan);

		return s_pos2D;
	}

	/////////////////////////////////////////////////
	// Music Streaming Update Thread
	/////////////////////////////////////////////////
	const u32 c_musicChunkSize = 4096;
	static u8 s_chunkData[c_musicBufferCount][c_musicChunkSize];
	static u32 s_currentMusicBuffer = 0;
	static u32 s_headMusicBuffer = 0;
	
	u32 XL_STDCALL streamMusic(void* userData)
	{
		while (1)
		{
			s_mutex->lock();
			if ( !s_musicPaused && !s_musicPlaying && s_musicCallback )
			{
				u8* chunkData = NULL;
				//get 2 buffers ready ahead of time
				s_musicCallback(s_musicUserData, c_musicChunkSize, s_chunkData[0]);
				s_musicCallback(s_musicUserData, c_musicChunkSize, s_chunkData[1]);
				
				//buffer the data and queue them up for playing
				alBufferData( s_musicBuffers[0], s_musicFormat, s_chunkData[0], c_musicChunkSize, s_musicSamplingRate );
				alBufferData( s_musicBuffers[1], s_musicFormat, s_chunkData[1], c_musicChunkSize, s_musicSamplingRate );
				alSourceQueueBuffers(s_musicSource, 2, s_musicBuffers);
				
				//play the music as a 2D sound
				const f32 zero[] = { 0.0f, 0.0f, 0.0f };
				alSourceStop(s_musicSource);
				alSourcef(s_musicSource, AL_ROLLOFF_FACTOR, 1.0f );
				alSourcei( s_musicSource, AL_SOURCE_RELATIVE, AL_TRUE );
				alSourcef( s_musicSource, AL_REFERENCE_DISTANCE, 15.0f );
				alSourcef( s_musicSource, AL_MAX_DISTANCE, 200.0f );
				alSourcefv( s_musicSource, AL_POSITION, zero );

				//set looping.
				alSourcei( s_musicSource, AL_LOOPING, AL_TRUE );

				//set the gain.
				alSourcef( s_musicSource, AL_GAIN, s_globalVolume );

				//finally play the sound.
				alSourcePlay( s_musicSource );

				s_headMusicBuffer = 0;
				s_currentMusicBuffer = 2;
				s_musicPlaying = true;
			}
			else if ( !s_musicPaused && s_musicCallback && s_musicBuffersProcessed )
			{
				s_musicCallback(s_musicUserData, c_musicChunkSize, s_chunkData[s_currentMusicBuffer]);
				alBufferData( s_musicBuffers[s_currentMusicBuffer], s_musicFormat, s_chunkData[s_currentMusicBuffer], c_musicChunkSize, s_musicSamplingRate );

				alSourceUnqueueBuffers(s_musicSource, 1, &s_musicBuffers[s_headMusicBuffer]);
				alSourceQueueBuffers(s_musicSource, 1, &s_musicBuffers[s_currentMusicBuffer]);

				s_headMusicBuffer++;
				s_musicBuffersProcessed--;
				s_currentMusicBuffer = (s_currentMusicBuffer+1) % c_musicBufferCount;
			}
			s_mutex->unlock();

			OS::sleep(1);
		};

		return 0;
	}
}
