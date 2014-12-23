#include "Win32PlatformApi.h"

#include "Win32Window.h"
#include "GpuApi.h"

#include <Windows.h>

namespace Ingenuity {

namespace Win32 {

struct PlatformApiMonitorSizeRequest
{
	unsigned requestedMonitorIndex;
	unsigned currentMonitorIndex;
	RECT monitorRect;
};

BOOL CALLBACK PlatformApiMonitorSizeCallback(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
	PlatformApiMonitorSizeRequest * request = (PlatformApiMonitorSizeRequest*) dwData;

	if(request->currentMonitorIndex == request->requestedMonitorIndex)
	{
		MONITORINFO monitorInfo = { 0 };
		monitorInfo.cbSize = sizeof(MONITORINFO);
		GetMonitorInfo(hMonitor, &monitorInfo);
		request->monitorRect = monitorInfo.rcMonitor;
		return false;
	}

	request->currentMonitorIndex++;
	return true;
}

PlatformApi::PlatformApi(Window * mainWindow) :
	mainWindow(mainWindow)
{
	__int64 countsPerSec = 0;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
	secsPerCpuCount = 1.0f / (float)countsPerSec;
}

void PlatformApi::BeginTimestamp(const wchar_t * name)
{
	__int64 timeStamp = 0;
	QueryPerformanceCounter((LARGE_INTEGER*)&timeStamp);
	timestampMap[name] = timeStamp;
}

void PlatformApi::EndTimestamp(const wchar_t * name)
{
	if(timestampMap.find(name) == timestampMap.end()) return;

	__int64 timeStamp = 0;
	QueryPerformanceCounter((LARGE_INTEGER*)&timeStamp);
	__int64 previousTimestamp = timestampMap[name];

	timestampTimes[name] = float(timeStamp - previousTimestamp) * secsPerCpuCount;
}

float PlatformApi::GetTimestampTime(const wchar_t * name)
{
	std::map<std::wstring, float>::iterator timestampIt = timestampTimes.find(name);
	if(timestampIt == timestampTimes.end()) return 0.0f;
	return timestampIt->second;
}

void PlatformApi::GetScreenSize(unsigned & width, unsigned & height, unsigned screen) const
{
	PlatformApiMonitorSizeRequest request;
	request.requestedMonitorIndex = screen;
	request.currentMonitorIndex = 0;
	request.monitorRect = RECT{ 0 };

	EnumDisplayMonitors(0, 0, PlatformApiMonitorSizeCallback, (LPARAM) &request);

	width = request.monitorRect.right - request.monitorRect.left;
	height = request.monitorRect.bottom - request.monitorRect.top;
}

void PlatformApi::GetScreenSize(unsigned & width, unsigned & height, PlatformWindow * window) const
{
	Win32::Window * win32window = static_cast<Win32::Window*>(window);

	RECT monitorRect = { 0 };
	HMONITOR windowMonitor = MonitorFromWindow(win32window->GetHandle(), MONITOR_DEFAULTTONULL);
	if(windowMonitor)
	{
		MONITORINFO monitorInfo = { 0 };
		monitorInfo.cbSize = sizeof(MONITORINFO);
		GetMonitorInfo(windowMonitor, &monitorInfo);
		monitorRect = monitorInfo.rcMonitor;
	}

	width = monitorRect.right - monitorRect.left;
	height = monitorRect.bottom - monitorRect.top;
}

PlatformWindow * PlatformApi::GetMainPlatformWindow()
{
	return mainWindow;
}

PlatformWindow * PlatformApi::CreatePlatformWindow(Gpu::Api * gpu, unsigned width, unsigned height)
{
	Win32::Window * window = new Win32::Window(GetModuleHandle(0), width, height);

	gpu->OnWindowCreated(window);
	gpu->OnWindowResized(window, 0, 0);

	window->Show();
	
	return window;
}

} // namespace Win32

} // namespace Ingenuity