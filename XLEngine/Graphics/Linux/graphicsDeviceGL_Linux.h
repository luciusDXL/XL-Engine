#pragma once

#include "../graphicsDevice_Platform.h"
#include "../graphicsDeviceList.h"

class GraphicsDeviceGL_Linux : public GraphicsDevicePlatform
{
    public:
        GraphicsDeviceGL_Linux();
        virtual ~GraphicsDeviceGL_Linux();

        void setWindowData(int nParam, void **param, GraphicsDeviceID deviceID, bool exclFullscreen=false);
		bool init();
        void present();

		GraphicsDeviceID autodetect(int nParam, void **param);
		void enableVSync(bool enable);

		bool queryExtension(const char* name);

	private:
		bool m_exclusiveFullscreen;
		bool m_initialized;
		bool m_adaptiveVsync;
		bool m_vsyncEnabled;
};
