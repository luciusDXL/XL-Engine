#include <stdlib.h>
#include "textureGL.h"
#include "../../log.h"
#include "../../Math/math.h"

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include <assert.h>

TextureGL::TextureGL(TextureHandle handle, bool dynamic)
{
	m_dynamic = dynamic;
	m_glID = 0;
	m_width  = 0;
	m_height = 0;
	m_handle = handle;
	m_updateBuffer = NULL;
}

TextureGL::~TextureGL()
{
	if (m_glID)
	{
		glDeleteTextures(1, &m_glID);
	}
	free(m_updateBuffer);
}

void TextureGL::bind(u32 slot)
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, GLuint(m_glID));
}

void TextureGL::clear(u32 slot)
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, 0);
}

bool TextureGL::createRGBA(const GraphicsDevice* gdev, u32 width, u32 height, const u32* data, const SamplerState& initSamplerState)
{
	glGenTextures(1, &m_glID);
	glBindTexture(GL_TEXTURE_2D, m_glID);

	setGLSamplerState(initSamplerState);

	//if this device does not support non-power of 2 textures, we need to round up to the next power of 2.
	u32* finalData = (u32*)data;
	if ( !gdev->supportsFeature(CAP_NON_POWER_2_TEX) )
	{
		u32 newWidth  = Math::nextPow2(width);
		u32 newHeight = Math::nextPow2(height);

		if (data && (newWidth != width || newHeight != height))
		{
			allocateUpdateBuffer(newWidth, newHeight);
			u32* buffer = m_updateBuffer;
			for (u32 y=0; y<height; y++)
			{
				memcpy(buffer, data, width*4);
				buffer += newWidth;
				data   += width;
			}
			finalData = m_updateBuffer;
		}

		width  = newWidth;
		height = newHeight;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, finalData);
	glBindTexture(GL_TEXTURE_2D, 0);

	m_width  = width;
	m_height = height;

	return true;
}

void TextureGL::setSamplerState(const SamplerState& state)
{
	glBindTexture(GL_TEXTURE_2D, m_glID);
	setGLSamplerState(state);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void TextureGL::update(const u32* data)
{
	glBindTexture(GL_TEXTURE_2D, m_glID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, data);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void TextureGL::update(u32 srcWidth, u32 srcHeight, const u32* data)
{
	//if the source size is the same as the texture size, just upload as is.
	if (srcWidth == m_width && srcHeight == m_height)
	{
		update(data);
		return;
	}

	u32 dstStride = m_width;	//assume RGBA for now.
	u32 srcStride = min(srcWidth, m_width) * 4;

	allocateUpdateBuffer(m_width, m_height);
	
	u32* buffer = m_updateBuffer;
	for (u32 y=0; y<min(srcHeight, m_height); y++)
	{
		memcpy(buffer, data, srcStride);
		buffer += dstStride;
		data   += srcWidth;
	}
	update(m_updateBuffer);
}

void TextureGL::setGLSamplerState(const SamplerState& state)
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

void TextureGL::allocateUpdateBuffer(u32 width, u32 height)
{
	if (!m_updateBuffer)
	{
		m_updateBuffer = (u32*)malloc( width * height * 4 );
		memset(m_updateBuffer, 0, width * height * 4 );

		assert(m_updateBuffer);
	}
}
