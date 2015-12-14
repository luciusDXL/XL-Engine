#include <stdlib.h>
#include "vertexBufferOGL.h"
#include "../../log.h"

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>

const char* VertexBufferOGL::c_vertexAttrib[]=
{
	"in_position",
	"in_normal",
	"in_color",
	"in_texcoord0",
	"in_texcoord1",
	"in_texcoord2",
	"in_texcoord3",
};

const u32 c_vertexTypeSize[] =
{
	//floating point types, can not be normalized.
	sizeof(s16)*1, //VTYPE_FLOAT16_1 = 0,
	sizeof(s16)*2, //VTYPE_FLOAT16_2,
	sizeof(s16)*3, //VTYPE_FLOAT16_3,
	sizeof(s16)*4, //VTYPE_FLOAT16_4,
	sizeof(f32)*1, //VTYPE_FLOAT32_1,
	sizeof(f32)*2, //VTYPE_FLOAT32_2,
	sizeof(f32)*3, //VTYPE_FLOAT32_3,
	sizeof(f32)*4, //VTYPE_FLOAT32_4,
	sizeof(f64)*1, //VTYPE_FLOAT64_1,
	sizeof(f64)*2, //VTYPE_FLOAT64_2,
	sizeof(f64)*3, //VTYPE_FLOAT64_3,
	sizeof(f64)*4, //VTYPE_FLOAT64_4,
	sizeof(s32)*1, //VTYPE_FIXED32_1,	//16.16 fixed point
	sizeof(s32)*2, //VTYPE_FIXED32_2,
	sizeof(s32)*3, //VTYPE_FIXED32_3,
	sizeof(s32)*4, //VTYPE_FIXED32_4,
	//Integer types, may be normalized.
	sizeof(s8)*1,  //VTYPE_INT8_1,
	sizeof(s8)*2,  //VTYPE_INT8_2,
	sizeof(s8)*4,  //VTYPE_INT8_4,
	sizeof(u8)*1,  //VTYPE_UINT8_1,
	sizeof(u8)*2,  //VTYPE_UINT8_2,
	sizeof(u8)*4,  //VTYPE_UINT8_4,
	sizeof(s16)*1, //VTYPE_INT16_1,
	sizeof(s16)*2, //VTYPE_INT16_2,
	sizeof(s16)*4, //VTYPE_INT16_4,
	sizeof(u16)*1, //VTYPE_UINT16_1,
	sizeof(u16)*2, //VTYPE_UINT16_2,
	sizeof(u16)*4, //VTYPE_UINT16_4,
	sizeof(s32)*1, //VTYPE_INT32_1,
	sizeof(s32)*2, //VTYPE_INT32_2,
	sizeof(s32)*4, //VTYPE_INT32_4,
	sizeof(u32)*1, //VTYPE_UINT32_1,
	sizeof(u32)*2, //VTYPE_UINT32_2,
	sizeof(u32)*4, //VTYPE_UINT32_4,
	//Mixed types
	sizeof(u32)*1, //VTYPE_INT_2_10_10_10_REV,	//4 signed   components - 2 bits, 10 bits * 3
	sizeof(u32)*1, //VTYPE_UINT_2_10_10_10_REV,	//4 unsigned components - 2 bits, 10 bits * 3
};

const u32 c_vertexTypeCount[] =
{
	//floating point types, can not be normalized.
	1, //VTYPE_FLOAT16_1 = 0,
	2, //VTYPE_FLOAT16_2,
	3, //VTYPE_FLOAT16_3,
	4, //VTYPE_FLOAT16_4,
	1, //VTYPE_FLOAT32_1,
	2, //VTYPE_FLOAT32_2,
	3, //VTYPE_FLOAT32_3,
	4, //VTYPE_FLOAT32_4,
	1, //VTYPE_FLOAT64_1,
	2, //VTYPE_FLOAT64_2,
	3, //VTYPE_FLOAT64_3,
	4, //VTYPE_FLOAT64_4,
	1, //VTYPE_FIXED32_1,	//16.16 fixed point
	2, //VTYPE_FIXED32_2,
	3, //VTYPE_FIXED32_3,
	4, //VTYPE_FIXED32_4,
	//Integer types, may be normalized.
	1, //VTYPE_INT8_1,
	2, //VTYPE_INT8_2,
	4, //VTYPE_INT8_4,
	1, //VTYPE_UINT8_1,
	2, //VTYPE_UINT8_2,
	4, //VTYPE_UINT8_4,
	1, //VTYPE_INT16_1,
	2, //VTYPE_INT16_2,
	4, //VTYPE_INT16_4,
	1, //VTYPE_UINT16_1,
	2, //VTYPE_UINT16_2,
	4, //VTYPE_UINT16_4,
	1, //VTYPE_INT32_1,
	2, //VTYPE_INT32_2,
	4, //VTYPE_INT32_4,
	1, //VTYPE_UINT32_1,
	2, //VTYPE_UINT32_2,
	4, //VTYPE_UINT32_4,
	//Mixed types
	4, //VTYPE_INT_2_10_10_10_REV,	//4 signed   components - 2 bits, 10 bits * 3
	4, //VTYPE_UINT_2_10_10_10_REV,	//4 unsigned components - 2 bits, 10 bits * 3
};

