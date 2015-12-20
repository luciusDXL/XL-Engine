#include <stdlib.h>
#include "renderTargetGL.h"
#include "../../log.h"

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>

RenderTargetGL::RenderTargetGL(RenderTargetHandle handle, TextureGL* texture)
{
	m_handle = handle;
	m_texture = texture;

	m_glID = 0;
	m_glDepthID = 0;
}

RenderTargetGL::~RenderTargetGL()
{
	//make sure the render target is not current bound.
	unbind();
	//the depth buffer
	if (m_glDepthID)
	{
		glDeleteRenderbuffersEXT(1, &m_glDepthID);
	}
	//and finally the frame buffer object itself.
	glDeleteFramebuffersEXT(1, &m_glID);
}

//the initial version will not support depth buffers.
bool RenderTargetGL::create(const GraphicsDevice* gdev, u32 createFlags, u32 width, u32 height, const SamplerState& initSamplerState)
{
	m_width  = width;
	m_height = height;

	//create the color texture
	m_texture->createRGBA(gdev, width, height, NULL, initSamplerState);

	//create the frame buffer object
	glGenFramebuffersEXT(1, &m_glID);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_glID);

	//attach 2D texture to this FBO
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, m_texture->m_glID, 0);

	//create the depth buffer if requested
	if (createFlags & RT_CREATE_DEPTH)
	{
		//create the depth buffer (render buffer, not texture)
		glGenRenderbuffersEXT(1, &m_glDepthID);
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, m_glDepthID);
		glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, width, height);
		//attach depth buffer to FBO
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, m_glDepthID);
	}

	//Does the GPU support current FBO configuration?
	GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT)
	{
		LOG( LOG_ERROR, "RenderTargetGL create failed! Status = %x", status );
		return false;
	}

	return true;
}

void RenderTargetGL::bind()
{
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_glID);
}

void RenderTargetGL::unbind()
{
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

TextureGL* RenderTargetGL::getTexture()
{
	return m_texture;
}
