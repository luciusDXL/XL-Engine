#include <stdlib.h>
#include "../../log.h"
#include "graphicsDeviceOGL.h"
#include "shaderOGL.h"

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>

GraphicsDeviceOGL::GraphicsDeviceOGL(GraphicsDevicePlatform* platform) : GraphicsDevice(platform)
{
	ShaderOGL::init();
	m_bufferIndex = 0;
	m_writeFrame  = 0;
	m_renderFrame = 0;

	m_frameBuffer_32bpp[0] = NULL;
	m_frameBuffer_32bpp[1] = NULL;

	m_bufferMutex = new Mutex();
}

GraphicsDeviceOGL::~GraphicsDeviceOGL()
{
	ShaderOGL::destroy();
	delete m_bufferMutex;
}

void GraphicsDeviceOGL::present()
{
	m_platform->present();
}

void GraphicsDeviceOGL::setWindowData(int nParam, void** param)
{
	m_platform->setWindowData(nParam, param, m_deviceID);
	glClearColor(0, 0, 0, 0);

	//default vsync to off for now.
	m_platform->enableVSync(false);
}

void GraphicsDeviceOGL::clear()
{
	glClear( GL_COLOR_BUFFER_BIT );
}

void GraphicsDeviceOGL::setBlendMode(BlendMode mode)
{
	const u32 srcBlend[] = { GL_SRC_ALPHA, GL_SRC_ALPHA, GL_ONE };	//OVER, ALPHA ADD, ADD
	const u32 dstBlend[] = { GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE };

	glBlendFunc(srcBlend[mode], dstBlend[mode]);
}

void GraphicsDeviceOGL::enableBlending(bool enable)
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

void GraphicsDeviceOGL::lockBuffer()
{
	m_bufferMutex->Lock();
}

void GraphicsDeviceOGL::unlockBuffer()
{
	m_bufferMutex->Unlock();
}

void GraphicsDeviceOGL::convertFrameBufferTo32bpp(u8* source, u32* pal)
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
