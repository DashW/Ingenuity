// Hello Direct3D Demo - text rendering - Richard Copperwaite

//#include "pch.h"
#include <GpuApi.h>
#include <RealtimeApp.h>

using namespace Ingenuity;

Gpu::Font* hellofont;

void RealtimeApp::Begin()
{
	gpu->SetClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	hellofont = gpu->CreateGpuFont(80, L"Arial", Gpu::FontStyle_Italic);
	//hellofont->pixelSpace = false;
}

void RealtimeApp::Update(float secs)
{
}

void RealtimeApp::Draw()
{
	unsigned screenWidth, screenHeight;
	gpu->GetBackbufferSize(screenWidth, screenHeight);
	gpu->DrawGpuText(hellofont, L"Hello Direct3D", float(screenWidth)/2.0f, float(screenHeight)/2.0f, true);
}

void RealtimeApp::End()
{
	delete hellofont;
}

MAIN()