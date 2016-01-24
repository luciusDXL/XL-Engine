#include "../../log.h"
#include "../../Math/math.h"
#include "graphicsDeviceGL_Linux.h"

#include "../graphicsGL_Inc.h"

//Linux stuff
#include <X11/X.h>     /* X11 constant (e.g. TrueColor) */

static int c_snglBuf[] = {GLX_RGBA, GLX_DEPTH_SIZE, 24, None};
static int c_dblBuf[]  = {GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None};

PFNGLXSWAPINTERVALEXTPROC glXSwapIntervalEXT = NULL;
static const GLubyte* s_glExtensions;

static Display*   s_display;
static Window     s_win;
static bool       s_doubleBuffer;

GraphicsDeviceGL_Linux::GraphicsDeviceGL_Linux()
{
	m_exclusiveFullscreen = false;
	m_initialized = false;
}

GraphicsDeviceGL_Linux::~GraphicsDeviceGL_Linux()
{
}

void GraphicsDeviceGL_Linux::present()
{
    if (s_doubleBuffer)
    {
        glXSwapBuffers(s_display, s_win);
    }
    else
    {
        glFlush();
    }

	//avoid buffering frames when vsync is enabled otherwise the input "lag" will be increased.
	if (m_vsyncEnabled)
	{
		glFinish();
	}
}

void GraphicsDeviceGL_Linux::setWindowData(int nParam, void **param, GraphicsDeviceID deviceID, bool exclFullscreen/*=false*/)
{
	if (m_initialized)
	{
		return;
	}

	m_exclusiveFullscreen = exclFullscreen;
	s_display = (Display*)param[0];
	size_t windowWidth  = size_t(param[1]);
	size_t windowHeight = size_t(param[2]);

    XVisualInfo         *vi;
    Colormap             cmap;
    XSetWindowAttributes swa;
    GLXContext           cx;
    int                  dummy;

    // make sure OpenGL's GLX extension supported
    if(!glXQueryExtension(s_display, &dummy, &dummy))
    {
        LOG( LOG_ERROR, "X server has no OpenGL GLX extension" );
        return;
    }

    // find an appropriate visual

    // find an OpenGL-capable RGB visual with depth buffer
    s_doubleBuffer = true;
    vi = glXChooseVisual(s_display, DefaultScreen(s_display), c_dblBuf);
    if (vi == NULL)
    {
        vi = glXChooseVisual(s_display, DefaultScreen(s_display), c_snglBuf);
        if (vi == NULL)
        {
            LOG( LOG_ERROR, "no RGB visual with depth buffer" );
        }
        s_doubleBuffer = false;
    }

    // create an OpenGL rendering context
    cx = glXCreateContext(s_display, vi, None, GL_TRUE);
    if (cx == NULL)
    {
        LOG( LOG_ERROR, "could not create rendering context" );
        return;
    }

    // create an X window with the selected visual

    // create an X colormap since probably not using default visual
    cmap = XCreateColormap(s_display, RootWindow(s_display, vi->screen), vi->visual, AllocNone);
    swa.colormap = cmap;
    swa.border_pixel = 0;
    swa.event_mask = KeyPressMask | ExposureMask | ButtonPressMask | StructureNotifyMask;

    u32 windowAttr = CWBorderPixel | CWColormap | CWEventMask;
    s_win = XCreateWindow(s_display, RootWindow(s_display, vi->screen), 0, 0, windowWidth, windowHeight, 0, vi->depth, InputOutput, vi->visual, windowAttr, &swa);
    XSetStandardProperties(s_display, s_win, "main", "main", None, NULL, 0, NULL);

    // bind the rendering context to the window
    glXMakeCurrent(s_display, s_win, cx);

    // request the X window to be displayed on the screen
    XMapWindow(s_display, s_win);

    //platform extensions
    glXSwapIntervalEXT = (PFNGLXSWAPINTERVALEXTPROC)glXGetProcAddress( (const GLubyte*)"glXSwapIntervalEXT" );

    LOG( LOG_MESSAGE, "OpenGL Window initialized." );
}

bool GraphicsDeviceGL_Linux::init()
{
	if (m_initialized)
	{
		return true;
	}

	const GLubyte* glVersion    = glGetString(GL_VERSION);
	s_glExtensions = glGetString(GL_EXTENSIONS);

	LOG( LOG_MESSAGE, (const char*)glVersion );

	//initialize GLEW for extension loading.
	/*
	const GLenum err = glewInit();
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
	*/

	m_initialized = true;
	return true;
}

GraphicsDeviceID GraphicsDeviceGL_Linux::autodetect(int nParam, void **param)
{
	GraphicsDeviceID devID = GDEV_OPENGL_1_3;

	setWindowData(nParam, param, GDEV_INVALID);
	if ( !init() )
	{
		return GDEV_INVALID;
	}

    /*
	if ( GLEW_VERSION_3_2 == GL_TRUE )
	{
		devID = GDEV_OPENGL_3_2;
	}
	else if ( GLEW_VERSION_2_0 == GL_TRUE )
	{
		devID = GDEV_OPENGL_2_0;
	}
	*/

	//hack - this is here until the OpenGL 3.2 device is implemented.
	//remove as soon as the device is available!
		if (devID == GDEV_OPENGL_3_2) { devID = GDEV_OPENGL_2_0; }
	//end hack

	return devID;
}

void GraphicsDeviceGL_Linux::enableVSync(bool enable)
{
	const s32 enableValue = 1;//m_adaptiveVsync ? -1 : 1;
	glXSwapIntervalEXT(s_display, s_win, enable ? enableValue : 0 );

	m_vsyncEnabled = enable;
}

bool GraphicsDeviceGL_Linux::queryExtension(const char* name)
{
    //will this work?
    return true;//strstr(s_glExtensions, name) != NULL;
}
