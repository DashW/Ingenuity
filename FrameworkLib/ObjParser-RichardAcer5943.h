#pragma once

#include "MeshParser.h"
#include "MtlParser.h"
#include <string>
#include <vector>

class ObjParser : public MeshParser
{
public:
	ObjParser() {}
	~ObjParser() {}

	virtual void ParseMesh(GpuApi *gpu, const char *text) override;
	virtual void ParseIndexedMesh(GpuApi *gpu, const char *text) override;

	bool hasMaterialLib() { return materialLib.size() != 0; }
	const char* GetMaterialLib() { return materialLib.c_str(); }

	void ApplyMaterials(MtlParser* mtl);

private:
	std::string materialLib;
	std::vector<std::string> materials;
};