#include <stdlib.h>
#include "graphicsDevice.h"
#include "../log.h"

//specific implementations of this interface.
#include "Devices/graphicsDeviceGL_1_3.h"
#include "Devices/graphicsDeviceGL_2_0.h"
#include "Devices/graphicsDeviceGL_3_2.h"

GraphicsDevice* GraphicsDevice::createDevice(GraphicsDeviceID deviceID, GraphicsDevicePlatform* platform)
{
	GraphicsDevice* gdev = NULL;
	switch (deviceID)
	{
		case GDEV_OPENGL_1_3:
			gdev = new GraphicsDeviceGL_1_3( platform );
		break;
		case GDEV_OPENGL_2_0:
			gdev = new GraphicsDeviceGL_2_0( platform );
		break;
		case GDEV_OPENGL_3_2:
			gdev = new GraphicsDeviceGL_3_2( platform );
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
