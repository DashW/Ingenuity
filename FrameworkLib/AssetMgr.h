#pragma once

#include "FilesApi.h"
#include "StepMgr.h"

#include <string>
#include <vector>
#include <map>

namespace Ingenuity {

namespace Audio {
class Api;
}
namespace Gpu {
class Api;
}
namespace Image {
class Api;
}

enum AssetType
{
	TextureAsset,
	CubeMapAsset,
	VolumeTexAsset,
	ShaderAsset,
	ModelAsset,
	WavefrontModelAsset,
	RawHeightMapAsset,
	SvgAsset,
	ImageAsset,
	AudioAsset,
};

struct IAsset
{
	virtual AssetType GetType() = 0;
	virtual IAsset * GetAsset() = 0;
	virtual ~IAsset() {}
};

struct AssetRequest
{
	Files::Directory * directory;
	std::wstring path;
	AssetType type;
	std::string name;

	AssetRequest(Files::Directory * directory, const wchar_t * path, AssetType type, const char * name = 0) :
		directory(directory), path(path ? path : L""), type(type), name(name ? name : "") {}
};

typedef std::vector<AssetRequest> AssetBatch;

struct AssetLoader
{
	Files::Directory * directory;
	std::wstring path;
	AssetType type;
	std::string name;
	bool added;

	AssetLoader(Files::Directory * d, const wchar_t * p, AssetType t) :
		directory(d), path(p ? p : L""), type(t), added(false) {}
	virtual ~AssetLoader() {}

	virtual void Load() = 0;
	virtual float GetAssetProgress() = 0;
	virtual bool IsAssetReady() = 0;
	virtual IAsset * GetAsset() = 0;
};

struct SimpleLoader : public AssetLoader, Files::Response
{
	Files::Api * files;
	IAsset * asset;

	SimpleLoader(Files::Api * files, Files::Directory * dir, const wchar_t * path, AssetType type)
		: AssetLoader(dir, path, type), files(files), asset(0) {}
	virtual ~SimpleLoader() {}

	virtual void Load() override
	{
		files->OpenAndRead(directory, path.c_str(), this);
	}
	virtual float GetAssetProgress() override { return progress; }
	virtual bool IsAssetReady() override { return complete; }
	virtual IAsset * GetAsset() override { return asset; }
};

struct SteppableLoader : public AssetLoader, Steppable
{
	SteppableLoader(StepMgr * steppables, Files::Directory * dir, const wchar_t * path, AssetType type)
		: AssetLoader(dir, path, type)
	{
		steppables->Add(this);
	}

	virtual float GetAssetProgress() override { return GetProgress(); }
	virtual bool IsAssetReady() override { return IsFinished(); }
};

class AssetMgr
{
	Audio::Api * audio;
	Files::Api * files;
	Gpu::Api   * gpu;
	Image::Api * imaging;

	StepMgr    * steppables;

	typedef std::vector<IAsset*> AssetBank;
	typedef std::multimap<std::wstring, unsigned> PathAssets;
	typedef std::multimap<std::string, unsigned> NamedAssets;
	typedef std::vector<AssetLoader*> Request;
	typedef std::vector<Request> RequestBank;

	AssetBank   assetBank;
	PathAssets  pathAssets;
	NamedAssets namedAssets;
	RequestBank requestBank;

	std::wstring FullPath(Files::Directory * directory, const wchar_t * subPath);

public:
	AssetMgr(Files::Api * files, Gpu::Api * gpu, Image::Api * imaging, StepMgr * steppables, Audio::Api * audio);
	~AssetMgr();

	static const int CRITICAL_TICKET = 0u;

	template <typename T>
	T * GetAsset(Files::Directory * directory, const wchar_t * path)
	{
		PathAssets::iterator mapping = pathAssets.find(FullPath(directory, path));
		if(mapping == pathAssets.end()) return 0;

		IAsset * asset = assetBank[mapping->second];
		if(!asset) return 0;

		return dynamic_cast<T*>(asset->GetAsset());
	}

	template <typename T>
	T * GetAsset(const char * name)
	{
		NamedAssets::iterator mapping = namedAssets.find(std::string(name));
		if(mapping == namedAssets.end()) return 0;

		IAsset * asset = assetBank[mapping->second];
		if(!asset) return 0;

		return dynamic_cast<T*>(asset->GetAsset());
	}

	void AddAsset(Files::Directory * directory, const wchar_t * path, IAsset * asset, const char * name = 0);

	Files::Api * GetFileApi() { return files; }

	int Load(AssetBatch & batch);
	int Load(AssetRequest & request, int ticket = -1);
	int Load(Files::Directory * directory, const wchar_t * path, AssetType type, const char * name = 0, int ticket = -1);

	float GetProgress(int ticket);
	bool IsLoaded(int ticket);

	void Unload(int ticket);
	void Unload(Files::Directory * directory, const wchar_t * path);
	void UnloadAll();

	void Update();
};

} // namespace Ingenuity
