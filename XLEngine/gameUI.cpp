#include "gameUI.h"
#include "settings.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <assert.h>

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN 1
	#include <Windows.h>
#endif

namespace GameUI
{
	enum ControlIDs
	{
		CTRL_ICON_VIDEO = 1,
		CTRL_ICON_SETTINGS,
		CTRL_ICON_CONTROLS,
		CTRL_ICON_DASHBOARD,
		CTRL_ICON_SEARCH,
		CTRL_ICON_TOOLS,
		
		CTRL_WIN_VIDEO,
		CTRL_WIN_SETTINGS,

		CTRL_GAME_ICON = 256,

		CTRL_MISC = 512,
	};

	enum Menus
	{
		MENU_VIDEO,
		MENU_SETTINGS,
		MENU_COUNT
	};

	bool s_showUI = true;
	static int s_menu = -1;

	StartGameFunc s_startGame = NULL;
	StopGameFunc  s_stopGame  = NULL;

	bool getShowUI()
	{
		return s_showUI;
	}

	void setShowUI(bool show, GraphicsDevice* gdev)
	{
		s_showUI = show;
		if (!show)
		{
			gdev->setVirtualViewport(true, 0, 0, 0, 0);
		}
	}

	void init(StartGameFunc startGame, StopGameFunc stopGame)
	{
		s_startGame = startGame;
		s_stopGame  = stopGame;
	}

	void handleMainIcons()
	{
		if (UISystem::buttonIcon(CTRL_ICON_VIDEO, 0, ICON_VIDEO, 4,  40 ))
		{
			if (s_menu < 0)
			{
				s_menu = MENU_VIDEO;
				UISystem::setCurrentLayer(1);
			}
		}
		if (UISystem::buttonIcon(CTRL_ICON_SETTINGS, 0, ICON_SETTINGS,  4,  80 ))
		{
			if (s_menu < 0)
			{
				s_menu = MENU_SETTINGS;
				UISystem::setCurrentLayer(1);
			}
		}
		UISystem::buttonIcon(CTRL_ICON_CONTROLS,  0, ICON_CONTROLS,  4, 120 );
		UISystem::buttonIcon(CTRL_ICON_DASHBOARD, 0, ICON_DASHBOARD, 4, 160 );
		UISystem::buttonIcon(CTRL_ICON_SEARCH,    0, ICON_SEARCH,    4, 200 );
		UISystem::buttonIcon(CTRL_ICON_TOOLS,     0, ICON_TOOLS,     4, 240 );
	}

	void handleGameSelection(GraphicsDevice* gdev, int winWidth, int winHeight, s32& gameRunning)
	{
		int gx = 100, gy = 64;
		for (int g=0; g<Settings::getGameCount(); g++)
		{
			if (gameRunning == g)
			{
				if (gx + 332 > winWidth-20)
				{
					gx  = 100;
					gy += 350;
				}

				gdev->setVirtualViewport(false, gx, winHeight-gy-200-50, 266, 200);

				DrawTextBuf* text = Draw2D::getDrawText();
				text->color = PACK_RGBA(0xa0, 0xff, 0xff, 0xa0);
				text->font = UISystem::getFont(18);
				text->x = gx+2; text->y = gy+252;

				GameInfo* info = Settings::getGameInfo(g);
				strcpy(text->msg, info->name);

				gx += 332;
				if (gx + 266 > winHeight-20)
				{
					gx  = 100;
					gy += 350;
				}
			}
			else
			{
				GameInfo* info = Settings::getGameInfo(g);
				if (UISystem::buttonIcon(CTRL_GAME_ICON+g, 0, info->iconID, gx, gy))
				{
					if (gameRunning != g)
					{
						s_stopGame();
						if (s_startGame( g ))
						{
							s_showUI = false;
							gdev->setVirtualViewport(true, 0, 0, 0, 0);
							return;
						}
					}
				}

				gx += 300;
				if (gx + 234 > winWidth-20)
				{
					gx  = 100;
					gy += 350;
				}
			}
		}
	}

