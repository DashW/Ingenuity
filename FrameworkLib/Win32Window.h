#pragma once

#if !defined(WINAPI_FAMILY) || WINAPI_FAMILY != WINAPI_FAMILY_APP

#include "InputState.h"
#include "PlatformApi.h"

#include <map>
#include <Windows.h>

namespace Ingenuity {
namespace Win32 {

const LPCWSTR WINDOW_CLASS_NAME = L"IngenuityAppWindow";

class Window;

struct WindowEventResponse
{
	virtual void Respond(Window*, WPARAM, LPARAM){}
};

// Class to control window operations in a Win32 environment
class Window : public PlatformWindow
{
	static WNDCLASSEX windowClass;
	static bool classRegistered;

	HINSTANCE instance;
	HWND handle;
	unsigned width;
	unsigned height;
	bool undecorated;
	bool visible;
	bool maximized;
	bool closed;
	bool destroyed;

	static std::map<HWND, Window*> windowsByHandle;

	static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

public:
	Window(HINSTANCE instance, unsigned width = 1020, unsigned height = 600);
	virtual ~Window();

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
	bool isClosed() { return closed; }
	bool isDestroyed() { return destroyed; }

	HWND GetHandle() { return handle; }

	void SetMaximized(bool b) { maximized = b; }
	
	virtual void SetSize(unsigned width, unsigned height) override;
	virtual void SetUndecorated(bool undecorated) override;
	virtual void SetResizeable(bool resizeable) override;

	// Window Events
	WindowEventResponse * onDestroy;
	WindowEventResponse * onResizing;
	WindowEventResponse * onMinimize;
	WindowEventResponse * onMaximize;
	WindowEventResponse * onUnmaximize;
	WindowEventResponse * onExitSizeMove;
	WindowEventResponse * onClose;
	WindowEventResponse * onActivate;
	WindowEventResponse * onDelete;

	InputState * inputMgr;

	static void ProcessEvents();
};

} // namespace Win32
} // namespace Ingenuity

#endif