#include "Debug.h"

#include <cstdarg>

//#if windows build
#define _X86_
#include <debugapi.h>
//#endif

Debug::Debug()
	:
	logFile(0)
{
}

Debug::~Debug()
{
}

void Debug::LogOnce(unsigned uniqueKey, const char * fmt, ...)
{
	if(uniqueKeys.find(uniqueKey) == uniqueKeys.end())
	{
		va_list args;
		va_start(args,fmt);
		Log(fmt,args);
		va_end(args);

		uniqueKeys.insert(uniqueKey);
	}
}

void Debug::Log(const char * fmt, ...)
{
	char buffer[MAX_LINE_LENGTH];

	va_list args;
	va_start(args,fmt);
	int charsWritten = vsnprintf_s(buffer, MAX_LINE_LENGTH, _TRUNCATE, fmt, args);
	va_end(args);

	std::string logline(buffer);
	if(charsWritten == -1) logline += "...";
	int firstNewline = -1;
	while( (firstNewline = logline.find("\n")) != -1 )
	{
		logLines.back().append(logline.substr(0,firstNewline));
		logline = logline.substr(firstNewline + strlen("\n"));
		logLines.emplace_back();
	}
	logLines.back().append(logline);

	if(logFile)
	{
		FILE * f;
		_wfopen_s(&f, logFile, L"a");
		if(f)
		{
			fprintf_s(f, buffer);
			fclose(f);
		}
	}

//#if windows build
	OutputDebugStringA(buffer);
//#endif
}

void Debug::Break()
{
	if(IsDebuggerPresent()) __debugbreak();
}

void Debug::SetLogFile(const wchar_t* logFile)
{
	this->logFile = logFile;

	FILE *f;
	_wfopen_s(&f, logFile, L"w");
	if(f) fclose(f);
}

int Debug::NumLogLines() const
{
	return logLines.size();
}

const char * Debug::GetLogLine(unsigned line)
{
	if(line < logLines.size() && line > -1)
	{
		return logLines[line].c_str();
	}
	return 0;
}

Debug Debug::instance = Debug();
