#include "Win32PlatformApi.h"

#include <Windows.h>

namespace Ingenuity {

Win32::PlatformApi::PlatformApi()
{
	__int64 countsPerSec = 0;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
	secsPerCpuCount = 1.0f / (float)countsPerSec;
}

void Win32::PlatformApi::BeginTimestamp(const wchar_t * name)
{
	__int64 timeStamp = 0;
	QueryPerformanceCounter((LARGE_INTEGER*)&timeStamp);
	timestampMap[name] = timeStamp;
}

void Win32::PlatformApi::EndTimestamp(const wchar_t * name)
{
	if(timestampMap.find(name) == timestampMap.end()) return;

	__int64 timeStamp = 0;
	QueryPerformanceCounter((LARGE_INTEGER*)&timeStamp);
	__int64 previousTimestamp = timestampMap[name];

	timestampTimes[name] = float(timeStamp - previousTimestamp) * secsPerCpuCount;
}

float Win32::PlatformApi::GetTimestampTime(const wchar_t * name)
{
	std::map<std::wstring, float>::iterator timestampIt = timestampTimes.find(name);
	if(timestampIt == timestampTimes.end()) return 0.0f;
	return timestampIt->second;
}

} // namespace Ingenuity