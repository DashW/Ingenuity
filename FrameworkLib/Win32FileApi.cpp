#include "stdafx.h"

#include "Win32FileApi.h"
#include "Win32Window.h"

#include <Shobjidl.h>
#include <Shlwapi.h>
#include <sstream>

#pragma comment(linker, "\"/manifestdependency:type='Win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

namespace Ingenuity {

Win32::FileApi::FileApi(Win32::Window * window) : activeEventCount(0), window(window)
{
	tempKnownDirectory = new Win32::Directory();
	tempKnownDirectory->directoryPath = L"";
}

Win32::FileApi::~FileApi()
{
	for(unsigned i = 0; i < createdDirectories.size(); ++i)
	{
		delete createdDirectories[i];
	}
	delete tempKnownDirectory;
}

void Win32::FileApi::HandleResponse(Files::Response * response)
{
	response->Respond();
	if(response->closeOnComplete) Close(&(response->file));
	if(response->deleteOnComplete) delete response;
	else response->complete = true;
}

Files::Directory * Win32::FileApi::GetKnownDirectory(Files::KnownDirectory option)
{
	// Should really use SHGetKnownFolderPath for the saveDir, and GetTempPath for the tempDir

	return tempKnownDirectory;
}

Files::Directory * Win32::FileApi::GetSubDirectory(Files::Directory * root, const wchar_t * path)
{
	if(root != 0 && path != 0 && wcslen(path) != 0)
	{
		for(unsigned i = 0; i < createdDirectories.size(); ++i)
		{
			if(createdDirectories[i]->directoryPath.compare(path) == 0)
			{
				return createdDirectories[i];
			}
		}
		Directory * newDirectory = new Directory();
		newDirectory->directoryPath = root->GetPath();
		newDirectory->directoryPath += path;
		createdDirectories.push_back(newDirectory);
		return newDirectory;
	}
	return 0;
}

COMDLG_FILTERSPEC c_rgSpecificTypes[] =
{
	{ L"Specific File Type (*.?)", L"*.xml" },
	{ L"All File Types (*.*)", L"*.*" }
};

COMDLG_FILTERSPEC c_rgAllFileTypes[] =
{
	{ L"All File Types (*.*)", L"*.*" }
};

// File Dialog Event Handler
// Taken from the official Microsoft Shell Application Samples
// http://archive.msdn.microsoft.com/shellapplication

class CDialogEventHandler : public IFileDialogEvents,
	public IFileDialogControlEvents
{
public:
	// IUnknown methods
	IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv)
	{
		static const QITAB qit[] = {
			QITABENT(CDialogEventHandler, IFileDialogEvents),
			QITABENT(CDialogEventHandler, IFileDialogControlEvents),
			{ 0 },
		};
		return QISearch(this, qit, riid, ppv);
	}

	IFACEMETHODIMP_(ULONG) AddRef()
	{
		return InterlockedIncrement(&_cRef);
	}

	IFACEMETHODIMP_(ULONG) Release()
	{
		long cRef = InterlockedDecrement(&_cRef);
		if(!cRef)
			delete this;
		return cRef;
	}

	// IFileDialogEvents methods
	IFACEMETHODIMP OnFileOk(IFileDialog *) { return S_OK; };
	IFACEMETHODIMP OnFolderChange(IFileDialog *) { return S_OK; };
	IFACEMETHODIMP OnFolderChanging(IFileDialog *, IShellItem *) { return S_OK; };
	IFACEMETHODIMP OnHelp(IFileDialog *) { return S_OK; };
	IFACEMETHODIMP OnSelectionChange(IFileDialog *) { return S_OK; };
	IFACEMETHODIMP OnShareViolation(IFileDialog *, IShellItem *, FDE_SHAREVIOLATION_RESPONSE *) { return S_OK; };
	IFACEMETHODIMP OnTypeChange(IFileDialog *pfd) { return S_OK; };
	IFACEMETHODIMP OnOverwrite(IFileDialog *, IShellItem *, FDE_OVERWRITE_RESPONSE *) { return S_OK; };

