#include <stdlib.h>
#include "../../log.h"
#include "graphicsDeviceGL.h"
#include "textureGL.h"
#include "renderTargetGL.h"
#include "shaderGL.h"

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include <assert.h>

/////////////////////////////////////////
// Implementation
/////////////////////////////////////////
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
	for (size_t r=0; r<m_renderTargets.size(); r++)
	{
		delete m_renderTargets[r];
	}
	m_renderTargets.clear();

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
		LOG( LOG_MESSAGE, "glExtension enabled: \"GL_EXT_framebuffer_object\"" );
	}

	if (!(m_caps.flags&CAP_NON_POWER_2_TEX) && glewGetExtension("GL_ARB_texture_non_power_of_two"))
	{
		m_caps.flags |= CAP_NON_POWER_2_TEX;
		LOG( LOG_MESSAGE, "glExtension enabled: \"GL_ARB_texture_non_power_of_two\"" );
	}

	glGetIntegerv(GL_MAX_TEXTURE_SIZE, (s32*)&m_caps.maxTextureSize2D);
	LOG( LOG_MESSAGE, "Maximum 2D Texture size: %u", m_caps.maxTextureSize2D );
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
		const s32 curIndex   = m_bufferIndex;
		const u32 pixelCount = m_frameWidth*m_frameHeight;

		u32* dest = m_frameBuffer_32bpp[curIndex];
		const u32* end = &dest[ pixelCount ];

		for (; dest != end; dest++, source++)
		{
			*dest = pal[ *source ];
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

void GraphicsDeviceGL::destroyTexture(TextureHandle texHandle)
{
	u32 index = u32( texHandle );
	if (index >= m_textures.size())
	{
		return;
	}

	destroyTexture_Internal(index);
}

RenderTargetHandle GraphicsDeviceGL::createRenderTarget(u32 width, u32 height, const SamplerState& initSamplerState)
{
	u32 newID = m_renderTargets.size();
	if (m_freeRenderTargets.size())
	{
		newID = m_freeRenderTargets.back();
	}

	TextureGL* texture = createTexture_Internal();
	if (!texture)
	{
		return NULL;
	}

	RenderTargetGL* renderTarget = new RenderTargetGL(newID, texture);
	renderTarget->create(this, 0, width, height, initSamplerState);

	if (newID != m_renderTargets.size())
	{
		m_renderTargets[newID] = renderTarget;
		m_freeRenderTargets.pop_back();
	}
	else
	{
		m_renderTargets.push_back( renderTarget );
	}

	return renderTarget->getHandle();
}

void GraphicsDeviceGL::destroyRenderTarget(RenderTargetHandle rtHandle)
{
	u32 index = u32( rtHandle );
	if (index >= m_renderTargets.size())
	{
		return;
	}

	destroyRenderTarget_Internal(index);
}

void GraphicsDeviceGL::bindRenderTarget(RenderTargetHandle handle)
{
	if (handle == INVALID_TEXTURE_HANDLE)
	{
		return;
	}

	const u32 index = u32( handle );
	RenderTargetGL* renderTarget = m_renderTargets[ index ];
	assert( renderTarget );

	//clear out the current texture
	setTexture( INVALID_TEXTURE_HANDLE, 0 );
	setTexture( INVALID_TEXTURE_HANDLE, 1 );

	renderTarget->bind();
	glViewport( 0, 0, renderTarget->getWidth(), renderTarget->getHeight() );
}

void GraphicsDeviceGL::unbindRenderTarget()
{
	RenderTargetGL::unbind();
	glViewport( m_fullViewport[0], m_fullViewport[1], m_fullViewport[2], m_fullViewport[3] );
}

TextureHandle GraphicsDeviceGL::getRenderTargetTexture(RenderTargetHandle handle)
{
	if (handle == INVALID_TEXTURE_HANDLE)
	{
		return NULL;
	}

	const u32 index = u32( handle );
	RenderTargetGL* renderTarget = m_renderTargets[ index ];
	assert( renderTarget );

	TextureGL* texture = renderTarget->getTexture();
	assert(texture);

	return texture->getHandle();
}

void GraphicsDeviceGL::setTexture(TextureHandle handle, int slot/*=0*/)
{
	if (handle == INVALID_TEXTURE_HANDLE)
	{
		TextureGL::unbind(slot);
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

/////////////////////////////////////////
// Internal Functions
/////////////////////////////////////////
TextureGL* GraphicsDeviceGL::createTexture_Internal(bool dynamic/*=false*/)
{
	u32 newID = m_textures.size();
	if (m_freeTextures.size())
	{
		newID = m_freeTextures.back();
	}

	TextureGL* texture = new TextureGL(newID, dynamic);

	if (newID != m_textures.size())
	{
		m_textures[newID] = texture;
		m_freeTextures.pop_back();
	}
	else
	{
		m_textures.push_back( texture );
	}

	return texture;
}

TextureGL* GraphicsDeviceGL::createTextureRGBA_Internal(u32 width, u32 height, const u32* data, const SamplerState& initSamplerState, bool dynamic/*=false*/)
{
	TextureGL* texture = createTexture_Internal(dynamic);
	if (!texture)
	{
		return NULL;
	}

	if (!texture->createRGBA(this, width, height, data, initSamplerState))
	{
		destroyTexture( texture->getHandle() );
		return NULL;
	}

	return texture;
}

void GraphicsDeviceGL::destroyTexture_Internal(u32 index)
{
	delete m_textures[index];

	m_textures[index] = NULL;
	m_freeTextures.push_back(index);
}

void GraphicsDeviceGL::destroyRenderTarget_Internal(u32 index)
{
	TextureGL* texture = m_renderTargets[index]->getTexture();
	if (texture)
	{
		destroyTexture( texture->getHandle() );
	}

	delete m_renderTargets[index];

	m_renderTargets[index] = NULL;
	m_freeRenderTargets.push_back(index);
}
