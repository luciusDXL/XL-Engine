/////////////////////////
// This is only a stub
// TO-DO: implement
/////////////////////////
#include <stdlib.h>
#include "graphicsShaders_GL_2_0.h"
#include "../../log.h"

const char* GraphicsShadersGL_2_0::m_shaderVS[SHADER_COUNT]=
{
	"Shaders/quadUI_20.vp",	//SHADER_QUAD_UI
};

const char* GraphicsShadersGL_2_0::m_shaderPS[SHADER_COUNT]=
{
	"Shaders/quadUI_20.fp",	//SHADER_QUAD_UI
};


GraphicsShadersGL_2_0::GraphicsShadersGL_2_0()
{
	for (u32 s=0; s<SHADER_COUNT; s++)
	{
		m_shaders[s] = new ShaderGL();
	}
}

GraphicsShadersGL_2_0::~GraphicsShadersGL_2_0()
{
	for (u32 s=0; s<SHADER_COUNT; s++)
	{
		delete m_shaders[s];
	}
}

void GraphicsShadersGL_2_0::loadShaders()
{
	for (u32 s=0; s<SHADER_COUNT; s++)
	{
		m_shaders[s]->load( m_shaderVS[s], m_shaderPS[s] );
	}
}

ShaderGL* GraphicsShadersGL_2_0::getShader( ShaderID shader )
{
	if (shader < 0 || shader > SHADER_COUNT)
	{
		return NULL;
	}

	return m_shaders[ shader ];
}
