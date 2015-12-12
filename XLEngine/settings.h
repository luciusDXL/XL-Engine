#pragma once
#include "types.h"
#include "xlServices.h"

enum XLSettingsFlags
{
	XL_FLAG_NONE           = 0,
	XL_FLAG_FULLSCREEN     = (1<<0),	//full screen
	XL_FLAG_IMMEDIATE_EXIT = (1<<1),	//exit the application when a game is exited.
	XL_FLAG_SHOW_ALL_GAMES = (1<<2),	//show all games even if they can't be run on the users system.
};

struct XLSettings
{
	u32 flags;
	s32 launchGameID;	//Game to launch on XL Engine startup (-1 to show the UI where the user can select a game)

	s32 windowScale;	//Window 4:3 scale, resolution = 320x240 x windowScale (not used when in fullscreen)
	s32 gameScale;		//Game scale, resolution = 320x200 x gameScale for 16:10 in 4:3 or 320x240 x gameScale for 4:3

	s32 windowWidth;	//Final window width  after taking into account fullscreen and window scale
	s32 windowHeight;	//Final window height after taking into account fullscreen and window scale

	s32 gameWidth;		//Final virtual game width  after taking into account game scale
	s32 gameHeight;		//Final virtual game height after taking into account game scale
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
};
