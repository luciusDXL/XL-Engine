#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include "../../log.h"
#include "graphicsDeviceGL_Win32.h"

#include <windows.h>
#include "../../Math/math.h"

#include <GL/glew.h>
#include <GL/wglew.h>
#include <GL/gl.h>
#include <GL/glu.h>

//WGL Extension crap... fortunately only on Windows.
typedef BOOL (WINAPI * PFNWGLSWAPINTERVALEXTPROC)(int interval);
typedef const char * (WINAPI * PFNWGLGETEXTENSIONSSTRINGEXTPROC)(void);

static HGLRC m_hRC;
static HDC   m_hDC;
static HWND  m_hWnd;

static WORD m_GammaRamp_Default[3][256];
static WORD m_GammaRamp[3][256];

bool wglExtensionSupported(const char *extension_name)
{
    // this is pointer to function which returns pointer to string with list of all wgl extensions
    PFNWGLGETEXTENSIONSSTRINGEXTPROC _wglGetExtensionsStringEXT = NULL;

    // determine pointer to wglGetExtensionsStringEXT function
    _wglGetExtensionsStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC)wglGetProcAddress("wglGetExtensionsStringEXT");

    if (strstr(_wglGetExtensionsStringEXT(), extension_name) == NULL)
    {
        // string was not found
        return false;
    }

    // extension is supported
    return true;
}



GraphicsDeviceGL_Win32::GraphicsDeviceGL_Win32()
{
	m_exclusiveFullscreen = false;
	m_initialized = false;
}

GraphicsDeviceGL_Win32::~GraphicsDeviceGL_Win32()
{
	//Restore the gamma ramp.
	if ( m_exclusiveFullscreen )
	{
		HDC hdc = GetDC(NULL);
		SetDeviceGammaRamp(hdc, m_GammaRamp_Default);
	}
}

void GraphicsDeviceGL_Win32::present()
{
	HDC hdc = GetDC(m_hWnd);
    SwapBuffers( hdc );
	ReleaseDC( m_hWnd, hdc );
}

