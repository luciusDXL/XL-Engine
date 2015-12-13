/******************************************************************************************************
 Platform specific implementation for the Graphics Device.
 With APIs such as OpenGL, most of the functionality can be  shared between platforms, implementations 
 of this interface handle the few details that are platform dependent.

 The implementation will be "composited" with the  device implementation reducing the permutations 
 between feature sets, APIs and platforms.
 ******************************************************************************************************/
#pragma once
#include "../types.h"
#include "graphicsDeviceList.h"

class GraphicsDevicePlatform
{
    public:
        GraphicsDevicePlatform() {}
        virtual ~GraphicsDevicePlatform() {}

        virtual void setWindowData(int nParam, void **param, GraphicsDeviceID deviceID, bool exclFullscreen=false)=0;
		virtual bool init()=0;
        virtual void present()=0;

		virtual void enableVSync(bool enable)=0;
};
