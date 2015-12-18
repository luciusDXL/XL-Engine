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

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>

//Temporary, to get basic rendering working.
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

	m_quadVB = new VertexBufferGL(true);
	m_quadVB->allocate( sizeof(QuadVertex), 4 );
	m_quadVB->setVertexDecl( c_quadVertexDecl, arraysize(c_quadVertexDecl) );

	u16 indices[6]=
	{
		0, 1, 2,
		0, 2, 3
	};
	m_quadIB = new IndexBufferGL();
	m_quadIB->allocate( sizeof(u16), 6, indices );

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

void GraphicsDeviceGL_2_0::setShaderResource(TextureHandle handle, u32 nameHash)
{
	s32 parmID = m_curShader->getParameter(nameHash);
	m_curShader->updateParameter(parmID, handle, 0);
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

	//for now lock and fill the vb everytime.
	//this is obviously a bad idea but will get things started.
	const QuadVertex vertex[]=
	{
		{ posScale[0], posScale[1], 0.0f, uvTop[0], uvTop[1], quad.color },
		{ posScale[2], posScale[1], 0.0f, uvBot[0], uvTop[1], quad.color },
		{ posScale[2], posScale[3], 0.0f, uvBot[0], uvBot[1], quad.color },
		{ posScale[0], posScale[3], 0.0f, uvTop[0], uvBot[1], quad.color },
	};
	m_quadVB->update(sizeof(QuadVertex)*4, (void*)vertex);
	m_curShader->uploadData(this);

	m_quadVB->bind( m_curShader->getRequiredVertexAttrib() );
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
	const QuadVertex vertex[]=
	{
		{ -1.0f,  1.0f, 0.0f, 0.0f, 0.0f, 0xffffffff },
		{  1.0f,  1.0f, 0.0f, 1.0f, 0.0f, 0xffffffff },
		{  1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0xffffffff },
		{ -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0xffffffff },
	};
	m_quadVB->update(sizeof(QuadVertex)*4, (void*)vertex);
	m_curShader->uploadData(this);

	m_quadVB->bind( m_curShader->getRequiredVertexAttrib() );
	m_quadIB->bind();

	glDrawRangeElements(GL_TRIANGLES, 0, 6, 6, (m_quadIB->m_stride==2)?GL_UNSIGNED_SHORT:GL_UNSIGNED_INT, 0);
}
