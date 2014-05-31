#include "WinRTFileApi.h"

#include <mfapi.h>
#include <mfidl.h>
#include <ppltasks.h>

using namespace concurrency;
using namespace Windows::ApplicationModel;
using namespace Windows::Foundation;
using namespace Windows::Storage;
using namespace Windows::Storage::AccessCache;
using namespace Windows::Storage::Pickers;
using namespace Windows::Storage::Streams;

namespace Ingenuity {

void WinRT::FileApi::TriggerResponse(Files::Response * response)
{
	response->Respond();
	if(response->closeOnComplete && response->file) Close(&(response->file));
	if(response->deleteOnComplete) delete response;
	else response->complete = true;
}

WinRT::FileApi::FileApi()
{
	knownDirectories[Files::AppDir].storageFolder = Package::Current->InstalledLocation;
	knownDirectories[Files::AppDir].openState = Files::Succeeded;

	knownDirectories[Files::SaveDir].storageFolder = ApplicationData::Current->RoamingFolder;
	knownDirectories[Files::SaveDir].openState = Files::Succeeded;

	knownDirectories[Files::TempDir].storageFolder = ApplicationData::Current->TemporaryFolder;
	knownDirectories[Files::TempDir].openState = Files::Succeeded;

	knownDirectories[Files::FrameworkDir].openState = Files::InProgress;

	concurrency::create_task(Package::Current->InstalledLocation->GetFolderAsync("FrameworkLibMetro"))
		.then([this](StorageFolder ^ folder)
	{
		knownDirectories[Files::FrameworkDir].storageFolder = folder;
		knownDirectories[Files::FrameworkDir].openState = folder ? Files::Succeeded : Files::Failed;
	});

	MFStartup(MF_VERSION);
}

WinRT::FileApi::~FileApi()
{
	MFShutdown();
}

unsigned WinRT::FileApi::FindPathSeparator(std::wstring pathString, unsigned & separatorSize)
{
	static const wchar_t * pathSeparators[4] = { L"//", L"\\\\", L"/", L"\\" };

	for(unsigned i = 0; i < 4; i++)
	{
		unsigned pathSeparatorIndex = pathString.find(pathSeparators[i]);
		if(pathSeparatorIndex != std::string::npos)
		{
			separatorSize = wcslen(pathSeparators[i]);
			return pathSeparatorIndex;
		}
	}

	return std::string::npos;
}

Files::Directory * WinRT::FileApi::GetKnownDirectory(Files::KnownDirectory option)
{
	return &knownDirectories[option];
}

void WinRT::FileApi::PickFile(Files::Directory * directory, const wchar_t * extension, Files::Response * response)
{
	if(!response) return;

	FileOpenPicker ^ picker = ref new FileOpenPicker();

	std::wstring extensions(extension);
	unsigned prevPos = 0;
	for(unsigned semiPos = extensions.find(L';'); semiPos < extensions.length(); semiPos = extensions.find(L';', semiPos + 1))
	{
		std::wstring extString = extensions.substr(prevPos, semiPos - prevPos);
		if(extString.find(L'*') == 0) extString = extString.substr(1);
		picker->FileTypeFilter->Append(ref new Platform::String(extString.data()));
		prevPos = semiPos + 1;
	}
	if(extensions.length() > 0)
	{
		std::wstring extString = extensions.substr(prevPos, extensions.length() - prevPos);
		if(extString.find(L'*') == 0) extString = extString.substr(1);
		picker->FileTypeFilter->Append(ref new Platform::String(extString.data()));
	}

	WinRT::Directory * winRTdir = static_cast<WinRT::Directory*>(directory);
	std::wstring dirFullPath(winRTdir->storageFolder->Path->Data());

	concurrency::create_task(picker->PickSingleFileAsync()).then([this, dirFullPath, response](StorageFile ^ storageFile)
	{
		if(storageFile)
		{
			std::wstring wideFilePath(storageFile->Path->Data());

			if(wideFilePath.substr(0, dirFullPath.length()).compare(dirFullPath) == 0)
			{
				wideFilePath = wideFilePath.substr(dirFullPath.length() + 1);
			}

			std::string shortFilePath(wideFilePath.begin(), wideFilePath.end());

			response->bufferLength = shortFilePath.length();
			response->buffer = new char[response->bufferLength + 1];
			shortFilePath.copy(response->buffer, response->bufferLength, 0);
			response->buffer[response->bufferLength] = '\0';
		}

		TriggerResponse(response);
	});
}

void WinRT::FileApi::Enumerate(Files::Directory * directory, bool subDirectories)
{
	WinRT::Directory * winRTdir = static_cast<WinRT::Directory*>(directory);

	if(winRTdir->fileNames > 0)
	{
		delete[] winRTdir->fileNames;
		winRTdir->fileNames = 0;
		winRTdir->numFiles = -1;
	}

	if(subDirectories)
	{
		create_task(winRTdir->storageFolder->GetItemsAsync())
			.then([this, winRTdir](Collections::IVectorView<IStorageItem^> ^ itemVec)
		{
			winRTdir->numFiles = (int)itemVec->Size;
			if(winRTdir->numFiles > 0)
			{
				winRTdir->fileNames = new std::wstring[winRTdir->numFiles];
				for(int i = 0; i < winRTdir->numFiles; ++i)
				{
					winRTdir->fileNames[i] = itemVec->GetAt(i)->Name->Data();
				}
			}
		});
	}
	else
	{
		create_task(winRTdir->storageFolder->GetFilesAsync())
			.then([this, winRTdir](Collections::IVectorView<StorageFile^> ^ itemVec)
		{
			winRTdir->numFiles = (int)itemVec->Size;
			if(winRTdir->numFiles > 0)
			{
				winRTdir->fileNames = new std::wstring[winRTdir->numFiles];
				for(int i = 0; i < winRTdir->numFiles; ++i)
				{
					winRTdir->fileNames[i] = itemVec->GetAt(i)->Name->Data();
				}
			}
		});
	}
}

// private
void WinRT::FileApi::OpenFileAsync(StorageFolder ^ storageFolder, const wchar_t * path, WinRT::File * winRTfile, Files::Response * response)
{
	std::wstring pathString(path);

	unsigned pathSeparatorSize = 0;
	unsigned pathSeparatorIndex = FindPathSeparator(pathString, pathSeparatorSize);
	if(pathSeparatorIndex != std::string::npos)
	{
		std::wstring subFolder = pathString.substr(0, pathSeparatorIndex);
		pathString = pathString.substr(pathSeparatorIndex + pathSeparatorSize, pathString.length());

		if(subFolder.compare(L"..") == 0)
		{
			create_task(storageFolder->GetParentAsync())
				.then([this, pathString, winRTfile, response](concurrency::task<StorageFolder^> folderTask)
			{
				try
				{
					StorageFolder ^ folder = folderTask.get();
					if(folder)
					{
						OpenFileAsync(folder, pathString.c_str(), winRTfile, response);
					}
					else
					{
						winRTfile->openState = Files::Failed;
						if(response)
						{
							TriggerResponse(response);
						}
					}
				}
				catch(Platform::Exception ^ exception)
				{
					winRTfile->openState = Files::Failed;
					if(response)
					{
						TriggerResponse(response);
					}
				}
			});
		}
		else
		{
			create_task(storageFolder->TryGetItemAsync(ref new Platform::String(subFolder.c_str())))
				.then([this, pathString, winRTfile, response](IStorageItem ^ item)
			{
				if(item && item->IsOfType(StorageItemTypes::Folder))
				{
					StorageFolder ^ folder = safe_cast<StorageFolder^>(item);
					OpenFileAsync(folder, pathString.c_str(), winRTfile, response);
				}
				else
				{
					winRTfile->openState = Files::Failed;
					if(response)
					{
						TriggerResponse(response);
					}
				}
			});
		}
	}
	else
	{
		create_task(storageFolder->TryGetItemAsync(ref new Platform::String(path)))
			.then([this, pathString, winRTfile, response](IStorageItem ^ item)
		{
			if(item && item->IsOfType(StorageItemTypes::File))
			{
				StorageFile ^ storageFile = safe_cast<StorageFile^>(item);
				winRTfile->storageFile = storageFile;
				OpenStreamAsync(storageFile, winRTfile, response);
			}
			else
			{
				winRTfile->openState = Files::Failed;
				if(response)
				{
					TriggerResponse(response);
				}
			}
		});
	}
}

//void WinRTFileApi::OpenFolderAsync(StorageFolder ^ storageFolder, const wchar_t * path, WinRTFileApi_Directory * winRTdir)
//{
//	std::wstring pathString(path);
//	std::wstring subFolder(path);
//
//	unsigned pathSeparatorIndex = pathString.find(L"//"); // should use findpathseparator!
//	if(pathSeparatorIndex != std::string::npos)
//	{
//		subFolder = pathString.substr(0, pathSeparatorIndex);
//		pathString = pathString.substr(pathSeparatorIndex + 2, pathString.length());
//	}
//	else
//	{
//		pathString = L"";
//	}
//
//	if(subFolder.compare(L"..") == 0)
//	{
//		create_task(storageFolder->GetParentAsync())
//			.then([this, pathString, winRTdir](concurrency::task<StorageFolder^> folderTask)
//		{
//			try
//			{
//				StorageFolder ^ folder = folderTask.get();
//				if(folder)
//				{
//					if(pathString.size() > 0)
//					{
//						OpenFolderAsync(folder, pathString.c_str(), winRTdir);
//					}
//					else
//					{
//						winRTdir->storageFolder = folder;
//						winRTdir->openState = Succeeded;
//					}
//				}
//				else
//				{
//					winRTdir->openState = Failed;
//				}
//			}
//			catch(Platform::Exception ^ exception)
//			{
//				winRTdir->openState = Failed;
//			}
//		});
//	}
//	else
//	{
//		create_task(storageFolder->TryGetItemAsync(ref new Platform::String(subFolder.c_str())))
//			.then([this, pathString, winRTdir](IStorageItem ^ item)
//		{
//			if(item && item->IsOfType(StorageItemTypes::Folder))
//			{
//				StorageFolder ^ folder = safe_cast<StorageFolder^>(item);
//				if(pathString.size() > 0)
//				{
//					OpenFolderAsync(folder, pathString.c_str(), winRTdir);
//				}
//				else
//				{
//					winRTdir->storageFolder = folder;
//					winRTdir->openState = Succeeded;
//				}
//			}
//			else
//			{
//				winRTdir->openState = Failed;
//			}
//		});
//	}
//}

void WinRT::FileApi::OpenStreamAsync(StorageFile ^ storageFile, WinRT::File * winRTfile, Files::Response * response)
{
	create_task(storageFile->OpenAsync(FileAccessMode::Read)).then([this, winRTfile, response](task<IRandomAccessStream^> streamTask)
	{
		try
		{
			IRandomAccessStream ^ stream = streamTask.get();
			winRTfile->stream = stream;
			winRTfile->byteLength = (unsigned)stream->Size;
			winRTfile->openState = Files::Succeeded;

			MFCreateMFByteStreamOnStreamEx((IUnknown *)stream, &winRTfile->byteStream);

			if(response)
			{
				Read(winRTfile, response);
			}
		}
		catch(Platform::Exception ^ exception)
		{
			winRTfile->storageFile = nullptr;
			winRTfile->openState = Files::Failed;

			if(response)
			{
				TriggerResponse(response);
			}
		}
	});
}

Files::File * WinRT::FileApi::Open(Files::Directory * directory, const wchar_t * path)
{
	if(!directory || !path) return 0;

	WinRT::Directory * winRTdir = static_cast<WinRT::Directory*>(directory);
	StorageFolder ^ storageFolder = winRTdir->storageFolder;
	WinRT::File * winRTfile = new WinRT::File(directory);

	winRTfile->openState = Files::InProgress;
	OpenFileAsync(storageFolder, path, winRTfile);

	return winRTfile;
}

void WinRT::FileApi::OpenAndRead(Files::Directory * directory, const wchar_t * path, Files::Response * response)
{
	if(!directory || !path) return;

	WinRT::Directory * winRTdir = static_cast<WinRT::Directory*>(directory);
	StorageFolder ^ storageFolder = winRTdir->storageFolder;
	WinRT::File * winRTfile = new WinRT::File(directory);

	winRTfile->openState = Files::InProgress;
	OpenFileAsync(storageFolder, path, winRTfile, response);
}

void WinRT::FileApi::Read(Files::File * file, Files::Response * response)
{
	if(!file || !response) return;

	WinRT::File * winRTfile = static_cast<WinRT::File*>(file);
	if(!winRTfile->stream || winRTfile->stream->Size > UINT32_MAX) return;

	//response->Initialise(this, file);

	unsigned streamSize = unsigned(winRTfile->stream->Size);

	IAsyncOperationWithProgress<IBuffer^, unsigned> ^ readOperation =
		winRTfile->stream->ReadAsync(ref new Buffer(streamSize), unsigned(winRTfile->stream->Size), InputStreamOptions::None);

	readOperation->Progress = ref new AsyncOperationProgressHandler<IBuffer^, unsigned>([response, streamSize]
		(IAsyncOperationWithProgress<IBuffer^, unsigned> ^ asyncInfo, unsigned progressInfo)
	{
		response->progress = float(progressInfo) / float(streamSize);
	});

	create_task(readOperation).then([this, response](IBuffer ^ buffer)
	{
		DataReader ^ dataReader = DataReader::FromBuffer(buffer);

		response->buffer = new char[buffer->Length + 1];
		response->bufferLength = buffer->Length;
		response->buffer[buffer->Length] = '\0'; // stringification

		Platform::ArrayReference<unsigned char> arrayRef(
			(unsigned char*)(response->buffer), response->bufferLength, false);
		dataReader->ReadBytes(arrayRef);

		TriggerResponse(response);
	});
}

const char * WinRT::FileApi::ReadChars(Files::File * file, unsigned & length, int offset)
{
	if(!file) return 0;

	WinRT::File * winRTfile = static_cast<WinRT::File*>(file);
	if(!winRTfile->byteStream) return 0;

	unsigned long long currentPosition = 0;
	winRTfile->byteStream->GetCurrentPosition(&currentPosition);

	if(offset != (int)currentPosition && offset < (int)winRTfile->byteLength && offset > -1)
	{
		winRTfile->byteStream->SetCurrentPosition(offset);
	}

	if(offset + length > winRTfile->byteLength)
		length = winRTfile->byteLength - offset;

	if(length > 0)
	{
		char * chunk = new char[length];
		unsigned long bytesRead = 0;
		winRTfile->byteStream->Read((BYTE*)chunk, length, &bytesRead);
		return chunk;
	}

	return 0;
}

void WinRT::FileApi::Close(Files::File ** file)
{
	if(!file || !(*file)) return;

	//WinRTFileApi_File * winRTfile = static_cast<WinRTFileApi_File*>(*file);
	//if(winRTfile->stream) winRTfile->stream->Dispose();

	delete (*file);
	(*file) = 0;
}

} // namespace Ingenuity
