#pragma once

#include "../graphicsDevice_Platform.h"

class GraphicsDeviceOGL_Win32 : public GraphicsDevicePlatform
{
    public:
        GraphicsDeviceOGL_Win32();
        virtual ~GraphicsDeviceOGL_Win32();

        void setWindowData(int nParam, void **param, bool exclFullscreen=false);
		bool init();
        void present();

		void enableVSync(bool enable);

	private:
		bool m_exclusiveFullscreen;
};
