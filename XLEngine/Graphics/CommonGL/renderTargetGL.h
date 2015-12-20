#pragma once
//////////////////////////////////////////////
// Very basic Render Target implementation
// Currently only supports 1 RGBA color buffer
// and no depth
//////////////////////////////////////////////

#include "graphicsDeviceGL.h"
#include "textureGL.h"
#include "../samplerState.h"

//TO-DO:
//Add support for multiple color buffers
//Add support for different formats
//Add support for depth textures

enum RenderTargetCreateFlags
{
	RT_CREATE_DEPTH = (1<<0),
};

class RenderTargetGL
{
    public:
        RenderTargetGL(RenderTargetHandle handle, TextureGL* texture);
        virtual ~RenderTargetGL();

		//the initial version will not support depth buffers.
		bool create(const GraphicsDevice* gdev, u32 createFlags, u32 width, u32 height, const SamplerState& initSamplerState);

		void bind();
		static void unbind();

		TextureGL* getTexture();
		RenderTargetHandle getHandle() const { return m_handle; }

		u32 getWidth()  const { return m_width;  }
		u32 getHeight() const { return m_height; }

	private:
		u32 m_glID;
		u32 m_glDepthID;
		TextureGL* m_texture;

		u32 m_width;
		u32 m_height;

		RenderTargetHandle m_handle;
};
