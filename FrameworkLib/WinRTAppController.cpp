#include "stdafx.h"
#include "WinRTAppController.h"
#include "WinRTFileApi.h"
#include "WinRTPlatformApi.h"
#include "DX11Api.h"
#include "InputState.h"
#include "AssetMgr.h"

#ifdef USE_XAUDIO2_AUDIOAPI
#include "XAudio2Api.h"
#else
#include "DummyAudioApi.h"
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

using namespace Windows::ApplicationModel::Core;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;

namespace Ingenuity {

RealtimeApp * WinRT::AppDeadDrop::app = 0;

WinRT::AppController::AppController()
	: windowClosed(false)
	, windowActivated(false)
{
	app = WinRT::AppDeadDrop::app;
}

void WinRT::AppController::Initialize(Windows::ApplicationModel::Core::CoreApplicationView^ appView)
{
	app->files = new WinRT::FileApi();
	app->platform = new WinRT::PlatformApi();

#ifdef USE_XAUDIO2_AUDIOAPI
	app->audio = new XAudio2::Api();
#else
	app->audio = new Audio::DummyApi();
#endif

	app->steppables = new StepMgr();

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

	appView->Activated +=
		ref new TypedEventHandler<CoreApplicationView^, Windows::ApplicationModel::Activation::IActivatedEventArgs^>
		(this, &WinRT::AppController::OnActivated);

	CoreApplication::Suspending += ref new EventHandler<Windows::ApplicationModel::SuspendingEventArgs^>
		(this, &WinRT::AppController::OnSuspending);

	CoreApplication::Resuming += ref new EventHandler<Platform::Object^>
		(this, &WinRT::AppController::OnResuming);
}

void WinRT::AppController::SetWindow(CoreWindow ^ window)
{
	DX11::Api * dx11gpu = new DX11::Api(app->files, window);

	app->assets = new AssetMgr(app->files, dx11gpu, app->imaging, app->steppables, app->audio);

	dx11gpu->Initialize(app->assets);

	app->gpu = dx11gpu;

	app->input = new InputState();

	window->Closed += ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>
		(this, &WinRT::AppController::OnWindowClosed);

	window->PointerPressed += ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>
		(this, &WinRT::AppController::OnPointerPressed);
	window->PointerReleased += ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>
		(this, &WinRT::AppController::OnPointerReleased);
	window->PointerMoved += ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>
		(this, &WinRT::AppController::OnPointerMoved);

	window->KeyDown += ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>
		(this, &WinRT::AppController::OnKeyDown);
	window->KeyUp += ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>
		(this, &WinRT::AppController::OnKeyUp);
	window->CharacterReceived += ref new TypedEventHandler<CoreWindow^, CharacterReceivedEventArgs^>
		(this, &WinRT::AppController::OnCharacterRecieved);
	
	// Hide pointer feedback!
	PointerVisualizationSettings ^ pointerSettings = PointerVisualizationSettings::GetForCurrentView();
	pointerSettings->IsContactFeedbackEnabled = false;
}

void WinRT::AppController::Load(Platform::String^ entryPoint)
{

}

void WinRT::AppController::Run()
{
	app->Begin();

	__int64 cntsPerSec = 0;
	QueryPerformanceFrequency((LARGE_INTEGER*)&cntsPerSec);
	float secsPerCnt = 1.0f / (float)cntsPerSec;

	__int64 startTimeStamp = 0;
	__int64 prevTimeStamp = 0;
	prevTimeStamp = GetTimeStamp();
	startTimeStamp = prevTimeStamp;

	while(!windowClosed)
	{
		__int64 curTimeStamp = GetTimeStamp();
		float dt = (curTimeStamp - prevTimeStamp) * secsPerCnt;
		prevTimeStamp = curTimeStamp;
		app->currentTime = curTimeStamp * secsPerCnt;

		// Dispatch window events
		CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);

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
		if(!windowActivated)
		{
			if(app->assets->IsLoaded(AssetMgr::CRITICAL_TICKET))
			{
				app->gpu->OnCriticalLoad(app->assets);
				CoreWindow::GetForCurrentThread()->Activate();
				windowActivated = true;
			}
		}

		// Update input devices
		app->input->Update();

		// Update and draw the app
		if(windowActivated)
		{
			app->Update(dt);

			app->gpu->Clear();
			app->gpu->BeginScene();
			app->Draw();
			app->gpu->EndScene();
			app->gpu->Present();
		}
	}

	app->End();
}

