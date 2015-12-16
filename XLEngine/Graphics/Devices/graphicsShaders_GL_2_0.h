#pragma once

#include "../CommonGL/graphicsDeviceGL.h"
#include "../CommonGL/shaderGL.h"

class GraphicsShadersGL_2_0
{
    public:
        GraphicsShadersGL_2_0();
        ~GraphicsShadersGL_2_0();

		void loadShaders();
		ShaderGL* getShader( ShaderID shader );

    private:
		ShaderGL*  m_shaders[SHADER_COUNT];
		static const char* m_shaderVS[SHADER_COUNT];
		static const char* m_shaderPS[SHADER_COUNT];
};
