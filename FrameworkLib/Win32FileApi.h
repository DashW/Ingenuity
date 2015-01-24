#pragma once

#include "FilesApi.h"
#include <Windows.h>
#include <string>
#include <vector>

namespace Ingenuity {
namespace Win32 {

class Window;

struct File : public Files::File
{
	HANDLE fileHandle;
	unsigned byteLength;

	virtual unsigned GetByteLength() override
	{
		return byteLength;
	}

	File(Files::Directory * directory) :
		Files::File(directory),
		fileHandle(0),
		byteLength(0) {}
	virtual ~File() {}
};

struct Directory : public Files::Directory
{
	std::wstring directoryPath;

	virtual std::wstring GetPath() override
	{
		return directoryPath;
	}

	Directory() : Files::Directory() {}
	virtual ~Directory() {}
};

class FileApi : public Files::Api
{
	struct FileEvent
	{
		OVERLAPPED * overlap;
		Files::Response * response;
		bool readEvent;
		char * buffer;
		unsigned bufferLength;
	};

	FileEvent activeEvents[MAXIMUM_WAIT_OBJECTS];
	HANDLE activeEventHandles[MAXIMUM_WAIT_OBJECTS];
	unsigned activeEventCount;

	Window * window;

	struct PendingFile
	{
		File * filePtr;
		std::wstring fullPath;
		Files::Response * response;

		PendingFile(File * filePtr, std::wstring fullPath) :
			filePtr(filePtr), fullPath(fullPath), response(0) {}
	};

	std::vector<PendingFile> pendingFiles;

	std::vector<Directory*> createdDirectories;

	Directory * tempKnownDirectory;

	void RemoveEvent(unsigned index);
	void HandleResponse(Files::Response * response);

public:
	FileApi(Window * window);
	virtual ~FileApi();

	virtual Files::Directory * GetKnownDirectory(Files::KnownDirectory option) override;
	virtual Files::Directory * GetSubDirectory(Files::Directory * root, const wchar_t * path);
	virtual void               PickFile(Files::Directory * directory, const wchar_t * extension, Files::Response * response) override;
	virtual void               Enumerate(Files::Directory * directory, bool subDirectories = true) override;

	virtual Files::File *      Open(Files::Directory * directory, const wchar_t * file) override;
	virtual void               OpenAndRead(Files::Directory * directory, const wchar_t * file, Files::Response * response) override;
	virtual void               Read(Files::File * file, Files::Response * response) override;
	virtual const char *       ReadChars(Files::File * file, unsigned & length, int offset = -1) override;
	virtual void               Close(Files::File ** file) override;

	virtual void Poll() override;
};

} // namespace Win32
} // namespace Ingenuity