	// IFileDialogControlEvents methods
	IFACEMETHODIMP OnItemSelected(IFileDialogCustomize *pfdc, DWORD dwIDCtl, DWORD dwIDItem) { return S_OK; };
	IFACEMETHODIMP OnButtonClicked(IFileDialogCustomize *, DWORD) { return S_OK; };
	IFACEMETHODIMP OnCheckButtonToggled(IFileDialogCustomize *, DWORD, BOOL) { return S_OK; };
	IFACEMETHODIMP OnControlActivating(IFileDialogCustomize *, DWORD) { return S_OK; };

	CDialogEventHandler() : _cRef(1) { };
private:
	~CDialogEventHandler() { };
	long _cRef;
};

// Instance creation helper
HRESULT CDialogEventHandler_CreateInstance(REFIID riid, void **ppv)
{
	*ppv = NULL;
	CDialogEventHandler *pDialogEventHandler = new (std::nothrow) CDialogEventHandler();
	HRESULT hr = pDialogEventHandler ? S_OK : E_OUTOFMEMORY;
	if(SUCCEEDED(hr))
	{
		hr = pDialogEventHandler->QueryInterface(riid, ppv);
		pDialogEventHandler->Release();
	}
	return hr;
}

void Win32::FileApi::PickFile(Files::Directory * directory, const wchar_t * extension, Files::Response * response)
{
	if(!response) return;

	wchar_t * pszFilePath = NULL;

	// CoCreate the File Open Dialog object.
	IFileDialog *pfd = NULL;
	HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
	if(SUCCEEDED(hr))
	{
		// Create an event handling object, and hook it up to the dialog.
		IFileDialogEvents *pfde = NULL;
		hr = CDialogEventHandler_CreateInstance(IID_PPV_ARGS(&pfde));
		if(SUCCEEDED(hr))
		{
			// Hook up the event handler.
			DWORD dwCookie;
			hr = pfd->Advise(pfde, &dwCookie);
			if(SUCCEEDED(hr))
			{
				// Set the options on the dialog.
				DWORD dwFlags;

				// Before setting, always get the options first in order 
				// not to override existing options.
				hr = pfd->GetOptions(&dwFlags);
				if(SUCCEEDED(hr))
				{
					// In this case, get shell items only for file system items.
					hr = pfd->SetOptions(dwFlags | FOS_FORCEFILESYSTEM);
					if(SUCCEEDED(hr))
					{
						const COMDLG_FILTERSPEC * filterSpecs = c_rgAllFileTypes;
						unsigned numFilterSpecs = 1;

						if(extension)
						{
							c_rgSpecificTypes[0].pszSpec = extension;
							filterSpecs = c_rgSpecificTypes;
							numFilterSpecs = 2;
						}

						// Set the file types to display only. 
						// Notice that this is a 1-based array.
						hr = pfd->SetFileTypes(numFilterSpecs, filterSpecs);
						if(SUCCEEDED(hr))
						{
							// Set the selected file type index to the first
							hr = pfd->SetFileTypeIndex(1);

							//if(extension)
							//{
							//	// Set the default extension to be ".doc" file.
							//	hr = pfd->SetDefaultExtension(extension);
							//}

							// Show the dialog
							hr = pfd->Show(window->GetHandle());
							if(SUCCEEDED(hr))
							{
								// Obtain the result once the user clicks 
								// the 'Open' button.
								// The result is an IShellItem object.
								IShellItem *psiResult;
								hr = pfd->GetResult(&psiResult);
								if(SUCCEEDED(hr))
								{
									// We are just going to store the name of the file
									hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

									psiResult->Release();
								}
							}
						}
					}
				}
				// Unhook the event handler.
				pfd->Unadvise(dwCookie);
			}
			pfde->Release();
		}
		pfd->Release();
	}

	if(pszFilePath)
	{
		std::wstring wideFilePath(pszFilePath);

		std::wstring currentDirString;
		wchar_t currentDirChars[256];
		if(SUCCEEDED(GetCurrentDirectory(256, currentDirChars)))
		{
			currentDirString = currentDirChars;

			if(wideFilePath.substr(0, currentDirString.length()).compare(currentDirString) == 0)
			{
				wideFilePath = wideFilePath.substr(currentDirString.length() + 1);
			}
		}

		std::string shortFilePath(wideFilePath.begin(), wideFilePath.end());

		response->bufferLength = shortFilePath.length();
		response->buffer = new char[response->bufferLength + 1];
		shortFilePath.copy(response->buffer, response->bufferLength, 0);
		response->buffer[response->bufferLength] = '\0';

		CoTaskMemFree(pszFilePath);
	}

	HandleResponse(response);
}

