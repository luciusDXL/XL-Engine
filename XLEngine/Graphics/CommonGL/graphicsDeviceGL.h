/******************************************************************************************************
 Common OpenGL functionality between all OpenGL based Graphics Devices.
 Anything that can be pulled into common code will reduce the amount of code duplication and the
 potential for errors.
 ******************************************************************************************************/
#pragma once

#include "../graphicsDevice.h"
#include "../../Threads/mutex.h"
#include <vector>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))
class TextureGL;

class GraphicsDeviceGL : public GraphicsDevice
{
	friend class ShaderGL;

    public:
        GraphicsDeviceGL(GraphicsDevicePlatform* platform);
		virtual ~GraphicsDeviceGL();

		//common functionality implemented by this class.
		virtual void setWindowData(int nParam, void** param);
        virtual void present();
		virtual void clear();
		virtual void setBlendMode(BlendMode mode);
		virtual void enableBlending(bool enable);
		virtual void enableTexturing(bool enable);
		virtual void convertFrameBufferTo32bpp(u8* source, u32* pal);
		virtual TextureHandle createTextureRGBA(u32 width, u32 height, const u32* data, const SamplerState& initSamplerState, bool dynamic=false);
		virtual void queryExtensions();

		//functionality that must be implemented by specific OpenGL based Graphics Devices.
		virtual void setShader(ShaderID shader)=0;

        virtual bool init(int w, int h, int& vw, int& vh)=0;
		virtual void drawVirtualScreen()=0;
		
		virtual void setShaderResource(TextureHandle handle, u32 nameHash)=0;

		virtual void drawQuad(const Quad& quad)=0;
		virtual void drawFullscreenQuad(TextureGL* tex)=0;

		virtual void setVirtualViewport(bool reset, int x, int y, int w, int h)=0;
    protected:
		typedef std::vector<TextureGL*> TextureList;

		virtual void setTexture(TextureHandle handle, int slot=0);
		TextureGL* createTextureRGBA_Internal(u32 width, u32 height, const u32* data, const SamplerState& initSamplerState, bool dynamic=false);

		void lockBuffer();
		void unlockBuffer();

		s32  m_windowWidth;
		s32  m_windowHeight;
		s32  m_frameWidth;
		s32  m_frameHeight;
		u32* m_frameBuffer_32bpp[2];

		s32 m_bufferIndex;
		s32 m_writeBufferIndex;
		s32 m_writeFrame;
		s32 m_renderFrame;

		s32 m_virtualViewport[4];
		s32 m_virtualViewportNoUI[4];
		s32 m_fullViewport[4];

		TextureGL* m_videoFrameBuffer;
		TextureList m_textures;

		Mutex* m_bufferMutex;
    private:
};
