#include "midi.h"
#include "sound.h"
#include "oggVorbis.h"
#include "../log.h"
#include "../filestream.h"
#include "../Threads/mutex.h"
#include <vorbis/vorbisfile.h>
#include <stdlib.h>

namespace oggVorbis
{
	#define LOCK()	 s_mutex->lock()
	#define UNLOCK() s_mutex->unlock()

	const  f32 c_volumeScale = 0.5f;

	static bool s_initialized = false;
	static FileStream s_fileStream;
	static OggVorbis_File s_oggFile;
	static Bool s_stereo;
	static u32  s_samplingRate;
	static f32  s_volume = c_volumeScale;

	static Mutex* s_mutex = NULL;
					
	void oggStreamCallback(void* userData, u32 requestedChunkSize, u8* chunkData);

	bool init()
	{
		free();
		s_mutex = Mutex::create();

		return true;
	}

	void free()
	{
		if (s_initialized)
		{
			stop();
			s_initialized = false;
		}
		delete s_mutex;
	}
	
	void setVolume(u32 volume)
	{
		s_volume = f32(volume) * 0.01f * c_volumeScale;
	}

	void playResume()
	{
		Sound::startMusic( oggStreamCallback, NULL, s_volume, s_stereo, 16, s_samplingRate );
	}

	void pause()
	{
		Sound::pauseMusic();
	}

	void stop()
	{
		Sound::stopMusic();
		LOCK();
			if (s_fileStream.isOpen())
			{
				ov_clear(&s_oggFile);
				s_fileStream.close();
			}
		UNLOCK();
	}

	bool loadOGG(const char* file)
	{
		LOCK();
			if (s_fileStream.open(file, FileStream::MODE_READ))
			{
				void* dataSource = s_fileStream.getFileHandle();
				if (ov_open_callbacks(dataSource, &s_oggFile, NULL, 0, OV_CALLBACKS_NOCLOSE) < 0)
				{
					LOG( LOG_ERROR, "The Ogg file \"%s\" is not a valid.", file);
					UNLOCK();
					return false;
				}

				const vorbis_info* info = ov_info(&s_oggFile, -1);
				s_stereo = info->channels > 1;
				s_samplingRate = info->rate;

				UNLOCK();
				return true;
			}
		
		UNLOCK();
		return false;
	}

	void oggStreamCallback(void* userData, u32 requestedChunkSize, u8* chunkData)
	{
		LOCK();

		s32 totalSize = 0;
		while (totalSize < (s32)requestedChunkSize)
		{
			s32 bitStream = 0;
			s32 soundSize = ov_read(&s_oggFile, (char*)&chunkData[totalSize], requestedChunkSize-totalSize, 0, 2, 1, &bitStream);

			//theoretically the sampling rate can change throughout the song...
			//however this is not supported by the XL Engine currently (it would require more work in the sound layer).

			//we've finished the loop, start the song all over again.
			if (soundSize == 0)
			{
				ov_time_seek(&s_oggFile, 0.0);
				soundSize = ov_read(&s_oggFile, (char*)&chunkData[totalSize], requestedChunkSize-totalSize, 0, 2, 1, &bitStream);
			}
			else if (soundSize < 0)	//error in the bit stream.
			{
				LOG( LOG_ERROR, "ogg streaming error: %d", soundSize );
				soundSize = 0;
				break;
			}

			totalSize += soundSize;
		};

		UNLOCK();
	}
};