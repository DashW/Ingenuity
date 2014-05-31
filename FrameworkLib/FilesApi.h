#pragma once

#include <string>

namespace Ingenuity {
namespace Files {

enum AsyncState
{
	NotStarted,
	InProgress,
	Succeeded,
	Failed
};

enum KnownDirectory
{
	FrameworkDir,
	AppDir,
	SaveDir,
	TempDir,

	NumDirectories
};

struct Directory
{
	volatile AsyncState openState;
	int numFiles;
	std::wstring * fileNames;

	virtual std::wstring GetPath() = 0;

	// INGENUITY SCRIPTING HACK :(
	bool isProjectDir;

	virtual ~Directory()
	{
		if(fileNames) delete[] fileNames;
	}

protected:
	Directory() :
		openState(NotStarted),
		numFiles(-1), // Not yet enumerated
		fileNames(0),
		isProjectDir(false) {}
};

struct File
{
	volatile AsyncState openState;
	Directory * directory;

	std::string lineString;
	unsigned lineOffset;

	virtual unsigned GetByteLength() = 0;

	virtual ~File() {}

protected:
	File(Directory * directory) :
		openState(NotStarted),
		directory(directory),
		lineOffset(0) {}
};

struct Response
{
	File * file;
	char * buffer;
	unsigned bufferLength;

	float progress;
	bool complete;

	bool closeOnComplete;
	bool deleteOnComplete;

	virtual void Respond() = 0;

	Response() :
		file(0),
		buffer(0),
		bufferLength(0),
		progress(0.0f),
		complete(false),
		closeOnComplete(true),
		deleteOnComplete(false) {}

	virtual ~Response()
	{
		if(buffer) delete[] buffer;
	}
};

/* Abstract class to interact with the file system of the host OS */
class Api {
public:
	virtual ~Api() {}

	virtual Directory *  GetKnownDirectory(KnownDirectory option) = 0;
	//virtual FileApi_Directory * PickDirectory() = 0;
	virtual void         PickFile(Directory * directory, const wchar_t * extension, Response * response) = 0;
	virtual void         Enumerate(Directory * directory, bool subDirectories = true) = 0;

	virtual File *       Open(Directory * directory, const wchar_t * file) = 0;
	virtual void         OpenAndRead(Directory * directory, const wchar_t * file, Response * response) = 0;
	virtual void         Read(File * file, Response * response) = 0;
	virtual const char * ReadChars(File * file, unsigned & length, int offset = -1) = 0;
	virtual void         Close(File ** file) = 0;

	virtual std::string  ReadLine(File * file, int offset = -1)
	{
		if(!file) return 0;
		if(offset > -1) file->lineOffset = (unsigned)offset;

		size_t newLinePos = file->lineString.find("\n");
		unsigned chunkSize = 128;

		while(newLinePos == std::string::npos)
		{
			const char * suffix = ReadChars(file, chunkSize, file->lineOffset);
			file->lineOffset += chunkSize;
			if(!suffix) break;
			file->lineString.append(suffix, chunkSize);
			delete[] suffix;
			newLinePos = file->lineString.find("\n");
		}

		if(newLinePos == std::string::npos)
		{
			std::string result = file->lineString;
			file->lineString = "";
			return result;
		}
		else
		{
			std::string result = file->lineString.substr(0, newLinePos + 1);
			file->lineString = file->lineString.substr(newLinePos + 1, file->lineString.size());
			return result;
		}
	}

	virtual void Poll() = 0;
};

} // namespace Files
} // namespace Ingenuity
