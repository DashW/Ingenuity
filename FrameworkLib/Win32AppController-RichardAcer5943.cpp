#include "stdafx.h"
#include "Win32AppController.h"
#include "Win32Window.h"
#include "WinAsyncFileMgr.h"
#include "RealtimeApp.h"
#include "InputMgr.h"
#ifdef USE_XAUDIO2_AUDIOAPI
#include "XAudio2Api.h"
#else
#include "DummyAudioApi.h"
#endif

#ifdef USE_DX9_GPUAPI
#include "DX9Api.h"
#include "DX9VertApi.h"
#endif

#ifdef USE_DX11_GPUAPI
#include "DX11Api.h"
#endif

#include <Windows.h>

struct OnDestroyResponse : WindowEventResponse
{
	Win32AppController* owner;
	OnDestroyResponse(Win32AppController* creator) {owner = creator;}
	void Respond(WPARAM wparam, LPARAM lparam) override
	{
		owner->running = false;
	}
};

// The fate of XAudio2 is inexorably
// tied to the fate of the window
struct OnCloseResponse : WindowEventResponse
{
#ifdef USE_XAUDIO2_AUDIOAPI
	XAudio2Api* xAudio;
	OnCloseResponse(XAudio2Api* audio) {xAudio = audio;}
	void Respond(WPARAM wparam, LPARAM lparam) override
	{
		if(xAudio) delete xAudio;
	}
#endif
};

struct OnResizeResponse : WindowEventResponse
{
	Win32AppController* owner;
	OnResizeResponse(Win32AppController* creator) {owner = creator;}
	void Respond(WPARAM wparam, LPARAM lparam) override
	{
		owner->OnResize();
	}
};

Win32AppController::Win32AppController(HINSTANCE instance, RealtimeApp* realtimeApp)
	: app(realtimeApp)
	, window(new Win32Window(instance))
{
	window->onDestroy = new OnDestroyResponse(this);
	window->onExitSizeMove = new OnResizeResponse(this);
	window->onMaximize = window->onExitSizeMove;
	window->onUnmaximize = window->onExitSizeMove;

	app->files = new WinAsyncFileMgr();
	app->files->SetRunning(false);

#ifdef USE_DX9_GPUAPI
	app->gpu = new DX9_GpuApi(window->getHandle());
#endif
#ifdef USE_DX11_GPUAPI
	DX11_GpuApi* dx11Gpu = new DX11_GpuApi(window->getHandle());

	FileMgr_File* vtxShader = app->files->OpenFile(L"VertexShaderPosCol.cso");
	FileMgr_File* pixShader = app->files->OpenFile(L"PixelShaderPosCol.cso");

	app->files->Read(vtxShader,new DX11_VertexShaderResponse(dx11Gpu,VertexType_PosCol));
	app->files->Read(pixShader,new DX11_PixelShaderResponse(dx11Gpu,VertexType_PosCol));

	vtxShader = app->files->OpenFile(L"VertexShaderPosNor.cso");
	pixShader = app->files->OpenFile(L"PixelShaderPosNor.cso");

	app->files->Read(vtxShader,new DX11_VertexShaderResponse(dx11Gpu,VertexType_PosNor));
	app->files->Read(pixShader,new DX11_PixelShaderResponse(dx11Gpu,VertexType_PosNor));

	vtxShader = app->files->OpenFile(L"VertexShaderPosNorTex.cso");
	pixShader = app->files->OpenFile(L"PixelShaderPosNorTex.cso");

	app->files->Read(vtxShader,new DX11_VertexShaderResponse(dx11Gpu,VertexType_PosNorTex));
	app->files->Read(pixShader,new DX11_PixelShaderResponse(dx11Gpu,VertexType_PosNorTex));

	app->gpu = dx11Gpu;
#endif
	app->vtx = app->gpu->GetVertApi();

	window->inputMgr = new InputMgr();
	app->input = (InputApi*) window->inputMgr;

	RECT windowRect;
	GetClientRect(window->getHandle(), &windowRect);
	app->windowRect.left = static_cast<float>(windowRect.left);
	app->windowRect.top = static_cast<float>(windowRect.top);
	app->windowRect.right = static_cast<float>(windowRect.right);
	app->windowRect.bottom = static_cast<float>(windowRect.bottom);

#ifdef USE_XAUDIO2_AUDIOAPI
	XAudio2Api* xAudio = new XAudio2Api();
	window->onClose = new OnCloseResponse(xAudio);
	app->audio = xAudio;
#else
	app->audio = new DummyAudioApi();
#endif
}

void Win32AppController::Run()
{
	app->Begin();

	__int64 cntsPerSec = 0;
	QueryPerformanceFrequency((LARGE_INTEGER*)&cntsPerSec);
	float secsPerCnt = 1.0f / (float) cntsPerSec;

	__int64 startTimeStamp = 0;
	__int64 prevTimeStamp = 0;
	QueryPerformanceCounter((LARGE_INTEGER*)&prevTimeStamp);
	startTimeStamp = prevTimeStamp;

	running = true;

	app->files->SetRunning(true);

	while(running)
	{
		window->ProcessEvents();

		//running = false;

		app->files->Poll();

		__int64 curTimeStamp = 0;
		QueryPerformanceCounter((LARGE_INTEGER*)&curTimeStamp);
		float dt = (curTimeStamp - prevTimeStamp) * secsPerCnt;
		//if((1.0f/dt) > 65.0)
		//{
		//	int i = 3;
		//}
		prevTimeStamp = curTimeStamp;

		app->currentTime = curTimeStamp * secsPerCnt;

		app->Update(dt);

		if(app->gpu->isInitialised() && !app->gpu->isDeviceLost())
		{
			app->gpu->Clear();
			app->gpu->BeginScene(); 
			app->Draw();
			app->gpu->EndScene();
			app->gpu->Present();
		}
	}

	app->files->SetRunning(false);

	app->End();
}

void Win32AppController::OnResize()
{
	RECT windowRect;
	GetClientRect(window->getHandle(), &windowRect);
	app->gpu->SetScreenSize(windowRect.right,windowRect.bottom);
	app->windowRect.left = static_cast<float>(windowRect.left);
	app->windowRect.top = static_cast<float>(windowRect.top);
	app->windowRect.right = static_cast<float>(windowRect.right);
	app->windowRect.bottom = static_cast<float>(windowRect.bottom);
}

Win32AppController::~Win32AppController()
{
	delete app->files;
	delete app->gpu;
	delete app;
	delete window;
}

