#pragma once

#include "../CommonGL/graphicsDeviceOGL.h"

class GraphicsDeviceOGL_3_2 : public GraphicsDeviceOGL
{
    public:
        GraphicsDeviceOGL_3_2(GraphicsDevicePlatform* platform);
        virtual ~GraphicsDeviceOGL_3_2();

		bool init(int w, int h, int vw, int vh);

		bool supportsShaders();
		void setShader(ShaderID shader);

		void setShaderResource(TextureHandle handle, u32 nameHash);
		void drawQuad(const Quad& quad);
		void drawFullscreenQuad();

		void drawVirtualScreen();
		void setVirtualViewport(bool reset, int x, int y, int w, int h);
};

