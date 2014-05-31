#include "ScriptLogger.h"

#include <cstdarg>

namespace Ingenuity {

ScriptLogger::ScriptLogger()
	:
	logFile(0),
	currentLine(0),
	numLines(LOG_LINES)
{
	logLines = new std::string[numLines];
}

ScriptLogger::~ScriptLogger()
{
	delete[] logLines;
}

void ScriptLogger::Log(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	Log(fmt, args);
	va_end(args);
}

void ScriptLogger::Log(const char* fmt, char* args)
{
	char* argStart = args;

	if(logFile)
	{
		FILE *f;
		_wfopen_s(&f, logFile, L"a");
		if(f)
		{
			vfprintf_s(f, fmt, args);
			fclose(f);
		}
	}

	args = argStart;

	char buffer[MAX_LINE_LENGTH];
	int charsWritten = vsnprintf_s(buffer, MAX_LINE_LENGTH, _TRUNCATE, fmt, args);
	std::string logline(buffer);
	if(charsWritten == -1) logline += "...";

	int firstNewline = -1;
	while((firstNewline = logline.find("\n")) != -1)
	{
		logLines[currentLine].append(logline.substr(0, firstNewline));
		logline = logline.substr(firstNewline + strlen("\n"));
		currentLine = (currentLine + 1 < numLines ? currentLine + 1 : 0);
		logLines[currentLine].clear();
	}
	logLines[currentLine].append(logline.c_str());

	args = argStart;

	vprintf(fmt, args);

	args = argStart;
}

void ScriptLogger::Clear()
{
	for(int i = 0; i < LOG_LINES; i++)
	{
		logLines[i].clear();
	}
}

void ScriptLogger::SetLogFile(const wchar_t* logFile)
{
	this->logFile = logFile;

	FILE *f;
	_wfopen_s(&f, logFile, L"w");
	if(f) fclose(f);
}

void ScriptLogger::SetNumLines(int lines)
{
	delete[] logLines;
	numLines = lines;
	logLines = new std::string[numLines];
}

int ScriptLogger::GetNumLines() const
{
	return numLines;
}

const char* ScriptLogger::GetLine(int line)
{
	if(line < numLines && line > -1)
	{
		line = currentLine + line + 1;
		if(line >= numLines)
			line -= numLines;
		return logLines[line].c_str();
	}
	return 0;
}

} // namespace Ingenuity