const u32 c_vertexGLType[] =
{
	//floating point types, can not be normalized.
	GL_HALF_FLOAT,					//VTYPE_FLOAT16_1 = 0,
	GL_HALF_FLOAT,					//VTYPE_FLOAT16_2,
	GL_HALF_FLOAT,					//VTYPE_FLOAT16_3,
	GL_HALF_FLOAT,					//VTYPE_FLOAT16_4,
	GL_FLOAT,						//VTYPE_FLOAT32_1,
	GL_FLOAT,						//VTYPE_FLOAT32_2,
	GL_FLOAT,						//VTYPE_FLOAT32_3,
	GL_FLOAT,						//VTYPE_FLOAT32_4,
	GL_DOUBLE,						//VTYPE_FLOAT64_1,
	GL_DOUBLE,						//VTYPE_FLOAT64_2,
	GL_DOUBLE,						//VTYPE_FLOAT64_3,
	GL_DOUBLE,						//VTYPE_FLOAT64_4,
	GL_FIXED,						//VTYPE_FIXED32_1,	//16.16 fixed point
	GL_FIXED,						//VTYPE_FIXED32_2,
	GL_FIXED,						//VTYPE_FIXED32_3,
	GL_FIXED,						//VTYPE_FIXED32_4,
	//Integer types, may be normalized.
	GL_BYTE,						//VTYPE_INT8_1,
	GL_BYTE,						//VTYPE_INT8_2,
	GL_BYTE,						//VTYPE_INT8_4,
	GL_UNSIGNED_BYTE,				//VTYPE_UINT8_1,
	GL_UNSIGNED_BYTE,				//VTYPE_UINT8_2,
	GL_UNSIGNED_BYTE,				//VTYPE_UINT8_4,
	GL_SHORT,						//VTYPE_INT16_1,
	GL_SHORT,						//VTYPE_INT16_2,
	GL_SHORT,						//VTYPE_INT16_4,
	GL_UNSIGNED_SHORT,				//VTYPE_UINT16_1,
	GL_UNSIGNED_SHORT,				//VTYPE_UINT16_2,
	GL_UNSIGNED_SHORT,				//VTYPE_UINT16_4,
	GL_INT,							//VTYPE_INT32_1,
	GL_INT,							//VTYPE_INT32_2,
	GL_INT,							//VTYPE_INT32_4,
	GL_UNSIGNED_INT,				//VTYPE_UINT32_1,
	GL_UNSIGNED_INT,				//VTYPE_UINT32_2,
	GL_UNSIGNED_INT,				//VTYPE_UINT32_4,
	//Mixed types
	GL_INT_2_10_10_10_REV,			//VTYPE_INT_2_10_10_10_REV,	//4 signed   components - 2 bits, 10 bits * 3
	GL_UNSIGNED_INT_2_10_10_10_REV, //VTYPE_UINT_2_10_10_10_REV,	//4 unsigned components - 2 bits, 10 bits * 3
};

VertexBufferOGL::VertexBufferOGL(bool dynamic)
{
	glGenBuffers(1, &m_glID);
	m_dynamic = dynamic;

	m_vertexDecl = NULL;

	m_size   = 0;
	m_stride = 0;
	m_count  = 0;
	m_elemCount = 0;
}

VertexBufferOGL::~VertexBufferOGL()
{
	glDeleteBuffers(1, &m_glID);
	delete [] m_vertexDecl;
}

bool VertexBufferOGL::setVertexDecl(const VertexElement* vertexDecl, u32 count)
{
	m_elemCount  = count;
	m_vertexDecl = new VertexElement[ count ];
	if (!m_vertexDecl)
	{
		return false;
	}

	u32 offset = 0;
	for (u32 v=0; v<count; v++)
	{
		m_vertexDecl[v] = vertexDecl[v];
		if (m_vertexDecl[v].offset == 0 && v > 0)
		{
			m_vertexDecl[v].offset = offset;
		}
		else
		{
			offset = m_vertexDecl[v].offset;
		}

		offset += c_vertexTypeSize[ m_vertexDecl[v].type ];
	}

	return true;
}

bool VertexBufferOGL::allocate(u32 stride, u32 count, void* data/*=NULL*/)
{
	m_size   = stride*count;
	m_stride = stride;
	m_count  = count;

	glBindBuffer(GL_ARRAY_BUFFER, m_glID);
	glBufferData(GL_ARRAY_BUFFER, m_size, data, m_dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return true;
}

void VertexBufferOGL::update(u32 size, void* data)
{
	glBindBuffer(GL_ARRAY_BUFFER, m_glID);

	//cause the "discard" buffer behavior, which is done by first calling glBufferData() with NULL data.
	glBufferData(GL_ARRAY_BUFFER, m_size, NULL, m_dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);

	//now lock and update the buffer.
	void* mem = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	if ( mem )
	{
		memcpy(mem, data, min(size, m_size));
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VertexBufferOGL::bind(u32 requiredAttributes)
{
	glBindBuffer(GL_ARRAY_BUFFER, m_glID);

	bool slotEnabled[ VATTR_COUNT ] = {false};
	//enable used slots and setup the format.
	for (u32 v=0; v<m_elemCount; v++)
	{
		VertexAttributes attr = m_vertexDecl[v].attr;
		if ( requiredAttributes & (1<<attr) )
		{
			glEnableVertexAttribArray(attr);
			glVertexAttribPointer(attr, c_vertexTypeCount[ m_vertexDecl[v].type ], c_vertexGLType[ m_vertexDecl[v].type ], m_vertexDecl[v].normalized, m_stride, BUFFER_OFFSET(m_vertexDecl[v].offset));
			slotEnabled[ attr ] = true;
		}
	}

	//disable unused slots
	for (u32 v=0; v<VATTR_COUNT; v++)
	{
		if (!slotEnabled[v]) { glDisableVertexAttribArray(v); }
	}
}

void VertexBufferOGL::clear()
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	for (u32 v=0; v<VATTR_COUNT; v++)
	{
		glDisableVertexAttribArray(v);
	}
}
