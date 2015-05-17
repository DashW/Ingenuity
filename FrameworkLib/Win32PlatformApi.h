#pragma once

#include "PlatformApi.h"

#include <map>
#include <string>

namespace Ingenuity {
namespace Win32 {

class Window;

class PlatformApi : public Ingenuity::PlatformApi
{
	std::map<std::wstring, __int64> timestampMap;
	std::map<std::wstring, float> timestampTimes;

	std::vector<std::string> commandLineArgs;

	Window * mainWindow;
	float secsPerCpuCount;

public:
	PlatformApi(Window * mainWindow);
	virtual ~PlatformApi() {}

	virtual void BeginTimestamp(const wchar_t * name) override;
	virtual void EndTimestamp(const wchar_t * name) override;
	virtual float GetTimestampTime(const wchar_t * name) override;

	virtual void GetScreenSize(unsigned & width, unsigned & height, unsigned screen = 0) const override;
	virtual void GetScreenSize(unsigned & width, unsigned & height, PlatformWindow * window) const override;

	virtual PlatformWindow * GetMainPlatformWindow() override;
	virtual PlatformWindow * CreatePlatformWindow(Gpu::Api * gpu, unsigned width = 1024, unsigned height = 600) override;

	virtual const std::vector<std::string> & GetCommandLineArgs() override { return commandLineArgs; }
};

} // namespace Win32
} // namespace Ingenuity