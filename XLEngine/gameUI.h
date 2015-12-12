#pragma once
#include "types.h"
#include "UI/UISystem.h"
#include "UI/Draw2D.h"

typedef bool (*StartGameFunc)(s32);
typedef void (*StopGameFunc)();

namespace GameUI
{
	void init(StartGameFunc startGame, StopGameFunc stopGame);
	void update(Driver3D_IPlatform* gdev, int winWidth, int winHeight, s32& gameRunning);

	bool getShowUI();
	void setShowUI(bool show, Driver3D_IPlatform* gdev);

	void enableCursor(bool enable);
};