#pragma once

#include "../graphicsDevice_Platform.h"
#include "../graphicsDeviceList.h"

class GraphicsDeviceOGL_Win32 : public GraphicsDevicePlatform
{
    public:
        GraphicsDeviceOGL_Win32();
        virtual ~GraphicsDeviceOGL_Win32();

        void setWindowData(int nParam, void **param, GraphicsDeviceID deviceID, bool exclFullscreen=false);
		bool init();
        void present();

		void enableVSync(bool enable);

	private:
		bool m_exclusiveFullscreen;
};
