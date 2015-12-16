#pragma once

#include "graphicsDeviceGL.h"
#include "../samplerState.h"

class TextureGL
{
    public:
        TextureGL(TextureHandle handle, bool dynamic);
        virtual ~TextureGL();

		void bind(u32 slot);

		bool createRGBA(const GraphicsDevice* gdev, u32 width, u32 height, const u32* data, const SamplerState& initSamplerState);
		void update(const u32* data);
		void update(u32 srcWidth, u32 srcHeight, const u32* data);

		void setSamplerState(const SamplerState& state);

		TextureHandle getHandle() { return m_handle; }
		void getDimensions(u32& w, u32& h) { w = m_width; h = m_height; }
		
		//clear
		static void clear(u32 slot);

	private:
		void setGLSamplerState(const SamplerState& state);
		void allocateUpdateBuffer(u32 width, u32 height);

		bool m_dynamic;
		u32 m_glID;
		u32 m_width;
		u32 m_height;

		u32* m_updateBuffer;

		TextureHandle m_handle;
};
