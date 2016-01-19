#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

#define MAX_MAPPING_COUNT 16
#define MAX_ACTION_COUNT  128

#define XL_FILE_READ		0
#define XL_FILE_WRITE		1
#define XL_FILE_READWRITE	2

#define XL_FILE_ORIGIN_START	0
#define XL_FILE_ORIGIN_END		1
#define XL_FILE_ORIGIN_CURRENT	2

typedef int Bool;

enum XLSoundType
{
	STYPE_RAW = 0,
	STYPE_VOC,
	STYPE_COUNT
};

typedef struct ActionMapping
{
	char action[256];
	int  mappingCount;
	char keys[MAX_MAPPING_COUNT][256];
} ActionMapping;

typedef struct GameInfo
{
	char name[256];			//Proper game name (for example "Dark Forces")
	char lib[512];			//Game library name with no extension
	char iconFile[512];		//Icon file name
	char path[4096];		//Game Data absolute path (original source data only)
	int  iconID;			//Icon ID used for rendering the UI icon.

	//key mapping.
	int actionCount;
	ActionMapping actionMapping[MAX_ACTION_COUNT];
} GameInfo;

typedef struct SoundInfo
{
	unsigned int samplingRate;	//11025, ...
	unsigned int bitRate;		//8 or 16
	unsigned int userValue;		//used for optional callbacks
	float volume;				//volume, range: [0, 1]
	float pan;					//stereo pan, range: [-1, 1]
	Bool stereo;				//true for stereo, else mono
} SoundInfo;

typedef void  (*XLSoundCallback)(unsigned int);

typedef void  (*XLSetPalette)(unsigned char*, int);
typedef void  (*XLGetPalette)(unsigned char*);
typedef void  (*XLCopyToFramebuffer)(unsigned char*);

typedef int   (*XLGetClock)(void);
typedef void  (*XLSetClock)(int);
typedef void  (*XLResetClock)(void);
typedef int   (*XLGetScreenSize)(void);

typedef void* (*XLMalloc)(size_t);
typedef void* (*XLCalloc)(size_t, size_t);
typedef void* (*XLRealloc)(void*, size_t);
typedef void  (*XLFree)(void*);

typedef void  (*XLDebugMessage)(char*, ...);
typedef void  (*XLKeyEvent)(int, int, int);
typedef void  (*XLGetMouseDelta)(int*, int*);
typedef void  (*XLGetMouseButtons)(int*);

typedef const char*     (*XLBuildGamePath)(const char*);
typedef const GameInfo* (*XLGetGameInfo)(void);

typedef void (*XLSoundSetCallback)(XLSoundCallback);
typedef unsigned int (*XLSoundPlay2D)(const char*, const void*, unsigned int, unsigned int, SoundInfo*, Bool);
typedef int  (*XLSoundIsPlaying)(unsigned int);
typedef void (*XLSoundSetPan)(unsigned int, float);
typedef void (*XLStopSound)(unsigned int);
typedef void (*XLStopAllSounds)(void);
typedef int  (*XLSoundsPlaying)(void);

typedef int    (*XLFileOpen)(const char*, int);
typedef void   (*XLFileClose)(int);
typedef size_t (*XLFileRead)(void*, size_t, size_t, int);
typedef size_t (*XLFileWrite)(const void*, size_t, size_t, int);
typedef void   (*XLFileSeek)(int, int, int);
typedef size_t (*XLFileTell)(int);

typedef struct
{
	//video
	XLSetPalette        setPalette;
	XLGetPalette        getPalette;
	XLCopyToFramebuffer copyToFramebuffer;

	//virtual screen
	XLGetScreenSize getScreenWidth;
	XLGetScreenSize getScreenHeight;

	//timing
	XLGetClock   getClock;
	XLSetClock   setClock;
	XLResetClock resetClock;

	//input
	XLKeyEvent keyEvent;
	XLGetMouseDelta getMouseDelta;
	XLGetMouseButtons getMouseButtons;

	//memory
	XLMalloc  malloc;
	XLCalloc  calloc;
	XLRealloc realloc;
	XLFree    free;

	//debug
	XLDebugMessage debugMsg;

	//file I/O
	XLBuildGamePath buildGamePath;
	//file I/O functions automatically remap paths to the game root path.
	XLFileOpen		fileOpen;
	XLFileClose		fileClose;
	XLFileRead		fileRead;
	XLFileWrite		fileWrite;
	XLFileSeek		fileSeek;
	XLFileTell		fileTell;

	//game info
	XLGetGameInfo getGameInfo;

	//sound
	XLSoundSetCallback  soundSetCallback;
	XLSoundPlay2D		soundPlay2D;
	XLSoundIsPlaying	soundIsPlaying;
	XLSoundSetPan		soundSetVolume;
	XLSoundSetPan		soundSetPan;
	XLStopSound			stopSound;
	XLStopAllSounds		stopAllSounds;
	XLSoundsPlaying		soundsPlaying;
} XLEngineServices;

#ifdef __cplusplus
};
#endif
