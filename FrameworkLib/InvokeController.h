#pragma once

#if !defined(WINAPI_FAMILY) || WINAPI_FAMILY != WINAPI_FAMILY_APP
#include <Win32AppController.h>
int WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance, PSTR cmdline, int showcmd)
{
	Win32AppController app(instance, new RealtimeApp());
	app.Run();
	return 0;
}
#else
#include <WinRTAppController.h>
[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^)
{
	WinRTAppDeadDrop::app = new RealtimeApp();
    auto winRTAppSource = ref new WinRTAppSource();
    Windows::ApplicationModel::Core::CoreApplication::Run(winRTAppSource);
    return 0;
}
#endif