void WinRT::AppController::Uninitialize()
{
	if(app)
	{
		if(app->physics) delete app->physics;
		if(app->assets) delete app->assets;
		if(app->input) delete app->input;
		if(app->imaging) delete app->imaging;
		if(app->steppables) delete app->steppables;
		if(app->audio) delete app->audio;
		if(app->platform) delete app->platform;
		if(app->files) delete app->files;
		if(app->gpu) delete app->gpu;
		delete app;
	}
}

__int64 WinRT::AppController::GetTimeStamp()
{
	__int64 timeStamp = 0;
	QueryPerformanceCounter((LARGE_INTEGER*)&timeStamp);
	return timeStamp;
}

void WinRT::AppController::OnWindowClosed(CoreWindow ^ sender, CoreWindowEventArgs ^ args)
{
	windowClosed = true;
}

void WinRT::AppController::OnActivated(CoreApplicationView ^ applicationView,
	Windows::ApplicationModel::Activation::IActivatedEventArgs ^ args)
{
	if(app->input)
	{
		Windows::Foundation::Point mousePoint = applicationView->CoreWindow->PointerPosition;
		InputState * input = static_cast<InputState*>(app->input);
		input->mouse.x = static_cast<int>(mousePoint.X);
		input->mouse.y = static_cast<int>(mousePoint.Y);
	}
}

void WinRT::AppController::OnSuspending(Platform::Object ^ sender,
	Windows::ApplicationModel::SuspendingEventArgs ^ args)
{
	Windows::ApplicationModel::SuspendingDeferral^ deferral = args->SuspendingOperation->GetDeferral();

	// Save application state after requesting a deferral. Holding a deferral
	// indicates that the application is busy performing suspending operations.
	// Be aware that a deferral may not be held indefinitely. After about five
	// seconds, the application will be forced to exit.

	deferral->Complete();
}

void WinRT::AppController::OnPointerPressed(CoreWindow^ sender, PointerEventArgs^ args)
{
	if(app->input)
	{
		InputState * input = static_cast<InputState*>(app->input);
		input->mouse.x = static_cast<int>(args->CurrentPoint->Position.X);
		input->mouse.y = static_cast<int>(args->CurrentPoint->Position.Y);
		input->mouse.leftDown = true;
		input->mouse.left = true;
	}
}
void WinRT::AppController::OnPointerReleased(CoreWindow^ sender, PointerEventArgs^ args)
{
	if(app->input)
	{
		InputState * input = static_cast<InputState*>(app->input);
		input->mouse.x = static_cast<int>(args->CurrentPoint->Position.X);
		input->mouse.y = static_cast<int>(args->CurrentPoint->Position.Y);
		input->mouse.leftUp = true;
		input->mouse.left = false;
	}
}
void WinRT::AppController::OnPointerMoved(CoreWindow^ sender, PointerEventArgs^ args)
{
	if(app->input)
	{
		InputState* input = static_cast<InputState*>(app->input);
		input->mouse.x = static_cast<int>(args->CurrentPoint->Position.X);
		input->mouse.y = static_cast<int>(args->CurrentPoint->Position.Y);
	}
}
void WinRT::AppController::OnKeyDown(CoreWindow ^ sender, KeyEventArgs ^ args)
{
	if(app->input)
	{
		InputState * input = static_cast<InputState*>(app->input);
		unsigned scancode = args->KeyStatus.ScanCode;
		input->keyboard.downKeys[scancode] = true;
		input->keyboard.keys[scancode] = true;
	}
}

void WinRT::AppController::OnKeyUp(CoreWindow ^ sender, KeyEventArgs ^ args)
{
	if(app->input)
	{
		InputState * input = static_cast<InputState*>(app->input);
		unsigned scancode = args->KeyStatus.ScanCode;
		input->keyboard.upKeys[scancode] = true;
		input->keyboard.keys[scancode] = false;
	}
}

void WinRT::AppController::OnCharacterRecieved(CoreWindow ^ sender, CharacterReceivedEventArgs ^ args)
{
	if(app->input)
	{
		InputState * input = static_cast<InputState*>(app->input);
		input->keyboard.text += args->KeyCode;
	}
}

void WinRT::AppController::OnResuming(Platform::Object^ sender, Platform::Object^ args)
{
}

Windows::ApplicationModel::Core::IFrameworkView^ WinRT::AppSource::CreateView()
{
	return ref new WinRT::AppController();
}

} // namespace Ingenuity
