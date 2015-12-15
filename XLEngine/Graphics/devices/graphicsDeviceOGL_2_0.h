#pragma once

#include "../CommonGL/graphicsDeviceOGL.h"

class GraphicsShadersOGL_2_0;
class VertexBufferOGL;
class IndexBufferOGL;

class GraphicsDeviceOGL_2_0 : public GraphicsDeviceOGL
{
    public:
        GraphicsDeviceOGL_2_0(GraphicsDevicePlatform* platform);
        virtual ~GraphicsDeviceOGL_2_0();

		bool init(int w, int h, int vw, int vh);

		bool supportsShaders();
		void setShader(ShaderID shader);

		void setShaderResource(TextureHandle handle, u32 nameHash);
		void drawQuad(const Quad& quad);
		void drawFullscreenQuad();

		void drawVirtualScreen();
		void setVirtualViewport(bool reset, int x, int y, int w, int h);
    protected:
		GraphicsShadersOGL_2_0* m_shaders;
		VertexBufferOGL* m_quadVB;
		IndexBufferOGL*  m_quadIB;

		ShaderOGL* m_curShader;
		u32 m_curShaderID;
};
