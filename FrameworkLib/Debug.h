#pragma once

#include <set>
#include <string>
#include <vector>

class Debug // NOT CONVINCED.
{
	Debug();
	~Debug();

	static const int MAX_LINE_LENGTH = 100;
	static Debug instance;

	const wchar_t * logFile;

	std::vector<std::string> logLines;
	std::set<unsigned> uniqueKeys;

public:
	static Debug & Get() { return instance; }

	void Log(const char * fmt, ...);
	void LogOnce(unsigned uniqueKey, const char * fmt, ...);

	void Break();

	void SetLogFile(const wchar_t * logFile);

	int NumLogLines() const;
	const char * GetLogLine(unsigned line);
};
