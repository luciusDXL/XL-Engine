#pragma once

#include "../common/graphicsDeviceOGL.h"
#include <map>

enum VertexAttributes
{
	VATTR_POSITION = 0,
	VATTR_NORMAL,
	VATTR_COLOR,
	VATTR_TEXCOORD0,
	VATTR_TEXCOORD1,
	VATTR_TEXCOORD2,
	VATTR_TEXCOORD3,
	VATTR_COUNT
};

enum VertexAttrFlags
{
	VAFLAG_POSITION  = (1<<0),
	VAFLAG_NORMAL    = (1<<1),
	VAFLAG_COLOR     = (1<<2),
	VAFLAG_TEXCOORD0 = (1<<3),
	VAFLAG_TEXCOORD1 = (1<<4),
	VAFLAG_TEXCOORD2 = (1<<5),
	VAFLAG_TEXCOORD3 = (1<<6),
};

enum VertexAttrType
{
	//floating point types, can not be normalized.
	VTYPE_FLOAT16_1 = 0,
	VTYPE_FLOAT16_2,
	VTYPE_FLOAT16_3,
	VTYPE_FLOAT16_4,
	VTYPE_FLOAT32_1,
	VTYPE_FLOAT32_2,
	VTYPE_FLOAT32_3,
	VTYPE_FLOAT32_4,
	VTYPE_FLOAT64_1,
	VTYPE_FLOAT64_2,
	VTYPE_FLOAT64_3,
	VTYPE_FLOAT64_4,
	VTYPE_FIXED32_1,	//16.16 fixed point
	VTYPE_FIXED32_2,
	VTYPE_FIXED32_3,
	VTYPE_FIXED32_4,
	//Integer types, may be normalized.
	VTYPE_INT8_1,
	VTYPE_INT8_2,
	VTYPE_INT8_4,
	VTYPE_UINT8_1,
	VTYPE_UINT8_2,
	VTYPE_UINT8_4,
	VTYPE_INT16_1,
	VTYPE_INT16_2,
	VTYPE_INT16_4,
	VTYPE_UINT16_1,
	VTYPE_UINT16_2,
	VTYPE_UINT16_4,
	VTYPE_INT32_1,
	VTYPE_INT32_2,
	VTYPE_INT32_4,
	VTYPE_UINT32_1,
	VTYPE_UINT32_2,
	VTYPE_UINT32_4,
	//Mixed types
	VTYPE_INT_2_10_10_10_REV,	//4 signed   components - 2 bits, 10 bits * 3
	VTYPE_UINT_2_10_10_10_REV,	//4 unsigned components - 2 bits, 10 bits * 3
};

struct VertexElement
{
	u32				 offset; //offset from the base, use 0 to calculate automatically.
	VertexAttributes attr;
	VertexAttrType   type;
	bool             normalized;
};

class VertexBufferOGL
{
    public:
        VertexBufferOGL(bool dynamic);
        virtual ~VertexBufferOGL();

		bool setVertexDecl(const VertexElement* vertexDecl, u32 count);
		bool allocate(u32 stride, u32 count, void* data=NULL);
		void update(u32 size, void* data);

		void bind(u32 requiredAttributes);

		//clear the vertex data
		static void clear();

	public:
		static const char* c_vertexAttrib[];
		u32 m_vertexAttrFlags;
		u32 m_glID;
		u32 m_size;
		u32 m_stride;
		u32 m_count;

		bool m_dynamic;

		VertexElement* m_vertexDecl;
		u32 m_elemCount;
};
