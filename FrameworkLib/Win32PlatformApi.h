#pragma once

#include "PlatformApi.h"

#include <map>
#include <string>

namespace Ingenuity {
namespace Win32 {

class PlatformApi : public Ingenuity::PlatformApi
{
	std::map<std::wstring, __int64> timestampMap;
	std::map<std::wstring, float> timestampTimes;
	float secsPerCpuCount;

public:
	PlatformApi();
	virtual ~PlatformApi() {}

	virtual void BeginTimestamp(const wchar_t * name) override;
	virtual void EndTimestamp(const wchar_t * name) override;
	virtual float GetTimestampTime(const wchar_t * name) override;
};

} // namespace Win32
} // namespace Ingenuity