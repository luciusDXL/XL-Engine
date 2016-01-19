#include "services.h"
#include "gameUI.h"
#include "clock.h"
#include "memoryPool.h"
#include "settings.h"
#include "input.h"
#include "Sound/sound.h"
#include "Graphics/graphicsDevice.h"
#include "log.h"
#include "filestream.h"
#include <stdio.h>
#include <stdarg.h>
#include <memory.h>
#include <assert.h>

namespace Services
{
	XLEngineServices s_services;
	u32 s_palette[256]={0};
	u8  s_origPalette[1024]={0};

	int s_gameScreenWidth;
	int s_gameScreenHeight;
	int s_clockTics = 0;
	int s_clockTicsBase = 0;
	u64 s_lastRealTime = 0;

	//File System
	#define MAX_OPEN_FILES 512
	FileStream s_fileStreams[ MAX_OPEN_FILES ];

	GraphicsDevice* s_gdev = NULL;

	void xlSetPalette(u8* palette, int mode)
	{
		const int rIndex = 0;
		const int gIndex = 1;
		const int bIndex = 2;
		const int aIndex = 3;

		const u32 shift = 2;

		for (int i=0; i<256; i++)
		{
			u32 r = palette[i*4+rIndex];
			u32 g = palette[i*4+gIndex];
			u32 b = palette[i*4+bIndex];
			u32 a = palette[i*4+aIndex];

			s_origPalette[i*4+0] = r;
			s_origPalette[i*4+1] = g;
			s_origPalette[i*4+2] = b;
			s_origPalette[i*4+3] = a;

			if (r < 64) { r <<= shift; }
			if (g < 64) { g <<= shift; }
			if (b < 64) { b <<= shift; }

			s_palette[i] = (r) | (g<<8) | (b<<16) | (a<<24);
		}
	}

	void xlGetPalette(u8* palette)
	{
		memcpy(palette, s_origPalette, 1024);	//256*4
	}

	void xlCopyToFramebuffer(u8* buffer)
	{
		s_gdev->convertFrameBufferTo32bpp(buffer, s_palette);
	}

	void setTime(u64 time)
	{
		s_lastRealTime = time;
	}

	void xlDebugMessage(char* msg, ...)
	{
		static char outMsg[4096];

		va_list arg;
		va_start(arg, msg);
		int res = vsprintf(outMsg, msg, arg);
		va_end(arg);

		if (res < 0)
		{
			LOG(LOG_ERROR, "xlDebugMessage() - bad string.");
			return;
		}
		LOG(LOG_MESSAGE, outMsg);
	}

	int  xlGetClock(void)
	{
		if (!GameUI::getShowUI())
		{
			u64 delta = Clock::getTime_uS() - s_lastRealTime;
			//1000000 = 1 second...
			//8333.3333333333333333333333333333 = 1/120 second.
			double tics = (double)delta / 8333.3333333333333333333333333333;
			s_clockTics = s_clockTicsBase + (int)tics;
		}
		else
		{
			s_clockTicsBase = s_clockTics;
			s_lastRealTime = Clock::getTime_uS();
		}

		return s_clockTics;
	}

	void xlSetClock(int newValue)
	{
		s_lastRealTime = Clock::getTime_uS();
		s_clockTics = newValue;
		s_clockTicsBase = newValue;
	}

	void xlResetClock(void)
	{
		s_lastRealTime = Clock::getTime_uS();
		s_clockTics = 0;
		s_clockTicsBase = 0;
	}

	int xlGetScreenWidth(void)
	{
		return s_gameScreenWidth;
	}

	int xlGetScreenHeight(void)
	{
		return s_gameScreenHeight;
	}

	void getMouseDelta(int* dx, int* dy)
	{
		Input::getMouseDelta(dx, dy);
	}

	void getMouseButtons(int* buttons)
	{
		buttons[0] = Input::getMouseButtonState( Input::MouseLeft )   ? 1 : 0;
		buttons[1] = Input::getMouseButtonState( Input::MouseMiddle ) ? 1 : 0;
		buttons[2] = Input::getMouseButtonState( Input::MouseRight )  ? 1 : 0;
	}
	
	const char* buildGamePath(const char* inpath)
	{
		static char filepath[4096];
		const s32 gameID = Settings::getGameID();

		assert(gameID >= 0);
		GameInfo* info = Settings::getGameInfo( gameID );	
		sprintf(filepath, "%s/%s", info->path, inpath);

		return filepath;
	}

	const GameInfo* getGameInfo(void)
	{
		const s32 currentGameID = Settings::getGameID();
		const GameInfo* info = Settings::getGameInfo( currentGameID );
		assert(currentGameID >= 0 && info != NULL);

		return info;
	}

