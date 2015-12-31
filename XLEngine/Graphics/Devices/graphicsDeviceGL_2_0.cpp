////////////////////////////////////////////////////////////////////
// This now works but its a really rough implementation.
// Several things have to be done for it to be good enough,
// such as being able to specify shaders, handling resources
// better, handling dynamic vertex data better (its super basic
// and slow right now), etc.
// Due to these issues this currently runs the UI at 1/2 the
// framerate as the OpenGL 1.3 device - though the 
// actual game view runs at the same speed.
////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include "graphicsDeviceGL_2_0.h"
#include "graphicsShadersGL_2_0.h"
#include "../CommonGL/textureGL.h"
#include "../../log.h"
#include "../../memoryPool.h"
#include "../CommonGL/shaderGL.h"
#include "../CommonGL/vertexBufferGL.h"
#include "../CommonGL/indexBufferGL.h"
#include "../CommonGL/streamingVertexBuffer.h"

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>

//Temporary, to get basic rendering working.
#define MAX_QUADS 16384

struct QuadVertex
{
	f32 pos[3];
	f32 uv[2];
	u32 color;
};

const VertexElement c_quadVertexDecl[]=
{
	{ 0, VATTR_POSITION,  VTYPE_FLOAT32_3, false },
	{ 0, VATTR_TEXCOORD0, VTYPE_FLOAT32_2, false },
	{ 0, VATTR_COLOR,     VTYPE_UINT8_4,   true  },
};


GraphicsDeviceGL_2_0::GraphicsDeviceGL_2_0(GraphicsDevicePlatform* platform) : GraphicsDeviceGL(platform)
{
	m_deviceID = GDEV_OPENGL_2_0;
	m_curShaderID = SHADER_COUNT;
	m_curShader = NULL;

	m_caps.flags |= CAP_SUPPORT_SHADERS;
	m_caps.flags |= CAP_NON_POWER_2_TEX;
}

GraphicsDeviceGL_2_0::~GraphicsDeviceGL_2_0()
{
	delete m_quadVB;
	delete m_quadIB;

	delete [] m_frameBuffer_32bpp[0];
	delete [] m_frameBuffer_32bpp[1];
}

void GraphicsDeviceGL_2_0::drawVirtualScreen()
{
	glViewport( m_virtualViewport[0], m_virtualViewport[1], m_virtualViewport[2], m_virtualViewport[3] );

	//update the video memory framebuffer.
	//TO-DO: This device should support PBOs which should allow for much faster transfers.
	lockBuffer();
		if (m_writeFrame > m_renderFrame)
		{
			m_videoFrameBuffer->update( m_frameBuffer_32bpp[m_writeBufferIndex] );
		}
		m_renderFrame = m_writeFrame;
	unlockBuffer();

	s32 baseTex = m_curShader->getParameter("baseTex");
	m_curShader->updateParameter(baseTex, m_videoFrameBuffer->getHandle(), 0, true);	//we have to force the update since the texture itself has changed.

	enableBlending(false);
	drawFullscreenQuad(m_videoFrameBuffer);

	glViewport( m_fullViewport[0], m_fullViewport[1], m_fullViewport[2], m_fullViewport[3] );
}

void GraphicsDeviceGL_2_0::setVirtualViewport(bool reset, int x, int y, int w, int h)
{
	if (reset)
	{
		for (int i=0; i<4; i++) { m_virtualViewport[i] = m_virtualViewportNoUI[i]; }
	}
	else
	{
		m_virtualViewport[0] = x;
		m_virtualViewport[1] = y;
		m_virtualViewport[2] = w;
		m_virtualViewport[3] = h;
	}
}

