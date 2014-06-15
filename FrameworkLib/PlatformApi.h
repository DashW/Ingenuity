#pragma once

namespace Ingenuity {

/* Abstract class to interface with utility functions of the host OS */
class PlatformApi
{
public:
	virtual ~PlatformApi() {}

	virtual void BeginTimestamp(const wchar_t * name) = 0;
	virtual void EndTimestamp(const wchar_t * name) = 0;
	virtual float GetTimestampTime(const wchar_t * name) = 0;
};

} // namespace Ingenuity