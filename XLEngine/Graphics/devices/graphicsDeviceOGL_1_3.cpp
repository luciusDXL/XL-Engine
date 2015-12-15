#include <stdlib.h>
#include "graphicsDeviceOGL_1_3.h"
#include "../CommonGL/textureOGL.h"
#include "../../log.h"

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>

GraphicsDeviceOGL_1_3::GraphicsDeviceOGL_1_3(GraphicsDevicePlatform* platform) : GraphicsDeviceOGL(platform)
{
	m_deviceID = GDEV_OPENGL_1_3;
}

GraphicsDeviceOGL_1_3::~GraphicsDeviceOGL_1_3()
{
	delete [] m_frameBuffer_32bpp[0];
	delete [] m_frameBuffer_32bpp[1];
}

void GraphicsDeviceOGL_1_3::drawVirtualScreen()
{
	glViewport( m_virtualViewport[0], m_virtualViewport[1], m_virtualViewport[2], m_virtualViewport[3] );

	//update the video memory framebuffer.
	lockBuffer();
		if (m_writeFrame > m_renderFrame)
		{
			m_videoFrameBuffer->update( m_frameBuffer_32bpp[m_writeBufferIndex] );
		}
		m_renderFrame = m_writeFrame;
	unlockBuffer();

	m_videoFrameBuffer->bind(0);
	drawFullscreenQuad();

	glViewport( m_fullViewport[0], m_fullViewport[1], m_fullViewport[2], m_fullViewport[3] );
}

void GraphicsDeviceOGL_1_3::setVirtualViewport(bool reset, int x, int y, int w, int h)
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

bool GraphicsDeviceOGL_1_3::init(int w, int h, int vw, int vh)
{
	m_platform->init();

    glDisable(GL_DEPTH_TEST); /* enable depth buffering */

	glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

	glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

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

bool GraphicsDeviceOGL_1_3::supportsShaders()
{
	return false;
}

void GraphicsDeviceOGL_1_3::setShader(ShaderID shader)
{
	shader;	//do nothing
}

void GraphicsDeviceOGL_1_3::setShaderResource(TextureHandle handle, u32 nameHash)
{
	setTexture(handle);
}

void GraphicsDeviceOGL_1_3::drawQuad(const Quad& quad)
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

	glBegin(GL_QUADS);
		glColor4ubv((GLubyte*)&quad.color);

		glTexCoord2f(uvTop[0], uvTop[1]);
		glVertex3f(posScale[0], posScale[1], -1.0f);
		glTexCoord2f(uvBot[0], uvTop[1]);
		glVertex3f(posScale[2], posScale[1], -1.0f);

		glTexCoord2f(uvBot[0], uvBot[1]);
		glVertex3f(posScale[2], posScale[3], -1.0f);
		glTexCoord2f(uvTop[0], uvBot[1]);
		glVertex3f(posScale[0], posScale[3], -1.0f);
	glEnd();
}

void GraphicsDeviceOGL_1_3::drawFullscreenQuad()
{
	u32 white = 0xffffffff;
	glBegin(GL_QUADS);
		glColor4ubv((GLubyte*)&white);

		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(-1.0f, 1.0f, -1.0f);
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f( 1.0f, 1.0f, -1.0f);

		glTexCoord2f(1.0f, 1.0f);
		glVertex3f( 1.0f, -1.0f, -1.0f);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(-1.0f, -1.0f, -1.0f);
	glEnd();
}