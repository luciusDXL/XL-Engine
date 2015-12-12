#include <stdlib.h>
#include "graphicsDevice.h"
#include "../log.h"

//specific implementations of this interface.
#include "devices/graphicsDeviceOGL_1_3.h"
#include "devices/graphicsDeviceOGL_2_0.h"
#include "devices/graphicsDeviceOGL_3_2.h"

GraphicsDevice* GraphicsDevice::createDevice(GraphicsDeviceID deviceID, GraphicsDevicePlatform* platform)
{
	GraphicsDevice* gdev = NULL;
	switch (deviceID)
	{
		case GDEV_OPENGL_1_3:
			gdev = new GraphicsDeviceOGL_1_3( platform );
		break;
		case GDEV_OPENGL_2_0:
			gdev = new GraphicsDeviceOGL_2_0( platform );
		break;
		case GDEV_OPENGL_3_2:
			gdev = new GraphicsDeviceOGL_3_2( platform );
		break;
	}

	if (gdev == NULL)
	{
		LOG( LOG_ERROR, "Invalid device ID %u!", deviceID);
	}

	return gdev;
}

void GraphicsDevice::destroyDevice(GraphicsDevice* device)
{
	delete device;
}
