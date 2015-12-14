#include "settings.h"
#include "iniReader.h"
#include "iniWriter.h"
#include "UI/uiSystem.h"
#include "filestream.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

namespace Settings
{
	#define MAX_GAME_COUNT 256

	static const char* c_versionName = "(Beta 1)";
	static const u32 c_xlEngineMajorVersion = 0;	//Major version - version 1 will be the first full release, 0 is Beta
	static const u32 c_xlEngineMinorVersion = 2;	//Minor version

	static char s_xlEngineVersion[512];				//Version string which will be filled in when the settings are read.
	static u32  s_xlEngineBuild = 0;				//Build number which will be filled in when the settings are read.

	//default values.
	static XLSettings s_settings = 
	{
		XL_FLAG_SHOW_ALL_GAMES,
		-1,
		4,
		4,
		320*4,
		240*4,
		320*4,
		200*4
	};
	
	static s32 s_gameCount  = 0;
	static s32 s_gameID     = -1;
	static s32 s_keyMapping = -1;
	static bool s_setDefaultMapping = false;
	static GameInfo s_games[MAX_GAME_COUNT];

	//Hard coded for now, later auto-detect at startup and/or read from the settings file.
	static GraphicsDeviceID s_graphicsDeviceID = GDEV_OPENGL_1_3;

	const char* getVersion()
	{
		return s_xlEngineVersion;
	}
	
	s32 getGameCount()
	{
		return s_gameCount;
	}

	GameInfo* getGameInfo(s32 id)
	{
		assert(id >= 0 && id < s_gameCount);
		if (id < 0 || id >= s_gameCount) 
		{ 
			return NULL; 
		}

		return &s_games[id];
	}

	void setGameID(s32 id)
	{
		assert(id >= 0 && id < s_gameCount);
		s_gameID = id;
	}

	s32  getGameID()
	{
		return s_gameID;
	}
	
	GraphicsDeviceID getGraphicsDeviceID()
	{
		return s_graphicsDeviceID;
	}

	void setGraphicsDeviceID(GraphicsDeviceID deviceID)
	{
		s_graphicsDeviceID = deviceID;
	}

	void chooseDefaultResolution(int monitorWidth, int monitorHeight)
	{
		const bool fullscreen = (s_settings.flags & XL_FLAG_FULLSCREEN) != 0;

		//for the window size, pick the largest scale under the monitor size unless fullscreen is enabled
		//which will set the resolution directly to the monitor resolution.
		int scale = 5;
		if (fullscreen)
		{
			s_settings.windowWidth  = monitorWidth;
			s_settings.windowHeight = monitorHeight;

			//find the largest game scale that produces a resolution less then or equal to the monitor resolution.
			for (; scale>=0; scale--)
			{
				if ( 320*scale <= monitorWidth && 200*scale <= monitorHeight)
				{
					s_settings.gameScale = scale;
					break;
				}
			}
		}
		else
		{
			for (; scale>=0; scale--)
			{
				if ( 320*scale <= monitorWidth && 240*scale <= monitorHeight)
				{
					s_settings.windowScale = scale;
					break;
				}
			}

			//default the game scale to match the window scale
			s_settings.gameScale = s_settings.windowScale;

			s_settings.windowWidth  = 320*s_settings.windowScale;
			s_settings.windowHeight = 240*s_settings.windowScale;
		}

		s_settings.gameWidth  = 320*s_settings.gameScale;
		s_settings.gameHeight = 200*s_settings.gameScale;
	}

	bool readBool(const char* value)
	{
		if (stricmp(value, "false") == 0 || stricmp(value, "0") == 0)
		{
			return false;
		}
		return true;
	}

	void assignKeyMappings(ActionMapping* action, const char* value)
	{
		//assign the list of actions.
		action->mappingCount = 0;
		s32 index = 0;
		char* keyStr = action->keys[0];

		const char* keys = value;
		while (*keys != '\r' && *keys != '\n' && *keys != 0)
		{
			if (*keys == ',' && *(keys+1) != ',')
			{
				keyStr[index] = 0;
				action->mappingCount++;

				keyStr = action->keys[action->mappingCount];
				index  = 0;
			}
			else
			{
				keyStr[index++] = *keys;
			}

			keys++;
		};
		keyStr[index] = 0;
		action->mappingCount++;
	}

