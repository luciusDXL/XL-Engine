#pragma once
#include "../types.h"
#include "graphicsDevice_Platform.h"
#include "graphicsDeviceList.h"

typedef u32 Color;

struct iPoint
{
	s32 x;
	s32 y;
};

struct fPoint
{
	f32 x;
	f32 y;
};

struct Quad
{
	iPoint p0;
	iPoint p1;
	fPoint uv0;
	fPoint uv1;
	Color color;
};

typedef u32 TextureHandle;
#define INVALID_TEXTURE_HANDLE 0xffffffff

enum BlendMode
{
	BLEND_OVER = 0,	//standard alpha blending
	BLEND_ALPHA_ADD,
	BLEND_ADD,
	BLEND_COUNT
};

enum ShaderID
{
	SHADER_QUAD_UI = 0,
	SHADER_COUNT
};

class GraphicsDevice
{
    public:
        GraphicsDevice(GraphicsDevicePlatform* platform) 
		{
			m_platform = platform;
			m_deviceID = GDEV_INVALID;
		}
        virtual ~GraphicsDevice() 
		{
			delete m_platform;
		}

        virtual void setWindowData(int nParam, void **param)=0;
		virtual bool init(int w, int h, int vw, int vh)=0;
		virtual void drawVirtualScreen()=0;
        virtual void present()=0;
		virtual void clear()=0;

		virtual bool supportsShaders()=0;
		virtual void setShader(ShaderID shader)=0;

		virtual void convertFrameBufferTo32bpp(u8 *pSource, u32 *pal)=0;
		virtual void setBlendMode(BlendMode mode)=0;
		virtual void enableBlending(bool enable)=0;
		virtual TextureHandle createTextureRGBA(int width, int height, u32* data)=0;
		virtual void setShaderResource(TextureHandle handle, u32 nameHash)=0;
		virtual void drawQuad(const Quad& quad)=0;

		virtual void setVirtualViewport(bool reset, int x, int y, int w, int h)=0;

		static GraphicsDevice* createDevice(GraphicsDeviceID deviceID, GraphicsDevicePlatform* platform);
		static void destroyDevice(GraphicsDevice* device);

    protected:
		virtual void setTexture(TextureHandle handle, int slot=0)=0;

		GraphicsDevicePlatform* m_platform;
		GraphicsDeviceID        m_deviceID;
    private:
};
