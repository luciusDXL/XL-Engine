#pragma once

#include "../CommonGL/graphicsDeviceGL.h"

class GraphicsDeviceGL_3_2 : public GraphicsDeviceGL
{
    public:
        GraphicsDeviceGL_3_2(GraphicsDevicePlatform* platform);
        virtual ~GraphicsDeviceGL_3_2();

		bool init(int w, int h, int vw, int vh);

		bool supportsShaders();
		void setShader(ShaderID shader);

		void setShaderResource(TextureHandle handle, u32 nameHash);
		void drawQuad(const Quad& quad);
		void drawFullscreenQuad();

		void drawVirtualScreen();
		void setVirtualViewport(bool reset, int x, int y, int w, int h);
};