	bool readCallback(const char* key, const char* value)
	{
		if (stricmp(key, "fullscreen") == 0)
		{
			if (readBool(value))
			{
				s_settings.flags |= XL_FLAG_FULLSCREEN;
			}
			else
			{
				s_settings.flags &= ~XL_FLAG_FULLSCREEN;
			}
		}
		else if (stricmp(key, "immediateExit") == 0)
		{
			if (readBool(value))
			{
				s_settings.flags |= XL_FLAG_IMMEDIATE_EXIT;
			}
			else
			{
				s_settings.flags &= ~XL_FLAG_IMMEDIATE_EXIT;
			}
		}
		else if (stricmp(key, "showAllGames") == 0)
		{
			if (readBool(value))
			{
				s_settings.flags |= XL_FLAG_SHOW_ALL_GAMES;
			}
			else
			{
				s_settings.flags &= ~XL_FLAG_SHOW_ALL_GAMES;
			}
		}
		else if (stricmp(key, "launchGame") == 0)
		{
			s_settings.launchGameID = -1;
			for (int g=0; g<s_gameCount; g++)
			{
				if (stricmp(value, s_games[g].name) == 0)
				{
					s_settings.launchGameID = g;
					break;
				}
			}
		}
		else if (stricmp(key, "windowScale") == 0)
		{
			char* endPtr = NULL;
			s_settings.windowScale = strtol(value, &endPtr, 10);
		}
		else if (stricmp(key, "gameScale") == 0)
		{
			char* endPtr = NULL;
			s_settings.gameScale = strtol(value, &endPtr, 10);
		}
		else if (stricmp(key, "gameCount") == 0)
		{
			char* endPtr = NULL;
			s_gameCount = strtol(value, &endPtr, 10);
		}
		else if (stricmp(key, "keyMapping") == 0)
		{
			char* endPtr = NULL;
			s_keyMapping = strtol(value, &endPtr, 10);
		}
		else if (s_keyMapping >= 0 && s_setDefaultMapping)
		{
			//read the action name, assume no repeats...
			GameInfo* info = &s_games[ s_keyMapping ];
			ActionMapping* action = &info->actionMapping[ info->actionCount ];

			strcpy(action->action, key);
			assignKeyMappings(action, value);
			info->actionCount++;
		}
		else if (s_keyMapping >= 0)
		{
			GameInfo* info = &s_games[ s_keyMapping ];

			//first find the action
			s32 actionID = -1;
			for (s32 a=0; a<info->actionCount; a++)
			{
				if ( stricmp(info->actionMapping[a].action, key) == 0 )
				{
					actionID = a;
					break;
				}
			}

			//then, if it exists, reassign the keys.
			if (actionID >= 0)
			{
				ActionMapping* action = &info->actionMapping[actionID];

				//clear out the old mapping
				memset(action->keys, 0, MAX_MAPPING_COUNT*256);

				//replace it with new mappings.
				assignKeyMappings(action, value);
			}
		}
		else
		{
			char nameKey[32];
			char libKey[32];
			char iconKey[256];
			char pathKey[4096];

			for (int g=0; g<s_gameCount; g++)
			{
				sprintf(nameKey, "game%dName", g);
				sprintf(libKey,  "game%dLib",  g);
				sprintf(iconKey, "game%dIcon", g);
				sprintf(pathKey, "game%dPath", g);

				if (stricmp(key, nameKey) == 0)
				{
					strcpy(s_games[g].name, value);
				}
				else if (stricmp(key, libKey) == 0)
				{
					strcpy(s_games[g].lib, value);
				}
				else if (stricmp(key, iconKey) == 0)
				{
					strcpy(s_games[g].iconFile, value);
				}
				else if (stricmp(key, pathKey) == 0)
				{
					strcpy(s_games[g].path, value);
				}
			}
		}

		return true;
	}

	void initGameData()
	{
		//load game icons
		for (int g=0; g<s_gameCount; g++)
		{
			s_games[g].iconID = UISystem::addIcon(s_games[g].iconFile);
		}
	}

	bool readGameData()
	{
		s_keyMapping = -1;
		s_setDefaultMapping = true;
		memset(s_games, 0, sizeof(GameInfo)*MAX_GAME_COUNT);

		return iniReader::readIni("xlgames.ini", readCallback);
	}

	void readBuildVersion()
	{
		FileStream buildVersion;
		if (buildVersion.open("buildVersion.txt", FileStream::MODE_READ))
		{
			//read characters
			char buildNum[256] = {0};
			int idx = 0;
			while (1)
			{
				char c;
				buildVersion.read(&c);

				if (c < '0' || c > '9')
				{
					break;
				}
				else
				{
					buildNum[idx++] = c;
				}
			}
			buildVersion.close();

			char* endPtr = NULL;
			buildNum[idx] = 0;
			s32 lBuildNum = strtol(buildNum, &endPtr, 10);
			s_xlEngineBuild = u32(lBuildNum);
		}
		sprintf(s_xlEngineVersion, "%d.%d.%d %s", c_xlEngineMajorVersion, c_xlEngineMinorVersion, s_xlEngineBuild, c_versionName);
	}

