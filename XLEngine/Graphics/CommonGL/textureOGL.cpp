#include <stdlib.h>
#include "textureOGL.h"
#include "../../log.h"

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>

TextureOGL::TextureOGL(TextureHandle handle, bool dynamic)
{
	m_dynamic = dynamic;
	m_glID = 0;
	m_width  = 0;
	m_height = 0;
	m_handle = handle;
}

TextureOGL::~TextureOGL()
{
	if (m_glID)
	{
		glDeleteTextures(1, &m_glID);
	}
}

void TextureOGL::bind(u32 slot)
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, GLuint(m_glID));
}

void TextureOGL::clear(u32 slot)
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, 0);
}

bool TextureOGL::createRGBA(u32 width, u32 height, const u32* data, const SamplerState& initSamplerState)
{
	glGenTextures(1, &m_glID);
	glBindTexture(GL_TEXTURE_2D, m_glID);

	setGLSamplerState(initSamplerState);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, data);
	glBindTexture(GL_TEXTURE_2D, 0);

	m_width  = width;
	m_height = height;

	return true;
}

void TextureOGL::setSamplerState(const SamplerState& state)
{
	glBindTexture(GL_TEXTURE_2D, m_glID);
	setGLSamplerState(state);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void TextureOGL::update(const u32* data)
{
	glBindTexture(GL_TEXTURE_2D, m_glID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, data);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void TextureOGL::setGLSamplerState(const SamplerState& state)
{
	//ignore the mip filter for now and anisotropic filtering for now.

	const GLenum glWrapMode[]  = { GL_REPEAT, GL_CLAMP, GL_MIRRORED_REPEAT };
	const GLenum glMagFilter[] = { GL_NEAREST, GL_LINEAR, GL_LINEAR };
	const GLenum glMinFilter[] = { GL_NEAREST, GL_LINEAR, GL_LINEAR };

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, glWrapMode[state.wrapU]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, glWrapMode[state.wrapV]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, glWrapMode[state.wrapW]);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, glMagFilter[state.minFilter]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, glMagFilter[state.magFilter]);
}