void Win32::FileApi::Enumerate(Files::Directory * directory, bool subDirectories)
{
	Win32::Directory * win32dir = static_cast<Win32::Directory*>(directory);

	WIN32_FIND_DATA ffd;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	DWORD dwError = 0;

	// Prepare string for use with FindFile functions.  First, copy the
	// string to a buffer, then append '\*' to the directory name.

	std::wstring path = win32dir->directoryPath;
	path += L"*";

	// Find the first file in the directory.

	hFind = FindFirstFile(path.data(), &ffd);

	if(INVALID_HANDLE_VALUE == hFind) return;

	// List all the files in the directory with some info about them.

	std::vector < std::wstring > wideNameVector;

	do
	{
		wideNameVector.emplace_back(ffd.cFileName);
	}
	while(FindNextFile(hFind, &ffd) != 0);

	dwError = GetLastError();
	if(dwError != ERROR_NO_MORE_FILES)
	{
		//DisplayErrorBox(TEXT("FindFirstFile"));
	}

	FindClose(hFind);

	if(win32dir->fileNames != 0)
	{
		delete[] win32dir->fileNames;
		win32dir->fileNames = 0;
		win32dir->numFiles = -1;
	}

	win32dir->numFiles = (int)wideNameVector.size();
	win32dir->fileNames = new std::wstring[win32dir->numFiles];

	for(int i = 0; i < win32dir->numFiles; ++i)
	{
		win32dir->fileNames[i] = wideNameVector[i];
	}
}

Files::File * Win32::FileApi::Open(Files::Directory * directory, LPCWSTR path)
{
	if(!directory) return false;

	Win32::Directory * win32dir = static_cast<Win32::Directory*>(directory);

	Win32::File * file = new Win32::File(directory);

	//file->fileHandle = (HANDLE) Openfilew(filePath.c_str(), 0, 0);

	wchar_t exeDir[128];
	GetCurrentDirectory(128, exeDir);

	SetCurrentDirectory(win32dir->GetPath().c_str());

	file->fileHandle = CreateFile(path,
		GENERIC_WRITE | GENERIC_READ,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
		NULL);

	SetCurrentDirectory(exeDir);

	if(file->fileHandle == INVALID_HANDLE_VALUE)
	{
		DWORD lastError = GetLastError();
		if(lastError == 0x20) // File already open
		{
			pendingFiles.emplace_back(file, path);
			file->openState = Files::InProgress;
		}
		else
		{
			std::wstringstream errStream;
			errStream << L"Could not open file " << win32dir->GetPath() << path << L"\n";
			OutputDebugString(errStream.str().c_str());
			file->openState = Files::Failed;
		}
		return file;
	}

	FILE_STREAM_INFO streamInfo[2];
	GetFileInformationByHandleEx(file->fileHandle, FileStreamInfo, &streamInfo, 2 * sizeof(FILE_STREAM_INFO));
	LARGE_INTEGER fileSize = streamInfo[0].StreamSize;

	if(fileSize.HighPart > 0)
	{
		std::wstringstream errStream;
		errStream << L"File " << path << L" is too large!\n";
		OutputDebugString(errStream.str().c_str());
		file->openState = Files::Failed;
	}
	else
	{
		file->byteLength = fileSize.LowPart;
		file->openState = Files::Succeeded;
	}
	return file;
}

