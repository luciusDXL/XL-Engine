#include "gameLoop.h"
#include "settings.h"
#include "services.h"
#include "memoryPool.h"
#include "gameUI.h"
#include "input.h"
#include "Sound/sound.h"
#include "PluginFramework/PluginManager.h"
#include "clock.h"
#include "log.h"

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN 1
	#include <Windows.h>
	#include "Graphics/Win32/graphicsDeviceGL_Win32.h"
#endif

namespace GameLoop
{
	static s32 s_gameRunning = -1;
	static s32 s_launchGameID = -1;
	static s32 s_uiKey = 192;
	static bool s_exitGame = false;
	
#ifdef _WIN32
	static HANDLE s_hGameThread;
	static GraphicsDevice* s_gdev;

	DWORD WINAPI GameLoop(LPVOID lpParameter);
#endif
	void exitGame();


	bool init(void* win_param[], GraphicsDeviceID deviceID)
	{
		XLSettings* settings = Settings::get();
		Log::open("Logs/log.txt");
		Clock::init();
		Input::init(win_param[0]);

		#ifdef _WIN32
			GraphicsDevicePlatform* platform = new GraphicsDeviceGL_Win32();
		#endif

		s_gdev = GraphicsDevice::createDevice(deviceID, platform);
		if (!s_gdev)
		{
			return false;
		}

		s_gdev->setWindowData(1, win_param);
		if (!s_gdev->init(settings->windowWidth, settings->windowHeight, settings->gameWidth, settings->gameHeight))
		{
			GraphicsDevice::destroyDevice( s_gdev );
			return false;
		}

		GameUI::init(startGame, stopGame);
		s_launchGameID = Settings::get()->launchGameID;

		MemoryPool::init();
		TextSystem::init(s_gdev);
		Draw2D::init(s_gdev);
		UISystem::init(s_gdev, settings->windowWidth, settings->windowHeight);
		Settings::initGameData();
		Sound::Init();
		
		PluginManager::init();
		s_gdev->setVirtualViewport(false, 100, settings->windowHeight-250, 320, 200);

		return true;
	}
	
	void destroy()
	{
		UISystem::destroy();
		Draw2D::destroy();
		TextSystem::destroy();
		Clock::destroy();
		MemoryPool::destroy();
		PluginManager::destroy();
		Sound::Free();
		Log::close();
		
		GraphicsDevice::destroyDevice(s_gdev);
	}

	bool startGame(s32 gameID)
	{
		const GameInfo* info = Settings::getGameInfo( gameID );
		Services::xlDebugMessage("Game %d \"%s\" starting...", gameID, info->name);

		//start the game thread.
		Settings::setGameID(gameID);
		s_gameRunning  = gameID;

		Services::setTime( Clock::getTime_uS() );
		GameUI::enableCursor(false);
		s_hGameThread  = CreateThread(NULL, 0, GameLoop, (LPVOID)gameID, 0, NULL);

		return true;
	}

	void stopGame()
	{
		if (s_hGameThread == 0)
		{
			return;
		}

		TerminateThread(s_hGameThread, 0); // Dangerous source of errors! TO-DO: non-crappy solution. :D-
		CloseHandle(s_hGameThread);

		exitGame();
	}

	bool checkExitGame()
	{
		if (s_exitGame)
		{
			exitGame();
			
			//do we exit from the application if a game is closed?
			return (Settings::get()->flags&XL_FLAG_IMMEDIATE_EXIT)!=0;
		}

		return false;
	}

	void update()
	{
		XLSettings* settings = Settings::get();

		//launch a game immediately?
		if (s_launchGameID >= 0)
		{
			GameUI::setShowUI(false, s_gdev);
			startGame(s_launchGameID);
			s_launchGameID = -1;
			settings->flags |= XL_FLAG_IMMEDIATE_EXIT;
		}

		if (Input::keyPressed(s_uiKey))
		{
			GameUI::setShowUI( !GameUI::getShowUI(), s_gdev );
			GameUI::enableCursor( GameUI::getShowUI() );
		}

		s_gdev->clear();
			GameUI::update(s_gdev, settings->windowWidth, settings->windowHeight, s_gameRunning);
			if (s_gameRunning >= 0)
			{
				s_gdev->setShader( SHADER_QUAD_UI );
				s_gdev->drawVirtualScreen();
			}
			Draw2D::draw();
		s_gdev->present();

		Input::finish();
	}

	void exitGame()
	{
		const GameInfo* info = Settings::getGameInfo( s_gameRunning );
		Services::xlDebugMessage("Game \"%s\" stopped...", info->name);

		s_hGameThread =  0;
		s_gameRunning = -1;
		s_exitGame    = false;
		MemoryPool::reset();

		GameUI::enableCursor(true);
		GameUI::setShowUI(true, s_gdev);

		Sound::UnloadSounds();
	}
	
#ifdef _WIN32
	DWORD WINAPI GameLoop(LPVOID lpParameter)
	{
		const s32 gameID = s32( lpParameter );
		Services::xlDebugMessage("GameLoop for game %d started...", gameID);

		const XLSettings* settings = Settings::get();
		Services::setup(settings->gameWidth, settings->gameHeight, s_gdev);

		const GameInfo* info = Settings::getGameInfo(gameID);
		if (!info)
		{
			LOG( LOG_ERROR, "Game ID=%d is not valid and cannot be run.", gameID );
			return 1;
		}

		char* argv[]=
		{
			(char*)info->name,
			//"-level21",
		};

		//get the run function.
		char libPath[4096];
		sprintf(libPath, "Games/%s", info->lib);

		XL_RunFunc runGame = PluginManager::initGame(libPath);
		if (!runGame)
		{
			LOG( LOG_ERROR, "The game library \"%s\" is not valid, game \"%s\" cannot be run.", libPath, info->name );
			return 1;
		}
		runGame( arraysize(argv), argv, Services::get() );

		s_exitGame = true;
		return 1;
	}
#endif
};