#pragma once

#include "../common/graphicsDeviceOGL.h"
#include "../common/shaderOGL.h"

class GraphicsShadersOGL_2_0
{
    public:
        GraphicsShadersOGL_2_0();
        ~GraphicsShadersOGL_2_0();

		void loadShaders();
		ShaderOGL* getShader( ShaderID shader );

    private:
		ShaderOGL*  m_shaders[SHADER_COUNT];
		static const char* m_shaderVS[SHADER_COUNT];
		static const char* m_shaderPS[SHADER_COUNT];
};
