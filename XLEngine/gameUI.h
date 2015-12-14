#pragma once
#include "types.h"
#include "UI/uiSystem.h"
#include "UI/draw2D.h"

typedef bool (*StartGameFunc)(s32);
typedef void (*StopGameFunc)();

namespace GameUI
{
	void init(StartGameFunc startGame, StopGameFunc stopGame);
	void update(GraphicsDevice* gdev, int winWidth, int winHeight, s32& gameRunning);

	bool getShowUI();
	void setShowUI(bool show, GraphicsDevice* gdev);

	void enableCursor(bool enable);
};
