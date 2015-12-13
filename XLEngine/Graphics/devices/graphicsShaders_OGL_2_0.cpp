/////////////////////////
// This is only a stub
// TO-DO: implement
/////////////////////////
#include <stdlib.h>
#include "graphicsShaders_OGL_2_0.h"
#include "../../log.h"

const char* GraphicsShadersOGL_2_0::m_shaderVS[SHADER_COUNT]=
{
	"Shaders/quadUI_20.vp",	//SHADER_QUAD_UI
};

const char* GraphicsShadersOGL_2_0::m_shaderPS[SHADER_COUNT]=
{
	"Shaders/quadUI_20.fp",	//SHADER_QUAD_UI
};


GraphicsShadersOGL_2_0::GraphicsShadersOGL_2_0()
{
	for (u32 s=0; s<SHADER_COUNT; s++)
	{
		m_shaders[s] = new ShaderOGL();
	}
}

GraphicsShadersOGL_2_0::~GraphicsShadersOGL_2_0()
{
	for (u32 s=0; s<SHADER_COUNT; s++)
	{
		delete m_shaders[s];
	}
}

void GraphicsShadersOGL_2_0::loadShaders()
{
	for (u32 s=0; s<SHADER_COUNT; s++)
	{
		m_shaders[s]->load( m_shaderVS[s], m_shaderPS[s] );
	}
}

ShaderOGL* GraphicsShadersOGL_2_0::getShader( ShaderID shader )
{
	if (shader < 0 || shader > SHADER_COUNT)
	{
		return NULL;
	}

	return m_shaders[ shader ];
}
