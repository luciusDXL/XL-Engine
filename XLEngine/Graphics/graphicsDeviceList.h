#pragma once

enum GraphicsDeviceID
{
	GDEV_OPENGL_1_3 = 0,
	GDEV_OPENGL_2_0,
	GDEV_OPENGL_3_2,
	GDEV_COUNT,
	GDEV_INVALID = GDEV_COUNT
};

extern const char* c_graphicsDeviceName[GDEV_COUNT];
