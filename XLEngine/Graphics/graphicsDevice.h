/************************************************************************************************************
 Graphics Device interface.

 There are 2 components to the device:
	- The platform (graphicsDevice_Platform) which handles OS and platform specific implementations.
	- The API "level" such as OpenGL 1.3, OpenGL 2.0 or DirectX 9, DirectX 11 if DirectX is ever supported.
 By seperating the two and ensuring the platform implementation is as small as possible, the amount
	of per-platform code is reduced, the majority of the code goes into supporting the various
	features and improvements with each API "level" which is where it should go.

 It create a device, pass in the deviceID and platform implementation to the static createDevice()
 Similarly destroy the device using the static destroy()
 Graphics Devices can be changed at runtime, allowing the user to pick the API level or even API itself
	depending on driver compatibility (or just to test).
 **************************************************************************************************************/
#pragma once
#include "graphicsDevice_Platform.h"
#include "graphicsDeviceList.h"
#include "graphicsTypes.h"

class GraphicsDevice
{
    public:
        virtual void setWindowData(int nParam, void **param)=0;
		virtual bool init(int w, int h, int vw, int vh)=0;
		virtual void drawVirtualScreen()=0;
        virtual void present()=0;
		virtual void clear()=0;

		virtual bool supportsShaders()=0;
		virtual void setShader(ShaderID shader)=0;

		virtual void convertFrameBufferTo32bpp(u8* source, u32 *pal)=0;
		virtual void setBlendMode(BlendMode mode)=0;
		virtual void enableBlending(bool enable)=0;
		virtual TextureHandle createTextureRGBA(int width, int height, u32* data)=0;
		virtual void setShaderResource(TextureHandle handle, u32 nameHash)=0;
		virtual void drawQuad(const Quad& quad)=0;

		virtual void setVirtualViewport(bool reset, int x, int y, int w, int h)=0;

		static GraphicsDevice* createDevice(GraphicsDeviceID deviceID, GraphicsDevicePlatform* platform);
		static void destroyDevice(GraphicsDevice* device);

    protected:
		GraphicsDevice(GraphicsDevicePlatform* platform) 
		{
			m_platform = platform;
			m_deviceID = GDEV_INVALID;
		}
        virtual ~GraphicsDevice() 
		{
			delete m_platform;
		}

		virtual void setTexture(TextureHandle handle, int slot=0)=0;

		GraphicsDevicePlatform* m_platform;
		GraphicsDeviceID        m_deviceID;
    private:
};
