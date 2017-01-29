#pragma once

#include <string>
#include <vector>

namespace Ingenuity {

namespace Gpu
{
	class Api;
}

class PlatformWindow
{
public:
	virtual ~PlatformWindow() {}

	virtual void SetSize(unsigned width, unsigned height) = 0;
	virtual void SetUndecorated(bool undecorated) = 0;
	virtual void SetResizeable(bool resizeable) = 0;

	virtual unsigned GetWidth() = 0;
	virtual unsigned GetHeight() = 0;

protected:
	PlatformWindow() {}
};

/* Abstract class to interface with utility functions of the host OS */
class PlatformApi
{
public:
	virtual ~PlatformApi() {}

	virtual void BeginTimestamp(const wchar_t * name) = 0;
	virtual void EndTimestamp(const wchar_t * name) = 0;
	virtual float GetTimestampTime(const wchar_t * name) = 0;

	virtual void GetScreenSize(unsigned & width, unsigned & height, unsigned screen = 0) const = 0;
	virtual void GetScreenSize(unsigned & width, unsigned & height, PlatformWindow * window) const = 0;

	virtual PlatformWindow * GetMainPlatformWindow() = 0;
	virtual PlatformWindow * CreatePlatformWindow(Gpu::Api * gpu, unsigned width = 640, unsigned height = 480) = 0;

	virtual const std::vector<std::string> & GetCommandLineArgs() = 0;
};

} // namespace Ingenuity