void GraphicsDeviceGL_Win32::setWindowData(int nParam, void **param, GraphicsDeviceID deviceID, bool exclFullscreen/*=false*/)
{
	if (m_initialized)
	{
		return;
	}

	m_exclusiveFullscreen = exclFullscreen;

    m_hWnd = (HWND)param[0];
	HDC m_hDC = GetDC(m_hWnd);
	
	BYTE bits = 32;
	static PIXELFORMATDESCRIPTOR pfd=					// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),					// Size Of This Pixel Format Descriptor
		1,												// Version Number
		PFD_DRAW_TO_WINDOW |							// Format Must Support Window
		PFD_SUPPORT_OPENGL |							// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,								// Must Support Double Buffering
		PFD_TYPE_RGBA,									// Request An RGBA Format
		bits,											// Select Our Color Depth
		0, 0, 0, 0, 0, 0,								// Color Bits Ignored
		0,												// No Alpha Buffer
		0,												// Shift Bit Ignored
		0,												// No Accumulation Buffer
		0, 0, 0, 0,										// Accumulation Bits Ignored
		24,												// 24Bit Z-Buffer (Depth Buffer)
		8,												// 8Bit Stencil Buffer
		0,												// No Auxiliary Buffer
		PFD_MAIN_PLANE,									// Main Drawing Layer
		0,												// Reserved
		0, 0, 0											// Layer Masks Ignored
	};

	int PixelFormat;
	if ( !(PixelFormat=ChoosePixelFormat(m_hDC, &pfd)) )	// Did Windows find a matching Pixel Format?
	{
		return;
	}

	if ( !SetPixelFormat(m_hDC, PixelFormat, &pfd) )		// Are we able to set the Pixel Format?
	{
		return;
	}

	//TO-DO: create a proper (modern) context specific to the version of OpenGL trying the be used.
	m_hRC = wglCreateContext(m_hDC);
	if ( m_hRC == 0 )
	{
		return;
	}

	if ( !wglMakeCurrent(m_hDC, m_hRC) )					// Try to activate the Rendering Context
	{
		return;
	}

	//Now enable or disable VSYNC based on settings.
	//If the extension can't be found then we're forced to use whatever the driver default is...
	//but this shouldn't happen. If it does, the engine will still run at least. :)
	if ( wglExtensionSupported("WGL_EXT_swap_control") )
	{
		// Extension is supported, init pointer.
		// Save so that the swap interval can be changed later (to be implemented).
		wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
	}

	m_adaptiveVsync = false;
	if ( wglExtensionSupported("EXT_swap_control_tear") )
	{
		m_adaptiveVsync = true;
	}

	//Only setup the gamma ramp in fullscreen.
	if ( m_exclusiveFullscreen )
	{
		//get the current gamma ramp so it can be restored on exit.
		GetDeviceGammaRamp(m_hDC, m_GammaRamp_Default);

		float fBrightness=1.0f, fContrast=1.0f, fGamma=1.0f;

		//apply brightness, contrast and gamma.
		float fIntensity = 0.0f;
		float fValue;
		const float fInc = 1.0f / 255.0f;
		for (int i=0; i<256; i++)
		{
			//apply contrast
			fValue = fContrast*(fIntensity - 0.5f) + 0.5f;
			//apply brightness
			fValue = fBrightness*fValue;
			//apply gamma
			fValue = powf(fValue, fGamma);
			//clamp.
			fValue = Math::saturate(fValue);
			
			int intValue = ((int)(fValue*255.0f)) << 8;

			m_GammaRamp[0][i] = intValue;
			m_GammaRamp[1][i] = intValue;
			m_GammaRamp[2][i] = intValue;

			fIntensity += fInc;
		}
		SetDeviceGammaRamp(m_hDC, m_GammaRamp);
	}
}

bool GraphicsDeviceGL_Win32::init()
{
	if (m_initialized)
	{
		return true;
	}

	const GLubyte* glVersion    = glGetString(GL_VERSION);
	const GLubyte* glExtensions = glGetString(GL_EXTENSIONS);

	//initialize GLEW for extension loading.
	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		LOG( LOG_ERROR, "GLEW failed to initialize, an OpenGL device cannot be created." );
		return false;
	}

	LOG( LOG_MESSAGE, "glVersion: %s", (const char*)glVersion );

	//check against the minimum version.
	if ( GLEW_VERSION_1_3 == GL_FALSE )
	{
		LOG( LOG_ERROR, "OpenGL Version 1.3 is not supported. Aborting XL Engine startup." );
		return false;
	}
	
	m_initialized = true;
	return true;
}

GraphicsDeviceID GraphicsDeviceGL_Win32::autodetect(int nParam, void **param)
{
	GraphicsDeviceID devID = GDEV_OPENGL_1_3;

	setWindowData(nParam, param, GDEV_INVALID);
	if ( !init() )
	{
		return GDEV_INVALID;
	}

	if ( GLEW_VERSION_3_2 == GL_TRUE )
	{
		devID = GDEV_OPENGL_3_2;
	}
	else if ( GLEW_VERSION_2_0 == GL_TRUE )
	{
		devID = GDEV_OPENGL_2_0;
	}

	//hack - this is here until the OpenGL 3.2 device is implemented.
	//remove as soon as the device is available!
		if (devID == GDEV_OPENGL_3_2) { devID = GDEV_OPENGL_2_0; }
	//end hack

	return devID;
}

void GraphicsDeviceGL_Win32::enableVSync(bool enable)
{
	if (wglSwapIntervalEXT)
	{
		s32 enableValue = m_adaptiveVsync ? -1 : 1;
		wglSwapIntervalEXT( enable ? enableValue : 0 );
	}
}
