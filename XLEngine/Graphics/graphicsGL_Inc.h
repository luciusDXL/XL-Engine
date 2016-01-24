#pragma once

#ifdef _WIN32
    #include <GL/glew.h>
    #include <GL/gl.h>
    #include <GL/glu.h>
#else
    #define GL_GLEXT_PROTOTYPES
    #include <GL/gl.h>
    #include <GL/glu.h>
    #include <GL/glx.h>
    #include "GL/glext.h"
#endif
