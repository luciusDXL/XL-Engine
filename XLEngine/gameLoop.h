#pragma once
#include "types.h"
#include "Driver3D_IPlatform.h"

namespace GameLoop
{
	bool init(void* win_param[]);
	void destroy();

	bool startGame(s32 gameID);
	void stopGame();

	bool checkExitGame();
	void update();
};