#include "gameLoop.h"
#include "settings.h"
#include "services.h"
#include "memoryPool.h"
#include "gameUI.h"
#include "input.h"
#include "Sound/sound.h"
#include "Sound/midi.h"
#include "Math/crc32.h"
#include "Math/math.h"
#include "PluginFramework/PluginManager.h"
#include "clock.h"
#include "osUtil.h"
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

	static RenderTargetHandle s_screenRenderTarget = INVALID_TEXTURE_HANDLE;
	static RenderTargetHandle s_blurTargets[6];
	static bool s_applyUIGlow = false;

	static f64 s_desiredFrameLimit = 0.0;
	static f64 s_avePresentTime    = 1.0;		//1.0ms
	static f64 s_presentAdapt      = 0.1;		//how fast the present time adjusts
	
#ifdef _WIN32
	static HANDLE s_hGameThread;
	static GraphicsDevice* s_gdev;

	DWORD WINAPI GameLoop(LPVOID lpParameter);
#endif
	void exitGame();


	bool init(void* win_param[], GraphicsDeviceID deviceID)
	{
		XLSettings* settings = Settings::get();
		
		Clock::init();
		Input::init(win_param[0]);

		#ifdef _WIN32
			GraphicsDevicePlatform* platform = new GraphicsDeviceGL_Win32();
		#endif
		//if no deviceID is specified, autodetect
		if (deviceID == GDEV_INVALID)
		{
			deviceID = platform->autodetect(1, win_param);
			//a proper device was not found... abort.
			if (deviceID == GDEV_INVALID)
			{
				delete platform;
				Log::close();
				Clock::destroy();
				return false;
			}
		}

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

		s_gdev->enableVsync( (settings->flags&XL_FLAG_VSYNC)!=0 );
		//do not apply the framerate limiter and vsync at the same time.
		if (settings->flags&XL_FLAG_VSYNC)
		{
			settings->frameLimit = 0;
		}

		GameUI::init(startGame, stopGame);
		s_launchGameID = Settings::get()->launchGameID;

		MemoryPool::init();
		TextSystem::init(s_gdev);
		Draw2D::init(s_gdev);
		UISystem::init(s_gdev, settings->windowWidth, settings->windowHeight);
		Settings::initGameData();
		Sound::init();
		Midi::init( settings->midiformat, settings->patchDataLoc );
		
		PluginManager::init();
		s_gdev->setVirtualViewport(false, 100, settings->windowHeight-250, 320, 200);

		s_desiredFrameLimit = settings->frameLimit ? 1.0 / f64(settings->frameLimit) : 0.0;
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
		Sound::free();
		Midi::free();
		GraphicsDevice::destroyDevice(s_gdev);
		Log::close();
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

	void setRenderTarget(XLSettings* settings)
	{
		if (!s_gdev->supportsFeature(CAP_RENDER_TARGET) || !GameUI::getShowUI() || !(settings->flags&XL_FLAG_UI_GLOW))
		{
			s_applyUIGlow = false;
			return;
		}
		s_applyUIGlow = true;

		if (s_screenRenderTarget == INVALID_TEXTURE_HANDLE)
		{
			const SamplerState samplerState=
			{
				WM_CLAMP, WM_CLAMP, WM_CLAMP,							//clamp on every axis
				TEXFILTER_LINEAR, TEXFILTER_LINEAR, TEXFILTER_POINT,	//filtering
				false													//no mipmapping
			};
			s_screenRenderTarget = s_gdev->createRenderTarget( settings->windowWidth, settings->windowHeight, samplerState );

			//now create smaller targets for blur...
			s_blurTargets[0] = s_gdev->createRenderTarget( settings->windowWidth>>1, settings->windowHeight>>1, samplerState );
			s_blurTargets[1] = s_gdev->createRenderTarget( settings->windowWidth>>1, settings->windowHeight>>1, samplerState );
		}
		s_gdev->bindRenderTarget( s_screenRenderTarget );
	}

	void applyUIglow(XLSettings* settings)
	{
		if (!s_applyUIGlow)
		{
			return;
		}

		s_gdev->unbindRenderTarget();
		TextureHandle screen = s_gdev->getRenderTargetTexture( s_screenRenderTarget );

		Quad rect=
		{
			{ 0, 0 },
			{ settings->windowWidth, settings->windowHeight },
			{ 0.0f, 1.0f },
			{ 1.0f, 0.0f },
			0xffffffff,
		};

		s_gdev->setShader( SHADER_BLUR );
		const char* paramName = "u_uvStep";
		u32 paramHash = CRC32::get( (u8*)paramName, strlen(paramName) );

		const char* baseTexName = "baseTex";
		u32 baseTex = CRC32::get( (u8*)baseTexName, strlen(baseTexName) );

		//fill vertex buffer.
		const u32 blurPassCount = 4;
		for (u32 p=0; p<blurPassCount; p++)
		{
			//horizontal + vertical
			s_gdev->addQuad(rect);
			s_gdev->addQuad(rect);
		}
		//final blit
		s_gdev->addQuad(rect);
		s_gdev->flush();

		for (u32 p=0; p<blurPassCount; p++)
		{
			//horizontal
			s_gdev->bindRenderTarget( s_blurTargets[0] );
			{
				f32 w = f32( p==0 ? (settings->windowWidth) : (settings->windowWidth>>1) );
				f32 data[] = { 1.0f / w, 0.0f };
				s_gdev->setShaderResource( p==0 ? screen : s_gdev->getRenderTargetTexture(s_blurTargets[1]), baseTex );
				s_gdev->setShaderParameter(data, sizeof(f32)*2, paramHash);
				s_gdev->drawQuadBatch(p*8, 1);
			}
			//vertical
			s_gdev->bindRenderTarget( s_blurTargets[1] );
			{
				f32 h = f32( settings->windowHeight>>1 );
				f32 data[] = { 0.0f, -1.0f / h };
				s_gdev->setShaderResource( s_gdev->getRenderTargetTexture(s_blurTargets[0]), baseTex );
				s_gdev->setShaderParameter(data, sizeof(f32)*2, paramHash);
				s_gdev->drawQuadBatch(p*8+4, 1);
			}
		}
		s_gdev->unbindRenderTarget();

		s_gdev->setShader( SHADER_GLOW_UI );

		const char* yScaleName = "u_yScale";
		u32 yScaleHash = CRC32::get( (u8*)yScaleName, strlen(yScaleName) );
		f32 timeFract = f32( 45.0 * (f64)Clock::getTime_uS_flt() * Math::twoPI / 1000000.0 );
		timeFract = sinf(timeFract)*0.1f + 0.9f;

		f32 yScale[] = { f32( settings->windowHeight ) * 0.5f, timeFract };
		s_gdev->setShaderParameter(yScale, sizeof(f32)*2, yScaleHash);

		const char* glowTexName = "glowTex";
		s_gdev->setShaderResource( screen, baseTex, 0 );
		s_gdev->setShaderResource( s_gdev->getRenderTargetTexture(s_blurTargets[1]), CRC32::get((u8*)glowTexName, strlen(glowTexName)), 1 );
		s_gdev->drawQuadBatch(blurPassCount*8, 1);
	}

	void update()
	{
		XLSettings* settings = Settings::get();
		Sound::update();
		
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

		setRenderTarget(settings);
		s_gdev->clear();
			GameUI::update(s_gdev, settings->windowWidth, settings->windowHeight, s_gameRunning);
			if (s_gameRunning >= 0)
			{
				if (settings->flags & XL_FLAG_COLOR_CORRECT)
				{
					s_gdev->setShader( SHADER_QUAD_COLOR_CORRECT );

					const char* paramName = "u_colorCorrect";
					u32 paramHash = CRC32::get( (u8*)paramName, strlen(paramName) );
					s_gdev->setShaderParameter(settings->colorCorrect, sizeof(f32)*4, paramHash);

					const char* viewSize = "u_viewSize";
					f32 view[] = { (f32)settings->windowWidth, (f32)settings->windowHeight };
					paramHash = CRC32::get( (u8*)viewSize, strlen(viewSize) );
					s_gdev->setShaderParameter(view, sizeof(f32)*2, paramHash);
				}
				else
				{
					s_gdev->setShader( SHADER_QUAD_UI );
				}

				s_gdev->drawVirtualScreen();
			}
			Draw2D::draw();
		applyUIglow(settings);

		//Wait until the elapsed frame time + estimated 'present' time = desired frame time.
		if (s_desiredFrameLimit)
		{
			f64 accum = Clock::getDeltaTime_f64();
			f64 presentTime = s_avePresentTime;
			while (accum < s_desiredFrameLimit - presentTime)
			{
				OS::sleep(0);	//give the system time to work.
				accum = Clock::getDeltaTime_f64();
			};
		}
		
		Clock::startTimer();
		s_gdev->present();
		Input::finish();

		f64 presentDt = Clock::getDeltaTime_f64();
		s_avePresentTime = s_avePresentTime*(1.0 - s_presentAdapt) + presentDt*s_presentAdapt;

		//start the next frame timing...
		Clock::startTimer();
	}

	void exitGame()
	{
		const GameInfo* info = Settings::getGameInfo( s_gameRunning );
		Services::xlDebugMessage("Game \"%s\" stopped...", info->name);

		s_hGameThread =  0;
		s_gameRunning = -1;
		s_exitGame    = false;
		MemoryPool::reset();
		Services::reset();

		GameUI::enableCursor(true);
		GameUI::setShowUI(true, s_gdev);

		Sound::reset();
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