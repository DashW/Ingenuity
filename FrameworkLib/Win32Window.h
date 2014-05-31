#pragma once

#if !defined(WINAPI_FAMILY) || WINAPI_FAMILY != WINAPI_FAMILY_APP

#include <Windows.h>
#include <map>
#include "InputState.h"

namespace Ingenuity {
namespace Win32 {

const LPCWSTR WINDOW_CLASS_NAME = L"FrameworkLibWindow";

struct WindowEventResponse
{
	virtual void Respond(WPARAM, LPARAM){}
};

// Class to control window operations in a Win32 environment
class Window
{
	HWND handle;
	WNDCLASSEX windowClass;

	void InitializeEvents();

	bool undecorated;
	bool visible;
	bool maximized;
	bool destroyed;

	static std::map<HWND, Window*> windowsByHandle;

public:
	Window(HINSTANCE instance);
	~Window();

	void Show()
	{
		ShowWindow(handle, SW_SHOW);
		visible = true;
	}
	void Hide()
	{
		ShowWindow(handle, SW_HIDE);
		visible = false;
	}

	bool IsVisible() { return visible; }
	bool isMaximized() { return maximized; }
	bool isDestroyed() { return destroyed; }

	HWND getHandle() { return handle; }

	void setMaximized(bool b) { maximized = b; }

	// Window Events
	WindowEventResponse * onDestroy;
	WindowEventResponse * onResizing;
	WindowEventResponse * onMinmize;
	WindowEventResponse * onMaximize;
	WindowEventResponse * onUnmaximize;
	WindowEventResponse * onExitSizeMove;
	WindowEventResponse * onClose;
	WindowEventResponse * onActivate;

	InputState * inputMgr;

	static void ProcessEvents();
	static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
};

} // namespace Win32
} // namespace Ingenuity

#endif