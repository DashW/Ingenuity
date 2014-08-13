#include "stdafx.h"
#include "Win32AppController.h"
#include "Win32Window.h"
#include "Win32FileApi.h"
#include "Win32PlatformApi.h"
#include "InputState.h"

#include "RealtimeApp.h"

#ifdef USE_XAUDIO2_AUDIOAPI
#include "XAudio2Api.h"
#else
#include "DummyAudioApi.h"
#endif

#ifdef USE_DX9_GPUAPI
#include "DX9Api.h"
#endif
#ifdef USE_DX11_GPUAPI
#include "DX11Api.h"
#endif

#ifdef USE_FREE_IMAGEAPI
#include "FreeImageApi.h"
#endif
#ifdef USE_WIC_IMAGEAPI
#include "WICImageApi.h"
#endif

#ifdef USE_NEWTON_PHYSICSAPI
#include "NewtonApi.h"
#endif

#include <Windows.h>

namespace Ingenuity {

struct OnDestroyResponse : Win32::WindowEventResponse
{
	Win32::AppController * owner;
	OnDestroyResponse(Win32::AppController * creator) : owner(creator) {}
	void Respond(WPARAM wparam, LPARAM lparam) override
	{
		owner->windowClosed = true;
	}
};

// The fate of XAudio2 is inexorably
// tied to the fate of the window
#ifdef USE_XAUDIO2_AUDIOAPI
struct OnCloseResponse : Win32::WindowEventResponse
{
	RealtimeApp * app;
	OnCloseResponse(RealtimeApp * app) : app(app) {}
	void Respond(WPARAM wparam, LPARAM lparam) override
	{
		if(app->audio) { app->audio->Stop(); }
	}
};
#endif

struct OnResizeResponse : Win32::WindowEventResponse
{
	Win32::AppController* owner;
	OnResizeResponse(Win32::AppController* creator) { owner = creator; }
	void Respond(WPARAM wparam, LPARAM lparam) override
	{
		owner->OnResize();
	}
};

Win32::AppController::AppController(HINSTANCE instance, RealtimeApp * realtimeApp)
	: app(realtimeApp)
	, window(new Win32::Window(instance))
	, windowClosed(false)
{
	window->Hide();
	window->onDestroy = new OnDestroyResponse(this);
	window->onExitSizeMove = new OnResizeResponse(this);
	window->onMaximize = window->onExitSizeMove;
	window->onUnmaximize = window->onExitSizeMove;

	app->files = new Win32::FileApi(window);
	app->platform = new Win32::PlatformApi();

	app->steppables = new StepMgr();

	if(FAILED(CoInitializeEx(0, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE)))
	{
		OutputDebugString(L"Failed to initialize the COM library, required by the DX11 texture loader and win32 file API!");
	}

#ifdef USE_DX9_GPUAPI
	app->gpu = new DX9::Api(app->files, app->steppables, window->getHandle());
#else
#ifdef USE_DX11_GPUAPI
	app->gpu = new DX11::Api(app->files, window->getHandle());
#else
#error "Gpu API not defined!"
#endif
#endif

#ifdef USE_XAUDIO2_AUDIOAPI
	app->audio = new XAudio2::Api();
	window->onClose = new OnCloseResponse(app);
#else
	app->audio = new Audio::DummyApi();
#endif

#ifdef USE_FREE_IMAGEAPI
	app->imaging = new FreeImage::Api();
#else
#ifdef USE_WIC_IMAGEAPI
	app->imaging = new WIC::Api();
#else
#error "Image API not defined!"
#endif
#endif

#ifdef USE_NEWTON_PHYSICSAPI
	app->physics = new NewtonApi();
#else
#error "Physics API not defined!"
#endif

	app->input = window->inputMgr = new InputState();

	app->assets = new AssetMgr(app->files, app->gpu, app->imaging, app->steppables, app->audio);

	app->gpu->Initialize(app->assets);
}

Win32::AppController::~AppController()
{
	delete app->imaging;
	delete app->physics;
	delete app->steppables;
	delete app->assets;
	if(app->audio) delete app->audio; app->audio = 0;
	delete app->platform;
	delete app->files;
	delete app->gpu;
	delete app;
	delete window->inputMgr;
	delete window->onDestroy;
	delete window->onExitSizeMove;
	if(window->onClose) delete window->onClose;
	delete window;

	CoUninitialize();
}

void Win32::AppController::Run()
{
	app->Begin();

	__int64 cntsPerSec = 0;
	QueryPerformanceFrequency((LARGE_INTEGER*)&cntsPerSec);
	float secsPerCnt = 1.0f / (float)cntsPerSec;

	__int64 startTimeStamp = 0;
	__int64 prevTimeStamp = 0;
	QueryPerformanceCounter((LARGE_INTEGER*)&prevTimeStamp);
	startTimeStamp = prevTimeStamp;

	while(app->running && !windowClosed)
	{
		__int64 curTimeStamp = GetTimeStamp();
		float dt = (curTimeStamp - prevTimeStamp) * secsPerCnt;
		prevTimeStamp = curTimeStamp;
		app->currentTime = curTimeStamp * secsPerCnt;

		// Dispatch window events
		Win32::Window::ProcessEvents();

		// Poll the file system
		app->files->Poll();

		// Update steppables
		while(app->steppables->IsUpdateRequired((GetTimeStamp() - prevTimeStamp) * secsPerCnt))
		{
			app->steppables->Update();
		}

		// Update asset loads
		app->assets->Update();

		// Check critical assets are loaded
		if(!window->IsVisible())
		{
			if(app->assets->IsLoaded(AssetMgr::CRITICAL_TICKET))
			{
				app->gpu->OnCriticalLoad(app->assets);
				window->Show();
			}
		}

		// Update the input devices
		app->input->Update();

		// Update and draw the app
		if(window->IsVisible() && !app->gpu->isDeviceLost())
		{
			app->Update(dt);

			app->gpu->Clear();
			app->gpu->BeginScene();
			app->Draw();
			app->gpu->EndScene();
			app->gpu->Present();
		}
	}
	
	app->audio->Stop();

	app->End();
}

void Win32::AppController::OnResize()
{
	RECT windowRect;
	GetClientRect(window->getHandle(), &windowRect);
	app->gpu->OnScreenResize(windowRect.right, windowRect.bottom);
	//app->windowRect.left = static_cast<float>(windowRect.left);
	//app->windowRect.top = static_cast<float>(windowRect.top);
	//app->windowRect.right = static_cast<float>(windowRect.right);
	//app->windowRect.bottom = static_cast<float>(windowRect.bottom);
}

__int64 Win32::AppController::GetTimeStamp()
{
	__int64 curTimeStamp = 0;
	QueryPerformanceCounter((LARGE_INTEGER*)&curTimeStamp);
	return curTimeStamp;
}

} // namespace Ingenuity