bool GraphicsDeviceGL_2_0::init(int w, int h, int& vw, int& vh)
{
	m_platform->init();
	queryExtensions();

	m_shaders = new GraphicsShadersGL_2_0();
	if (!m_shaders)
	{
		return false;
	}
	m_shaders->loadShaders();

	//support up to MAX_QUADS quads per frame.
	m_quadVB = new StreamingVertexBuffer( c_quadVertexDecl, arraysize(c_quadVertexDecl), sizeof(QuadVertex), MAX_QUADS * 4 );

	u16* indices = (u16*)malloc( sizeof(u16) * MAX_QUADS * 6 );
	for (u32 q=0, v=0; q<MAX_QUADS; q++, v+=4)
	{
		indices[q*6 + 0] = v;
		indices[q*6 + 1] = v + 1;
		indices[q*6 + 2] = v + 2;

		indices[q*6 + 3] = v;
		indices[q*6 + 4] = v + 2;
		indices[q*6 + 5] = v + 3;
	}

	m_quadIB = new IndexBufferGL();
	m_quadIB->allocate( sizeof(u16), MAX_QUADS * 6, indices );
	free(indices);

	/* establish initial viewport */
	// fix the height to the actual screen height.
	// calculate the width such that wxh has a 4:3 aspect ratio (assume square pixels for modern LCD monitors).
	// offset the viewport by half the difference between the actual width and 4x3 width
	// to automatically pillar box the display.
	// Unfortunately, for modern monitors, only 1200p yields exact integer scaling. So we do all the scaling on
	// the horizontal axis, where there are more pixels, to limit the visible distortion.
	int w4x3 = (h<<2) / 3;
	int dw   = w - w4x3;
	m_virtualViewportNoUI[0] = dw>>1;
	m_virtualViewportNoUI[1] = 0;
	m_virtualViewportNoUI[2] = w4x3;
	m_virtualViewportNoUI[3] = h;

	for (int i=0; i<4; i++) { m_virtualViewport[i] = m_virtualViewportNoUI[i]; }

	m_fullViewport[0] = 0;
	m_fullViewport[1] = 0;
	m_fullViewport[2] = w;
	m_fullViewport[3] = h;

	m_windowWidth  = w;
	m_windowHeight = h;
	
	//frame size - this is the 32 bit version of the framebuffer. The 8 bit framebuffer, rendered by the software renderer, 
	//is converted to 32 bit (using the current palette) - this buffer - before being uploaded to the video card.
	m_frameWidth  = vw;
	m_frameHeight = vh;
	m_frameBuffer_32bpp[0] = new u32[ m_frameWidth*m_frameHeight ];
	m_frameBuffer_32bpp[1] = new u32[ m_frameWidth*m_frameHeight ];

	glDisable(GL_DEPTH_TEST); // disable depth buffering
	glDisable(GL_CULL_FACE);

	//Create a copy of the framebuffer on the GPU so we can upload the results there.
	const SamplerState samplerState=
	{
		WM_CLAMP, WM_CLAMP, WM_CLAMP,							//clamp on every axis
		TEXFILTER_LINEAR, TEXFILTER_POINT, TEXFILTER_POINT,		//filtering
		false													//no mipmapping
	};
	m_videoFrameBuffer = createTextureRGBA_Internal(m_frameWidth, m_frameHeight, NULL, samplerState);
	enableTexturing(true);
	
	glFlush();

	return true;
}

void GraphicsDeviceGL_2_0::setShader(ShaderID shader)
{
	if (m_curShaderID != shader)
	{
		m_curShader = m_shaders->getShader( shader );
		if (m_curShader)
		{
			m_curShader->bind();
		}
	}
	m_curShaderID = shader;
}

void GraphicsDeviceGL_2_0::clearShaderParamCache()
{
	m_curShader->clearParamCache();
}

void GraphicsDeviceGL_2_0::setShaderResource(TextureHandle handle, u32 nameHash, u32 slot)
{
	s32 parmID = m_curShader->getParameter(nameHash);
	m_curShader->updateParameter(parmID, handle, slot);
}

