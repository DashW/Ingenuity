#include "AssetMgr.h"

#include "AudioApi.h"
#include "GpuApi.h"
#include "FilesApi.h"
#include "HeightParser.h"
#include "ImageApi.h"
#include "SvgParser.h"
#include "WavefrontLoader.h"
#include "ShaderParser.h"
#include "SoundFileReader.h"

namespace Ingenuity {

std::wstring AssetMgr::FullPath(Files::Directory * directory, const wchar_t * subPath)
{
	return directory ? directory->GetPath() + subPath : subPath;
}

AssetMgr::AssetMgr(Files::Api * files, Gpu::Api * gpu, Image::Api * imaging, StepMgr * steppables, Audio::Api * audio) :
files(files), gpu(gpu), imaging(imaging), steppables(steppables), audio(audio)
{
	// CRITICAL_TICKET
	requestBank.emplace_back();
}

AssetMgr::~AssetMgr()
{
	// Loaders
	{
		RequestBank::iterator request;
		for(request = requestBank.begin(); request != requestBank.end(); ++request)
		{
			Request::iterator loader;
			for(loader = request->begin(); loader != request->end(); ++loader)
			{
				//if(loader->steppable) delete loader->steppable;
				delete (*loader);
			}
			request->clear();
		}
		requestBank.clear();
	}

	// Assets
	{
		AssetBank::iterator assetIt;
		for(assetIt = assetBank.begin(); assetIt != assetBank.end(); ++assetIt)
		{
			if(*assetIt) delete (*assetIt);
		}
		assetBank.clear();
	}
}

AssetLoader * AssetMgr::GetLoader(const char * name)
{
	for(unsigned i = 0; i < requestBank.size(); ++i)
	{
		Request & request = requestBank[i];
		for(unsigned j = 0; j < request.size(); ++j)
		{
			AssetLoader * loader = request[j];
			if(loader->name.compare(name) == 0)
			{
				return loader;
			}
		}
	}
	return 0;
}

void AssetMgr::AddAsset(Files::Directory * directory, const wchar_t * path, IAsset * asset, const char * name)
{
	unsigned assetIndex = assetBank.size();
	assetBank.emplace_back(asset->GetAsset());
	pathAssets.emplace(FullPath(directory, path), assetIndex);
	if(name)
	{
		namedAssets.emplace(name, assetIndex);
	}
}

int AssetMgr::Load(AssetBatch & batch)
{
	requestBank.emplace_back();

	for(unsigned i = 0; i < batch.size(); ++i)
	{
		Load(batch[i], requestBank.size() - 1);
	}

	return requestBank.size() - 1;
}

int AssetMgr::Load(AssetRequest & request, int ticket)
{
	AssetType type = request.type;
	Files::Directory * directory = request.directory;
	const wchar_t * path = request.path.c_str();
	const char * name = request.name.c_str();

	return Load(directory, path, type, name, ticket);
}

int AssetMgr::Load(Files::Directory * directory, const wchar_t * path, AssetType type, const char * name, int ticket)
{
	PathAssets::iterator pathMapping = pathAssets.find(FullPath(directory, path));
	if(pathMapping != pathAssets.end())
	{
		if(name)
		{
			NamedAssets::iterator mapping = namedAssets.find(std::string(name));
			if(mapping == namedAssets.end())
			{
				namedAssets.emplace(name, pathMapping->second);
			}
		}

		if(ticket > -1 && ticket < (int)requestBank.size())
		{
			return ticket;
		}
		else
		{
			requestBank.emplace_back();
			return requestBank.size() - 1;
		}
	}

	AssetLoader * loader = 0;

	if(type == TextureAsset || type == CubeMapAsset || type == VolumeTexAsset)
	{
		loader = new Gpu::TextureLoader(gpu, files, directory, path, type);
	}
	if(type == ShaderAsset)
	{
		loader = gpu->CreateGpuShaderLoader(directory, path);
	}
	if(type == WavefrontModelAsset)
	{
		loader = new WavefrontLoader(steppables, gpu, this, directory, path);
	}
	if(type == RawHeightMapAsset)
	{
		loader = new RawHeightLoader(gpu, files, directory, path);
	}
	if(type == SvgAsset)
	{
		loader = new SvgLoader(this, gpu, directory, path);
	}
	if(type == ImageAsset)
	{
		loader = new Image::Loader(imaging, files, directory, path);
	}
	if(type == AudioAsset)
	{
		loader = new SoundFileReader(audio, files, directory, path);
	}

	if(loader)
	{
		if(name) loader->name = name;

		loader->Load();

		if(ticket > -1 && ticket < (int)requestBank.size())
		{
			requestBank[ticket].emplace_back(loader);
			return ticket;
		}
		else
		{
			requestBank.emplace_back();
			requestBank.back().emplace_back(loader);
			return requestBank.size() - 1;
		}
	}

	return -1;
}

float AssetMgr::GetProgress(int ticket)
{
	if(ticket < 0 || ticket >= (int)requestBank.size()) return -1.0f; // unloaded

	Request & request = requestBank[ticket];

	if(request.size() == 0) return 1.0f;

	float sum = 0.0f;

	for(unsigned i = 0; i < request.size(); ++i)
	{
		sum += request[i]->GetAssetProgress();
	}

	return sum / (float)request.size();
}

bool AssetMgr::IsLoaded(int ticket)
{
	if(ticket < 0 || ticket >= (int)requestBank.size()) return false;

	Request & request = requestBank[ticket];

	if(request.size() == 0) return true;

	for(unsigned i = 0; i < request.size(); ++i)
	{
		if(!request[i]->added) return false;
	}
	return true;
}

void AssetMgr::Update()
{
	RequestBank::iterator request;
	for(request = requestBank.begin(); request != requestBank.end(); ++request)
	{
		Request::iterator loaderIt;
		for(loaderIt = request->begin(); loaderIt != request->end(); ++loaderIt)
		{
			AssetLoader * loader = *loaderIt;
			if(loader->IsAssetReady() && !loader->added)
			{
				IAsset * asset = loader->GetAsset();
				if(asset != 0)
				{
					unsigned assetIndex = assetBank.size();
					assetBank.emplace_back(asset);
					pathAssets.emplace(FullPath(loader->directory, loader->path.c_str()), assetIndex);
					if(loader->name.size() > 0) namedAssets.emplace(loader->name, assetIndex);
				}
				loader->added = true;
			}
		}
	}
}

void AssetMgr::Unload(Files::Directory * directory, const wchar_t * path)
{
	PathAssets::iterator pathToAsset = pathAssets.find(FullPath(directory, path));

	if(pathToAsset != pathAssets.end())
	{
		IAsset * asset = assetBank[pathToAsset->second];

		if(asset)
		{
			delete asset;
		}
	}
}

void AssetMgr::Unload(int ticket)
{
	if(ticket < (int)requestBank.size() && ticket != CRITICAL_TICKET)
	{
		Request::iterator loaderIt;
		for(loaderIt = requestBank[ticket].begin(); loaderIt != requestBank[ticket].end(); ++loaderIt)
		{
			AssetLoader * loader = *loaderIt;

			Unload(loader->directory, loader->path.c_str());
		}
	}
}

} // namespace Ingenuity
