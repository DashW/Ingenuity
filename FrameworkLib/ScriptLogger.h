#pragma once

#include <string>

namespace Ingenuity {

class ScriptLogger
{
public:

	ScriptLogger();
	~ScriptLogger();

	void Log(const char* fmt, ...);
	void Log(const char* fmt, char *args);

	void Clear();

	void SetLogFile(const wchar_t* logFile);

	void SetNumLines(int logLines);
	int GetNumLines() const;

	const char* GetLine(int line);

	static const int LOG_LINES = 20;

private:
	static const int MAX_LINE_LENGTH = 200;

	const wchar_t* logFile;

	int currentLine;
	int numLines;
	std::string* logLines;
};

} // namespace Ingenuity