void GraphicsDeviceGL_2_0::setShaderParamter(void* data, u32 size, u32 nameHash)
{
	s32 parmID = m_curShader->getParameter(nameHash);
	m_curShader->updateParameter(parmID, data, size);
}

void GraphicsDeviceGL_2_0::drawQuad(const Quad& quad)
{
	//scale and display.
	float posScale[] = {-1.0f, -1.0f, 2.0f, 2.0f};
	float uvTop[] = { 0, 1 };
	float uvBot[] = { 1, 0 };

	posScale[0] =  2.0f * float(quad.p0.x) / float(m_windowWidth)  - 1.0f;
	posScale[1] = -2.0f * float(quad.p0.y) / float(m_windowHeight) + 1.0f;

	posScale[2] =  2.0f * float(quad.p1.x) / float(m_windowWidth)  - 1.0f;
	posScale[3] = -2.0f * float(quad.p1.y) / float(m_windowHeight) + 1.0f;

	uvTop[0] = quad.uv0.x;
	uvTop[1] = quad.uv0.y;
	uvBot[0] = quad.uv1.x;
	uvBot[1] = quad.uv1.y;

	u32 quadOffset = 0;
	QuadVertex* vertex = (QuadVertex*)m_quadVB->getWritePointer(4, quadOffset);
	vertex[0].pos[0] = posScale[0]; vertex[0].pos[1] = posScale[1]; vertex[0].pos[2] = 0.0f;
	vertex[0].uv[0] = uvTop[0]; vertex[0].uv[1] = uvTop[1];
	vertex[0].color = quad.color;

	vertex[1].pos[0] = posScale[2]; vertex[1].pos[1] = posScale[1]; vertex[1].pos[2] = 0.0f;
	vertex[1].uv[0] = uvBot[0]; vertex[1].uv[1] = uvTop[1];
	vertex[1].color = quad.color;

	vertex[2].pos[0] = posScale[2]; vertex[2].pos[1] = posScale[3]; vertex[2].pos[2] = 0.0f;
	vertex[2].uv[0] = uvBot[0]; vertex[2].uv[1] = uvBot[1];
	vertex[2].color = quad.color;

	vertex[3].pos[0] = posScale[0]; vertex[3].pos[1] = posScale[3]; vertex[3].pos[2] = 0.0f;
	vertex[3].uv[0] = uvTop[0]; vertex[3].uv[1] = uvBot[1];
	vertex[3].color = quad.color;

	m_curShader->uploadData(this);

	u32 outVertexCount = 0;
	VertexBufferGL* vb = m_quadVB->update(outVertexCount);
	vb->bind( m_curShader->getRequiredVertexAttrib() );
	m_quadIB->bind();

	glDrawRangeElements(GL_TRIANGLES, 0, 6, 6, (m_quadIB->m_stride==2)?GL_UNSIGNED_SHORT:GL_UNSIGNED_INT, 0);
}

void GraphicsDeviceGL_2_0::drawFullscreenQuad(TextureGL* tex)
{
	//scale and display.
	const float uvTop[] = { 0, 1 };
	const float uvBot[] = { 1, 0 };

	//for now lock and fill the vb everytime.
	//this is obviously a bad idea but will get things started.
	u32 quadOffset = 0;
	QuadVertex* vertex = (QuadVertex*)m_quadVB->getWritePointer(4, quadOffset);
	vertex[0].pos[0] = -1.0f; vertex[0].pos[1] = 1.0f; vertex[0].pos[2] = 0.0f;
	vertex[0].uv[0] = 0.0f; vertex[0].uv[1] = 0.0f;
	vertex[0].color = 0xffffffff;

	vertex[1].pos[0] = 1.0f; vertex[1].pos[1] = 1.0f; vertex[1].pos[2] = 0.0f;
	vertex[1].uv[0] = 1.0f; vertex[1].uv[1] = 0.0f;
	vertex[1].color = 0xffffffff;

	vertex[2].pos[0] = 1.0f; vertex[2].pos[1] = -1.0f; vertex[2].pos[2] = 0.0f;
	vertex[2].uv[0] = 1.0f; vertex[2].uv[1] = 1.0f;
	vertex[2].color = 0xffffffff;

	vertex[3].pos[0] = -1.0f; vertex[3].pos[1] = -1.0f; vertex[3].pos[2] = 0.0f;
	vertex[3].uv[0] = 0.0f; vertex[3].uv[1] = 1.0f;
	vertex[3].color = 0xffffffff;

	u32 outVertexCount = 0;
	VertexBufferGL* vb = m_quadVB->update(outVertexCount);
	m_curShader->uploadData(this);

	vb->bind( m_curShader->getRequiredVertexAttrib() );
	m_quadIB->bind();

	glDrawRangeElements(GL_TRIANGLES, 0, 6, 6, (m_quadIB->m_stride==2)?GL_UNSIGNED_SHORT:GL_UNSIGNED_INT, 0);
}

