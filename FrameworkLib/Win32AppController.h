// Class to control all common operations
// of a realtime application in Win32
// **** Front Controller Pattern ****
#pragma once

#define _X86_

#include <WinDef.h>

namespace Ingenuity {
class RealtimeApp;

namespace Win32 {
class Window;

class AppController
{
	Window * window;
	RealtimeApp * app;

public:
	AppController(HINSTANCE instance, RealtimeApp* app);
	~AppController();

	void Run();
	void OnResize();

	__int64 GetTimeStamp();

	volatile bool running;
};

} // namespace Win32
} // namespace Ingenuity
