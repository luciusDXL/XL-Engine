#pragma once

#include "../common/graphicsDeviceOGL.h"

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

		TextureHandle createTextureRGBA(int width, int height, u32* data);
		void setShaderResource(TextureHandle handle, u32 nameHash);
		void drawQuad(const Quad& quad);
		void drawFullscreenQuad();

		void drawVirtualScreen();
		void setVirtualViewport(bool reset, int x, int y, int w, int h);
    protected:
		void setTexture(TextureHandle handle, int slot=0);

		bool m_textureEnabled;
		GraphicsShadersOGL_2_0* m_shaders;
		VertexBufferOGL* m_quadVB;
		IndexBufferOGL*  m_quadIB;

		ShaderOGL* m_curShader;
		u32 m_curShaderID;

    private:
};
