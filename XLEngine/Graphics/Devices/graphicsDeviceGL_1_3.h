#pragma once

#include "../CommonGL/graphicsDeviceGL.h"

class GraphicsDeviceGL_1_3 : public GraphicsDeviceGL
{
    public:
        GraphicsDeviceGL_1_3(GraphicsDevicePlatform* platform);
        virtual ~GraphicsDeviceGL_1_3();

		bool init(int w, int h, int& vw, int& vh);

		void setShader(ShaderID shader);

		void clearShaderParamCache();
		void setShaderResource(TextureHandle handle, u32 nameHash, u32 slot=0);
		void setShaderParamter(void* data, u32 size, u32 nameHash);

		void drawQuad(const Quad& quad);
		void drawFullscreenQuad(TextureGL* tex);

		void drawVirtualScreen();
		void setVirtualViewport(bool reset, int x, int y, int w, int h);
};
