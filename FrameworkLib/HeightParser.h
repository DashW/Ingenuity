// Class to parse a RAW height map into a normal'd mesh surface
#pragma once

#include "AssetMgr.h"
#include "GpuApi.h"
#include <vector>

namespace Ingenuity {

struct LocalMesh;

class HeightParser : public IAsset
{
	static float epsilon;

	std::vector<float> heights;
	float width, depth, scale;
	unsigned sideLength;

	bool  inBounds(int i, int j);
	float sampleHeight3x3(int i, int j);

public:
	HeightParser() : width(1.0f), depth(1.0f), scale(1.0f) {}

	bool ParseHeightmap(const char* heightmap, unsigned sideLength);

	void SetScale(float width, float height, float depth);
	LocalMesh * GetMesh(Gpu::Rect* texRect = 0);
	float GetHeight(float x, float z);
	//float GetHeight(float x, float z, GpuComplexModel * complexModel, unsigned modelIndex);

	virtual AssetType GetType() override { return RawHeightMapAsset; }
	virtual IAsset * GetAsset() override { return this; }
};

struct RawHeightLoader : public SimpleLoader
{
	Gpu::Api * gpu;
	//HeightParser parser;

	RawHeightLoader(Gpu::Api * gpu, Files::Api * files, Files::Directory * directory, const wchar_t * path) :
		SimpleLoader(files, directory, path, RawHeightMapAsset), gpu(gpu) {}

	virtual void Respond() override
	{
		if(buffer)
		{
			unsigned sideLength = (unsigned)sqrt(bufferLength);

			HeightParser * heightmap = new HeightParser();
			if(heightmap->ParseHeightmap(buffer, sideLength))
			{
				//GpuMesh * mesh = parser.GetMesh(&GpuRect(0.0f, 0.0f, 1.0f, 1.0f))->GpuOnly(gpu);
				//GpuComplexModel * model = new GpuComplexModel(1);
				//model->models[0].mesh = mesh;
				//asset = model;
				asset = heightmap;
			}
			else
			{
				delete heightmap;
			}
		}
	}
};

} // namespace Ingenuity