	bool read(int monitorWidth, int monitorHeight)
	{
		//read the build version...
		readBuildVersion();

		//read the base game data (names, data paths, icon files, etc.)
		if (!readGameData())
		{
			return false;
		}

		//read addition/user configuration data.
		chooseDefaultResolution(monitorWidth, monitorHeight);

		bool writeRequired = true;
		s_keyMapping = -1;
		s_setDefaultMapping = false;
		if (iniReader::readIni("xlsettings.ini", readCallback))
		{
			s_settings.windowWidth  = 320*s_settings.windowScale;
			s_settings.windowHeight = 240*s_settings.windowScale;

			s_settings.gameWidth  = 320*s_settings.gameScale;
			s_settings.gameHeight = 200*s_settings.gameScale;

			writeRequired = false;

			//fixup broken resolution settings [changing monitors or desktop resolution for example]
			while (s_settings.windowScale > 3 && (s_settings.windowWidth > monitorWidth || s_settings.windowHeight > monitorHeight))
			{
				s_settings.windowScale--;
				s_settings.windowWidth  = 320*s_settings.windowScale;
				s_settings.windowHeight = 240*s_settings.windowScale;
				writeRequired = true;
			}

			if (s_settings.flags & XL_FLAG_FULLSCREEN)	//fullscreen = monitor resolution
			{
				s_settings.windowWidth  = monitorWidth;
				s_settings.windowHeight = monitorHeight;
			}
			else if (s_settings.windowScale < 3)		//minimum window size for the UI (960x720)
			{
				s_settings.windowScale = 3;
				s_settings.windowWidth  = 320*s_settings.windowScale;
				s_settings.windowHeight = 240*s_settings.windowScale;
				writeRequired = true;
			}

			//fixup broken resolution settings [changing monitors or desktop resolution for example]
			while (s_settings.gameScale > 1 && (s_settings.gameWidth > s_settings.windowWidth || s_settings.gameHeight > s_settings.windowHeight))
			{
				s_settings.gameScale--;
				s_settings.gameWidth  = 320*s_settings.gameScale;
				s_settings.gameHeight = 200*s_settings.gameScale;
				writeRequired = true;
			}
		}

		//write out the fixed values.
		if (writeRequired)
		{
			write();
		}

		return true;
	}
	
	void write()
	{
		iniWriter::open("xlsettings.ini");

		iniWriter::comment("Flags");
		iniWriter::write("fullscreen",    bool( (s_settings.flags&XL_FLAG_FULLSCREEN)!=0 ));
		iniWriter::write("immediateExit", bool( (s_settings.flags&XL_FLAG_IMMEDIATE_EXIT)!=0 ));
		iniWriter::write("showAllGames",  bool( (s_settings.flags&XL_FLAG_SHOW_ALL_GAMES)!=0 ));
		iniWriter::newLine();

		iniWriter::comment("Video");
		iniWriter::write("windowScale", s_settings.windowScale);
		iniWriter::write("gameScale",   s_settings.gameScale);
		iniWriter::newLine();

		iniWriter::comment("Engine Settings");
		iniWriter::write("launchGame", s_settings.launchGameID < 0 ? "None" : s_games[s_settings.launchGameID].name);
		iniWriter::newLine();

		iniWriter::comment("Game Data");
		char gamePath[4096];
		for (int g=0; g<s_gameCount; g++)
		{
			sprintf(gamePath, "game%dPath", g);
			iniWriter::write(gamePath, s_games[g].path);
		}
		iniWriter::newLine();

		//write the action/key mappings
		iniWriter::comment("Action/Key Mapping");
		char str[512];
		for (s32 g=0; g<s_gameCount; g++)
		{
			sprintf(str, "Game %d (%s)", g, s_games[g].name);
			iniWriter::comment(str);

			iniWriter::write("keyMapping", g);
			for (s32 a=0; a<s_games[g].actionCount; a++)
			{
				const ActionMapping* action = &s_games[g].actionMapping[a];

				str[0] = 0;
				for (s32 m=0; m<action->mappingCount; m++)
				{
					strcat(str, action->keys[m]);
					if (m < action->mappingCount-1)
					{
						strcat(str, ",");
					}
				}
				iniWriter::writeStrNoQuotes(action->action, str);
			}
			iniWriter::newLine();
		}
		
		iniWriter::close();
	}

	XLSettings* get()
	{
		return &s_settings;
	}
};
