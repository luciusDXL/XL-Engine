#pragma once
#include "types.h"
#include "xlServices.h"

class Driver3D_IPlatform;

namespace Services
{
	void setup(int gameWidth, int gameHeight, Driver3D_IPlatform* gdev);
	XLEngineServices* get();

	void setTime(u64 time);
	void xlDebugMessage(char* msg, ...);
};