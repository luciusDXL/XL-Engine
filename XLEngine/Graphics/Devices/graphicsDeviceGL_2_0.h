#pragma once

#include "../CommonGL/graphicsDeviceGL.h"

class GraphicsShadersGL_2_0;
class StreamingVertexBuffer;
class VertexBufferGL;
class IndexBufferGL;
class ShaderGL;

class GraphicsDeviceGL_2_0 : public GraphicsDeviceGL
{
    public:
        GraphicsDeviceGL_2_0(GraphicsDevicePlatform* platform);
        virtual ~GraphicsDeviceGL_2_0();

		bool init(int w, int h, int& vw, int& vh);

		void setShader(ShaderID shader);

		void clearShaderParamCache();
		void setShaderResource(TextureHandle handle, u32 nameHash, u32 slot=0);
		void setShaderParameter(void* data, u32 size, u32 nameHash);
		void drawQuad(const Quad& quad);
		void drawFullscreenQuad(TextureGL* tex);

		void drawVirtualScreen();
		void setVirtualViewport(bool reset, int x, int y, int w, int h);

		//batching
		void flush();
		u32 addQuad(const Quad& quad);
		void drawQuadBatch(u32 vertexOffset, u32 count);
    protected:
		GraphicsShadersGL_2_0* m_shaders;
		StreamingVertexBuffer* m_quadVB;
		IndexBufferGL*  m_quadIB;

		ShaderGL* m_curShader;
		u32 m_curShaderID;

		//batching
		VertexBufferGL* m_streamBuffer;
		u32 m_streamVertexCount;
};
