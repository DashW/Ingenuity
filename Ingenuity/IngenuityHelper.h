#pragma once

#include <FilesApi.h>

class IngenuityHelper
{
	static Ingenuity::Files::Directory * dataDirectory;
	static bool completed;
	static void PickDataDirectory();

public:
	static Ingenuity::Files::Directory * GetDataDirectory();
	static void RequestDataDirectory(Ingenuity::Files::Api * files);
	static bool IsRequestCompleted();
	static void DeleteDataDirectory();
};