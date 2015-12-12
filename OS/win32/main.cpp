// game.cpp : Defines the entry point for the application.
//

#include "../../stdafx.h"
#include "main.h"
#include "../../XLEngine/input.h"
#include "../../XLEngine/services.h"
#include "../../XLEngine/settings.h"
#include "../../XLEngine/gameLoop.h"
#include "../../XLEngine/gameUI.h"
#include <stdio.h>

HWND _hwnd;

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	int monitorWidth  = GetSystemMetrics(SM_CXSCREEN);
	int monitorHeight = GetSystemMetrics(SM_CYSCREEN);
	Settings::read(monitorWidth, monitorHeight);
	XLSettings* settings = Settings::get();
	sprintf(szTitle, "XL Engine %s", Settings::getVersion());

	LoadString(hInstance, IDC_XLENGINE, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		Services::xlDebugMessage("InitInstance() failed.");
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_XLENGINE));

	void *win_param[] = { (void *)_hwnd };
	GameLoop::init(win_param);

	// Main message loop:
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
	wcex.lpszClassName	= szWindowClass;
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
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;
	hInst = hInstance; // Store instance handle in our global variable

	//The window style.
	DWORD dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_BORDER | WS_MINIMIZEBOX;
	XLSettings* settings = Settings::get();
	if ( settings->flags&XL_FLAG_FULLSCREEN )
	{
		dwStyle = WS_POPUP;
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

	hWnd = CreateWindow(szWindowClass, szTitle, dwStyle, 0, 0, desiredSize.right-desiredSize.left, desiredSize.bottom-desiredSize.top, NULL, NULL, hInstance, NULL);
	if (!hWnd)
	{
		return FALSE;
	}

	_hwnd = hWnd;

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
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_KEYDOWN:
	{
		if (wParam >= 0 && wParam < 256)
		{
			Input::setKeyState(u32(wParam), true);
		}

		int keyCode = (int)wParam;
		if ( wParam == VK_SHIFT )
		{
			if ( GetAsyncKeyState(VK_LSHIFT) < 0 )
			{
				keyCode = VK_LSHIFT;
			}
			else if ( GetAsyncKeyState(VK_RSHIFT) < 0 )
			{
				keyCode = VK_RSHIFT;
			}
		}
		else if ( wParam == VK_CONTROL )
		{
			if ( GetAsyncKeyState(VK_LCONTROL) < 0 )
			{
				keyCode = VK_LCONTROL;
			}
			else if ( GetAsyncKeyState(VK_RCONTROL) < 0 )
			{
				keyCode = VK_RCONTROL;
			}
		}
		if (Services::get()->keyEvent)
		{
			keyCode = Input::mapVirtualKey(keyCode);
			if (keyCode > 0)
			{
				Services::get()->keyEvent(0, keyCode, 1);
			}
		}
	}
		break;
	case WM_KEYUP:
	{
		if (wParam >= 0 && wParam < 256)
		{
			Input::setKeyState(u32(wParam), false);
		}
		int keyCode = (int)wParam;
		if (Services::get()->keyEvent)
		{
			keyCode = Input::mapVirtualKey(keyCode);
			if (keyCode > 0)
			{
				Services::get()->keyEvent(0, keyCode, 0);
			}
		}
	}
		break;
	case WM_LBUTTONDOWN:
		Input::setMouseButtonState(Input::MouseLeft, true);
		break;
	case WM_RBUTTONDOWN:
		Input::setMouseButtonState(Input::MouseRight, true);
		break;
	case WM_MBUTTONDOWN:
		Input::setMouseButtonState(Input::MouseMiddle, true);
		break;
	case WM_XBUTTONDOWN:
		//to-do
		break;
	case WM_LBUTTONUP:
		Input::setMouseButtonState(Input::MouseLeft, false);
		break;
	case WM_RBUTTONUP:
		Input::setMouseButtonState(Input::MouseRight, false);
		break;
	case WM_MBUTTONUP:
		Input::setMouseButtonState(Input::MouseMiddle, false);
		break;
	case WM_XBUTTONUP:
		//to-do
		break;
	case WM_MOUSEMOVE:
		{
			Input::setMousePos( s16(lParam), s16(lParam >> 16) );
		}
		break;
	case WM_MOUSEWHEEL:
		Input::incMouseWheel( ((short)HIWORD(wParam)) > 0 ? 1 : -1 );
		break;
	case WM_CHAR:
		{
			if (wParam > 0 && wParam < 128)
			{
				Input::addChar( char(wParam) );
			}

			if (Services::get()->keyEvent)
			{
				Services::get()->keyEvent(wParam&0x7f, 0, 1);
			}
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
