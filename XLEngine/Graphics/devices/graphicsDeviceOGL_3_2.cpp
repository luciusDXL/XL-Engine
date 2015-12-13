/////////////////////////
// This is only a stub
// TO-DO: implement
/////////////////////////
#include <stdlib.h>
#include "graphicsDeviceOGL_3_2.h"
#include "../../log.h"

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>


GraphicsDeviceOGL_3_2::GraphicsDeviceOGL_3_2(GraphicsDevicePlatform* platform) : GraphicsDeviceOGL(platform)
{
	m_deviceID = GDEV_OPENGL_3_2;
	m_textureEnabled = true;
}

GraphicsDeviceOGL_3_2::~GraphicsDeviceOGL_3_2()
{
}

void GraphicsDeviceOGL_3_2::drawVirtualScreen()
{
}

void GraphicsDeviceOGL_3_2::setVirtualViewport(bool reset, int x, int y, int w, int h)
{
}

bool GraphicsDeviceOGL_3_2::init(int w, int h, int vw, int vh)
{
	return false;
}

bool GraphicsDeviceOGL_3_2::supportsShaders()
{
	return true;
}

void GraphicsDeviceOGL_3_2::setShader(ShaderID shader)
{
}

TextureHandle GraphicsDeviceOGL_3_2::createTextureRGBA(int width, int height, unsigned int* data)
{
	return INVALID_TEXTURE_HANDLE;
}

void GraphicsDeviceOGL_3_2::setShaderResource(TextureHandle handle, u32 nameHash)
{
}

void GraphicsDeviceOGL_3_2::setTexture(TextureHandle handle, int slot/*=0*/)
{
}

void GraphicsDeviceOGL_3_2::drawQuad(const Quad& quad)
{
}

void GraphicsDeviceOGL_3_2::drawFullscreenQuad()
{
}