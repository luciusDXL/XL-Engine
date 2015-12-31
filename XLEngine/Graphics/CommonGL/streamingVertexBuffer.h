#pragma once

#include "graphicsDeviceGL.h"

struct VertexElement;
class VertexBufferGL;

#define STREAMING_BUFFER_COUNT 2
//Errors
#define STREAMING_VERTEX_OK      0
#define STREAMING_VERTEX_TOOMANY 0xffffffff

class StreamingVertexBuffer
{
public:
	StreamingVertexBuffer(const VertexElement* vertexDecl, u32 declCount, u32 stride, u32 maxVertexCount);
	~StreamingVertexBuffer();

	VertexBufferGL* update(u32& outVertexCount);

	u32   addVertices(const void* vertices, u32 count);
	void* getWritePointer(u32 count, u32& outOffset);

	u32   getError();

private:
	u8* m_buffer;
	u32 m_bufferOffset;
	u32 m_bufferIndex;
	u32 m_stride;
	u32 m_bufferSize;
	u32 m_lastError;
	VertexBufferGL* m_glBuffers[STREAMING_BUFFER_COUNT];
};