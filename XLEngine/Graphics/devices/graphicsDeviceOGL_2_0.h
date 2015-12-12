#pragma once

#include "../common/graphicsDeviceOGL.h"

class GraphicsDeviceOGL_2_0 : public GraphicsDeviceOGL
{
    public:
        GraphicsDeviceOGL_2_0(GraphicsDevicePlatform* platform);
        virtual ~GraphicsDeviceOGL_2_0();

		bool init(int w, int h, int vw, int vh);

		TextureHandle createTextureRGBA(int width, int height, u32* data);
		void setTexture(TextureHandle handle, int slot=0);
		void drawQuad(const Quad& quad);

		void drawVirtualScreen();
		void setVirtualViewport(bool reset, int x, int y, int w, int h);
    protected:
		bool m_textureEnabled;
    private:
};
