#pragma once
#include "types.h"
#include "xlServices.h"
#include "Sound/midi.h"
#include "Graphics/graphicsDeviceList.h"

enum XLSettingsFlags
{
	XL_FLAG_NONE           = 0,
	XL_FLAG_FULLSCREEN     = (1<<0),	//full screen
	XL_FLAG_VSYNC		   = (1<<1),	//vsync
	XL_FLAG_IMMEDIATE_EXIT = (1<<2),	//exit the application when a game is exited.
	XL_FLAG_SHOW_ALL_GAMES = (1<<3),	//show all games even if they can't be run on the users system.
	XL_FLAG_UI_GLOW		   = (1<<4),	//enable the UI glow effect (visual only).
	XL_FLAG_COLOR_CORRECT  = (1<<5),	//enable color correction for the game view (slightly slower).
};

struct XLSettings
{
	u32 flags;
	s32 launchGameID;	//Game to launch on XL Engine startup (-1 to show the UI where the user can select a game)
	s32 frameLimit;		//Framerate limit (for the main thread)

	s32 windowScale;	//Window 4:3 scale, resolution = 320x240 x windowScale (not used when in fullscreen)
	s32 gameScale;		//Game scale, resolution = 320x200 x gameScale for 16:10 in 4:3 or 320x240 x gameScale for 4:3

	s32 windowWidth;	//Final window width  after taking into account fullscreen and window scale
	s32 windowHeight;	//Final window height after taking into account fullscreen and window scale

	s32 gameWidth;		//Final virtual game width  after taking into account game scale
	s32 gameHeight;		//Final virtual game height after taking into account game scale

	f32 colorCorrect[4];//Color correction values: brightness, saturation, contrast, gamma

	//Sound
	MidiFormat midiformat;
	char patchDataLoc[256];
	u32  musicVolume;
	u32  soundVolume;
};

namespace Settings
{
	const char* getVersion();

	bool read(int monitorWidth, int monitorHeight);
	void write();
	void initGameData();

	XLSettings* get();

	s32 getGameCount();
	GameInfo* getGameInfo(s32 id);

	void setGameID(s32 id);
	s32  getGameID();

	GraphicsDeviceID getGraphicsDeviceID();
	void setGraphicsDeviceID(GraphicsDeviceID deviceID);
};