void GraphicsDeviceGL_2_0::flush()
{
	m_streamBuffer = m_quadVB->update( m_streamVertexCount );
}

u32 GraphicsDeviceGL_2_0::addQuad(const Quad& quad)
{
	//scale and display.
	float posScale[] = {-1.0f, -1.0f, 2.0f, 2.0f};
	float uvTop[] = { 0, 1 };
	float uvBot[] = { 1, 0 };

	posScale[0] =  2.0f * float(quad.p0.x) / float(m_windowWidth)  - 1.0f;
	posScale[1] = -2.0f * float(quad.p0.y) / float(m_windowHeight) + 1.0f;

	posScale[2] =  2.0f * float(quad.p1.x) / float(m_windowWidth)  - 1.0f;
	posScale[3] = -2.0f * float(quad.p1.y) / float(m_windowHeight) + 1.0f;

	uvTop[0] = quad.uv0.x;
	uvTop[1] = quad.uv0.y;
	uvBot[0] = quad.uv1.x;
	uvBot[1] = quad.uv1.y;

	u32 quadOffset = 0;
	QuadVertex* vertex = (QuadVertex*)m_quadVB->getWritePointer(4, quadOffset);
	vertex[0].pos[0] = posScale[0]; vertex[0].pos[1] = posScale[1]; vertex[0].pos[2] = 0.0f;
	vertex[0].uv[0] = uvTop[0]; vertex[0].uv[1] = uvTop[1];
	vertex[0].color = quad.color;

	vertex[1].pos[0] = posScale[2]; vertex[1].pos[1] = posScale[1]; vertex[1].pos[2] = 0.0f;
	vertex[1].uv[0] = uvBot[0]; vertex[1].uv[1] = uvTop[1];
	vertex[1].color = quad.color;

	vertex[2].pos[0] = posScale[2]; vertex[2].pos[1] = posScale[3]; vertex[2].pos[2] = 0.0f;
	vertex[2].uv[0] = uvBot[0]; vertex[2].uv[1] = uvBot[1];
	vertex[2].color = quad.color;

	vertex[3].pos[0] = posScale[0]; vertex[3].pos[1] = posScale[3]; vertex[3].pos[2] = 0.0f;
	vertex[3].uv[0] = uvTop[0]; vertex[3].uv[1] = uvBot[1];
	vertex[3].color = quad.color;

	return quadOffset;
}

void GraphicsDeviceGL_2_0::drawQuadBatch(u32 vertexOffset, u32 count)
{
	m_curShader->uploadData(this);

	m_streamBuffer->bind( m_curShader->getRequiredVertexAttrib() );
	m_quadIB->bind();

	const u32 indexBufferOffset = (vertexOffset>>2)*6*m_quadIB->m_stride;
	glDrawRangeElements(GL_TRIANGLES, 0, m_streamVertexCount, count*6, (m_quadIB->m_stride==2)?GL_UNSIGNED_SHORT:GL_UNSIGNED_INT, (void*)indexBufferOffset);
}
