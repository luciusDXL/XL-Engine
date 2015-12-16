#include <stdlib.h>
#include "../../log.h"
#include "graphicsDeviceGL.h"
#include "textureGL.h"
#include "shaderGL.h"

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>

GraphicsDeviceGL::GraphicsDeviceGL(GraphicsDevicePlatform* platform) : GraphicsDevice(platform)
{
	ShaderGL::init();
	m_bufferIndex = 0;
	m_writeFrame  = 0;
	m_renderFrame = 0;

	m_frameBuffer_32bpp[0] = NULL;
	m_frameBuffer_32bpp[1] = NULL;

	m_bufferMutex = new Mutex();
}

GraphicsDeviceGL::~GraphicsDeviceGL()
{
	for (size_t t=0; t<m_textures.size(); t++)
	{
		delete m_textures[t];
	}
	m_textures.clear();

	ShaderGL::destroy();
	delete m_bufferMutex;
}

void GraphicsDeviceGL::present()
{
	m_platform->present();
}

void GraphicsDeviceGL::queryExtensions()
{
	if (!(m_caps.flags&CAP_RENDER_TARGET) && glewGetExtension("GL_EXT_framebuffer_object"))
	{
		m_caps.flags |= CAP_RENDER_TARGET;
	}

	if (!(m_caps.flags&CAP_NON_POWER_2_TEX) && glewGetExtension("GL_ARB_texture_non_power_of_two"))
	{
		m_caps.flags |= CAP_NON_POWER_2_TEX;
	}

	glGetIntegerv(GL_MAX_TEXTURE_SIZE, (s32*)&m_caps.maxTextureSize2D);
}

void GraphicsDeviceGL::setWindowData(int nParam, void** param)
{
	m_platform->setWindowData(nParam, param, m_deviceID);
	glClearColor(0, 0, 0, 0);

	//default vsync to off for now.
	m_platform->enableVSync(false);
}

void GraphicsDeviceGL::clear()
{
	glClear( GL_COLOR_BUFFER_BIT );
}

void GraphicsDeviceGL::setBlendMode(BlendMode mode)
{
	const u32 srcBlend[] = { GL_SRC_ALPHA, GL_SRC_ALPHA, GL_ONE };	//OVER, ALPHA ADD, ADD
	const u32 dstBlend[] = { GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE };

	glBlendFunc(srcBlend[mode], dstBlend[mode]);
}

void GraphicsDeviceGL::enableBlending(bool enable)
{
	if (enable)
	{
		glEnable(GL_BLEND);
	}
	else
	{
		glDisable(GL_BLEND);
	}
}

void GraphicsDeviceGL::lockBuffer()
{
	m_bufferMutex->Lock();
}

void GraphicsDeviceGL::unlockBuffer()
{
	m_bufferMutex->Unlock();
}

void GraphicsDeviceGL::convertFrameBufferTo32bpp(u8* source, u32* pal)
{
	lockBuffer();
		const s32 curIndex = m_bufferIndex;
		u32 *dest = m_frameBuffer_32bpp[curIndex];

		u32 pixelCount = m_frameWidth*m_frameHeight;
		for (u32 p=0; p<pixelCount; p++)
		{
			*dest = pal[ *source ];
			dest++;
			source++;
		}
		m_writeBufferIndex = m_bufferIndex;
		m_bufferIndex = (m_bufferIndex+1)&1;
		m_writeFrame++;
	unlockBuffer();
}

TextureHandle GraphicsDeviceGL::createTextureRGBA(u32 width, u32 height, const u32* data, const SamplerState& initSamplerState, bool dynamic)
{
	TextureGL* texture = createTextureRGBA_Internal(width, height, data, initSamplerState, dynamic);
	return texture ? texture->getHandle() : INVALID_TEXTURE_HANDLE;
}

TextureGL* GraphicsDeviceGL::createTextureRGBA_Internal(u32 width, u32 height, const u32* data, const SamplerState& initSamplerState, bool dynamic/*=false*/)
{
	TextureGL* texture = new TextureGL(m_textures.size(), dynamic);
	if (!texture)
	{
		return NULL;
	}

	if (!texture->createRGBA(this, width, height, data, initSamplerState))
	{
		delete texture;
		return NULL;
	}

	m_textures.push_back( texture );
	return texture;
}

void GraphicsDeviceGL::setTexture(TextureHandle handle, int slot/*=0*/)
{
	if (handle == INVALID_TEXTURE_HANDLE)
	{
		TextureGL::clear(slot);
		return;
	}

	const u32 index = u32( handle );
	m_textures[index]->bind(slot);
}

void GraphicsDeviceGL::enableTexturing(bool enable)
{
	if (enable)
	{
		glEnable(GL_TEXTURE_2D);
	}
	else
	{
		glDisable(GL_TEXTURE_2D);
	}
}
