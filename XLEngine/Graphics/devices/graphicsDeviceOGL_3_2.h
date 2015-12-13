#pragma once

#include "../common/graphicsDeviceOGL.h"

class GraphicsDeviceOGL_3_2 : public GraphicsDeviceOGL
{
    public:
        GraphicsDeviceOGL_3_2(GraphicsDevicePlatform* platform);
        virtual ~GraphicsDeviceOGL_3_2();

		bool init(int w, int h, int vw, int vh);

		TextureHandle createTextureRGBA(int width, int height, u32* data);
		void setShaderResource(TextureHandle handle, u32 nameHash);
		void drawQuad(const Quad& quad);
		void drawFullscreenQuad();

		void drawVirtualScreen();
		void setVirtualViewport(bool reset, int x, int y, int w, int h);
    protected:
		void setTexture(TextureHandle handle, int slot=0);

		bool m_textureEnabled;
    private:
};

