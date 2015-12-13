#pragma once

#include <stdlib.h>
#include "../common/graphicsDeviceOGL.h"

class IndexBufferOGL
{
    public:
        IndexBufferOGL();
        virtual ~IndexBufferOGL();

		bool allocate(u32 stride, u32 count, void* data=NULL);
		void update(u32 size, void* data);

		void bind();

		//clear the vertex data
		static void clear();

	public:
		u32 m_glID;
		u32 m_size;
		u32 m_stride;
		u32 m_count;
};
