#include "streamingVertexBuffer.h"
#include "vertexBufferGL.h"
#include "../../log.h"

#include <stdlib.h>
#include <memory.h>

StreamingVertexBuffer::StreamingVertexBuffer(const VertexElement* vertexDecl, u32 declCount, u32 stride, u32 maxVertexCount)
{
	m_bufferSize = stride*maxVertexCount;
	m_buffer = (u8*)malloc( m_bufferSize );
	m_bufferIndex  = 0;
	m_bufferOffset = 0;
	m_stride = stride;

	m_lastError = STREAMING_VERTEX_OK;

	for (s32 b=0; b<STREAMING_BUFFER_COUNT; b++)
	{
		m_glBuffers[b] = new VertexBufferGL(true);
		m_glBuffers[b]->allocate( stride, maxVertexCount, NULL );
		m_glBuffers[b]->setVertexDecl( vertexDecl, declCount );
	}
}

StreamingVertexBuffer::~StreamingVertexBuffer()
{
	for (s32 b=0; b<STREAMING_BUFFER_COUNT; b++)
	{
		delete m_glBuffers[b];
	}
	free(m_buffer);
}

VertexBufferGL* StreamingVertexBuffer::update(u32& outVertexCount)
{
	VertexBufferGL* buffer = m_glBuffers[ m_bufferIndex ];
	buffer->update( m_bufferOffset, m_buffer );

	outVertexCount = m_bufferOffset / m_stride;

	m_bufferOffset = 0;
	m_bufferIndex  = (m_bufferIndex+1)%STREAMING_BUFFER_COUNT;

	return buffer;
}

u32 StreamingVertexBuffer::addVertices(const void* vertices, u32 count)
{
	const u32 size = m_stride * count;
	if (m_bufferOffset + size > m_bufferSize)
	{
		//Too many vertices.
		LOG( LOG_ERROR, "StreamingVertexBuffer - too many vertices requested, memory requested %u of %u.", m_bufferOffset+size, m_bufferSize );
		return STREAMING_VERTEX_TOOMANY;
	}

	const u32 offset = m_bufferOffset;
	memcpy(&m_buffer[offset], vertices, size);
	m_bufferOffset += size;

	return offset / m_stride;
}

void* StreamingVertexBuffer::getWritePointer(u32 count, u32& outOffset)
{
	const u32 size = m_stride * count;
	if (m_bufferOffset + size > m_bufferSize)
	{
		//Too many vertices.
		LOG( LOG_ERROR, "StreamingVertexBuffer - too many vertices requested, memory requested %u of %u.", m_bufferOffset+size, m_bufferSize );
		return NULL;
	}

	outOffset = m_bufferOffset / m_stride;
	void* outBuffer = &m_buffer[ m_bufferOffset ];
	m_bufferOffset += size;

	return outBuffer;
}

u32 StreamingVertexBuffer::getError()
{
	u32 lastError = m_lastError;
	m_lastError = STREAMING_VERTEX_OK;
	return lastError;
}
