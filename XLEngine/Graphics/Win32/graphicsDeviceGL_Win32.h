#pragma once

#include "../graphicsDevice_Platform.h"
#include "../graphicsDeviceList.h"

class GraphicsDeviceGL_Win32 : public GraphicsDevicePlatform
{
    public:
        GraphicsDeviceGL_Win32();
        virtual ~GraphicsDeviceGL_Win32();

        void setWindowData(int nParam, void **param, GraphicsDeviceID deviceID, bool exclFullscreen=false);
		bool init();
        void present();

		GraphicsDeviceID autodetect(int nParam, void **param);

		void enableVSync(bool enable);

	private:
		bool m_exclusiveFullscreen;
		bool m_initialized;
		bool m_adaptiveVsync;
		bool m_vsyncEnabled;
};
