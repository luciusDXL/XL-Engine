#pragma once
#include "types.h"
#include "Graphics/graphicsDeviceList.h"
#include "Graphics/graphicsDevice.h"

namespace GameLoop
{
	bool init(void* win_param[], GraphicsDeviceID deviceID);
	void destroy();

	bool startGame(s32 gameID);
	void stopGame();

	bool checkExitGame();
	void update();
};