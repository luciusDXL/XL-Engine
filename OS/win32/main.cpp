// game.cpp : Defines the entry point for the application.
//

#include "main.h"
#include "../../stdafx.h"
#include "../../XLEngine/input.h"
#include "../../XLEngine/clock.h"
#include "../../XLEngine/services.h"
#include "../../XLEngine/settings.h"
#include "../../XLEngine/gameLoop.h"
#include "../../XLEngine/gameUI.h"
#include "../../XLEngine/log.h"
#include <stdio.h>

#if XL_CHECK_MEMORY
#include <crtdbg.h>
#endif

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE s_hInst;								// current instance
TCHAR s_title[MAX_LOADSTRING];					// The title bar text
TCHAR s_windowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HWND& hWnd, HINSTANCE hInstance, int nCmdShow);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);

int win32MapShiftAndControl( int vkey );

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nCmdShow)
{
	#if XL_CHECK_MEMORY
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_CHECK_ALWAYS_DF );
	#endif

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	Log::open("Logs/log.txt");
	LOG( LOG_MESSAGE, "Log opened." );

	//read the settings from disk or apply the defaults and generate a new settings file.
	int monitorWidth  = GetSystemMetrics(SM_CXSCREEN);
	int monitorHeight = GetSystemMetrics(SM_CYSCREEN);
	Settings::read(monitorWidth, monitorHeight);

	//get the engine version for display
	XLSettings* settings = Settings::get();
	sprintf(s_title, "XL Engine %s", Settings::getVersion());
	LOG( LOG_MESSAGE, s_title );

	LoadString(hInstance, IDC_XLENGINE, s_windowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// perform application initialization:
	HWND hWnd;
	if (!InitInstance(hWnd, hInstance, nCmdShow))
	{
		Services::xlDebugMessage("InitInstance() failed.");
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_XLENGINE));

	// initialize the game loop
	void *win_param[] = { (void *)hWnd };
	if ( !GameLoop::init(win_param, Settings::getGraphicsDeviceID()) )
	{
		LOG( LOG_ERROR, "Engine initialization failed! Could not create a graphics device." );
		return 0;
	}

	// main message loop:
	Clock::startTimer();
	while (true)
	{
		if ( PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) )
		{
			if ( msg.message == WM_QUIT )
			{
				GameLoop::stopGame();
				break;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			GameLoop::update();
		}

		if (GameLoop::checkExitGame())
		{
			break;
		}
	};

	GameLoop::destroy();

	return (int)msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_XLENGINE));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= 0;
	wcex.lpszClassName	= s_windowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HWND& hWnd, HINSTANCE hInstance, int nCmdShow)
{
	s_hInst = hInstance; // Store instance handle in our global variable

	//The window style.
	DWORD dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_BORDER | WS_MINIMIZEBOX;
	XLSettings* settings = Settings::get();
	if ( settings->flags&XL_FLAG_FULLSCREEN )
	{
		dwStyle = WS_POPUP;	//for fullscreen just display the client area.
		settings->windowWidth  = GetSystemMetrics(SM_CXSCREEN);
		settings->windowHeight = GetSystemMetrics(SM_CYSCREEN);
	}

	//compute the final window size to get a specific client window size.
	RECT desiredSize;
	desiredSize.left   = 0;
	desiredSize.top    = 0;
	desiredSize.right  = settings->windowWidth;
	desiredSize.bottom = settings->windowHeight;
	AdjustWindowRect(&desiredSize, dwStyle, FALSE);

	hWnd = CreateWindow(s_windowClass, s_title, dwStyle, 0, 0, desiredSize.right-desiredSize.left, desiredSize.bottom-desiredSize.top, NULL, NULL, hInstance, NULL);
	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	GameUI::enableCursor( true );
	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	XLEngineServices* services = Services::get();

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;
	case WM_KEYDOWN:
	case WM_KEYUP:
		{
			if (wParam >= 0 && wParam < 256)
			{
				Input::setKeyState(u32(wParam), (message==WM_KEYDOWN));
			}

			int keyCode = win32MapShiftAndControl( (int)wParam );
			if (services->keyEvent)
			{
				keyCode = Input::mapVirtualKey(keyCode);
				if (keyCode > 0)
				{
					services->keyEvent(0, keyCode, (message==WM_KEYDOWN) ? 1 : 0);
				}
			}
		}
		break;
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
		Input::setMouseButtonState(Input::MouseLeft,  (message==WM_LBUTTONDOWN));
		break;
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
		Input::setMouseButtonState(Input::MouseRight,  (message==WM_RBUTTONDOWN));
		break;
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
		Input::setMouseButtonState(Input::MouseMiddle, (message==WM_MBUTTONDOWN));
		break;
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
		//to-do
		break;
	case WM_MOUSEMOVE:
		Input::setMousePos( s16(lParam), s16(lParam >> 16) );
		break;
	case WM_MOUSEWHEEL:
		Input::incMouseWheel( ((short)HIWORD(wParam)) > 0 ? 1 : -1 );
		break;
	case WM_CHAR:
		if (wParam > 0 && wParam < 128)
		{
			Input::addChar( char(wParam) );
		}

		if (services->keyEvent)
		{
			services->keyEvent(wParam&0x7f, 0, 1);
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

int win32MapShiftAndControl( int vkey )
{
	if ( vkey == VK_SHIFT )
	{
		if ( GetAsyncKeyState(VK_LSHIFT) < 0 )
		{
			vkey = VK_LSHIFT;
		}
		else if ( GetAsyncKeyState(VK_RSHIFT) < 0 )
		{
			vkey = VK_RSHIFT;
		}
	}
	else if ( vkey == VK_CONTROL )
	{
		if ( GetAsyncKeyState(VK_LCONTROL) < 0 )
		{
			vkey = VK_LCONTROL;
		}
		else if ( GetAsyncKeyState(VK_RCONTROL) < 0 )
		{
			vkey = VK_RCONTROL;
		}
	}
	return vkey;
}
