/////////////////////////
// This is only a stub
// TO-DO: implement
/////////////////////////
#include <stdlib.h>
#include "graphicsDeviceOGL_2_0.h"
#include "../../log.h"

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>


GraphicsDeviceOGL_2_0::GraphicsDeviceOGL_2_0(GraphicsDevicePlatform* platform) : GraphicsDeviceOGL(platform)
{
	m_deviceID = GDEV_OPENGL_2_0;
	m_textureEnabled = true;
}

GraphicsDeviceOGL_2_0::~GraphicsDeviceOGL_2_0()
{
}

void GraphicsDeviceOGL_2_0::drawVirtualScreen()
{
}

void GraphicsDeviceOGL_2_0::setVirtualViewport(bool reset, int x, int y, int w, int h)
{
}

bool GraphicsDeviceOGL_2_0::init(int w, int h, int vw, int vh)
{
	return false;
}

TextureHandle GraphicsDeviceOGL_2_0::createTextureRGBA(int width, int height, unsigned int* data)
{
	return INVALID_TEXTURE_HANDLE;
}

void GraphicsDeviceOGL_2_0::setTexture(TextureHandle handle, int slot/*=0*/)
{
}

void GraphicsDeviceOGL_2_0::drawQuad(const Quad& quad)
{
}
