#pragma once

#include "AssetMgr.h"
#include "GpuApi.h"
#include "ObjParser.h"
#include "MtlParser.h"
#include <string>

namespace Ingenuity {

struct LocalMesh;
struct TangentGenerator;
class StepMgr;

class WavefrontLoader : public SteppableLoader
{
	enum State
	{
		Idle,
		Initializing,
		OpeningMesh,
		LoadingMesh,
		OpeningMtl,
		LoadingMtl,
		CreatingMeshes,
		GeneratingTangents,
		//CopyingMeshes,
		Finalizing
	} state;

	//std::vector<GpuMeshCopy*> copyTasks;
	std::vector<TangentGenerator*> tangentTasks;

	std::wstring subDirectory;
	std::wstring path;

	class Response : public Files::Response
	{
		virtual void Respond() override
		{
			closeOnComplete = true; deleteOnComplete = false;
		}
	};

	Response response;
	std::string fileString;

	Gpu::ComplexModel * model;
	Gpu::Mesh * debugMesh;

	Gpu::Api * gpu;
	AssetMgr * assets;
	StepMgr * steppables;

	ObjParser objParser;
	MtlParser mtlParser;

	bool noTangents;
	bool genDebugMesh;

	bool finished;
	bool debugMeshAcquired;
	float progress;
	unsigned fileOffset;

	void GenerateDebugMesh(LocalMesh * input);

	inline std::string GetNextLine()
	{
		size_t newLinePos = fileString.find("\n", fileOffset);
		std::string result;

		if(newLinePos == std::string::npos)
		{
			if(fileOffset < fileString.length())
			{
				result = fileString.substr(fileOffset);
			}
			fileString.clear();
		}
		else
		{
			result = fileString.substr(fileOffset, newLinePos + 1 - fileOffset);
			fileOffset = newLinePos + 1;
		}

		return result;
	}

public:
	WavefrontLoader(
		StepMgr * steppables,
		AssetMgr * assets,
		Gpu::Api * gpu,
		Files::Directory * directory,
		const wchar_t * path,
		bool noNormals = false,
		bool noTangents = false,
		bool consolidate = false);
	virtual ~WavefrontLoader();

	virtual Gpu::ComplexModel * LoadModel();

	virtual void Load() override {};
	virtual void Step() override;
	virtual float GetProgress() override;
	virtual bool IsFinished() override;

	// Do not generate vertex normals if the model does not already have them
	virtual void SetNoNormals(bool noNormals) { objParser.SetNoNormals(noNormals); }

	// Do not generate vertex tangents
	virtual void SetNoTangents(bool noTangents) { this->noTangents = noTangents; }

	// Generate a vertex-tangent debug mesh
	virtual void SetGenerateDebugMesh(bool genDebugMesh) { this->genDebugMesh = genDebugMesh; }

	// Avoid duplicating vertices (expensive)
	virtual void SetConsolidate(bool consolidate) { objParser.SetConsolidate(consolidate); }

	// Get the finished model
	virtual Gpu::ComplexModel * GetModel() { return model; }

	// Get the number of meshes in the model
	virtual unsigned GetNumMeshes() { return objParser.GetNumMeshes(); }

	// Get an individual mesh as a LocalMesh
	virtual LocalMesh * GetMesh(unsigned index) { return objParser.GetMesh(index); }

	// Get the finished model as an IAsset
	virtual IAsset * GetAsset() override { return static_cast<IAsset*>(model); }

	// Get the generated vertex-tangent debug mesh (only if SetGenDebugMesh was true)
	Gpu::Mesh * GetDebugMesh();
};

} // namespace Ingenuity
