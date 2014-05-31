#pragma once

// Are we going to need a complete resource-loading subsystem??? That could be an entire application in itself!!! :(

#include <string>
#include <vector>

class GpuApi;
struct GpuIndexedMesh;
struct GpuTexture;

struct WavefrontMtl
{
	std::string name;
	float r, g, b, a;
	GpuTexture* tex;
};

class MtlParser
{
public:
	void ParseMtl(GpuApi *gpu, const char *text);

	void ApplyMtl(GpuIndexedMesh *mesh);

	int GetNumMtls() { return materials.size(); }
	WavefrontMtl& GetMtl(int index) { return materials[index]; };

private:
	std::vector<WavefrontMtl> materials;
};