	void handleVideoMenu(GraphicsDevice* gdev, int winWidth, int winHeight, s32& gameRunning)
	{
		char msg[512];
		XLSettings* settings = Settings::get();

		if (UISystem::window(CTRL_WIN_VIDEO, 1, "Video Settings", 32, 32, 400, 400))
		{
			s_menu = -1;
			UISystem::setCurrentLayer(0);
		}
		else
		{
			bool fullscreen = (settings->flags&XL_FLAG_FULLSCREEN)!=0;
			bool advancedGL = false;

			UISystem::staticText(1, fullscreen ? "[X] Fullscreen" : "[ ] Fullscreen", 40, 60);
			UISystem::staticText(1, "[ ] Vsync", 40, 80);
			UISystem::staticText(1, "[ ] Stretch to Fit", 40, 100);
			UISystem::staticText(1, "------------- Game Resolution -------------", 40, 140);
			UISystem::staticText(1, "[X] Use 320x200 multiple (16:10 in 4:3)", 40, 160);
			UISystem::staticText(1, "[ ] Use 4:3 aspect ratio", 40, 180);
			sprintf(msg, "Game Resolution: [%dx%d]", settings->gameWidth, settings->gameHeight);
			UISystem::staticText(1, msg, 40, 200);

			Color winResColor = PACK_RGBA(190, 255, 255, 255);
			if (fullscreen) { winResColor = PACK_RGBA(128, 128, 128, 255); }
			UISystem::staticText(1, "--------------- Window Size ---------------", 40, 240, winResColor);

			sprintf(msg, "Window Resolution: [%dx%d]", settings->windowWidth, settings->windowHeight);
			UISystem::staticText(1, msg, 40, 260, winResColor);

			Color advOptColor = PACK_RGBA(190, 255, 255, 255);
			if (!advancedGL) { advOptColor = PACK_RGBA(128, 128, 128, 255); }
			UISystem::staticText(1, "---------------- Graphics -----------------", 40, 300);
			UISystem::staticText(1, "Backend: [OpenGL 1.5 - Fixed Function]", 40, 320);
			UISystem::staticText(1, "[ ] UI Glow", 40, 340, advOptColor);
		}
	}

	void handleSettingsMenu(GraphicsDevice* gdev, int winWidth, int winHeight, s32& gameRunning)
	{
		char msg[512];
		XLSettings* settings = Settings::get();

		if (UISystem::window(CTRL_WIN_SETTINGS, 1, "General Settings", 32, 32, 550, 512))
		{
			s_menu = -1;
			UISystem::setCurrentLayer(0);
		}
		else
		{
			UISystem::staticText(1, "[About]", 40, 60);

			sprintf(msg, "[%c] Show all games (uncheck to show only available games)", (settings->flags&XL_FLAG_SHOW_ALL_GAMES) ? 'X' : ' ');
			UISystem::staticText(1, msg, 40, 80);

			sprintf(msg, "[%c] Exit Application on game exit", (settings->flags&XL_FLAG_IMMEDIATE_EXIT) ? 'X' : ' ');
			UISystem::staticText(1, msg, 40, 100);

			GameInfo* info = settings->launchGameID > -1 ? Settings::getGameInfo(settings->launchGameID) : NULL;
			sprintf(msg, "[%c] Launch Game on Startup: \"%s\"", (settings->launchGameID > -1) ? 'X' : ' ', (settings->launchGameID > -1) ? info->name : "None");
			UISystem::staticText(1, msg, 40, 120);
		}
	}

	void update(GraphicsDevice* gdev, int winWidth, int winHeight, s32& gameRunning)
	{
		if (!s_showUI)
			return;

		UISystem::predraw();
		UISystem::begin();

		DrawTextBuf* text = Draw2D::getDrawText();
		text->color = PACK_RGBA(0xa0, 0xff, 0xff, 0xa0);
		text->font = UISystem::getFont(18);
		text->x = 4; text->y = 4;
		sprintf(text->msg, "XL Engine  version %s", Settings::getVersion());

		handleMainIcons();
		handleGameSelection(gdev, winWidth, winHeight, gameRunning);

		if (s_menu == MENU_VIDEO)
		{
			handleVideoMenu(gdev, winWidth, winHeight, gameRunning);
		}
		else if (s_menu == MENU_SETTINGS)
		{
			handleSettingsMenu(gdev, winWidth, winHeight, gameRunning);
		}

		UISystem::finish();
	}

	//cursor display is reference counted (i.e. cursor is displayed if value >= 0)
	//so this function will only change the cursor state if required, avoiding
	//adding or removing extra references.
	void enableCursor(bool enable)
	{
	#ifdef _WIN32
		CURSORINFO ci;
		ci.cbSize        = sizeof(CURSORINFO);
		ci.flags         = 0;
		ci.hCursor       = 0;
		ci.ptScreenPos.x = 0;
		ci.ptScreenPos.y = 0;

		if ( GetCursorInfo(&ci) )
		{
			const bool cursorShowing = (ci.flags&CURSOR_SHOWING)!=0;
			if (cursorShowing != enable)
			{
				ShowCursor(enable);
			}
		}
	#endif
	}
};