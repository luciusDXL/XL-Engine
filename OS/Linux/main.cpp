#include <stdio.h>
#include <stdlib.h>
#include "../../XLEngine/input.h"
#include "../../XLEngine/clock.h"
#include "../../XLEngine/services.h"
#include "../../XLEngine/settings.h"
#include "../../XLEngine/gameLoop.h"
#include "../../XLEngine/gameUI.h"
#include "../../XLEngine/log.h"

#include <GL/glx.h>    // this includes the necessary X headers
#include <X11/X.h>     // X11 constant (e.g. TrueColor)
#include <X11/keysym.h>
#include <X11/extensions/Xrandr.h>

static char s_title[256];
static Display* s_display = NULL;

int getScreenSize(int& width, int& height)
{
    int num_sizes;
    Rotation original_rotation;

    Window root = RootWindow(s_display, 0);
    XRRScreenSize *xrrs = XRRSizes(s_display, 0, &num_sizes);

    XRRScreenConfiguration *conf = XRRGetScreenInfo(s_display, root);
    short original_rate          = XRRConfigCurrentRate(conf);
    SizeID original_size_id      = XRRConfigCurrentConfiguration(conf, &original_rotation);

    //xrrs[2] is correct, so I have to figure out how to determine this programmatically.
    width  = xrrs[original_size_id].width;
    height = xrrs[original_size_id].height;
 }

int main(int argc, char **argv)
{
    Log::open("Logs/log.txt");
	LOG( LOG_MESSAGE, "Log opened." );

	// open a connection to the X server
    s_display = XOpenDisplay(NULL);
    if (s_display == NULL)
    {
        LOG( LOG_ERROR, "could not open display.");
        return 0;
    }

    //this will give the resolution of the whole virtual screen if using multiple monitors.
    //I'm still looking for a way to get the actual monitor resolution - not the desktop resolution.
    //getScreenSize() above returns the same results.
    Screen* screen = DefaultScreenOfDisplay(s_display);
    int monitorWidth, monitorHeight;
    //getScreenSize(monitorWidth, monitorHeight);
    monitorWidth   = screen->width;
    monitorHeight  = screen->height;

	//read the settings from disk or apply the defaults and generate a new settings file.
	Settings::read(monitorWidth, monitorHeight);

	//get the engine version for display
	XLSettings* settings = Settings::get();
	sprintf(s_title, "XL Engine %s", Settings::getVersion());
	LOG( LOG_MESSAGE, s_title );

	// initialize the game loop
	void *win_param[] = { s_display, (void*)settings->windowWidth, (void*)settings->windowHeight };
	if ( !GameLoop::init(win_param, Settings::getGraphicsDeviceID()) )
	{
		LOG( LOG_ERROR, "Engine initialization failed! Could not create a graphics device." );
		return 0;
	}

	Clock::startTimer();

	//unfortunately we need to get the display back in order to process window events.
	XEvent event;

    // dispatch X events
    while (1)
    {
        //process the window events (input).
        //while ( XEventsQueued(s_display, QueuedAlready) > 0 )
        while (XPending(s_display) > 0)
        {
            XNextEvent(s_display, &event);
            switch (event.type)
            {
                case KeyPress:
                {
                    KeySym     keysym;
                    XKeyEvent *kevent;
                    char       buffer[1];

                    /* It is necessary to convert the keycode to a
                    * keysym before checking if it is an escape */
                    kevent = (XKeyEvent *)&event;
                    XLookupString((XKeyEvent *)&event, buffer, 1, &keysym, NULL);
                        //keysym contains the key, XK_Escape for example
                    break;
                }
                case ButtonPress:
                    switch (event.xbutton.button)
                    {
                    }
                    break;
                case ConfigureNotify:
                break;
                case Expose:
                break;
            }
        }; /* loop to compress events */

        //update
        GameLoop::update();

        //exit if we're done.
        if (GameLoop::checkExitGame())
		{
			break;
		}
    }

    return 0;
}
