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
#include "samplerState.h"
#include "capabilities.h"

class GraphicsDevice
{
    public:
        virtual void setWindowData(int nParam, void **param)=0;
		virtual bool init(int w, int h, int& vw, int& vh)=0;
		virtual void drawVirtualScreen()=0;
        virtual void present()=0;
		virtual void clear()=0;
		virtual void queryExtensions()=0;

		virtual void setShader(ShaderID shader)=0;

		virtual void convertFrameBufferTo32bpp(u8* source, u32 *pal)=0;
		virtual void setBlendMode(BlendMode mode)=0;
		virtual void enableBlending(bool enable)=0;
		virtual void enableTexturing(bool enable)=0;
		
		virtual TextureHandle createTextureRGBA(u32 width, u32 height, const u32* data, const SamplerState& initSamplerStat, bool dynamic=false)=0;
		virtual RenderTargetHandle createRenderTarget(u32 width, u32 height, const SamplerState& initSamplerState)=0;
		virtual void destroyTexture(TextureHandle texHandle)=0;
		virtual void destroyRenderTarget(RenderTargetHandle rtHandle)=0;

		virtual void bindRenderTarget(RenderTargetHandle handle)=0;
		virtual void unbindRenderTarget()=0;
		virtual TextureHandle getRenderTargetTexture(RenderTargetHandle handle)=0;

		virtual void clearShaderParamCache()=0;
		virtual void setShaderResource(TextureHandle handle, u32 nameHash, u32 slot=0)=0;
		virtual void setShaderParameter(void* data, u32 size, u32 nameHash)=0;
		virtual void drawQuad(const Quad& quad)=0;

		virtual void setVirtualViewport(bool reset, int x, int y, int w, int h)=0;

		//batching (not used by all devices)
		virtual void flush()=0;
		virtual u32 addQuad(const Quad& quad)=0;
		virtual void drawQuadBatch(u32 vertexOffset, u32 count)=0;

		//It is expected that an implementation will fill out the 'm_caps' variable.
		bool supportsFeature(CapabilityFlags feature) const { return (m_caps.flags&feature)!=0; }
		u32  getMaximumTextureSize() const { return m_caps.maxTextureSize2D; }
		GraphicsDeviceID getDeviceID() const { return m_deviceID; }

		void enableVsync(bool enable) { m_platform->enableVSync(enable); }

		static GraphicsDevice* createDevice(GraphicsDeviceID deviceID, GraphicsDevicePlatform* platform);
		static void destroyDevice(GraphicsDevice* device);

    protected:
		GraphicsDevice(GraphicsDevicePlatform* platform) 
		{
			m_platform = platform;
			m_deviceID = GDEV_INVALID;
			m_caps.flags = 0;
			m_caps.maxTextureSize2D = 0;
		}
        virtual ~GraphicsDevice() 
		{
			delete m_platform;
		}

		virtual void setTexture(TextureHandle handle, int slot=0)=0;

		GraphicsDevicePlatform* m_platform;
		GraphicsDeviceID        m_deviceID;
		Capabilities		    m_caps;	//see CapabilityFlags
    private:
};
