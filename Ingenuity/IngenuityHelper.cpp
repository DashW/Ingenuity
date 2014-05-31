#include "IngenuityHelper.h"

#include <ppltasks.h>
#if defined(WINAPI_FAMILY) && WINAPI_FAMILY == WINAPI_FAMILY_PC_APP
#include <WinRTFileApi.h>

using namespace concurrency;
using namespace Windows::Foundation;
using namespace Windows::Storage;
using namespace Windows::Storage::AccessCache;
using namespace Windows::Storage::Pickers;
#else
#include <Win32FileApi.h>
#endif

Ingenuity::Files::Directory * IngenuityHelper::dataDirectory = 0;
bool IngenuityHelper::completed = true;

void IngenuityHelper::PickDataDirectory()
{
#if defined(WINAPI_FAMILY) && WINAPI_FAMILY == WINAPI_FAMILY_PC_APP
	completed = false;

	FolderPicker ^ picker = ref new FolderPicker;
	picker->SuggestedStartLocation = Pickers::PickerLocationId::DocumentsLibrary;

	picker->FileTypeFilter->Append(".lua");
	picker->FileTypeFilter->Append(".xml");

	create_task(picker->PickSingleFolderAsync()).then([](StorageFolder ^ folder)
	{
		if(folder)
		{
			Platform::String ^ listToken = StorageApplicationPermissions::FutureAccessList->Add(folder);

			ApplicationDataContainer ^ localSettings = ApplicationData::Current->LocalSettings;

			localSettings->Values->Insert("IngenuityDataDirectory", listToken);

			Ingenuity::WinRT::Directory * tmp = new Ingenuity::WinRT::Directory();
			tmp->storageFolder = folder;
			IngenuityHelper::dataDirectory = tmp;
		}
		completed = true;
	});
#endif
}

Ingenuity::Files::Directory * IngenuityHelper::GetDataDirectory()
{
	return dataDirectory;
}

void IngenuityHelper::RequestDataDirectory(Ingenuity::Files::Api * files)
{
#if defined(WINAPI_FAMILY) && WINAPI_FAMILY == WINAPI_FAMILY_PC_APP
	completed = false;

	ApplicationDataContainer ^ localSettings = ApplicationData::Current->LocalSettings;

	Platform::String ^ token = safe_cast<Platform::String^>(localSettings->Values->Lookup("IngenuityDataDirectory"));

	if(token)
	{
		create_task(StorageApplicationPermissions::FutureAccessList->GetFolderAsync(token)).then([](StorageFolder ^ folder)
		{
			if(folder)
			{
				Ingenuity::WinRT::Directory * tmp = new Ingenuity::WinRT::Directory();
				tmp->storageFolder = folder;
				dataDirectory = tmp;

				completed = true;
			}
			else
			{
				PickDataDirectory();
			}
		});
	}
	else
	{
		PickDataDirectory();
	}
#else
	dataDirectory = new Ingenuity::Win32::Directory();
#endif
}

bool IngenuityHelper::IsRequestCompleted()
{
#if defined(WINAPI_FAMILY) && WINAPI_FAMILY == WINAPI_FAMILY_PC_APP
	return completed;
#else
	return true;
#endif
}

void IngenuityHelper::DeleteDataDirectory()
{
#if !defined(WINAPI_FAMILY) || WINAPI_FAMILY != WINAPI_FAMILY_PC_APP
	if(dataDirectory) { delete dataDirectory; dataDirectory = 0; }
#endif
}