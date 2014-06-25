#if !defined(WINAPI_FAMILY) || WINAPI_FAMILY != WINAPI_FAMILY_APP

#include "stdafx.h"
#include "Win32Window.h"
#include <string>
#include <dwmapi.h>

namespace Ingenuity {

std::map<HWND, Win32::Window*> Win32::Window::windowsByHandle;

LRESULT CALLBACK Win32::Window::WndProc(HWND handle, UINT message, WPARAM wparam, LPARAM lparam)
{
	if(windowsByHandle[handle])
	{
		Win32::Window* window = windowsByHandle[handle];
		switch(message)
		{
		case WM_DESTROY:
			if(window->onDestroy) { window->onDestroy->Respond(wparam, lparam); }
			break;

		case WM_CLOSE:
			if(window->onClose) { window->onClose->Respond(wparam, lparam); }
			break;

		case WM_ACTIVATE:
			if(window->onActivate) { window->onActivate->Respond(wparam, lparam); }
			break;

		case WM_SIZE:
			switch(wparam)
			{
			case SIZE_RESTORED:
				if(window->isMaximized())
				{
					window->setMaximized(false);
					if(window->onUnmaximize) { window->onUnmaximize->Respond(wparam, lparam); }
				}
				else
				{
					if(window->onResizing) { window->onResizing->Respond(wparam, lparam); }
				}
				break;
			case SIZE_MINIMIZED:
				if(window->onMinmize) { window->onMinmize->Respond(wparam, lparam); }
				break;
			case SIZE_MAXIMIZED:
				window->setMaximized(true);
				if(window->onMaximize) { window->onMaximize->Respond(wparam, lparam); }
				break;
			}
			break;

		case WM_EXITSIZEMOVE:
			if(window->onExitSizeMove) { window->onExitSizeMove->Respond(wparam, lparam); }
			break;

		case WM_KEYDOWN:
		{
			UINT scancode = (lparam & 0x00ff0000) >> 16;
			window->inputMgr->keyboard.downKeys[scancode] = true;
			window->inputMgr->keyboard.keys[scancode] = true;
		}
			break;

		case WM_KEYUP:
		{
			UINT scancode = (lparam & 0x00ff0000) >> 16;
			window->inputMgr->keyboard.upKeys[scancode] = true;
			window->inputMgr->keyboard.keys[scancode] = false;
		}
			break;

		case WM_CHAR:
			window->inputMgr->keyboard.text += (char)wparam;;
			break;

		case WM_LBUTTONDOWN:
			window->inputMgr->mouse.leftDown = true;
			window->inputMgr->mouse.left = true;
			break;

		case WM_LBUTTONUP:
			window->inputMgr->mouse.leftUp = true;
			window->inputMgr->mouse.left = false;
			break;

		case WM_RBUTTONDOWN:
			window->inputMgr->mouse.rightDown = true;
			window->inputMgr->mouse.right = true;
			break;

		case WM_RBUTTONUP:
			window->inputMgr->mouse.rightUp = true;
			window->inputMgr->mouse.right = false;
			break;

		case WM_MOUSEMOVE:
			if(!window->inputMgr->mouse.over)
			{
				window->inputMgr->mouse.entered = true;

				TRACKMOUSEEVENT request;
				request.cbSize = sizeof(TRACKMOUSEEVENT);
				request.dwFlags = TME_LEAVE;
				request.hwndTrack = handle;
				request.dwHoverTime = HOVER_DEFAULT;
				TrackMouseEvent(&request);
			}
			window->inputMgr->mouse.over = true;
			window->inputMgr->mouse.x = LOWORD(lparam);
			window->inputMgr->mouse.y = HIWORD(lparam);
			break;

		case WM_MOUSELEAVE:
			if(window->inputMgr->mouse.over) window->inputMgr->mouse.exited = true;
			window->inputMgr->mouse.over = false;
			window->inputMgr->mouse.left = false;
			window->inputMgr->mouse.middle = false;
			window->inputMgr->mouse.right = false;
			break;

		case WM_TOUCH:
			unsigned numTouches = wparam;
			HTOUCHINPUT touchHandle = (HTOUCHINPUT)lparam;
			TOUCHINPUT touches[10];
			if(numTouches > 10) numTouches = 10;
			if(GetTouchInputInfo(touchHandle, numTouches, &touches[0], sizeof(TOUCHINPUT)))
			{

			}
			break;
		}
	}
	return DefWindowProc(handle, message, wparam, lparam);
}

Win32::Window::Window(HINSTANCE instance) :
handle(0),
windowClass({ 0 }),
undecorated(false),
visible(false),
maximized(false),
destroyed(false),
fullscreen(false),
onDestroy(0),
onResizing(0),
onMinmize(0),
onMaximize(0),
onUnmaximize(0),
onExitSizeMove(0),
onClose(0),
onActivate(0)
{
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = WndProc;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = instance;
	windowClass.hIcon = LoadIcon(0, IDI_APPLICATION);
	windowClass.hCursor = LoadCursor(0, IDC_ARROW);
	windowClass.hbrBackground = 0;
	windowClass.lpszMenuName = 0;
	windowClass.lpszClassName = WINDOW_CLASS_NAME;
	windowClass.hIconSm = LoadIcon(0, IDI_APPLICATION);

	if(!RegisterClassEx(&windowClass))
	{
		//FAILURE
	}

	RECT r = { 0, 0, 1024, 600 };
	if(!undecorated)
	{
		// FIXME - This function doesn't work, backbuffer size is wrong!
		AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW, false);
	}
	else if(fullscreen)
	{
		r.right = GetSystemMetrics(SM_CXSCREEN);
		r.bottom = GetSystemMetrics(SM_CYSCREEN);
	}
	handle = CreateWindowEx(
		WS_EX_COMPOSITED,
		WINDOW_CLASS_NAME,
		L"Main Window Caption",
		undecorated ? WS_POPUP : WS_OVERLAPPEDWINDOW,
		undecorated ? 0 : 100,
		undecorated ? 0 : 100,
		r.right,
		r.bottom,
		0,
		0,
		instance,
		0);

	if(!handle)
	{
		//FAILURE
	}

	MARGINS g_mgDWMMargins = { -1, -1, -1, -1 };
	DwmExtendFrameIntoClientArea(handle, &g_mgDWMMargins); // For transparency

	InitializeEvents();

	windowsByHandle[handle] = this;

	//ShowWindow(handle, SW_SHOW);
	UpdateWindow(handle);
}

Win32::Window::~Window()
{
	windowsByHandle.erase(handle);
}

void Win32::Window::InitializeEvents()
{
	onDestroy =
		onResizing =
		onMinmize =
		onMaximize =
		onUnmaximize =
		onExitSizeMove =
		onClose =
		onActivate =
		0;
}

void Win32::Window::ProcessEvents()
{
	MSG msg;

	ZeroMemory(&msg, sizeof(MSG));

	while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

} // namespace Ingenuity

#endif