void Win32::FileApi::OpenAndRead(Files::Directory * directory, const wchar_t * file, Files::Response * response)
{
	Files::File * openedFile = Open(directory, file);
	if(openedFile && openedFile->openState == Files::Succeeded)
	{
		Read(openedFile, response);
	}
	else if(openedFile && openedFile->openState == Files::InProgress)
	{
		for(unsigned i = 0; i < pendingFiles.size(); ++i)
		{
			if(pendingFiles[i].filePtr == openedFile)
			{
				pendingFiles[i].response = response;
				break;
			}
		}
	}
	else
	{
		HandleResponse(response);
		delete openedFile;
	}
}

void Win32::FileApi::Read(Files::File * file, Files::Response * response)
{
	if(!file || !response) return;

	Win32::File * winFile = static_cast<Win32::File*>(file);
	response->file = file;

	if(winFile->byteLength < 1)
	{
		HandleResponse(response);
		return;
	}

	if(activeEventCount >= MAXIMUM_WAIT_OBJECTS)
	{
		OutputDebugString(L"Reached maximum file events, ignoring read.\n");
		HandleResponse(response);
		return;
	}

	OVERLAPPED * readOverlap = new OVERLAPPED();
	readOverlap->Offset = 0;
	readOverlap->OffsetHigh = 0;
	readOverlap->hEvent = CreateEventEx(NULL, 0, NULL, EVENT_ALL_ACCESS);

	BYTE * readBuffer = new BYTE[winFile->byteLength];
	ZeroMemory(readBuffer, (winFile->byteLength) * sizeof(BYTE));

	BOOL readDone = ReadFile(winFile->fileHandle, readBuffer, winFile->byteLength, NULL, readOverlap);

	if(GetLastError() == ERROR_IO_PENDING)
	{
		activeEvents[activeEventCount].overlap = readOverlap;
		activeEvents[activeEventCount].buffer = (char*)readBuffer;
		activeEvents[activeEventCount].bufferLength = winFile->byteLength;
		activeEvents[activeEventCount].response = response;
		activeEvents[activeEventCount].readEvent = false;
		activeEventHandles[activeEventCount] = readOverlap->hEvent;

		activeEventCount++;
	}
	else // Respond instantly and clean up..
	{
		if(SUCCEEDED(readOverlap->Internal))
		{
			response->buffer = (char*)readBuffer;
			response->bufferLength = readOverlap->InternalHigh;
		}

		HandleResponse(response);

		delete readOverlap;
	}
}

void Win32::FileApi::Close(Files::File ** file)
{
	if(!file || !(*file)) return;
	Win32::File * winFile = static_cast<Win32::File*>(*file);
	if(winFile->fileHandle) CloseHandle(winFile->fileHandle);
	delete (*file);
	(*file) = 0;
}

const char * Win32::FileApi::ReadChars(Files::File* file, unsigned & length, int offset)
{
	if(length < 1) return 0;
	DWORD error = 0;

	Win32::File * winFile = static_cast<Win32::File*>(file);

	OVERLAPPED readOverlap;
	readOverlap.Offset = offset;
	readOverlap.OffsetHigh = 0;
	readOverlap.hEvent = CreateEventEx(NULL, 0, NULL, EVENT_ALL_ACCESS);

	const char * charBuffer = new char[length + 1];
	ZeroMemory((void*)charBuffer, (length + 1) * sizeof(BYTE));

	BOOL readDone = ReadFile(winFile->fileHandle, (void*)charBuffer, length, 0, &readOverlap);

	if(!readDone)
	{
		DWORD dwReturn = WaitForSingleObjectEx(readOverlap.hEvent, 16, TRUE);
		if(dwReturn != WAIT_OBJECT_0)
		{
			error = GetLastError();
			delete[] charBuffer;
			return 0;
		}
	}

	if(FAILED(readOverlap.Internal))
	{
		error = GetLastError();
		delete[] charBuffer;
		return 0;
	}

	length = readOverlap.InternalHigh;

	return charBuffer;
}


