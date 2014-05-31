#pragma once

// Are we going to need a complete resource-loading subsystem??? 
// That could be an entire application in itself!!! :(7

// But it looks like it's got to be done, especially if we're going to 
// be able to load and unload things in scripts!

// castle = Ingenuity.GetResource("castle.obj")
// Ingenuity.AddToScene(castle)

#include <string>
#include <vector>

namespace Ingenuity {

class AssetMgr;

namespace Files {
struct Directory;
}

namespace Gpu {
struct Texture;
}

struct WavefrontMtl
{
	std::string name;
	std::wstring texPath;
	std::wstring normalPath;

	Gpu::Texture * tex;
	Gpu::Texture * normal;

	WavefrontMtl() : tex(0), normal(0) {}
};

class MtlParser
{
	bool ready;
	int textureTicket;

	AssetMgr * assets;
	Files::Directory * directory;
	std::wstring subDirectory;
	std::vector<WavefrontMtl> materials;

public:
	MtlParser(AssetMgr *assets) :
		ready(false), textureTicket(-1), assets(assets) {}

	void SetDirectory(Files::Directory * directory, const wchar_t * subDirectory)
	{
		this->directory = directory;
		this->subDirectory = subDirectory;
	}

	void ParseMtl(std::string text);
	void ParseLine(std::string line);

	std::string GetArguments(std::string line, const char * command, int pos);

	bool IsReady() { return ready; }

	int GetNumMtls() { return materials.size(); }
	WavefrontMtl* GetMtl(const char* name);
};

} // namespace Ingenuity