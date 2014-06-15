#pragma once

#include "GpuStructs.h"

namespace Ingenuity {

class AssetMgr;
class InputState;
class StepMgr;
class PlatformApi;

namespace Audio {
class Api;
}
namespace Files {
class Api;
}
namespace Gpu {
class Api;
}
namespace Image {
class Api;
}

class RealtimeApp 
{
public:
	Audio::Api * audio;
	Files::Api * files;
	Gpu::Api   * gpu;
	Image::Api * imaging;
	PlatformApi * platform;

	AssetMgr   * assets;
	InputState * input;
	StepMgr    * steppables;

	float currentTime;

	volatile bool running;

	RealtimeApp() :
		audio(0),
		files(0),
		gpu(0),
		imaging(0),
		platform(0),
		assets(0),
		input(0),
		steppables(0),
		running(true) {}
	virtual ~RealtimeApp() {}
	virtual void Begin();
	virtual void End();
	virtual void Update(float secs);
	virtual void Draw();
};

} // namespace Ingenuity

#define BREAK_ALLOC_NUM -1

#if !defined(WINAPI_FAMILY) || WINAPI_FAMILY != WINAPI_FAMILY_APP
#include "Win32AppController.h"
#define MAIN() int WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance, PSTR cmdline, int showcmd) \
{ \
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF ); \
	_CrtSetBreakAlloc(BREAK_ALLOC_NUM); \
	Ingenuity::Win32::AppController app(instance, new Ingenuity::RealtimeApp()); \
	app.Run(); \
	return 0; \
}
#define MAIN_WITH(appName) void Ingenuity::RealtimeApp::Begin() {} \
	void Ingenuity::RealtimeApp::End() {} \
	void Ingenuity::RealtimeApp::Update(float) {} \
	void Ingenuity::RealtimeApp::Draw() {} \
int WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance, PSTR cmdline, int showcmd) \
{ \
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF ); \
	_CrtSetBreakAlloc(BREAK_ALLOC_NUM); \
	Ingenuity::Win32::AppController app(instance, new appName()); \
	app.Run(); \
	return 0; \
}
#else
#include <WinRTAppController.h>
#define MAIN() [Platform::MTAThread] \
int main(Platform::Array<Platform::String^>^) \
{ \
	Ingenuity::WinRT::AppDeadDrop::app = new Ingenuity::RealtimeApp(); \
    auto winRTAppSource = ref new Ingenuity::WinRT::AppSource(); \
    Windows::ApplicationModel::Core::CoreApplication::Run(winRTAppSource); \
    return 0; \
}
#define MAIN_WITH(appName) void Ingenuity::RealtimeApp::Begin() {} \
	void Ingenuity::RealtimeApp::End() {} \
	void Ingenuity::RealtimeApp::Update(float) {} \
	void Ingenuity::RealtimeApp::Draw() {} \
int main(Platform::Array<Platform::String^>^) \
{ \
	Ingenuity::WinRT::AppDeadDrop::app = new appName(); \
    auto winRTAppSource = ref new Ingenuity::WinRT::AppSource(); \
    Windows::ApplicationModel::Core::CoreApplication::Run(winRTAppSource); \
    return 0; \
}
#endif