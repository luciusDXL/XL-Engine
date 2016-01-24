/////////////////////////
// This is only a stub
// TO-DO: implement
/////////////////////////
#include <stdlib.h>
#include "graphicsDeviceGL_3_2.h"
#include "../../log.h"
#include "../graphicsGL_Inc.h"


GraphicsDeviceGL_3_2::GraphicsDeviceGL_3_2(GraphicsDevicePlatform* platform) : GraphicsDeviceGL(platform)
{
	m_deviceID = GDEV_OPENGL_3_2;
	m_caps.flags |= CAP_SUPPORT_SHADERS;
	m_caps.flags |= CAP_RENDER_TARGET;
	m_caps.flags |= CAP_NON_POWER_2_TEX;
}

GraphicsDeviceGL_3_2::~GraphicsDeviceGL_3_2()
{
}

void GraphicsDeviceGL_3_2::drawVirtualScreen()
{
}

void GraphicsDeviceGL_3_2::setVirtualViewport(bool reset, int x, int y, int w, int h)
{
}

bool GraphicsDeviceGL_3_2::init(int w, int h, int& vw, int& vh)
{
	return false;
}

void GraphicsDeviceGL_3_2::setShader(ShaderID shader)
{
}

void GraphicsDeviceGL_3_2::clearShaderParamCache()
{
}

void GraphicsDeviceGL_3_2::setShaderResource(TextureHandle handle, u32 nameHash, u32 slot)
{
}

void GraphicsDeviceGL_3_2::setShaderParameter(void* data, u32 size, u32 nameHash)
{
}

void GraphicsDeviceGL_3_2::drawQuad(const Quad& quad)
{
}

void GraphicsDeviceGL_3_2::drawFullscreenQuad(TextureGL* tex)
{
}
