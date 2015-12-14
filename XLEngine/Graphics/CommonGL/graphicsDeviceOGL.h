/******************************************************************************************************
 Common OpenGL functionality between all OpenGL based Graphics Devices.
 Anything that can be pulled into common code will reduce the amount of code duplication and the
 potential for errors.
 ******************************************************************************************************/
#pragma once

#include "../graphicsDevice.h"
#include "../../Threads/mutex.h"

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

class GraphicsDeviceOGL : public GraphicsDevice
{
	friend class ShaderOGL;

    public:
        GraphicsDeviceOGL(GraphicsDevicePlatform* platform);
		virtual ~GraphicsDeviceOGL();

		//common functionality implemented by this class.
		virtual void setWindowData(int nParam, void** param);
        virtual void present();
		virtual void clear();
		virtual void setBlendMode(BlendMode mode);
		virtual void enableBlending(bool enable);
		virtual void convertFrameBufferTo32bpp(u8* source, u32* pal);

		//functionality that must be implemented by specific OpenGL based Graphics Devices.
		virtual bool supportsShaders()=0;
		virtual void setShader(ShaderID shader)=0;

        virtual bool init(int w, int h, int vw, int vh)=0;
		virtual void drawVirtualScreen()=0;
		
		virtual TextureHandle createTextureRGBA(int width, int height, u32* data)=0;
		virtual void setShaderResource(TextureHandle handle, u32 nameHash)=0;

		virtual void drawQuad(const Quad& quad)=0;
		virtual void drawFullscreenQuad()=0;

		virtual void setVirtualViewport(bool reset, int x, int y, int w, int h)=0;
    protected:
		virtual void setTexture(TextureHandle handle, int slot=0)=0;

		void lockBuffer();
		void unlockBuffer();

		s32  m_windowWidth;
		s32  m_windowHeight;
		s32  m_frameWidth;
		s32  m_frameHeight;
		u32  m_videoFrameBuffer;
		u32* m_frameBuffer_32bpp[2];

		s32 m_bufferIndex;
		s32 m_writeBufferIndex;
		s32 m_writeFrame;
		s32 m_renderFrame;

		s32 m_virtualViewport[4];
		s32 m_virtualViewportNoUI[4];
		s32 m_fullViewport[4];

		Mutex* m_bufferMutex;
    private:
};
