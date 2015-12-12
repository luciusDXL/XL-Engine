#pragma once

#include "../graphicsDevice.h"

class GraphicsDeviceOGL : public GraphicsDevice
{
    public:
        GraphicsDeviceOGL(GraphicsDevicePlatform* platform);
		virtual ~GraphicsDeviceOGL();

		//shared
		virtual void setWindowData(int nParam, void **param);
        virtual void present();
		virtual void clear();
		virtual void setBlendMode(BlendMode mode);
		virtual void enableBlending(bool enable);
		virtual void convertFrameBufferTo32bpp(u8 *pSource, u32 *pal);

		//left to specific implementations.
        virtual bool init(int w, int h, int vw, int vh)=0;
		virtual void drawVirtualScreen()=0;
		
		virtual TextureHandle createTextureRGBA(int width, int height, u32* data)=0;
		virtual void setTexture(TextureHandle handle, int slot=0)=0;
		virtual void drawQuad(const Quad& quad)=0;

		virtual void setVirtualViewport(bool reset, int x, int y, int w, int h)=0;
    protected:
		s32  m_nWindowWidth;
		s32  m_nWindowHeight;
		s32  m_FrameWidth;
		s32  m_FrameHeight;
		u32  m_VideoFrameBuffer;
		u32* m_pFrameBuffer_32bpp;

		s32  m_virtualViewport[4];
		s32  m_virtualViewportNoUI[4];
		s32  m_fullViewport[4];
    private:
};
