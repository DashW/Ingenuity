#pragma once

#include "FilesApi.h"

struct IMFByteStream;

namespace Ingenuity {
namespace WinRT {

struct File : public Files::File
{
	Windows::Storage::StorageFile ^ storageFile;
	Windows::Storage::Streams::IRandomAccessStream ^ stream;
	IMFByteStream * byteStream;

	unsigned byteLength;

	virtual unsigned GetByteLength() override
	{
		return byteLength;
	}

	File(Files::Directory * directory) :
		Files::File(directory),
		byteStream(0),
		byteLength(0) {}
};

struct Directory : public Files::Directory
{
	Windows::Storage::StorageFolder ^ storageFolder;

	virtual std::wstring GetPath() override
	{
		if(storageFolder)
		{
			const wchar_t * pathData = storageFolder->Path->Data();
			if(pathData)
			{
				return std::wstring(pathData);
			}
		}
		return std::wstring();
	}
};

class FileApi : public Files::Api
{
	unsigned FindPathSeparator(std::wstring path, unsigned & separatorSize);
	void OpenFileAsync(Windows::Storage::StorageFolder ^ storageFolder, const wchar_t * path, File * winRTfile, Files::Response * response = 0);
	void OpenStreamAsync(Windows::Storage::StorageFile ^ storageFile, File * winRTfile, Files::Response * response = 0);
	void TriggerResponse(Files::Response * response);

	Directory knownDirectories[Files::NumDirectories];

public:
	FileApi();
	virtual ~FileApi();

	virtual Files::Directory * GetKnownDirectory(Files::KnownDirectory option) override;
	virtual void               PickFile(Files::Directory * directory, const wchar_t * extension, Files::Response * response) override;
	virtual void               Enumerate(Files::Directory * directory, bool subDirectories = true) override;

	virtual Files::File *      Open(Files::Directory * directory, const wchar_t * file) override;
	virtual void               OpenAndRead(Files::Directory * directory, const wchar_t * file, Files::Response * response) override;
	virtual void               Read(Files::File * file, Files::Response * response) override;
	virtual const char *       ReadChars(Files::File * file, unsigned & length, int offset = -1) override;
	virtual void               Close(Files::File ** file) override;

	virtual void Poll() override {}
};

} // namespace WinRT
} // namespace Ingenuity
