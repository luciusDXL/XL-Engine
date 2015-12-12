#pragma once
#include "types.h"
#include "xlServices.h"

class GraphicsDevice;

namespace Services
{
	void setup(int gameWidth, int gameHeight, GraphicsDevice* gdev);
	XLEngineServices* get();

	void setTime(u64 time);
	void xlDebugMessage(char* msg, ...);
};