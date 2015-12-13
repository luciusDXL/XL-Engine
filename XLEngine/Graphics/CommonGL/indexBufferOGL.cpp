#include <stdlib.h>
#include <memory.h>
#include "indexBufferOGL.h"
#include "../../log.h"

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include <algorithm>

IndexBufferOGL::IndexBufferOGL()
{
	glGenBuffers(1, &m_glID);

	m_size   = 0;
	m_stride = 0;
	m_count  = 0;
}

IndexBufferOGL::~IndexBufferOGL()
{
	glDeleteBuffers(1, &m_glID);
}

bool IndexBufferOGL::allocate(u32 stride, u32 count, void* data/*=NULL*/)
{
	m_size   = stride*count;
	m_stride = stride;
	m_count  = count;

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_glID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_size, data, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return true;
}

void IndexBufferOGL::update(u32 size, void* data)
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_glID);

	//cause the "discard" buffer behavior, which is done by first calling glBufferData() with NULL data.
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_size, NULL, GL_STATIC_DRAW);

	//now lock and update the buffer.
	void* mem = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
	if ( mem )
	{
		memcpy(mem, data, std::min(size, m_size));
	}
	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void IndexBufferOGL::bind()
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_glID);
}

void IndexBufferOGL::clear()
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