	//file system
	int xlFileOpen(const char* path, int fileMode)
	{
		const char* gamePath = buildGamePath(path);

		//search for an open file stream.
		s32 streamIndex = -1;
		for (u32 f=0; f<MAX_OPEN_FILES; f++)
		{
			if ( !s_fileStreams[f].isOpen() )
			{
				streamIndex = f;
				break;
			}
		}

		if (streamIndex >= 0)
		{
			if ( !s_fileStreams[ streamIndex ].open( gamePath, FileStream::FileMode(fileMode) ) )
			{
				return -1;
			}
		}

		return streamIndex;
	}

	void xlFileClose(int handle)
	{
		if (handle >= 0 && handle < MAX_OPEN_FILES)
		{
			assert( s_fileStreams[handle].isOpen() );
			s_fileStreams[handle].close();
		}
	}

	size_t xlFileRead(void* data, size_t size, size_t count, int fileHandle)
	{
		if (fileHandle < 0 && fileHandle >= MAX_OPEN_FILES)
		{
			return 0;
		}
		assert( s_fileStreams[fileHandle].isOpen() );

		s_fileStreams[fileHandle].readBuffer(data, size, count);
		return size*count;
	}

	size_t xlFileWrite(const void* data, size_t size, size_t count, int fileHandle)
	{
		if (fileHandle < 0 && fileHandle >= MAX_OPEN_FILES)
		{
			return 0;
		}
		assert( s_fileStreams[fileHandle].isOpen() );

		s_fileStreams[fileHandle].writeBuffer(data, size, count);
		return size*count;
	}

	void xlFileSeek(int offset, int origin, int fileHandle)
	{
		if (fileHandle < 0 && fileHandle >= MAX_OPEN_FILES)
		{
			return;
		}
		assert( s_fileStreams[fileHandle].isOpen() );

		s_fileStreams[fileHandle].seek(offset, Stream::Origin(origin));
	}

	size_t xlFileTell(int fileHandle)
	{
		if (fileHandle < 0 && fileHandle >= MAX_OPEN_FILES)
		{
			return 0;
		}
		assert( s_fileStreams[fileHandle].isOpen() );

		return s_fileStreams[fileHandle].getLoc();
	}

	void setup(int gameWidth, int gameHeight, GraphicsDevice* gdev)
	{
		s_gameScreenWidth  = gameWidth;
		s_gameScreenHeight = gameHeight;
		s_gdev   = gdev;

		s_services.setPalette = xlSetPalette;
		s_services.getPalette = xlGetPalette;
		s_services.copyToFramebuffer = xlCopyToFramebuffer;

		s_services.getClock   = xlGetClock;
		s_services.setClock   = xlSetClock;
		s_services.resetClock = xlResetClock;

		s_services.getScreenWidth  = xlGetScreenWidth;
		s_services.getScreenHeight = xlGetScreenHeight;

		s_services.malloc  = MemoryPool::xlMalloc;
		s_services.calloc  = MemoryPool::xlCalloc;
		s_services.realloc = MemoryPool::xlRealloc;
		s_services.free    = MemoryPool::xlFree;

		s_services.debugMsg        = xlDebugMessage;
		s_services.getMouseDelta   = getMouseDelta;
		s_services.getMouseButtons = getMouseButtons;
		s_services.keyEvent        = NULL;	//this will be filled in by the game if it wants to get keyboard events.

		s_services.buildGamePath = buildGamePath;
		s_services.fileOpen		 = xlFileOpen;
		s_services.fileClose	 = xlFileClose;
		s_services.fileRead		 = xlFileRead;
		s_services.fileWrite	 = xlFileWrite;
		s_services.fileSeek	 	 = xlFileSeek;
		s_services.fileTell		 = xlFileTell;

		s_services.getGameInfo   = getGameInfo;

		s_services.soundSetCallback = Sound::setCallback;
		s_services.soundPlay2D		= Sound::playSound2D;
		s_services.soundIsPlaying	= Sound::isPlaying;
		s_services.soundSetPan		= Sound::setPan;
		s_services.soundSetVolume	= Sound::setVolume;
		s_services.stopSound		= Sound::stopSound;
		s_services.stopAllSounds	= Sound::stopAllSounds;
		s_services.soundsPlaying	= Sound::soundsPlaying;
	}

	void reset()
	{
		//close any open file streams.
		for (u32 f=0; f<MAX_OPEN_FILES; f++)
		{
			s_fileStreams[f].close();
		}
	}

	XLEngineServices* get()
	{
		return &s_services;
	}
};