void Win32::FileApi::Poll()
{
	bool done = false;
	while(activeEventCount > 0 && !done)
	{
		//OutputDebugString(L"Polling\n");

		DWORD dwReturn = WaitForMultipleObjectsEx(activeEventCount, activeEventHandles, FALSE, 0, FALSE);

		if(dwReturn < WAIT_ABANDONED_0)
		{
			//OutputDebugString(L"Event Signalled!\n");

			unsigned index = dwReturn - WAIT_OBJECT_0;

			FileEvent * event = static_cast<FileEvent*>(&activeEvents[index]);

			if(SUCCEEDED(event->overlap->Internal))
			{
				unsigned bufferLength = event->overlap->InternalHigh;
				char * buffer = new char[event->overlap->InternalHigh + 1];
				memcpy(buffer, event->buffer, bufferLength);
				buffer[bufferLength] = '\0';

				event->response->buffer = buffer;
				event->response->bufferLength = bufferLength;

				HandleResponse(event->response);

				delete[] event->buffer;
				RemoveEvent(index);
			}
			else
			{
				OutputDebugString(L"File operation failed\n");
			}
		}
		else if(dwReturn < WAIT_IO_COMPLETION)
		{
			//OutputDebugString(L"Wait abandoned!\n");
		}
		else done = true;

		if(dwReturn == WAIT_FAILED)
		{
			std::stringstream ss;
			char errorcode[32];
			_itoa_s(GetLastError(), errorcode, 10);
			ss << "Wait Failed " << errorcode << std::endl;

			//WinAsyncFileEvent * event = static_cast<WinAsyncFileEvent*>(&activeEvents[index]);

			// need to clean up the event and overlap :/

			//OutputDebugStringA(ss.str().c_str());
		}
	}

	for(unsigned i = 0; i < pendingFiles.size(); ++i)
	{
		HANDLE fileHandle = CreateFile(pendingFiles[i].fullPath.c_str(),
			GENERIC_WRITE | GENERIC_READ,
			0,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
			NULL);

		if(fileHandle == INVALID_HANDLE_VALUE)
		{
			if(GetLastError() != 0x20)
			{
				std::wstringstream errStream;
				errStream << L"Could not open file " << pendingFiles[i].fullPath.c_str() << L"\n";
				OutputDebugString(errStream.str().c_str());
				pendingFiles[i].filePtr->openState = Files::Failed;
				pendingFiles.erase(pendingFiles.begin() + i);
				i--;
			}
		}
		else
		{
			Win32::File * file = pendingFiles[i].filePtr;
			file->fileHandle = fileHandle;

			FILE_STREAM_INFO streamInfo[2];
			GetFileInformationByHandleEx(file->fileHandle, FileStreamInfo, &streamInfo, 2 * sizeof(FILE_STREAM_INFO));
			LARGE_INTEGER fileSize = streamInfo[0].StreamSize;

			if(fileSize.HighPart > 0)
			{
				std::wstringstream errStream;
				errStream << L"File " << pendingFiles[i].fullPath.c_str() << L" is too large!\n";
				OutputDebugString(errStream.str().c_str());
				file->openState = Files::Failed;
			}
			else
			{
				file->byteLength = fileSize.LowPart;
				file->openState = Files::Succeeded;

				if(pendingFiles[i].response)
				{
					Read(pendingFiles[i].filePtr, pendingFiles[i].response);
				}
			}

			pendingFiles.erase(pendingFiles.begin() + i);
			i--;
		}
	}
}


void Win32::FileApi::RemoveEvent(unsigned index)
{
	delete activeEvents[index].overlap;

	for(unsigned i = index; i < activeEventCount - 1; i++)
	{
		activeEvents[i] = activeEvents[i + 1];
		activeEventHandles[i] = activeEventHandles[i + 1];
	}

	activeEventCount--;
}

} // namespace Ingenuity
